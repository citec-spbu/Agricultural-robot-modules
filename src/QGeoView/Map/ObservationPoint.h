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
#include <QJsonObject>
#include <QImage>
#include <QColor>

class ObservationPoint : public QGVDrawItem
{
    Q_OBJECT

public:
    ObservationPoint(QGVMap* map, const QGV::GeoPos& pos, const QJsonObject& json = QJsonObject(), double radiusMeters = 2.0, QObject* parent = nullptr);
    ObservationPoint(const QJsonObject&, QGVMap* map, double radiusMeters = 2.0, QObject* parent = nullptr);
    ~ObservationPoint()  = default;

    void setRobotData(const QJsonObject& obj);
    void setMLResults(const QJsonObject& obj);

    const QJsonObject& robotData() const { return mRobotData; }
    const QJsonObject& mlResults() const { return mMLResults; }

    QGV::GeoPos pos() const { return mPos; }
    QColor color() const;
    QImage image() const { return mImage; }

    void setRadiusMeters(double r) { mRadiusMeters = r; }
    double radiusMeters() const { return mRadiusMeters; }

    void showData();

protected:
    void onProjection(QGVMap* map) override;
    void onUpdate() override;
    QPainterPath projShape() const override;
    void projPaint(QPainter* p) override;

private:
    QImage decodeBase64Image(const QString& base64) const;

private:
    QGVMap* mMap;

    QGV::GeoPos mPos;
    QJsonObject mRobotData;
    QJsonObject mMLResults;
    QImage mImage;
    double mRadiusMeters;

    QPointF m_projPos; // позиция в координатах проекции
};
