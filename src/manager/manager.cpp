#include "manager.h"
#include "logmanager.h"


void Manager::handle(const QString& type, const QJsonObject& json)
{
    if (type == "spec") {
        qDebug() << "[Manager]" << LogMsg::MANAGER_NEW_SPEC;
        db->addSensorSpec(json);
        return;
    }

    if (type == "data") {
        qDebug() << "[Manager]" << LogMsg::MANAGER_NEW_DATA;

        lastPointId = createPointFromJson(json);

        if (lastPointId < 0) {
            qWarning() << LogMsg::MANAGER_FAILED_POINT;
            return;
        }

        // создаём observation
        if (db->addObservation(lastPointId) != StatusCode::SUCCESS) {
            qWarning() << LogMsg::MANAGER_FAILED_OBS;
            return;
        }

        // получим ID observation (последняя вставка)
        lastObservationId = db->lastInsertId();

        if (!robotInitialized) {
            emit initRobotPos(json);
            robotInitialized = true;
        } else {
            emit updateRobotPos(json);
        }

        emit sendCommand("ml", json);  // отправка на ML
        return;
    }

    if (type == "ml_res") {
        qDebug() << "[Manager] ML RESULT:" << json;

        if (lastObservationId < 0) {
            qWarning() << LogMsg::MANAGER_ML_BEFORE_OBS;
            return;
        }

        QString module = json["module"].toString("ml");
        QJsonObject res = json["data"].toObject();


        db->addMLResult(lastObservationId, module, res);

        emit newMlResults(res);
        return;
    }

    if (type == "command") {
        QString where = json["to"].toString();
        QJsonObject payload = json["data"].toObject();

        emit sendCommand(where, payload);
        return;
    }

    qWarning() << LogMsg::MANAGER_UNKNOWN_TYPE << type;
}

// Создание точки
int Manager::createPointFromJson(const QJsonObject& json)
{
    double lat = json.value("latitude").toDouble(0);
    double lon = json.value("longitude").toDouble(0);

    // field_id = 1, session_id = 1 (пока статично)
    StatusCode st = db->addPoint(
        1,  // field
        1,  // session
        lat,
        lon,
        json // сохраняем весь json
        );

    if (st != StatusCode::SUCCESS)
        return -1;

    return db->lastInsertId();
}
