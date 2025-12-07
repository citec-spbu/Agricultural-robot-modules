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
