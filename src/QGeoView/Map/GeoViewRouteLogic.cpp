#include "GeoViewRouteLogic.h"

#include <QJsonObject>
#include <QtMath>
#include <limits>
#include <algorithm>

#include <QGeoView/QGVProjection.h>

std::optional<QGV::GeoPos> GeoViewRouteLogic::segmentIntersection(const QGV::GeoPos& a, const QGV::GeoPos& b,
                                                                  const QGV::GeoPos& c, const QGV::GeoPos& d)
{
    double x1 = a.longitude(), y1 = a.latitude();
    double x2 = b.longitude(), y2 = b.latitude();
    double x3 = c.longitude(), y3 = c.latitude();
    double x4 = d.longitude(), y4 = d.latitude();

    double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (denom == 0.0)
        return std::nullopt;

    double px = ((x1*y2 - y1*x2)*(x3-x4) - (x1-x2)*(x3*y4 - y3*x4)) / denom;
    double py = ((x1*y2 - y1*x2)*(y3-y4) - (y1-y2)*(x3*y4 - y3*x4)) / denom;

    auto onSegment = [](double p, double q1, double q2) {
        return (p >= std::min(q1, q2) - 1e-9) && (p <= std::max(q1, q2) + 1e-9);
    };

    if (onSegment(px, x1, x2) && onSegment(px, x3, x4) &&
        onSegment(py, y1, y2) && onSegment(py, y3, y4))
        return QGV::GeoPos(py, px);

    return std::nullopt;
}

QVector<QGV::GeoPos> GeoViewRouteLogic::polygonSelfIntersections(const QVector<QGV::GeoPos>& points)
{
    QVector<QGV::GeoPos> intersections;
    size_t n = points.size();
    if (n < 4)
        return intersections;

    for (size_t i = 0; i < n - 1; ++i) {
        for (size_t j = i + 1; j < n - 1; ++j) {
            if (j == i + 1)
                continue;
            if (i == 0 && j == n - 2)
                continue;

            auto pt = segmentIntersection(points[i], points[i+1], points[j], points[j+1]);
            if (pt)
                intersections.push_back(*pt);
        }
    }

    return intersections;
}

QVector<QGV::GeoPos> GeoViewRouteLogic::buildRouteWithAngle(QGVMap* map,
                                                            const QVector<QGV::GeoPos>& contourPoints,
                                                            double stepMeters,
                                                            double angleDegrees,
                                                            double offsetFromContour,
                                                            double offsetCut)
{
    if (!map || contourPoints.size() < 2)
        return {};

    QVector<QPointF> projPoints;
    for (const auto& p : contourPoints)
        projPoints.push_back(map->getProjection()->geoToProj(p));

    const QVector<QPointF>& workContour = projPoints;

    double angleRad = angleDegrees * M_PI / 180.0;
    QPointF dir(std::cos(angleRad), std::sin(angleRad));
    QPointF normal(-dir.y(), dir.x());

    double minProj = std::numeric_limits<double>::max();
    double maxProj = -std::numeric_limits<double>::max();
    for (const auto& pt : workContour) {
        double proj = pt.x() * normal.x() + pt.y() * normal.y();
        minProj = std::min(minProj, proj);
        maxProj = std::max(maxProj, proj);
    }

    QVector<QVector<QGV::GeoPos>> finalLines;

    double startP = minProj + offsetFromContour;
    if (startP > maxProj)
        return {};

    for (double p = startP; p <= maxProj; p += stepMeters) {
        QVector<QPointF> intersections;
        for (int i = 0; i < workContour.size() - 1; ++i) {
            QPointF a = workContour[i];
            QPointF b = workContour[i + 1];
            double da = a.x()*normal.x() + a.y()*normal.y() - p;
            double db = b.x()*normal.x() + b.y()*normal.y() - p;

            if (da*db <= 0 && std::abs(da - db) > 1e-6) {
                double t = da / (da - db);
                QPointF pt = a + (b - a) * t;
                intersections.push_back(pt);
            }
        }

        if (intersections.size() < 2)
            continue;

        std::sort(intersections.begin(), intersections.end(),
                  [&](const QPointF& x, const QPointF& y) {
                      double px = x.x()*dir.x() + x.y()*dir.y();
                      double py = y.x()*dir.x() + y.y()*dir.y();
                      return px < py;
                  });

        QPointF A = intersections.front();
        QPointF B = intersections.back();

        QPointF v = B - A;
        double L = std::hypot(v.x(), v.y());
        if (L < offsetCut * 2)
            continue;
        QPointF tdir(v.x() / L, v.y() / L);
        QPointF A_cut = A + tdir * offsetCut;
        QPointF B_cut = B - tdir * offsetCut;

        QVector<QGV::GeoPos> line;
        line.push_back(map->getProjection()->projToGeo(A_cut));
        line.push_back(map->getProjection()->projToGeo(B_cut));
        finalLines.push_back(line);
    }

    QVector<QGV::GeoPos> snakePath;

    for (int i = 0; i < finalLines.size(); ++i) {
        QVector<QGV::GeoPos> L = finalLines[i];
        if (i % 2 == 1)
            std::reverse(L.begin(), L.end());
        if (!snakePath.isEmpty() && snakePath.last() != L.first())
            snakePath.push_back(L.first());
        snakePath += L;
    }

    return snakePath;
}

QVector<QGV::GeoPos> GeoViewRouteLogic::selectBestParallelRoute(QGVMap* map,
                                                                const QVector<QGV::GeoPos>& contourPoints,
                                                                const QGV::GeoPos& robotPos,
                                                                double stepMeters)
{
    if (!map || contourPoints.size() < 2)
        return {};

    double bestAngle = 0;
    double minDist = std::numeric_limits<double>::max();
    QVector<QGV::GeoPos> bestPath;

    for (double angle = 0; angle < 180; angle += 10) {
        QVector<QGV::GeoPos> path = buildRouteWithAngle(map, contourPoints, stepMeters, angle);

        if (path.isEmpty())
            continue;

        double dxStart = robotPos.longitude() - path.front().longitude();
        double dyStart = robotPos.latitude() - path.front().latitude();
        double distStart = dxStart*dxStart + dyStart*dyStart;

        double dxEnd = robotPos.longitude() - path.back().longitude();
        double dyEnd = robotPos.latitude() - path.back().latitude();
        double distEnd = dxEnd*dxEnd + dyEnd*dyEnd;

        double dist = std::min(distStart, distEnd);

        if (dist < minDist) {
            minDist = dist;
            bestAngle = angle;
            bestPath = path;

            if (distEnd < distStart)
                std::reverse(bestPath.begin(), bestPath.end());
        }
    }

    return bestPath;
}

QVector<QGV::GeoPos> GeoViewRouteLogic::reorderPointsForShortestRoute(const QVector<QGV::GeoPos>& points)
{
    if (points.size() < 2)
        return points;

    QVector<QGV::GeoPos> ordered;
    ordered.reserve(points.size());
    QVector<bool> used(points.size(), false);

    int current = 0;
    ordered.append(points[current]);
    used[current] = true;

    while (ordered.size() < points.size()) {
        int nearest = -1;
        double minDist = std::numeric_limits<double>::max();
        const QGV::GeoPos& from = ordered.last();
        for (int i = 0; i < points.size(); ++i) {
            if (used[i])
                continue;
            double d = haversineDistance(from, points[i]);
            if (d < minDist) {
                minDist = d;
                nearest = i;
            }
        }
        if (nearest < 0)
            break;
        ordered.append(points[nearest]);
        used[nearest] = true;
    }

    return ordered;
}

double GeoViewRouteLogic::haversineDistance(const QGV::GeoPos& start, const QGV::GeoPos& end)
{
    double lat1 = qDegreesToRadians(start.latitude());
    double lat2 = qDegreesToRadians(end.latitude());
    double dLat = lat2 - lat1;
    double dLon = qDegreesToRadians(end.longitude() - start.longitude());

    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(lat1) * cos(lat2) *
               sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return EARTH_RADIUS_METERS * c;
}

double GeoViewRouteLogic::calculateBearing(const QGV::GeoPos& start, const QGV::GeoPos& end)
{
    double lat1 = qDegreesToRadians(start.latitude());
    double lat2 = qDegreesToRadians(end.latitude());
    double dLon = qDegreesToRadians(end.longitude() - start.longitude());

    double y = sin(dLon) * cos(lat2);
    double x = cos(lat1) * sin(lat2) -
               sin(lat1) * cos(lat2) * cos(dLon);

    double bearingRad = atan2(y, x);
    double bearingDeg = qRadiansToDegrees(bearingRad);

    if (bearingDeg > 180) bearingDeg -= 360;
    if (bearingDeg < -180) bearingDeg += 360;

    return bearingDeg;
}

double GeoViewRouteLogic::calculateRosYaw(const QGV::GeoPos& start, const QGV::GeoPos& end)
{
    double bearingDeg = calculateBearing(start, end);
    double bearingRad = qDegreesToRadians(bearingDeg);

    double yawRos = M_PI_2 - bearingRad;

    if (yawRos > M_PI) yawRos -= 2 * M_PI;
    if (yawRos < -M_PI) yawRos += 2 * M_PI;

    return yawRos;
}

QPointF GeoViewRouteLogic::computeGazeboPoint(const QGV::GeoPos& start, const QGV::GeoPos& end)
{
    double distance = haversineDistance(start, end);
    double bearingDeg = calculateBearing(start, end);
    double bearingRad = qDegreesToRadians(bearingDeg);

    double xEast  = distance * sin(bearingRad);
    double yNorth = distance * cos(bearingRad);

    return QPointF(xEast, yNorth);
}

QJsonArray GeoViewRouteLogic::buildRouteCommands(const QVector<QGV::GeoPos>& routePoints,
                                                 const QGV::GeoPos& robotPos,
                                                 double robotAngleDegrees)
{
    QJsonArray commands;

    QGV::GeoPos prevPos = robotPos;
    double prevAngle = robotAngleDegrees;

    auto normalizeAngle = [](double a) {
        while (a > 180) a -= 360;
        while (a < -180) a += 360;
        return a;
    };

    for (const auto& nextPos : routePoints) {
        double lat1 = qDegreesToRadians(prevPos.latitude());
        double lon1 = qDegreesToRadians(prevPos.longitude());
        double lat2 = qDegreesToRadians(nextPos.latitude());
        double lon2 = qDegreesToRadians(nextPos.longitude());

        double dLat = lat2 - lat1;
        double dLon = lon2 - lon1;

        double a = qSin(dLat/2)*qSin(dLat/2) + qCos(lat1)*qCos(lat2)*qSin(dLon/2)*qSin(dLon/2);
        double c = 2 * qAtan2(qSqrt(a), qSqrt(1 - a));
        double distance = EARTH_RADIUS_METERS * c;

        if (distance < 0.2) {
            prevPos = nextPos;
            continue;
        }

        double x = qSin(dLon) * qCos(lat2);
        double y = qCos(lat1)*qSin(lat2) - qSin(lat1)*qCos(lat2)*qCos(dLon);
        double azimuth = qRadiansToDegrees(qAtan2(x, y));
        if (azimuth < 0) azimuth += 360.0;

        double deltaAngle = normalizeAngle(azimuth - prevAngle);

        if (qAbs(deltaAngle) > 150 && distance < 2.0)
            deltaAngle = 0;

        if (qFabs(deltaAngle) > 1e-3) {
            QJsonObject rotateCmd;
            rotateCmd["cmd"] = "rotate";
            QJsonObject data;
            data["delta_angle"] = deltaAngle;
            rotateCmd["data"] = data;
            commands.append(rotateCmd);
            prevAngle = normalizeAngle(prevAngle + deltaAngle);
        } else {
            prevAngle = azimuth;
        }

        QJsonObject moveCmd;
        moveCmd["cmd"] = "move";
        QJsonObject data;
        data["distance_m"] = distance;
        moveCmd["data"] = data;
        commands.append(moveCmd);

        prevPos = nextPos;
    }

    return commands;
}
