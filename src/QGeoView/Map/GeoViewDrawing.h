#pragma once

#include <QImage>
#include <QJsonDocument>
#include <QPen>
#include <QUrl>

#include <QGeoView/QGVLayer.h>
#include <QGeoView/QGVMap.h>

class GeoPolyline;
class QGVIcon;

/** Отвечает за рисование на карте: маршруты, иконки, очистка слоёв. */
class GeoViewDrawing
{
public:
    GeoViewDrawing() = default;

    void drawRoute(QGVLayer* layer,
                   QGVMap* map,
                   const QVector<QGV::GeoPos>& pts,
                   const QPen& pen = QPen(QColor(220, 60, 60, 180), 2),
                   bool drawArrow = false,
                   bool replaceExisting = false);

    void clearRouteLayer(QGVLayer* routeLayer, QVector<QGV::GeoPos>& routePoints, QJsonDocument*& routeCommands);
    void clearRobotLayer(QGVLayer* robotLayer, QGVIcon*& robotItem, QGV::GeoPos& robotPos, double& robotAngle);
    void clearRobotRouteLayer(QGVLayer* robotRouteLayer, QVector<QGV::GeoPos>& robotRoutePoints);

    void preloadImages(QImage& robotIcon, QImage& waypointIcon);
    void loadImage(QImage& dest, QUrl url);
};
