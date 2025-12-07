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

