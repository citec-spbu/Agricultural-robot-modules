#ifndef MANAGER_H
#define MANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QDebug>
#include "sqlitedb.h"

class Manager : public QObject {
    Q_OBJECT

public:
    explicit Manager(SQLiteDb* db, QObject* parent = nullptr)
        : QObject(parent), db(db) {}

    // вызывается сетевым модулем!
    void handle(const QString& type, const QJsonObject& json);

signals:
    void sendCommand(const QString& where, const QJsonObject& data);

    // сигналы для карты
    void initRobotPos(const QJsonObject& json);
    void updateRobotPos(const QJsonObject& json);
    void newMlResults(const QJsonObject& json);

private:
    SQLiteDb* db;

    int lastPointId = -1;       // для привязки ML результатов
    int lastObservationId = -1; // для ML

    bool robotInitialized = false;

    int createPointFromJson(const QJsonObject& json);
};

#endif
