#include "GeoViewDrawing.h"
#include "GeoPolyline.h"

#include <QObject>
#include <QPainter>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <QGeoView/QGVGlobal.h>
#include <QGeoView/QGVMap.h>
#include <QGeoView/Raster/QGVIcon.h>

void GeoViewDrawing::drawRoute(QGVLayer* layer,
                               QGVMap* map,
                               const QVector<QGV::GeoPos>& pts,
                               const QPen& pen,
                               bool drawArrow,
                               bool replaceExisting)
{
    if (!layer || !map)
        return;

    if (!replaceExisting)
        layer->deleteItems();

    if (pts.size() < 2)
        return;

    auto* poly = new GeoPolyline(map);
    poly->points = pts;
    poly->setPen(pen);
    poly->drawArrowOnEnd = drawArrow;

    layer->addItem(poly);
}

void GeoViewDrawing::clearRouteLayer(QGVLayer* routeLayer,
                                     QVector<QGV::GeoPos>& routePoints,
                                     QJsonDocument*& routeCommands)
{
    if (routeLayer)
        routeLayer->deleteItems();

    if (!routePoints.empty())
        routePoints.clear();

    if (routeCommands) {
        delete routeCommands;
        routeCommands = nullptr;
    }
}

void GeoViewDrawing::clearRobotLayer(QGVLayer* robotLayer,
                                     QGVIcon*& robotItem,
                                     QGV::GeoPos& robotPos,
                                     double& robotAngle)
{
    if (robotLayer)
        robotLayer->deleteItems();

    robotItem = nullptr;
    robotPos = QGV::GeoPos(0, 0);
    robotAngle = 0.0;
}

void GeoViewDrawing::clearRobotRouteLayer(QGVLayer* robotRouteLayer,
                                          QVector<QGV::GeoPos>& robotRoutePoints)
{
    if (robotRouteLayer)
        robotRouteLayer->deleteItems();

    if (!robotRoutePoints.empty())
        robotRoutePoints.clear();
}

void GeoViewDrawing::preloadImages(QImage& robotIcon, QImage& waypointIcon)
{
    robotIcon.load(":/icons/robot_icons/robot_icon.png");

    const int size = 20;
    waypointIcon = QImage(size, size, QImage::Format_ARGB32);
    waypointIcon.fill(Qt::transparent);
    QPainter p(&waypointIcon);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.setPen(QPen(QColor(50, 120, 255), 2));
    p.setBrush(QBrush(QColor(50, 120, 255, 220)));
    p.drawEllipse(2, 2, size - 4, size - 4);
    p.end();
}

void GeoViewDrawing::loadImage(QImage& dest, QUrl url)
{
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent",
                         "Mozilla/5.0 (Windows; U; MSIE "
                         "6.0; Windows NT 5.1; SV1; .NET "
                         "CLR 2.0.50727)");
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);

    QNetworkReply* reply = QGV::getNetworkManager()->get(request);
    QObject::connect(reply, &QNetworkReply::finished, reply, [reply, &dest]() {
        if (reply->error() != QNetworkReply::NoError) {
            qgvCritical() << "ERROR" << reply->errorString();
            reply->deleteLater();
            return;
        }
        dest.loadFromData(reply->readAll());
        reply->deleteLater();
    });

    qgvDebug() << "request" << url;
}
