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

#include "Contour.h"

Contour::Contour(QGVMap* map)
    : mMap(map)
{}

void Contour::setPoints(const QVector<QGV::GeoPos>& points)
{
    mPoints = points;
}

const QVector<QGV::GeoPos>& Contour::points() const
{
    return mPoints;
}

void Contour::draw()
{
    if (!mMap || mPoints.isEmpty()) return;

    if (!mPolyline) {
        mPolyline = new GeoPolyline(mMap);
        mPolyline->points = mPoints;
        mMap->addItem(mPolyline);
    }

    mPolyline->points = mPoints;
}

void Contour::clear()
{
    if (mPolyline && mMap) {
        mMap->removeItem(mPolyline);
        delete mPolyline;
        mPolyline = nullptr;
    }
    mPoints.clear();
}

