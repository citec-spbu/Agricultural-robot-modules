#include "GeoViewWidget.h"
#include "GeoViewRouteLogic.h"
#include "GeoPolyline.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QFileDialog>
#include <QAbstractButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QEvent>
#include <QPainter>
#include <helpers.h>
#include <rectangle.h>
#include <optional>
#include <limits>

#include <QGeoView/QGVLayerOSM.h>
#include <QGeoView/QGVMapQGView.h>
#include <QGeoView/QGVDrawItem.h>
#include <QGeoView/QGVWidgetCompass.h>
#include <QGeoView/QGVWidgetScale.h>
#include <QGeoView/QGVWidgetZoom.h>
#include <QGeoView/Raster/QGVIcon.h>
#include <QGeoView/Raster/QGVImage.h>

GeoViewWidget::GeoViewWidget(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("Map");

    // Создаём основной layout для виджета
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Инициализация карты и добавление виджетов
    Helpers::setupCachedNetworkAccessManager(this);

    mMap = new QGVMap(this);
    layout->addWidget(mMap);  // Добавляем карту в layout

    // Добавляем виджеты компаса, зума и масштаба
    mMap->addWidget(new QGVWidgetCompass());
    mMap->addWidget(new QGVWidgetZoom());
    mMap->addWidget(new QGVWidgetScale());

    // Слой с картой
    auto osmLayer = new QGVLayerOSM();
    mMap->addItem(osmLayer);

    // Растерный слой
    mLayer = new QGVLayer();
    mLayer->setName("MainLayer");

    // Слой для маршрута
    mRouteLayer = new QGVLayer();
    mRouteLayer->setName("RouteLayer");

    // Слой для пути робота
    mRobotRouteLayer = new QGVLayer();
    mRobotRouteLayer->setName("mRobotRouteLayer");

    // Слой для наблюдений
    mObservationLayer = new ObservationsLayer(mMap);
    mObservationLayer->setName("ObservationLayer");

    // Слой для иконки робота
    mRobotLayer = new QGVLayer();
    mRobotLayer->setName("RobotLayer");

    // Добавляем все слои на карту
    mMap->addItem(mLayer);
    mMap->addItem(mRouteLayer);
    mMap->addItem(mRobotRouteLayer);
    mMap->addItem(mObservationLayer);
    mMap->addItem(mRobotLayer);

    mMap->setMouseTracking(true);

    mToolbar = new GeoViewToolbar(this);
    QWidget* widget = new QWidget();
    QHBoxLayout* widgetLayout = new QHBoxLayout(widget);
    widgetLayout->addWidget(createOptionsList());
    widgetLayout->addWidget(createInfoList());
    layout->addWidget(widget);

    connect(mToolbar, &GeoViewToolbar::setInitialRobotPositionRequested, this, [this]() {
        mWaitingForRobotPos = true;
        QMessageBox::information(this,
                                 tr("Задать позицию робота"),
                                 tr("Нажмите на карту, чтобы задать начальную позицию робота."));
    });
    connect(mToolbar, &GeoViewToolbar::setInitialRobotPositionFromFileRequested, this, [this]() {
        QString path = QFileDialog::getOpenFileName(this, tr("Выбрать конфиг робота"), QString(), tr("JSON (*.json);;Все файлы (*)"));
        if (path.isEmpty())
            return;
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось открыть файл: %1").arg(path));
            return;
        }
        QByteArray data = file.readAll();
        file.close();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            QMessageBox::warning(this, tr("Ошибка"), tr("Неверный JSON: %1").arg(err.errorString()));
            return;
        }
        QJsonObject obj = doc.object();
        if (!obj.contains("latitude") || !obj.contains("longitude")) {
            QMessageBox::warning(this, tr("Ошибка"), tr("В конфиге должны быть поля latitude и longitude (и опционально rotation_angle)."));
            return;
        }
        addRobotOnMapFromJson(obj);
    });
    connect(mToolbar, &GeoViewToolbar::addContourRequested, this, &GeoViewWidget::addContour);
    connect(mToolbar, &GeoViewToolbar::buildParallelRouteAutoRequested, this, [this]() {
        generateParallelRoute(4);
    });
    connect(mToolbar, &GeoViewToolbar::buildParallelRouteWithAngleRequested, this, [this]() {
        bool ok = false;
        double angle = QInputDialog::getDouble(this,
                                               tr("Угол параллельного маршрута"),
                                               tr("Угол в градусах (0–180):"),
                                               0.0, -360.0, 360.0, 1, &ok);
        if (!ok)
            return;
        generateParallelRouteWithAngle(4, angle);
    });
    connect(mToolbar, &GeoViewToolbar::buildCommandsShowJsonRequested, this, [this]() {
        createRoute();
        showRobotCommandsJson();
        qDebug() << "Маршрут и команды обновлены.";
    });
    connect(mToolbar, &GeoViewToolbar::buildCommandsShowPointsRequested, this, [this]() {
        createRoute();
        showRobotCommandsAsPoints();
    });
    connect(mToolbar, &GeoViewToolbar::startManualRouteShortestRequested, this, [this]() { startManualRouteMode(true); });
    connect(mToolbar, &GeoViewToolbar::startManualRouteInOrderRequested, this, [this]() { startManualRouteMode(false); });
    connect(mToolbar, &GeoViewToolbar::toggleManualRouteRequested, this, &GeoViewWidget::toggleManualRouteMode);
    connect(mToolbar, &GeoViewToolbar::removeContourRequested, this, &GeoViewWidget::removeContour);
    connect(mToolbar, &GeoViewToolbar::clearAllRequested, this, &GeoViewWidget::clearAll);
    connect(mToolbar, &GeoViewToolbar::saveGazeboJsonRequested, this, [this]() {
        QJsonDocument jsonDoc = generateGazeboJson();
        if (jsonDoc.isEmpty()) return;
        QString filename = QFileDialog::getSaveFileName(this, tr("Сохранить JSON для Gazebo"), QString(), tr("JSON Files (*.json)"));
        if (!filename.isEmpty()) {
            QFile file(filename);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(jsonDoc.toJson());
                file.close();
            } else {
                QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось сохранить файл"));
            }
        }
    });
    connect(mToolbar, &GeoViewToolbar::saveRouteLatLonRequested, this, [this]() {
        if (mRoutePoints.isEmpty()) {
            QMessageBox::warning(this, tr("Нет маршрута"),
                                 tr("Сначала постройте маршрут (параллельный или ручной), затем экспортируйте его."));
            return;
        }

        QString filename = QFileDialog::getSaveFileName(this,
                                                        tr("Сохранить список координат"),
                                                        QString(),
                                                        tr("Text Files (*.txt);;All Files (*)"));
        if (filename.isEmpty())
            return;

        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось сохранить файл"));
            return;
        }

        QTextStream out(&file);
        for (const auto& p : std::as_const(mRoutePoints)) {
            out << QString::number(p.latitude(), 'f', 8)
                << ", "
                << QString::number(p.longitude(), 'f', 8)
                << "\n";
        }

        file.close();
    });

    QTimer::singleShot(100, this, [this]() {
        mMap->cameraTo(QGVCameraActions(mMap).scaleTo(targetArea()));
    });

    // Загрузка изображений
    preloadImages();

    // Подключение к событию клика на карту
    connect(mMap, &QGVMap::mapMousePress, this, [this](QPointF projPos) {
        if (mWaitingForRobotPos) {
            addRobotOnMapFromMousePress(projPos);
            return;
        }

        QGV::GeoPos geoPos = mMap->getProjection()->projToGeo(projPos);
        mObservationLayer->handleMapClick(geoPos);

        if (!mManualRouteMode) return;

        handleMapClick(geoPos);
    });
}

GeoViewWidget::~GeoViewWidget()
{
    clearAll();
}

QGV::GeoRect GeoViewWidget::targetArea() const
{
    return QGV::GeoRect(QGV::GeoPos(50, 14), QGV::GeoPos(52, 15));
}

QGroupBox* GeoViewWidget::createOptionsList()
{
    return mToolbar->createOptionsList(&mManualRouteButton);
}

void GeoViewWidget::startProcessingPoints(const QString& jsonPoints)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonPoints.toUtf8());
    QJsonArray pointsArray = doc.array();

    if (pointsArray.isEmpty()) {
        qWarning() << "JSON пустой или не массив!";
        return;
    }

    int currentIndex = 0;

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, timer, pointsArray = pointsArray, currentIndex]() mutable {
        if (currentIndex >= pointsArray.size()) {
            timer->stop();
            timer->deleteLater();
            return;
        }

        QJsonObject point = pointsArray[currentIndex].toObject();

        double lat = point.value("latitude").toDouble();
        double lon = point.value("longitude").toDouble();

        if (lat != 0 || lon != 0)
        {
            if (!mRobotItem.item) {
                double angle = point.value("rotation_angle").toDouble();
                addRobot(lat, lon, angle);
            } else {
                updateRobotOnMapFromJson(point);
            }
        }

        currentIndex++;
    });

    timer->start(2000); // каждые 2 секунды
}


QGroupBox* GeoViewWidget::createInfoList()
{
    QGroupBox* box = mToolbar->createInfoList(mInfoList);
    updateInfoList();
    return box;
}

void GeoViewWidget::preloadImages()
{
    mDrawing.preloadImages(mRobotIcon, mWaypointIcon);
}

void GeoViewWidget::loadImage(QImage& dest, QUrl url)
{
    mDrawing.loadImage(dest, url);
}

void GeoViewWidget::addContour()
{
    // Контур — полигон из GeoJSON (поле/участок). Внутри него строятся параллельные линии
    // (маршрут движения робота). Кнопка «Добавить контур» нужна для автоматического
    // построения маршрута по полю: загружается контур → по нему генерируются параллельные
    // полосы с заданным шагом, затем экспорт в JSON для симуляции в Gazebo.
    QString fileName = QFileDialog::getOpenFileName(this, "Выберите файл GeoJSON", "", "*.geojson");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Не удалось открыть файл");
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning("Ошибка парсинга JSON: %s", qPrintable(parseError.errorString()));
        return;
    }

    if (!doc.isObject()) {
        qWarning("Некорректный формат GeoJSON");
        return;
    }

    QJsonObject obj = doc.object();
    if (!obj.contains("features") || !obj["features"].isArray()) {
        qWarning("GeoJSON не содержит features");
        return;
    }

    QJsonArray features = obj["features"].toArray();
    for (const auto& f : std::as_const(features)) {
        if (!f.isObject()) continue;
        QJsonObject feature = f.toObject();
        if (!feature.contains("geometry") || !feature["geometry"].isObject()) continue;

        QJsonObject geometry = feature["geometry"].toObject();
        if (!geometry.contains("coordinates") || !geometry["coordinates"].isArray()) continue;

        QString type = geometry["type"].toString();
        QJsonArray coordsArray = geometry["coordinates"].toArray();

        QVector<QGV::GeoPos> points;

        if (type == "Polygon" && !coordsArray.isEmpty()) {
            // Берем первый контур полигона
            QJsonArray ring = coordsArray[0].toArray();
            for (const auto& pt : std::as_const(ring)) {
                QJsonArray pair = pt.toArray();
                if (pair.size() >= 2) {
                    double lon = pair[0].toDouble();
                    double lat = pair[1].toDouble();
                    points.push_back({lat, lon});
                }
            }
        } else if (type == "LineString") {
            for (const auto& pt : std::as_const(coordsArray)) {
                QJsonArray pair = pt.toArray();
                if (pair.size() >= 2) {
                    double lon = pair[0].toDouble();
                    double lat = pair[1].toDouble();
                    points.push_back({lat, lon});
                }
            }
        } else {
            continue; // Игнорируем другие типы геометрии
        }

        if (points.isEmpty())
            continue;

        const auto& a = points.first();
        const auto& b = points.last();

        double eps = 1e-7;
        if (std::abs(a.latitude() - b.latitude()) > eps || std::abs(a.longitude() - b.longitude()) > eps)
            points.push_back(a);

        QVector<QGV::GeoPos> intersections = GeoViewRouteLogic::polygonSelfIntersections(points);

        if (!intersections.empty()) {
            mPrevDialog = new ContourPreviewDialog(points, intersections, this);
            mPrevDialog->exec();
            return;
        }

        if(mContour)
            mContour->clear();

        mContour = new Contour(mMap);

        mContour->setPoints(points);

        mContour->draw();

        double minLat = std::numeric_limits<double>::max();
        double maxLat = std::numeric_limits<double>::lowest();
        double minLon = std::numeric_limits<double>::max();
        double maxLon = std::numeric_limits<double>::lowest();

        for (const auto& p : std::as_const(points)) {
            if (p.latitude() < minLat) minLat = p.latitude();
            if (p.latitude() > maxLat) maxLat = p.latitude();
            if (p.longitude() < minLon) minLon = p.longitude();
            if (p.longitude() > maxLon) maxLon = p.longitude();
        }

        QGV::GeoRect bounds({minLat, minLon}, {maxLat, maxLon});
        mMap->cameraTo(QGVCameraActions(mMap).scaleTo(bounds));
    }
}

void GeoViewWidget::generateParallelRoute(double stepMeters)
{
    if (!mContour || mContour->points().size() < 2 || !mRouteLayer)
        return;

    if (!mRobotItem.item) {
        QMessageBox::warning(this, tr("Нет робота"),
                             tr("Сначала разместите робота на карте, затем нажмите «Построить параллельный маршрут»."));
        return;
    }

    QVector<QGV::GeoPos> bestPath = GeoViewRouteLogic::selectBestParallelRoute(
        mMap, mContour->points(), mRobotItem.pos, stepMeters);

    if (bestPath.isEmpty())
        return;

    buildParallelRouteFromPath(bestPath);
}

void GeoViewWidget::generateParallelRouteWithAngle(double stepMeters, double angleDegrees)
{
    if (!mContour || mContour->points().size() < 2 || !mRouteLayer)
        return;

    if (!mRobotItem.item) {
        QMessageBox::warning(this, tr("Нет робота"),
                             tr("Сначала разместите робота на карте, затем нажмите «Построить параллельный маршрут»."));
        return;
    }

    QVector<QGV::GeoPos> path = GeoViewRouteLogic::buildRouteWithAngle(
        mMap, mContour->points(), stepMeters, angleDegrees);

    if (path.isEmpty())
        return;

    double dxStart = mRobotItem.pos.longitude() - path.front().longitude();
    double dyStart = mRobotItem.pos.latitude() - path.front().latitude();
    double distStart = dxStart*dxStart + dyStart*dyStart;

    double dxEnd = mRobotItem.pos.longitude() - path.back().longitude();
    double dyEnd = mRobotItem.pos.latitude() - path.back().latitude();
    double distEnd = dxEnd*dxEnd + dyEnd*dyEnd;

    if (distEnd < distStart)
        std::reverse(path.begin(), path.end());

    buildParallelRouteFromPath(path);
}

void GeoViewWidget::removeContour()
{
    if (!mContour) {
        return;
    }

    mContour->clear();
    clearRouteLayer();

    updateInfoList();
}

void GeoViewWidget::updateInfoList()
{
    if(!mInfoList) return;

    mInfoList->clear();

    for (int i = 0; i < mLayer->countItems(); i++) {
        QGVItem* item = mLayer->getItem(i);
        mInfoList->addItem(item->metaObject()->className());
    }
}

void GeoViewWidget::handleMapClick(const QGV::GeoPos& pos)
{
    if (mContour && mContour->points().size() >= 3) {
        if (!GeoViewRouteLogic::isPointInsidePolygon(mContour->points(), pos)) {
            QMessageBox::warning(this, tr("Точка вне контура"),
                                 tr("Вы кликнули за пределами контура. Добавляйте точки маршрута только внутри контура."));
            if (mMap && mMap->geoView())
                mMap->geoView()->cleanState();
            return;
        }
    }

    auto* item = new QGVIcon();
    item->setGeometry(pos);
    item->loadImage(mWaypointIcon);
    mRouteLayer->addItem(item);

    mRoutePoints.append(pos);
}

void GeoViewWidget::drawRoute(QGVLayer* layer,
                              const QVector<QGV::GeoPos>& pts,
                              const QPen& pen,
                              bool drawArrow,
                              bool replaceExisting)
{
    mDrawing.drawRoute(layer, mMap, pts, pen, drawArrow, replaceExisting);
}

void GeoViewWidget::toggleManualRouteMode()
{
    if (!mManualRouteMode)
        return;

    mManualRouteMode = false;
    if (mToolbar)
        mToolbar->setManualRouteMode(false);

    if (mRoutePoints.size() >= 2) {
        mRouteIsParallel = false;
        if (mManualRouteShortest)
            mRoutePoints = GeoViewRouteLogic::reorderPointsForShortestRoute(mRoutePoints);

        mRouteLayer->deleteItems();
        for (int i = 1; i < mRoutePoints.size(); ++i) {
            auto* iconItem = new QGVIcon();
            iconItem->setGeometry(mRoutePoints[i]);
            iconItem->loadImage(mWaypointIcon);
            mRouteLayer->addItem(iconItem);
        }
        drawRoute(mRouteLayer, mRoutePoints, mRouteColor, true, true);

        QString finishMsg = mManualRouteShortest
            ? tr("Создано %1 точек, маршрут построен по кратчайшему пути.").arg(mRoutePoints.size())
            : tr("Создано %1 точек, маршрут построен в порядке указания.").arg(mRoutePoints.size());
        QMessageBox::information(this, tr("Готово"), finishMsg);
    }
}

void GeoViewWidget::startManualRouteMode(bool shortest)
{
    if (!mRobotItem.item) {
        QMessageBox::warning(this,
                             tr("Маршрут вручную недоступен"),
                             tr("Задайте позицию робота, чтобы указать маршрут вручную."));
        return;
    }

    mManualRouteShortest = shortest;
    mManualRouteMode = true;
    mRoutePoints.clear();
    mRoutePoints.prepend(mRobotItem.pos);

    if (!mRouteLayer) {
        mRouteLayer = new QGVLayer();
        mMap->addItem(mRouteLayer);
    }
    mRouteLayer->deleteItems();

    if (mToolbar)
        mToolbar->setManualRouteMode(true);
}

void GeoViewWidget::addRobotOnMapFromMousePress(QPointF projPos)
{
    if (!mWaitingForRobotPos)
        return;

    auto geo = mMap->getProjection()->projToGeo(projPos);

    double latitude = geo.latitude();
    double longitude = geo.longitude();

    addRobot(latitude, longitude, 0);

    mWaitingForRobotPos = false;
}

void GeoViewWidget::addRobotOnMapFromJson(const QJsonObject& obj)
{
    double latitude       = obj.value("latitude").toDouble();
    double longitude      = obj.value("longitude").toDouble();
    double rotation_angle = obj.value("rotation_angle").toDouble();

    if (latitude == 0 && longitude == 0) {
        qWarning() << "Invalid coordinates in JSON";
        return;
    }

    addRobot(latitude, longitude, rotation_angle);

    mObservationLayer->addPoint(obj);

    const double delta = 0.002;
    QGV::GeoRect bounds(QGV::GeoPos(latitude - delta, longitude - delta),
                        QGV::GeoPos(latitude + delta, longitude + delta));
    mMap->cameraTo(QGVCameraActions(mMap).scaleTo(bounds));
}

void GeoViewWidget::addRobot(double latitude, double longitude, double angle)
{
    if (mRobotItem.item){
        clearRobotLayer();
        clearRobotRouteLayer();
        clearObservationLayer();
        mRoutePoints.pop_front();
    }

    auto geoPoint = QGV::GeoPos(latitude, longitude);
    auto projPos = mMap->getProjection()->geoToProj(geoPoint);

    QPixmap iconPixmap = QPixmap::fromImage(mRobotIcon);
    QPixmap rotated = iconPixmap.transformed(
            QTransform().rotate(angle),
            Qt::SmoothTransformation
            );

    auto* item = new QGVIcon();
    item->setGeometry(projPos);
    item->loadImage(rotated.toImage());
    item->setZValue(10.0);

    mRobotLayer->addItem(item);

    mRobotItem.item = item;
    mRobotItem.pos = QGV::GeoPos(latitude, longitude);
    mRobotItem.angle = angle;

    mRoutePoints.prepend(mRobotItem.pos);

    mRobotRoutePoints.prepend(mRobotItem.pos);

    drawRoute(mRouteLayer, mRoutePoints, mRouteColor, true);

    mObservationLayer->addPoint(mRobotItem.pos);

    refreshRouteAfterRobotChange();
    updateInfoList();
}

void GeoViewWidget::updateRobotOnMapFromMousePress(QPointF projPos)
{
    if (!mRobotItem.item)
        return;

    auto geo = mMap->getProjection()->projToGeo(projPos);

    updateRobot(geo.latitude(), geo.longitude(), mRobotItem.angle);
}

void GeoViewWidget::updateRobotOnMapFromJson(const QJsonObject& obj)
{
    double latitude       = obj.value("latitude").toDouble();
    double longitude      = obj.value("longitude").toDouble();
    double rotation_angle = obj.value("rotation_angle").toDouble();

    if (latitude == 0 && longitude == 0) {
        qWarning() << "Invalid coordinates in JSON";
        return;
    }

    updateRobot(latitude, longitude, rotation_angle);

    mObservationLayer->addPoint(obj);
}

void GeoViewWidget::updateRobot(double latitude, double longitude, double angle)
{
    if (!mRobotItem.item) {
        addRobot(latitude, longitude, angle);
        return;
    }

    mRobotLayer->removeItem(mRobotItem.item);

    auto geoPoint = QGV::GeoPos(latitude, longitude);
    auto projPos = mMap->getProjection()->geoToProj(geoPoint);

    QPixmap iconPixmap = QPixmap::fromImage(mRobotIcon);
    QPixmap rotated = iconPixmap.transformed(
            QTransform().rotate(angle),
            Qt::SmoothTransformation
            );

    auto* item = new QGVIcon();
    item->setGeometry(projPos);
    item->loadImage(rotated.toImage());
    item->setZValue(10.0);

    mRobotLayer->addItem(item);

    mRobotItem.item = item;
    mRobotItem.pos = QGV::GeoPos(latitude, longitude);
    mRobotItem.angle = angle;

    mRobotRoutePoints.append(mRobotItem.pos);

    drawRoute(mRobotRouteLayer, mRobotRoutePoints, mRobotRouteColor, true);

    refreshRouteAfterRobotChange();
    updateInfoList();
}

void GeoViewWidget::createRoute()
{
    clearRouteCommands();

    if (mRoutePoints.isEmpty() || mRobotItem.item == nullptr || mRobotRoutePoints.size() > 1) {
        QMessageBox::warning(this,
                             tr("Невозможно построить команды"),
                             tr("Для построения команд для маршрута необходимо:\n"
                                "- указать начальную позицию робота\n"
                                "- задать маршрут\n"
                                "- робот не должен выполнять команды"));
        return;
    }

    QJsonArray commands = GeoViewRouteLogic::buildRouteCommands(
        mRoutePoints, mRobotItem.pos, mRobotItem.angle);

    mRouteCommands = new QJsonDocument(commands);
    emit routeBuilt(*mRouteCommands);
    qDebug() << "Маршрут построен." << commands.size();
}

void GeoViewWidget::showRobotCommandsJson()
{
    if (!mInfoList || !mRouteCommands) return;

    mInfoList->clear();

    QString jsonString = mRouteCommands->toJson(QJsonDocument::Indented);

    mInfoList->addItem(jsonString);
}

void GeoViewWidget::showRobotCommandsAsPoints()
{
    if (!mInfoList) return;

    mInfoList->clear();

    for (const auto& p : std::as_const(mRoutePoints)) {
        mInfoList->addItem(QString("%1, %2")
                               .arg(p.latitude(), 0, 'f', 8)
                               .arg(p.longitude(), 0, 'f', 8));
    }
}

QJsonDocument GeoViewWidget::getRouteCommands() const
{
    if (mRouteCommands)
        return *mRouteCommands;
    return QJsonDocument();
}

inline void GeoViewWidget::addObservation(const QJsonObject& RobotData)
{
    if(!mObservationLayer) return;

    mObservationLayer->addPoint(RobotData);
}

void GeoViewWidget::addMlResults(const QJsonObject& mlResults)
{
    if (!mObservationLayer) return;

    if (!mlResults.contains("latitude") || !mlResults.contains("longitude"))
        return;

    double lat = mlResults["latitude"].toDouble();
    double lon = mlResults["longitude"].toDouble();

    QGV::GeoPos jsonPos(lat, lon);

    ObservationPoint* targetPoint = nullptr;

    for (auto* p : mObservationLayer->points())
    {
        if (!p) continue;

        QGV::GeoPos pos = p->pos();

        constexpr double eps = 1e-7;
        if (std::abs(pos.latitude() - jsonPos.latitude()) < eps &&
            std::abs(pos.longitude() - jsonPos.longitude()) < eps)
        {
            targetPoint = p;
            break;
        }
    }

    if (!targetPoint)
    {
        qDebug() << "ML results: point not found for" << lat << lon;
        return;
    }

    targetPoint->setMLResults(mlResults);
}

void GeoViewWidget::updateRobotPos(const QJsonObject& json){

    double latitude       = json.value("latitude").toDouble();
    double longitude      = json.value("longitude").toDouble();
    double rotation_angle = json.value("rotation_angle").toDouble();

    if (latitude == 0 && longitude == 0) {
        qWarning() << "Invalid coordinates in JSON";
        return;
    }

    addRobot(latitude, longitude, rotation_angle);

    mObservationLayer->addPoint(QGV::GeoPos(latitude, longitude));
}

void GeoViewWidget::setMlResults(const QJsonObject& json){
    addMlResults(json);
}

QJsonDocument GeoViewWidget::generateGazeboJson() {
    if (!mRobotItem.item || !mRouteCommands) {
        QMessageBox::warning(this, "Недостаточно данных",
                             "Для построения JSON необходимо:\n"
                             "- начальное положение робота\n"
                             "- построенный маршрут (кнопка «Построить команды»)");
        return QJsonDocument();
    }

    if (!mRouteCommands->isArray() || mRouteCommands->array().isEmpty()) {
        QMessageBox::warning(this, "Недостаточно данных", "Маршрут пуст. Постройте команды маршрута.");
        return QJsonDocument();
    }

    QGV::GeoPos startPos = mRobotItem.pos;
    QJsonObject rootObj;

    if (mContour && mContour->points().size() >= 2) {
        QJsonArray contourPointsArray;
        const auto& cp = mContour->points();
        for (int i = 0; i < cp.size() - 1; ++i) {
            QPointF gazeboPoint = GeoViewRouteLogic::computeGazeboPoint(startPos, cp[i]);
            QJsonObject pointObj;
            pointObj["x"] = gazeboPoint.y();
            pointObj["y"] = -gazeboPoint.x();
            pointObj["z"] = 0.0;
            contourPointsArray.append(pointObj);
        }
        QJsonObject contourObj;
        contourObj["points"] = contourPointsArray;
        rootObj["contour"] = contourObj;
    }

    rootObj["commands"] = mRouteCommands->array();
    return QJsonDocument(rootObj);
}


void GeoViewWidget::clearRouteLayer()
{
    mDrawing.clearRouteLayer(mRouteLayer, mRoutePoints, mRouteCommands);
    mRouteIsParallel = false;
}

void GeoViewWidget::buildParallelRouteFromPath(const QVector<QGV::GeoPos>& path)
{
    clearRouteLayer();

    QPen thinPen(mRouteColor.color(), 1);
    thinPen.setStyle(mRouteColor.style());

    QVector<QGV::GeoPos> robotToStart;
    robotToStart.push_back(mRobotItem.pos);
    robotToStart.push_back(path.front());
    auto* robotLine = new GeoPolyline(mMap);
    QPen robotLinePen = thinPen;
    robotLinePen.setStyle(Qt::DashLine);
    robotLine->setPen(robotLinePen);
    robotLine->points = robotToStart;
    mRouteLayer->addItem(robotLine);

    auto* poly = new GeoPolyline(mMap);
    poly->setPen(thinPen);
    poly->points = path;
    mRouteLayer->addItem(poly);

    mRoutePoints = path;
    mRouteIsParallel = true;
}

void GeoViewWidget::refreshRouteAfterRobotChange()
{
    if (!mRobotItem.item || mRoutePoints.isEmpty())
        return;

    if (mRouteIsParallel) {
        if (mContour && mContour->points().size() >= 2)
            generateParallelRoute(4);
        return;
    }

    mRoutePoints[0] = mRobotItem.pos;
    if (mManualRouteShortest)
        mRoutePoints = GeoViewRouteLogic::reorderPointsForShortestRoute(mRoutePoints);

    if (mRouteLayer) {
        mRouteLayer->deleteItems();
        for (int i = 1; i < mRoutePoints.size(); ++i) {
            auto* iconItem = new QGVIcon();
            iconItem->setGeometry(mRoutePoints[i]);
            iconItem->loadImage(mWaypointIcon);
            mRouteLayer->addItem(iconItem);
        }
        drawRoute(mRouteLayer, mRoutePoints, mRouteColor, true, true);
    }

    clearRobotRouteLayer();
    mRobotRoutePoints.append(mRobotItem.pos);
    createRoute();
    showRobotCommandsJson();
}

void GeoViewWidget::clearRobotLayer()
{
    mDrawing.clearRobotLayer(mRobotLayer, mRobotItem.item, mRobotItem.pos, mRobotItem.angle);
}

void GeoViewWidget::clearRobotRouteLayer()
{
    mDrawing.clearRobotRouteLayer(mRobotRouteLayer, mRobotRoutePoints);
}

void GeoViewWidget::clearAll()
{
    clearMainLayer();
    clearRouteLayer();
    clearRobotLayer();
    clearRobotRouteLayer();
    clearRouteCommands();
    clearObservationLayer();

    if(mContour) mContour->clear();

    if(mInfoList) mInfoList->clear();

    if(mPrevDialog)
    {
        delete mPrevDialog;
        mPrevDialog = nullptr;
    };

    updateInfoList();

    qDebug() << "Все данные очищены.";
}
