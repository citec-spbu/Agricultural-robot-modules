#include "Application.h"

Application::Application(int argc, char *argv[]) :
                                                   app(argc, argv),
                                                   widget(nullptr),
                                                   database(),
                                                   manager(&database),
                                                   robot_service()
{
    m_sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    log("Session: " + m_sessionId);

    widget.resize(900, 600);
    widget.show();

    initWidgetConnections();
    initManagerConnections();
    initMLServiceConnections();
    initRobotServiceConnections();
}

Application::~Application() {}

void Application::log(const QString &msg)
{
    qDebug() << msg;
}

int Application::exec() {
    return app.exec();
}

void Application::initWidgetConnections() {
    QObject::connect(&widget, &Widget::ConnectRobotClicked, this, [this](const QString& robotHost, ushort robotPort){
        QUrl url;
        url.setScheme("http");
        url.setHost(robotHost);
        url.setPort(robotPort);

        log("Robot interface: " + robotHost
            + ":" + QString::number(robotPort));

        robot_service.open(url);
    });

    QObject::connect(&widget, &Widget::FetchSpecClicked, this, [this](){
        robot_service.getSpec();
    });
}

void Application::initManagerConnections() {
//    connect(&manager, Manager::sendCommand(), this, [this](const QString& type, const QJsonObject& json) {
//        if (type == "robot") {
//            //robot_service.send(json);
//        } else if (type == "ml") {
//            //ml_service.send(json);
//        } else {
//            qDebug() << "Unknown type";
//        }
//    });
}

void Application::initMLServiceConnections() {

}

void Application::initRobotServiceConnections() {
    QObject::connect(&robot_service, &RobotService::connected, this, [this](){
       log("Successful connection to the robot service.");
    });

    QObject::connect(&robot_service, &RobotService::received, this, [this](const QByteArray& json){
        widget.showRobotSpec(json);
        log("Robot service received specs.");
    });
}


