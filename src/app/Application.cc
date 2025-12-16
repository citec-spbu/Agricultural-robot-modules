#include "Application.h"

Application::Application(int argc, char *argv[]) :
                                                   app(argc, argv),
                                                   database(),
                                                   manager(&database),
                                                   robot_service()
{
    map = new GeoViewWidget();
    widget = new Widget(map);


    m_sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    log("Session: " + m_sessionId);

    widget->resize(900, 600);
    widget->show();

    initConnections();
}

Application::~Application() {}

void Application::log(const QString &msg)
{
    qDebug() << msg;
}

int Application::exec() {
    return app.exec();
}

void Application::initConnections() {
    QObject::connect(widget, &Widget::ConnectRobotClicked, this, [this](const QString& robotHost, ushort robotPort) {
        QUrl url;
        url.setScheme("http");
        url.setHost(robotHost);
        url.setPort(robotPort);

        log("Robot interface: " + robotHost + ":" + QString::number(robotPort));

        robot_service.open(url);
    });

    QObject::connect(widget, &Widget::FetchSpecClicked, this, [this]() {
        log("FetchSpecClicked");
        QNetworkReply* reply = robot_service.getSpecification();

        QObject::connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() != QNetworkReply::NoError) {
                log(reply->errorString());
                reply->deleteLater();
                return;
            }

            QByteArray data = reply->readAll();

            log("Robot service received specification");
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(data, &err);
            if (err.error == QJsonParseError::NoError) {
                QString str = doc.toJson(QJsonDocument::Indented);
                widget->showRobotSpec(str);
            } else {
                log(err.errorString());
                widget->showRobotSpec(QString(data));
            }

            reply->deleteLater();
        });
    });

    QObject::connect(widget, &Widget::ConnectMLClicked, this, [this](const QString& MLHost, ushort MLPort) {
        QUrl url;
        url.setScheme("http");
        url.setHost(MLHost);
        url.setPort(MLPort);

        log("ML interface: " + MLHost + ":" + QString::number(MLPort));

        ml_service.open(url);
    });

    QObject::connect(widget, &Widget::StartClicked, this, [this](){
        sendNextCommand();
    });

    QObject::connect(widget, &Widget::StopClicked, this, [this](){
        readyToStart = false;
        robot_service.postStop();
        mState = State::Idle;
        ExecutionTimer.stop();
        DelayTimer.stop();
        CollectTimer.stop();
    });

    QObject::connect(&manager, &Manager::sendCommand, this, [this](const QString& type, const QJsonObject& json) {
        if (type == "robot") {
            // robot_service.send(json);
        } else if (type == "ml") {
            // ml_service.send(json);
        } else {
            qDebug() << "Unknown type";
        }
    });

    QObject::connect(&robot_service, &RobotService::connected, this, [this]() {
        log("Successful connection to the robot service.");

        QNetworkReply* reply = robot_service.getCollect();

        QObject::connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() != QNetworkReply::NoError) {
                log(reply->errorString());
                reply->deleteLater();
                return;
            }

            QByteArray data = reply->readAll();

            log("Robot service received collect");
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(data, &err);
            if (err.error == QJsonParseError::NoError) {
                QJsonObject obj = doc.object();

                QString str = doc.toJson(QJsonDocument::Indented);
                //log(str);

                map->initRobotPos(obj);
            } else {
                log(err.errorString());
            }

            reply->deleteLater();
        });
    });

    QObject::connect(&ml_service, &MLService::connected, this, [this]() {
        log("Successful connection to the ML service.");
    });

    QObject::connect(map, &GeoViewWidget::routeBuilt, this, [this](const QJsonDocument& jsonDoc) {
        log("map: routeBuilt");

        if (!jsonDoc.isArray()) {
            log("Route is not array");
            return;
        }

        CommandQueue = jsonDoc.array();
        CommandIndex = 0;
        mState = State::Idle;

        readyToStart = true;
    });

}

void Application::sendNextCommand() {
    if (CommandIndex >= CommandQueue.size()) {
        log("All commands finished");
        return;
    }

    QJsonObject cmd = CommandQueue[CommandIndex].toObject();
    robot_service.sendCommand(cmd);

    QString str = QJsonDocument(cmd).toJson(QJsonDocument::Indented);
    log("Send command:\n" + str);

    startExecutionTimer(cmd);

    mState = State::WaitingExecution;
}

void Application::startExecutionTimer(const QJsonObject& cmd)
{
    int execMs = 0;

    QString type = cmd["cmd"].toString();
    QJsonObject data = cmd["data"].toObject();

    if (type == "move") {
        double dist = data["distance_m"].toDouble();
        execMs = static_cast<int>((dist / 1.0) * 1000);
    }
    else if (type == "rotate") {
        execMs = 200;
    }

    if (execMs < 0) {
        execMs = 0;
    }

    ExecutionTimer.setSingleShot(true);
    connect(&ExecutionTimer, &QTimer::timeout,this,
            &Application::onCommandExecuted,Qt::UniqueConnection);

    ExecutionTimer.start(execMs);

    connect(&CollectTimer, &QTimer::timeout,
            this, &Application::collectRobotState,
            Qt::UniqueConnection);

    CollectTimer.start(1000);
}

void Application::onCommandExecuted()
{
    CollectTimer.stop();

    QNetworkReply* reply = robot_service.getCollect();

    connect(reply, &QNetworkReply::finished,
            this, [this, reply]() {

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);

            if (doc.isObject()) {
                map->updateRobotOnMapFromJson(doc.object());
            }
        }

        reply->deleteLater();

        startDelay();
    });
}

void Application::startDelay()
{
    DelayTimer.setSingleShot(true);

    connect(&DelayTimer, &QTimer::timeout,
            this, &Application::onDelayFinished,
            Qt::UniqueConnection);

    DelayTimer.start(2000);
}

void Application::onDelayFinished()
{
    ++CommandIndex;
    sendNextCommand();
}

void Application::collectRobotState()
{
    QNetworkReply* reply = robot_service.getCollect();

    connect(reply, &QNetworkReply::finished,
            this, [this, reply]() {

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();

            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(data, &err);

            QJsonObject obj = doc.object();
            QString latitude = QString::number(obj.value("latitude").toDouble(), 'f', 5);
            QString longitude = QString::number(obj.value("longitude").toDouble(), 'f', 5);
            QString rotation_angle = QString::number(obj.value("rotation_angle").toDouble(), 'f', 5);
            log("GET collect: Robot service reply coords: " + latitude + ", " + longitude + ", " + rotation_angle);


            if (err.error == QJsonParseError::NoError && doc.isObject()) {

                map->updateRobotOnMapFromJson(doc.object());

                QNetworkReply* r = ml_service.postDetect(doc.object());
                connect(r, &QNetworkReply::finished, [r, this](){
                    QByteArray data = r->readAll();

                    //log("ML_Service: QByteArray:  " + data);

                    QJsonParseError err;
                    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
                    if (err.error == QJsonParseError::NoError) {

                        //QString str = doc.toJson(QJsonDocument::Indented);
                        //log("STRRRR: " + str);

                        map->setMlResults(doc.object());
                    } else {
                        log("ML_Service: parse json error:  " + err.errorString());
                    }
                });

            }
        }

        reply->deleteLater();
    });
}