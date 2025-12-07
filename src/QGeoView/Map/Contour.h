#pragma once
#include <QVector>
#include <QGeoView/QGVMap.h>
#include "GeoPolyline.h"

class Contour
{
public:
    Contour(QGVMap* map);

    void setPoints(const QVector<QGV::GeoPos>& points);
    const QVector<QGV::GeoPos>& points() const;

    void draw();
    void clear();
    void generateParallelRoute(double spacing);

private:
    QGVMap* mMap = nullptr;
    QVector<QGV::GeoPos> mPoints;
    GeoPolyline* mPolyline = nullptr;
};
