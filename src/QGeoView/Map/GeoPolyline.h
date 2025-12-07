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

#include <QGeoView/QGVDrawItem.h>
#include <QGeoView/QGVProjection.h>
#include <QPainterPath>
#include <QPen>


class GeoPolyline : public QGVDrawItem
{
    Q_OBJECT
public:
    explicit GeoPolyline(QGVMap* map, QObject* parent = nullptr);

    QVector<QGV::GeoPos> points;

    void setPen(const QPen& pen) { mPen = pen; update(); }

    bool drawArrowOnEnd = false;
protected:

    void onProjection(QGVMap* map) override;
    void onUpdate() override;
    QPainterPath projShape() const override;
    void projPaint(QPainter* p) override;

private:
    QGVMap* mMap = nullptr;
    QVector<QPointF> mProjPoints;
    QPen mPen = QPen(QColor(255, 0, 0, 180), 2); // красная линия по умолчанию
    QPainterPath mCachedPath;

    void rebuild();
    void drawArrow(QPainter* p, const QPointF& start, const QPointF& end);
};
