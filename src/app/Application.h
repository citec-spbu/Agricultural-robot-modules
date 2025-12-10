#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <QApplication>
#include <QUuid>

#include "manager.h"
#include "widget.h"
#include "sqlitedb.h"
#include "RobotService.h"
//#include "MLService.h"

class Application : public QObject {
    Q_OBJECT
public:
    Application(int argc, char *argv[]);
    ~Application();
    int exec();
    void log(const QString &msg);

private:
    void initWidgetConnections();
    void initManagerConnections();
    void initMLServiceConnections();
    void initRobotServiceConnections();

private:
    QApplication app;
    QString m_sessionId;

    SQLiteDb database;
    Widget widget;
    Manager manager;
    RobotService robot_service;
    //MLService ml_service;
};

#endif // ! APPLICATION_H_