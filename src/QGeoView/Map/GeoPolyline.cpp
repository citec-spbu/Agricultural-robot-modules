#include "GeoPolyline.h"
#include <QGeoView/QGVMap.h>
#include <QPainter.h>
#include <cmath>

GeoPolyline::GeoPolyline(QGVMap* map, QObject* parent)
    : QGVDrawItem()
      , mMap(map)
{
    Q_UNUSED(parent);
}

void GeoPolyline::onProjection(QGVMap* map)
{
    QGVDrawItem::onProjection(map);
    rebuild();
}

void GeoPolyline::onUpdate()
{
    rebuild();
}

void GeoPolyline::rebuild()
{
    mProjPoints.clear();
    mCachedPath = QPainterPath();

    if (!mMap)
        return;

    auto* proj = mMap->getProjection();
    if (!proj)
        return;

    for (const auto& gp : std::as_const(points))
        mProjPoints.push_back(proj->geoToProj(gp));

    if (!mProjPoints.isEmpty()) {
        mCachedPath.moveTo(mProjPoints.first());
        for (int i = 1; i < mProjPoints.size(); i++)
            mCachedPath.lineTo(mProjPoints[i]);
    }
}

QPainterPath GeoPolyline::projShape() const
{
    return mCachedPath;
}

void GeoPolyline::projPaint(QPainter* p)
{
    if (mProjPoints.size() > 100)
        p->setRenderHint(QPainter::Antialiasing, false);
    else
        p->setRenderHint(QPainter::Antialiasing, true);

    p->setPen(mPen);
    p->drawPath(mCachedPath);

    if(drawArrowOnEnd && mProjPoints.size() >= 2)
    {
        QPointF start = mProjPoints[mProjPoints.size() - 2];
        QPointF end = mProjPoints[mProjPoints.size() - 1];
        drawArrow(p, start, end);
    }
}

void GeoPolyline::drawArrow(QPainter* p, const QPointF& start, const QPointF& end)
{
    p->setRenderHint(QPainter::Antialiasing, false);

    p->setPen(Qt::NoPen);
    p->setBrush(mPen.color());

    QLineF line(start, end);
    double angle = std::atan2(line.dy(), line.dx());

    double arrowLength = 15;
    double arrowWidth  = 4;

    QPointF tip = end;

    QPointF left = tip - QPointF(arrowLength * std::cos(angle) - arrowWidth * std::sin(angle),
                                 arrowLength * std::sin(angle) + arrowWidth * std::cos(angle));

    QPointF right = tip - QPointF(arrowLength * std::cos(angle) + arrowWidth * std::sin(angle),
                                  arrowLength * std::sin(angle) - arrowWidth * std::cos(angle));

    QPolygonF arrowHead;
    arrowHead << tip << left << right;

    p->drawPolygon(arrowHead, Qt::WindingFill);

    p->setPen(mPen);
    p->drawPolygon(arrowHead);
}

