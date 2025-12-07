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

#pragma once
#include <QObject>
#include <QVector>
#include <QImage>
#include <QGeoView/QGVLayer.h>
#include <QGeoView/QGVMap.h>
#include "ObservationPoint.h"

class ObservationsLayer : public QGVLayer
{
    Q_OBJECT

public:
    ObservationsLayer(QGVMap* map);
    ~ObservationsLayer() override;

    void addPoint(const QGV::GeoPos& pos);
    void addPoint(const QGV::GeoPos& pos, const QJsonObject& json);
    void addPoint(const QJsonObject& json);

    void setPointRadiusMeters(double r) { mPointRadiusMeters = r; }
    double pointRadiusMeters() const { return mPointRadiusMeters; }

    const QVector<ObservationPoint*>& points() const { return mPoints; }

    void clear();

public slots:
    void handleMapClick(const QGV::GeoPos& clickPos);

private:
    QGVMap* mMap = nullptr;
    QVector<ObservationPoint*> mPoints;
    QImage mPointImage;
    double mPointRadiusMeters = 1.5; // единый радиус для всех точек
};
