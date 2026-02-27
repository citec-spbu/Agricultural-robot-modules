#pragma once

#include <QJsonArray>
#include <QPointF>
#include <optional>

#include <QGeoView/QGVMap.h>

constexpr double EARTH_RADIUS_METERS = 6371000.0;

/**
 * Логика построения маршрутов и гео-вычислений:
 * пересечения, расстояния, азимуты, генерация команд для робота.
 */
class GeoViewRouteLogic
{
public:
    static std::optional<QGV::GeoPos> segmentIntersection(const QGV::GeoPos& a, const QGV::GeoPos& b,
                                                          const QGV::GeoPos& c, const QGV::GeoPos& d);

    static QVector<QGV::GeoPos> polygonSelfIntersections(const QVector<QGV::GeoPos>& points);

    /** Проверка, лежит ли точка внутри полигона (ray casting). */
    static bool isPointInsidePolygon(const QVector<QGV::GeoPos>& polygon, const QGV::GeoPos& point);

    /** Построение параллельных линий внутри контура. */
    static QVector<QGV::GeoPos> buildRouteWithAngle(QGVMap* map,
                                                    const QVector<QGV::GeoPos>& contourPoints,
                                                    double stepMeters,
                                                    double angleDegrees,
                                                    double offsetFromContour = 2.0,
                                                    double offsetCut = 2.0);

    /** Выбор лучшего пути по углу (ближе к robotPos) и возврат упорядоченных точек. */
    static QVector<QGV::GeoPos> selectBestParallelRoute(QGVMap* map,
                                                        const QVector<QGV::GeoPos>& contourPoints,
                                                        const QGV::GeoPos& robotPos,
                                                        double stepMeters);

    static QVector<QGV::GeoPos> reorderPointsForShortestRoute(const QVector<QGV::GeoPos>& points);

    static double haversineDistance(const QGV::GeoPos& pos1, const QGV::GeoPos& pos2);
    static double calculateBearing(const QGV::GeoPos& start, const QGV::GeoPos& end);
    static double calculateRosYaw(const QGV::GeoPos& start, const QGV::GeoPos& end);
    static QPointF computeGazeboPoint(const QGV::GeoPos& start, const QGV::GeoPos& end);

    /** Генерация JSON-команд маршрута (rotate/move) по точкам и начальной позиции робота. */
    static QJsonArray buildRouteCommands(const QVector<QGV::GeoPos>& routePoints,
                                         const QGV::GeoPos& robotPos,
                                         double robotAngleDegrees);
};
