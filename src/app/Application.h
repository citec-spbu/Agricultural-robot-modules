#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <QApplication>
#include <QUuid>
#include <QJsonArray>

#include "manager.h"
#include "widget.h"
#include "sqlitedb.h"
#include "RobotService.h"
#include "MLService.h"
#include "GeoViewWidget.h"

class Application : public QObject {
    Q_OBJECT
public:
    Application(int argc, char *argv[]);
    ~Application();
    int exec();
    void log(const QString &msg);

private:
    void initConnections();

    void sendNextCommand();
    void startExecutionTimer(const QJsonObject& cmd);
    void onCommandExecuted();
    void startDelay();
    void onDelayFinished();
    void collectRobotState();

private:
    QApplication app;
    QString m_sessionId;

    SQLiteDb database;
    Widget* widget;
    GeoViewWidget* map;
    Manager manager;
    RobotService robot_service;
    MLService ml_service;





    QJsonArray CommandQueue;
    int CommandIndex = 0;
    bool readyToStart = false;

    QTimer ExecutionTimer;
    QTimer DelayTimer;
    QTimer CollectTimer;

    enum class State {
        Idle,
        WaitingExecution,
        WaitingDelay
    };

    State mState = State::Idle;
};

#endif // ! APPLICATION_H_