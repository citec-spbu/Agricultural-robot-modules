#pragma once
#include "Contour.h"
#include "ObservaionsLayer.h"
#include "ContourPreviewDialog.h"

#include <QWidget>
#include <QGroupBox>
#include <QListWidget>
#include <QPushButton>
#include <QImage>

#include <QGeoView/Raster/QGVIcon.h>
#include <QGeoView/QGVLayer.h>
#include <QGeoView/QGVMap.h>
#include <qjsondocument.h>

class GeoViewWidget : public QWidget
{
    Q_OBJECT

private:
    struct RobotItem{
        QGVIcon* item = nullptr;
        QGV::GeoPos pos = QGV::GeoPos(0, 0);
        double angle = 0;
    };

public:
    GeoViewWidget(QWidget* parent = nullptr);
    ~GeoViewWidget();

    QGV::GeoRect targetArea() const;

    QGroupBox* createOptionsList();
    QGroupBox* createInfoList();

    void preloadImages();
    void loadImage(QImage& dest, QUrl url);

    void addRobotOnMapFromMousePress(QPointF projPos);
    void addRobotOnMapFromJson(const QJsonObject& obj);
    void addRobot(double latitude, double longitude, double angle);

    void updateRobotOnMapFromMousePress(QPointF projPos);
    void updateRobotOnMapFromJson(const QJsonObject& obj);
    void updateRobot(double latitude, double longitude, double angle);

    void startProcessingPoints(const QString& jsonPoints);

    void updateInfoList();

    void generateParallelRoute(double stepMeters);

    QVector<QGV::GeoPos> buildRouteWithAngle(double stepMeters,
                                             double angleDegrees,
                                             double offsetFromContour = 2.0,
                                             double offsetCut = 2.0);

    void handleMapClick(const QGV::GeoPos& pos);

    void drawRoute( QGVLayer* layer,
                   const QVector<QGV::GeoPos>& pts,
                   const QPen& pen = QPen(QColor(220, 60, 60, 180), 2),
                   bool drawArrow = false,
                   bool replaceExisting = false);

    QJsonDocument getRouteCommands() const;
    const RobotItem& getRobotItem() const;

protected:
    inline void addObservation(const QJsonObject& RobotData);

    std::optional<QGV::GeoPos> segmentIntersection(const QGV::GeoPos& a, const QGV::GeoPos& b,
                                                   const QGV::GeoPos& c, const QGV::GeoPos& d);

    QVector<QGV::GeoPos> polygonSelfIntersections(const QVector<QGV::GeoPos>& points);

    void addMlResults(const QJsonObject& MlResults);

    inline void clearMainLayer(){ if(mLayer) mLayer->deleteItems(); }
    inline void clearObservationLayer(){ if(mObservationLayer) mObservationLayer->clear(); }

    inline void clearRouteCommands();

    void clearRouteLayer();
    void clearRobotRouteLayer();
    void clearRobotLayer();

signals:
    void routeBuilt(const QJsonDocument& jsonDoc);

public slots:
    void addContour();
    void createRoute();
    void toggleManualRouteMode();
    void removeContour();
    void showRobotCommandsJson();

    void initRobotPos(const QJsonObject& json);
    void updateRobotPos(const QJsonObject& json);
    void setMlResults(const QJsonObject& json);

    void clearAll();

private:
    QGVMap* mMap;

    QGVLayer* mLayer = nullptr;
    QGVLayer* mRouteLayer = nullptr;
    QGVLayer* mRobotRouteLayer = nullptr;
    QGVLayer* mRobotLayer = nullptr;
    ObservationsLayer* mObservationLayer = nullptr;

    Contour* mContour = nullptr;
    ContourPreviewDialog* mPrevDialog = nullptr;

    QVector<QGV::GeoPos> mRoutePoints;
    QVector<QGV::GeoPos> mRobotRoutePoints;

    QPen mContourColor = QPen(QColor(220, 60, 60, 180), 3);
    QPen mRouteColor = QPen(QColor(50, 120, 255, 200), 2);
    QPen mRobotRouteColor = QPen(QColor(0, 180, 70, 200), 1);

    QJsonDocument* mRouteCommands = nullptr;

    QListWidget* mInfoList = nullptr;

    RobotItem mRobotItem;

    bool mManualRouteMode = false;
    bool mWaitingForRobotPos = false;

    QImage mRobotIcon;
    QImage mArrow;

    QPushButton* mCreateRouteButton = nullptr;
};
