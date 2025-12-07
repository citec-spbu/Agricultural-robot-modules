/***************************************************************************
 * QGeoView is a Qt / C ++ widget for visualizing geographic data.
 * Copyright (C) 2018-2025 Andrey Yaroshenko.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see https://www.gnu.org/licenses.
 ****************************************************************************/


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






