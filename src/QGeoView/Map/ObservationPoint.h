#pragma once
#include <QGeoView/QGVDrawItem.h>
#include <QJsonObject>
#include <QImage>
#include <QColor>

class ObservationPoint : public QGVDrawItem
{
    Q_OBJECT

public:
    ObservationPoint(QGVMap* map, const QGV::GeoPos& pos, const QJsonObject& json = QJsonObject(), double radiusMeters = 1.0, QObject* parent = nullptr);
    ObservationPoint(const QJsonObject&, QGVMap* map, double radiusMeters = 1.0, QObject* parent = nullptr);
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
