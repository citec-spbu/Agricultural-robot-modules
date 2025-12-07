#include "ObservaionsLayer.h"
#include <QGeoView/Raster/QGVIcon.h>
#include <QPainter>
#include <QDebug>
#include <QTimer>

ObservationsLayer::ObservationsLayer(QGVMap* map)
    :mMap(map)
{}

ObservationsLayer::~ObservationsLayer()
{
    clear();
}

void ObservationsLayer::addPoint(const QGV::GeoPos& pos, const QJsonObject& json)
{
    for (auto* point : std::as_const(mPoints))
    {
        if (point->pos() == pos)
        {
            point->setRobotData(json);
            return;
        }
    }

    ObservationPoint* point = new ObservationPoint(mMap, pos, json);
    mPoints.append(point);
    addItem(point);
}

void ObservationsLayer::addPoint(const QGV::GeoPos& pos)
{
    ObservationPoint* point = new ObservationPoint(mMap, pos);
    mPoints.append(point);
    addItem(point);
}

void ObservationsLayer::addPoint(const QJsonObject& json)
{
    double lat = json.value("latitude").toDouble();
    double lon = json.value("longitude").toDouble();
    QGV::GeoPos pos(lat, lon);
    addPoint(pos, json);
}

void ObservationsLayer::clear()
{
    deleteItems();

    mPoints.clear();
}

void ObservationsLayer::handleMapClick(const QGV::GeoPos& clickPos)
{
    if (!mMap) return;

    QPointF clickProj = mMap->getProjection()->geoToProj(clickPos);

    for (auto* point : std::as_const(mPoints))
    {
        QPointF pointProj = mMap->getProjection()->geoToProj(point->pos());

        double dx = clickProj.x() - pointProj.x();
        double dy = clickProj.y() - pointProj.y();
        double distance = std::sqrt(dx*dx + dy*dy);

        if (distance <= mPointRadiusMeters)
        {
            QTimer::singleShot(200, [point](){
                point->showData();
            });
            break;
        }

    }
}






