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

#include "GeoViewWidget.h"
#include "TestJson.h"

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
#include <QMessageBox>
#include <QEvent>

#include <helpers.h>
#include <rectangle.h>

#include <QGeoView/QGVLayerOSM.h>
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

    QWidget* widget = new QWidget();
    QHBoxLayout* widgetLayout = new QHBoxLayout(widget);
    widgetLayout->addWidget(createOptionsList());
    widgetLayout->addWidget(createInfoList());
    layout->addWidget(widget);

    QTimer::singleShot(100, this, [this]() {
        mMap->cameraTo(QGVCameraActions(mMap).scaleTo(targetArea()));
    });

           // Загрузка изображений
    preloadImages();

           // Подключение к событию клика на карту
    connect(mMap, &QGVMap::mapMousePress, this, [this](QPointF projPos) {
        QGV::GeoPos geoPos = mMap->getProjection()->projToGeo(projPos);
        mObservationLayer->handleMapClick(geoPos);

        if (!mManualRouteMode) return;

        handleMapClick(geoPos);
    });
}


GeoViewWidget::~GeoViewWidget()
{
}

QGV::GeoRect GeoViewWidget::targetArea() const
{
    return QGV::GeoRect(QGV::GeoPos(50, 14), QGV::GeoPos(52, 15));
}

QGroupBox* GeoViewWidget::createOptionsList()
{
    QGroupBox* groupBox = new QGroupBox(tr("Управление"));
    groupBox->setLayout(new QVBoxLayout);

    {
        QPushButton* button = new QPushButton("Добавить контур (GeoJson)");
        groupBox->layout()->addWidget(button);

        connect(button, &QPushButton::clicked, this, &GeoViewWidget::addContour);
    }

    {
        QPushButton* button = new QPushButton("Указать маршрут вручную");
        groupBox->layout()->addWidget(button);

        connect(button, &QPushButton::clicked, this, [this, button](){
            toggleManualRouteMode();
            button->setText(mManualRouteMode ? "Завершить построение маршрута" : "Указать маршрут вручную");
        });
    }

    {
        QPushButton* button = new QPushButton("Построить команды");
        groupBox->layout()->addWidget(button);

        connect(button, &QPushButton::clicked, this, [this]() {
            createRoute();
            showRobotCommandsJson();

            qDebug() << "Маршрут и команды обновлены.";
        });
    }

    {
        QPushButton* button = new QPushButton("Удалить контур");
        groupBox->layout()->addWidget(button);

        connect(button, &QPushButton::clicked, this, &GeoViewWidget::removeContour);
    }

    {
        QPushButton* button = new QPushButton("Очистить все");
        groupBox->layout()->addWidget(button);

        connect(button, &QPushButton::clicked, this, &GeoViewWidget::clearAll);
    }

    {
        QPushButton* button = new QPushButton("Задать начальную позицию робота");
        groupBox->layout()->addWidget(button);

        connect(button, &QPushButton::clicked, this, [this]() {
            addRobotOnMapFromJson(TEST_JSON);

            QTimer::singleShot(100, this, [this]() {
                if (!mRobotItem.item) return;

                mMap->cameraTo(QGVCameraActions(mMap).moveTo(mRobotItem.pos));

                QTimer::singleShot(50, this, [this]() {
                    auto zoomWidget = mMap->findChild<QGVWidgetZoom*>();
                    if (zoomWidget) {
                        zoomWidget->minus();
                    }
                });
            });
        });
    }

    {
        QPushButton* button = new QPushButton("Следующая позиция робота");
        groupBox->layout()->addWidget(button);

        connect(button, &QPushButton::clicked, this, [this](){
            updateRobotOnMapFromJson(ML_JSON);
        });
    }

    {
        QPushButton* button = new QPushButton("Старт");
        groupBox->layout()->addWidget(button);

        connect(button, &QPushButton::clicked, this, [this]() {
            startProcessingPoints(without_img);

            qDebug() << "Маршрут и команды обновлены.";
        });
    }

    return groupBox;
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
    QGroupBox* groupBox = new QGroupBox(tr("Info"));
    groupBox->setLayout(new QVBoxLayout);

    mInfoList = new QListWidget();
    mInfoList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    mInfoList->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mInfoList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    groupBox->layout()->addWidget(mInfoList);
    updateInfoList();

    return groupBox;
}

void GeoViewWidget::preloadImages()
{
    loadImage(mRobotIcon, QUrl{ "https://earth.google.com/images/kml-icons/track-directional/track-0.png" });
}

void GeoViewWidget::loadImage(QImage& dest, QUrl url)
{
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent",
                         "Mozilla/5.0 (Windows; U; MSIE "
                         "6.0; Windows NT 5.1; SV1; .NET "
                         "CLR 2.0.50727)");
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);

    QNetworkReply* reply = QGV::getNetworkManager()->get(request);
    connect(reply, &QNetworkReply::finished, reply, [reply, &dest]() {
        if (reply->error() != QNetworkReply::NoError) {
            qgvCritical() << "ERROR" << reply->errorString();
            reply->deleteLater();
            return;
        }
        dest.loadFromData(reply->readAll());
        reply->deleteLater();
    });

    qgvDebug() << "request" << url;
}

void GeoViewWidget::addContour()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Выберите файл GeoJSON", "", "*.geojson");
    if (fileName.isEmpty())
        return;
    else if(mContour)
        mContour->clear();


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

        mContour = new Contour(mMap);

        const auto& a = points.first();
        const auto& b = points.last();

        double eps = 1e-7;
        if (std::abs(a.latitude() - b.latitude()) > eps || std::abs(a.longitude() - b.longitude()) > eps)
            points.push_back(a);

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

        if(mRobotItem.item)
            generateParallelRoute(4);
    }
}

QVector<QGV::GeoPos> GeoViewWidget::buildRouteWithAngle(double stepMeters,
                                                     double angleDegrees,
                                                     double offsetFromContour,
                                                     double offsetCut
                                                     )
{
    if (mContour->points().size() < 2) return {};

    QVector<QPointF> projPoints;
    for (auto& p : mContour->points())
        projPoints.push_back(mMap->getProjection()->geoToProj(p));

    QVector<QPointF>& workContour = projPoints;

    double angleRad = angleDegrees * M_PI / 180.0;
    QPointF dir(std::cos(angleRad), std::sin(angleRad));
    QPointF normal(-dir.y(), dir.x());

    double minProj = std::numeric_limits<double>::max();
    double maxProj = -std::numeric_limits<double>::max();
    for (auto& pt : workContour)
    {
        double proj = pt.x()*normal.x() + pt.y()*normal.y();
        minProj = std::min(minProj, proj);
        maxProj = std::max(maxProj, proj);
    }


    QVector<QVector<QGV::GeoPos>> finalLines;

           // Смещаем старт цикла на offsetFromContour от minProj
           // При этом не выходим за maxProj
    double startP = minProj + offsetFromContour;
    if (startP > maxProj)
        return {};  // ничего строить, если сдвиг больше максимума

    for (double p = startP; p <= maxProj; p += stepMeters)
    {
        QVector<QPointF> intersections;
        for (int i = 0; i < workContour.size() - 1; ++i)
        {
            QPointF a = workContour[i];
            QPointF b = workContour[i+1];
            double da = a.x()*normal.x() + a.y()*normal.y() - p;
            double db = b.x()*normal.x() + b.y()*normal.y() - p;

            if (da*db <= 0 && std::abs(da-db) > 1e-6)
            {
                double t = da / (da - db);
                QPointF pt = a + (b - a)*t;
                intersections.push_back(pt);
            }
        }

        if (intersections.size() < 2) continue;

        std::sort(intersections.begin(), intersections.end(),
                  [&](const QPointF &x, const QPointF &y){
                      double px = x.x()*dir.x() + x.y()*dir.y();
                      double py = y.x()*dir.x() + y.y()*dir.y();
                      return px < py;
                  });

        QPointF A = intersections.front();
        QPointF B = intersections.back();

        QPointF v = B - A;
        double L = std::hypot(v.x(), v.y());
        if (L < offsetCut*2) continue;
        QPointF tdir(v.x()/L, v.y()/L);
        QPointF A_cut = A + tdir*offsetCut;
        QPointF B_cut = B - tdir*offsetCut;

        QVector<QGV::GeoPos> line;
        line.push_back(mMap->getProjection()->projToGeo(A_cut));
        line.push_back(mMap->getProjection()->projToGeo(B_cut));
        finalLines.push_back(line);
    }

    QVector<QGV::GeoPos> snakePath;

    for (int i = 0; i < finalLines.size(); ++i)
    {
        QVector<QGV::GeoPos> L = finalLines[i];
        if (i % 2 == 1) std::reverse(L.begin(), L.end());
        if (!snakePath.isEmpty() && snakePath.last() != L.first())
            snakePath.push_back(L.first());
        snakePath += L;
    }

    return snakePath;
}


// Основная функция с подбором угла
void GeoViewWidget::generateParallelRoute(double stepMeters)
{
    if (mContour->points().size() < 2 || !mRouteLayer) return;

    // Перебираем углы с шагом 10°
    double bestAngle = 0;
    double minDist = std::numeric_limits<double>::max();
    QVector<QGV::GeoPos> bestPath;

    for (double angle = 0; angle < 180; angle += 10)
    {
        QVector<QGV::GeoPos> path = buildRouteWithAngle(stepMeters, angle);

        if (path.isEmpty())
            continue;

        // Проверяем расстояние от робота до начала пути
        double dxStart = mRobotItem.pos.longitude() - path.front().longitude();
        double dyStart = mRobotItem.pos.latitude() - path.front().latitude();
        double distStart = dxStart*dxStart + dyStart*dyStart;

        // Проверяем расстояние от робота до конца пути (если ближе, перевернем)
        double dxEnd = mRobotItem.pos.longitude() - path.back().longitude();
        double dyEnd = mRobotItem.pos.latitude() - path.back().latitude();
        double distEnd = dxEnd*dxEnd + dyEnd*dyEnd;

        double dist = std::min(distStart, distEnd);

        if (dist < minDist)
        {
            minDist = dist;
            bestAngle = angle;
            bestPath = path;

            // Если ближе к концу, перевернем путь
            if (distEnd < distStart)
                std::reverse(bestPath.begin(), bestPath.end());
        }
    }

    if (bestPath.isEmpty())
        return;

    // Очищаем слой
    clearRouteLayer();

    // Линия от робота до начала маршрута
    QVector<QGV::GeoPos> robotToStart;
    robotToStart.push_back(mRobotItem.pos);
    robotToStart.push_back(bestPath.front());
    auto* robotLine = new GeoPolyline(mMap);
    QPen RobotLineQPen = mRouteColor;
    RobotLineQPen.setStyle(Qt::DashLine);
    robotLine->setPen(RobotLineQPen);
    robotLine->points = robotToStart;
    mRouteLayer->addItem(robotLine);

    // Основной маршрут
    auto* poly = new GeoPolyline(mMap);
    poly->setPen(mRouteColor);
    poly->points = bestPath;
    mRouteLayer->addItem(poly);

    mRoutePoints = bestPath;
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
    auto* item = new QGVIcon();
    item->setGeometry(pos);
    item->loadImage(mRobotIcon);
    mRouteLayer->addItem(item);

    mRoutePoints.append(pos);

    drawRoute(mRouteLayer, mRoutePoints, mRouteColor, true);
}

void GeoViewWidget::drawRoute(
        QGVLayer* layer,
        const QVector<QGV::GeoPos>& pts,
        const QPen& pen,
        bool drawArrow,
        bool replaceExisting)
{
    if (!mRouteLayer)
        return;

    // TODO: пофиксить достроение пути (сейчас для каждой новой точки путь перерисовывается)
    if (!replaceExisting)
        layer->deleteItems();

    if (pts.size() < 2)
        return;

    auto* poly = new GeoPolyline(mMap);
    poly->points = pts;
    poly->setPen(pen);
    poly->drawArrowOnEnd = drawArrow;

    layer->addItem(poly);
}

void GeoViewWidget::toggleManualRouteMode()
{
    mManualRouteMode = !mManualRouteMode;

    if (mManualRouteMode)
    {

        mRoutePoints.clear();

        if(mRobotItem.item)
            mRoutePoints.prepend(mRobotItem.pos);

        if (!mRouteLayer) {
            mRouteLayer = new QGVLayer();
            mMap->addItem(mRouteLayer);
        }

        mRouteLayer->deleteItems();

        QMessageBox::information(this,
                                 "Режим задания траектории",
                                 "Кликните по карте, чтобы добавить точки маршрута.\n"
                                 "Кликните ещё раз по кнопке — чтобы завершить.");
    }
    else
    {
        QMessageBox::information(this,
                                 "Готово",
                                 QString("Создано %1 точек").arg(mRoutePoints.size()));
    }
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
}

void GeoViewWidget::addRobot(double latitude, double longitude, double angle)
{
    if (mRobotItem.item){
        clearRobotLayer();
        clearRobotRouteLayer();
        clearObservationLayer();
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

    mRobotLayer->addItem(item);

    mRobotItem.item = item;
    mRobotItem.pos = QGV::GeoPos(latitude, longitude);
    mRobotItem.angle = angle;

    mRoutePoints.prepend(mRobotItem.pos);

    mRobotRoutePoints.prepend(mRobotItem.pos);

    drawRoute(mRouteLayer, mRoutePoints, mRouteColor, true);

    mObservationLayer->addPoint(mRobotItem.pos);

    if(mContour && !mContour->points().empty())
        generateParallelRoute(4);

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

    mRobotLayer->addItem(item);

    mRobotItem.item = item;
    mRobotItem.pos = QGV::GeoPos(latitude, longitude);
    mRobotItem.angle = angle;

    mRobotRoutePoints.append(mRobotItem.pos);

    drawRoute(mRobotRouteLayer, mRobotRoutePoints, mRobotRouteColor, true);

    updateInfoList();
}


void GeoViewWidget::createRoute()
{
    clearRouteCommands();

    if (mRoutePoints.isEmpty() || mRobotItem.item == nullptr || mRobotRoutePoints.size() > 1) {
        QMessageBox::warning(
                this,
                tr("Невозможно построить команды"),
                tr("Для построения команд для маршрута необходимо:\n"
                   "- указать начальную позицию робота\n"
                   "- задать маршрут\n"
                   "- робот не должен выполнять команды")
                );
        return;
    }

    QJsonArray commands;

    QGV::GeoPos prevPos = mRobotItem.pos;
    double prevAngle = mRobotItem.angle;  // 0° = север

    auto normalizeAngle = [](double a) {
        while (a > 180) a -= 360;
        while (a < -180) a += 360;
        return a;
    };

    constexpr double R = 6371000.0; // радиус Земли в метрах

    for (const auto& nextPos : std::as_const(mRoutePoints))
    {
        double lat1 = qDegreesToRadians(prevPos.latitude());
        double lon1 = qDegreesToRadians(prevPos.longitude());
        double lat2 = qDegreesToRadians(nextPos.latitude());
        double lon2 = qDegreesToRadians(nextPos.longitude());

        double dLat = lat2 - lat1;
        double dLon = lon2 - lon1;

        // Haversine расстояние
        double a = qSin(dLat/2)*qSin(dLat/2) + qCos(lat1)*qCos(lat2)*qSin(dLon/2)*qSin(dLon/2);
        double c = 2 * qAtan2(qSqrt(a), qSqrt(1-a));
        double distance = R * c;

        // Игнорируем очень короткие сегменты
        if (distance < 0.2) { // меньше 20 см
            prevPos = nextPos;
            continue;
        }

        // Географический азимут (0° = север)
        double x = qSin(dLon) * qCos(lat2);
        double y = qCos(lat1)*qSin(lat2) - qSin(lat1)*qCos(lat2)*qCos(dLon);
        double azimuth = qRadiansToDegrees(qAtan2(x, y));  // -180...+180
        if (azimuth < 0) azimuth += 360.0;  // 0..360°

        // Дельта-угол
        double deltaAngle = normalizeAngle(azimuth - prevAngle);

        // Если скачок ±180° на очень коротком сегменте — игнорируем
        if (qAbs(deltaAngle) > 150 && distance < 2.0)
            deltaAngle = 0;

        // Команда поворота
        if (qFabs(deltaAngle) > 1e-3)
        {
            QJsonObject rotateCmd;
            rotateCmd["cmd"] = "rotate";
            QJsonObject data;
            data["delta_angle"] = deltaAngle;
            rotateCmd["data"] = data;
            commands.append(rotateCmd);

            prevAngle = normalizeAngle(prevAngle + deltaAngle); // безопасное обновление
        }
        else
        {
            prevAngle = azimuth;
        }

        // Команда движения
        QJsonObject moveCmd;
        moveCmd["cmd"] = "move";
        QJsonObject data;
        data["distance_m"] = distance;
        moveCmd["data"] = data;
        commands.append(moveCmd);

        prevPos = nextPos;
    }

    if (mRouteCommands)
        delete mRouteCommands;

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

void GeoViewWidget::initRobotPos(const QJsonObject& json){

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

void GeoViewWidget::clearRouteLayer(){
    if(mRouteLayer) mRouteLayer->deleteItems();

    if(!mRoutePoints.empty()) mRoutePoints.clear();

    if (mRouteCommands) {
        delete mRouteCommands;
        mRouteCommands = nullptr;
    }
}

void GeoViewWidget::clearRobotLayer(){
    if(mRobotLayer) mRobotLayer->deleteItems();

    if(mRobotItem.item){
        mRobotItem.item = nullptr;
        mRobotItem.pos = QGV::GeoPos(0, 0);
        mRobotItem.angle = 0.;
    }
}

void GeoViewWidget::clearRobotRouteLayer(){
    if(mRobotRouteLayer) mRobotRouteLayer->deleteItems();

    if(!mRobotRoutePoints.empty()) mRobotRoutePoints.clear();
}

inline void GeoViewWidget::clearRouteCommands()
{
    if(mRouteCommands)
    {
        delete mRouteCommands;
        mRouteCommands = nullptr;
    }
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

    updateInfoList();

    qDebug() << "Все данные очищены.";
}
