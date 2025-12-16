#include "ObservationPoint.h"
#include <QByteArray>
#include <QJsonDocument>
#include <QPainter>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QPixmap>
#include <QTimer>
#include <QScrollArea>

ObservationPoint::ObservationPoint(QGVMap* map, const QGV::GeoPos& pos, const QJsonObject& json, double radiusMeters, QObject* parent)
    : mMap(map), mPos(pos), mRadiusMeters(radiusMeters)
{
    if (!json.isEmpty()) setRobotData(json);
    //setMLResults(json);
}

ObservationPoint::ObservationPoint(const QJsonObject& json, QGVMap* map, double radiusMeters, QObject* parent)
    : mMap(map), mRadiusMeters(radiusMeters)
{
    if (json.contains("latitude") && json.contains("longitude")) {
        mPos.setLat(json.value("latitude").toDouble());
        mPos.setLon(json.value("longitude").toDouble());
    }
    if (!json.isEmpty()) setRobotData(json);
}

void ObservationPoint::setRobotData(const QJsonObject& obj)
{
    mRobotData = obj;
    if (obj.contains("img_base64")) mImage = decodeBase64Image(obj.value("img_base64").toString());
}

void ObservationPoint::setMLResults(const QJsonObject& obj)
{
    mMLResults = obj;

    if (mMap) {
        this->repaint();
    }
}


QColor ObservationPoint::color() const
{
    int alpha = 150; // прозрачность

    if (mRobotData.empty()) return QColor(128, 128, 128, alpha);
    if (mMLResults.empty()) return QColor(238, 130, 238, alpha);
    return QColor(255, 165, 0, alpha);
}

QImage ObservationPoint::decodeBase64Image(const QString& base64) const
{
    QByteArray ba = QByteArray::fromBase64(base64.toUtf8());
    QImage img;
    img.loadFromData(ba);
    return img;
}

void ObservationPoint::onProjection(QGVMap* map)
{
    QGVDrawItem::onProjection(map);
    if (!map) return;
    auto* proj = map->getProjection();
    if (!proj) return;
    m_projPos = proj->geoToProj(mPos);
}

void ObservationPoint::onUpdate()
{
    onProjection(mMap);
}

QPainterPath ObservationPoint::projShape() const
{
    QPainterPath path;
    if (!mMap) return path;

    auto* proj = mMap->getProjection();
    if (!proj) return path;

    QPointF center = proj->geoToProj(mPos);

    QGV::GeoPos offsetPos = mPos;
    double earthRadius = 6378137.0;
    double dLat = (mRadiusMeters / earthRadius) * (180.0 / M_PI);
    offsetPos.setLat(mPos.latitude() + dLat);

    QPointF offsetPoint = proj->geoToProj(offsetPos);
    double pixelRadius = std::abs(offsetPoint.y() - center.y());

    path.addEllipse(center, pixelRadius, pixelRadius);
    return path;
}


void ObservationPoint::projPaint(QPainter* p)
{
    if (!p || !mMap) return;

    auto* proj = mMap->getProjection();
    if (!proj) return;

    QPointF center = proj->geoToProj(mPos);

    QGV::GeoPos offsetPos = mPos;
    double earthRadius = 6378137.0;
    double dLat = (mRadiusMeters / earthRadius) * (180.0 / M_PI);
    offsetPos.setLat(mPos.latitude() + dLat);

    QPointF offsetPoint = proj->geoToProj(offsetPos);

    double pixelRadius = std::abs(offsetPoint.y() - center.y());

    p->setRenderHint(QPainter::Antialiasing, true);
    p->setPen(Qt::NoPen);
    p->setBrush(color());
    p->setOpacity(0.9);
    p->drawEllipse(center, pixelRadius, pixelRadius);
}


void ObservationPoint::showData()
{
    QTimer::singleShot(100, [this]() {
        QDialog dlg;
        dlg.setWindowTitle("Observation Point Data");
        dlg.resize(1000, 650);

        QHBoxLayout* mainLayout = new QHBoxLayout(&dlg);

        QTextEdit* textEdit = new QTextEdit;
        textEdit->setReadOnly(true);
        textEdit->setMinimumWidth(450);


        QJsonObject robotClean = mRobotData;
        QJsonObject mlClean = mMLResults;

        auto removeBase64Fields = [](QJsonObject& obj) {
            QStringList removeKeys;
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                if (it.key().contains("_base64", Qt::CaseInsensitive)) {
                    removeKeys << it.key();
                }
            }
            for (const QString& k : std::as_const(removeKeys))
                obj.remove(k);
        };

        removeBase64Fields(robotClean);
        removeBase64Fields(mlClean);

        QString msg =
                "**Robot Data:**\n" +
                QString::fromUtf8(QJsonDocument(robotClean).toJson(QJsonDocument::Indented)) +
                "\n\n**ML Results:**\n" +
                QString::fromUtf8(QJsonDocument(mlClean).toJson(QJsonDocument::Indented));

        textEdit->setPlainText(msg);
        mainLayout->addWidget(textEdit, 1);

        QScrollArea* scrollArea = new QScrollArea;
        scrollArea->setWidgetResizable(true);
        QWidget* scrollWidget = new QWidget;
        QVBoxLayout* imgLayout = new QVBoxLayout(scrollWidget);

        auto extractImages = [&](const QJsonObject& obj, const QString& sourceName) {
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                QString key = it.key();

                if (!key.contains("_base64", Qt::CaseInsensitive))
                    continue;

                QString base64 = it.value().toString();
                QImage img = decodeBase64Image(base64);
                if (img.isNull())
                    continue;

                QLabel* caption = new QLabel(sourceName + " → " + key);
                caption->setStyleSheet("font-weight: bold; margin-top: 10px;");

                QLabel* imgLabel = new QLabel;
                imgLabel->setPixmap(QPixmap::fromImage(img)
                                            .scaled(450, 450,
                                                    Qt::KeepAspectRatio,
                                                    Qt::SmoothTransformation));

                imgLabel->setAlignment(Qt::AlignCenter);

                imgLayout->addWidget(caption);
                imgLayout->addWidget(imgLabel);
            }
        };

        extractImages(mRobotData, "Robot Data");
        extractImages(mMLResults, "ML Results");

        imgLayout->addStretch();

        scrollArea->setWidget(scrollWidget);
        scrollArea->setMinimumWidth(480);

        mainLayout->addWidget(scrollArea, 1);

        dlg.exec();
    });
}



