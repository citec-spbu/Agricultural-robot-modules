#include "sqlitedb.h"
#include "logmessages.h"
#include "statusmapper.h"
#include "config.h"


#include <QSqlError>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <QSqlRecord>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>




SQLiteDb::SQLiteDb() {}

SQLiteDb::~SQLiteDb() {
    disconnect();
}

void SQLiteDb::disconnect()
{
    if (db.isOpen()) {
        db.close();
        qDebug() << LogMsg::DB_DISCONNECTED;
    }
}

StatusCode SQLiteDb::connect(const QString &connectionInfo)
{
    try {
        if (QSqlDatabase::contains(Config::DB_CONNECTION_NAME)){
            db = QSqlDatabase::database(Config::DB_CONNECTION_NAME);
        }
        else {
            db = QSqlDatabase::addDatabase("QSQLITE", Config::DB_CONNECTION_NAME);
        }

        db.setDatabaseName(connectionInfo);

        if (!db.open()) {
            qWarning() << LogMsg::DB_CONNECT_FAILED << db.lastError().text();
            return StatusCode::DB_CONNECTION_FAILED;
        }

        qDebug() << LogMsg::DB_CONNECTED;
        return initDatabase();
    }
    catch (...) {
        qWarning() << statusToMessage(StatusCode::UNKNOWN_ERROR);
        return StatusCode::UNKNOWN_ERROR;
    }
}


// Вспомогательная функция для выполнения команды с логированием
bool SQLiteDb::createTable(QSqlQuery &query, const QString &sql, const QString &tableName)
{
    if (!query.exec(sql)) {
        qWarning() << "Ошибка создания таблицы" << tableName << ":" << query.lastError().text();
        return false;
    }
    return true;
}




StatusCode SQLiteDb::initDatabase()
{
    QFile file(Config::DB_SCHEMA_PATH);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << statusToMessage(StatusCode::DB_INIT_FAILED);
        return StatusCode::DB_INIT_FAILED;
    }

    QTextStream in(&file);
    QString sql = in.readAll();
    file.close();

    QSqlQuery query(db);

    QStringList commands = sql.split(';', Qt::SkipEmptyParts);

    for (QString cmd : commands) {
        cmd = cmd.trimmed();
        if (cmd.isEmpty()){
            continue;
        }

        // Получаем имя таблицы для логирования
        QString tableName;
        QRegularExpression re(R"(CREATE TABLE IF NOT EXISTS\s+['\"\[]?([\w_]+)['\"\]]?)",
                              QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = re.match(cmd);
        tableName = match.hasMatch() ? match.captured(1) : "unknown";

        if (!createTable(query, cmd, tableName)) {
            qWarning() << statusToMessage(StatusCode::DB_TABLE_CREATE_FAILED) << tableName;
            return StatusCode::DB_TABLE_CREATE_FAILED;
        }
    }

    qDebug() << LogMsg::DB_INIT_SUCCESS;
    return StatusCode::SUCCESS;
}


SQLResult SQLiteDb::executeSQL(const QString &queryText)
{
    SQLResult result;
    result.code = StatusCode::SUCCESS;
    result.data = QVariant();

    if (queryText.trimmed().isEmpty()) {
        result.code = StatusCode::DB_QUERY_FAILED;
        return result;
    }

    QSqlQuery query(db);

    if (!query.exec(queryText)) {
        qWarning() << statusToMessage(StatusCode::DB_QUERY_FAILED) << query.lastError().text();
        result.code = StatusCode::DB_QUERY_FAILED;
        return result;
    }

    if (queryText.trimmed().startsWith("SELECT", Qt::CaseInsensitive)) {
        QVector<QVariantMap> rows;

        while (query.next()) {
            QVariantMap row;
            QSqlRecord rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                row.insert(rec.fieldName(i), query.value(i));
            }
            rows.append(row);
        }
        result.data = QVariant::fromValue(rows);
    }

    return result;
}

// -------------------- Основные функции добавления --------------------


StatusCode SQLiteDb::execQuery(QSqlQuery &query, StatusCode errCode)
{
    if (!query.exec()) {
        qWarning() << statusToMessage(errCode) << query.lastError().text();
        return errCode;
    }
    return StatusCode::SUCCESS;
}


StatusCode SQLiteDb::addSession(const QString& description)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO Sessions (description) VALUES (:description)");
    query.bindValue(":description", description);

    return execQuery(query);
}


StatusCode SQLiteDb::addField(const QString& name, const QJsonObject& boundary, int sessionId)
{
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO Fields (name, boundary_json, session_id) "
        "VALUES (:name, :boundary, :session_id)"
        );

    query.bindValue(":name", name);
    query.bindValue(":boundary", QString(QJsonDocument(boundary).toJson(QJsonDocument::Compact)));
    query.bindValue(":session_id", sessionId);

    return execQuery(query);
}


StatusCode SQLiteDb::addSensorSpec(const QJsonObject& spec)
{
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO Sensor_specs (spec_json) VALUES (:spec_json)"
        );

    query.bindValue(":spec_json", QString(QJsonDocument(spec).toJson(QJsonDocument::Compact)));

    return execQuery(query);
}


StatusCode SQLiteDb::addPoint(int fieldId, int sessionId, double latitude, double longitude, const QJsonObject& data)
{
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO Points (field_id, session_id, latitude, longitude, data_json) "
        "VALUES (:field_id, :session_id, :lat, :lon, :data_json)"
        );

    query.bindValue(":field_id", fieldId);
    query.bindValue(":session_id", sessionId);
    query.bindValue(":lat", latitude);
    query.bindValue(":lon", longitude);
    query.bindValue(":data_json", QString(QJsonDocument(data).toJson(QJsonDocument::Compact)));

    return execQuery(query);
}


StatusCode SQLiteDb::addObservation(int pointId)
{
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO Observations (point_id) VALUES (:point_id)"
        );

    query.bindValue(":point_id", pointId);

    return execQuery(query);
}


StatusCode SQLiteDb::addMLResult(int observationId, const QString& moduleName, const QJsonObject& result)
{
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO ML_results (observation_id, module_name, results_json) "
        "VALUES (:obs, :module, :json)"
        );

    query.bindValue(":obs", observationId);
    query.bindValue(":module", moduleName);
    query.bindValue(":json", QString(QJsonDocument(result).toJson(QJsonDocument::Compact)));

    return execQuery(query);
}


StatusCode SQLiteDb::addRecommendation(int observationId, const QString& text)
{
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO Recommendations (observation_id, text) "
        "VALUES (:obs, :text)"
        );

    query.bindValue(":obs", observationId);
    query.bindValue(":text", text);

    return execQuery(query);
}



int SQLiteDb::lastInsertId()
{
    QSqlQuery query(db);
    if (!query.exec("SELECT last_insert_rowid();")) {
        qWarning() << statusToMessage(StatusCode::DB_QUERY_FAILED) << query.lastError().text();
        return -1;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    qWarning() << statusToMessage(StatusCode::DB_QUERY_FAILED) << "lastInsertId: no rows returned";
    return -1;
}
