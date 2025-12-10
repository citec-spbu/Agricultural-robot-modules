#ifndef SQLITEDB_H
#define SQLITEDB_H

#include "dbinterface.h"
#include "statuscodes.h"


#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class SQLiteDb : public DbInterface
{
public:
    SQLiteDb();
    ~SQLiteDb();

    StatusCode connect(const QString &connectionInfo) override;
    SQLResult executeSQL(const QString &query) override;
    void disconnect() override;

    // Методы для безопасного добавления данных
    StatusCode addSession(const QString& description);
    StatusCode addField(const QString& name,const QJsonObject& boundary,int sessionId);
    StatusCode addSensorSpec(const QJsonObject& spec);
    StatusCode addPoint(int fieldId, int sessionId,double latitude,double longitude, const QJsonObject& data);
    StatusCode addObservation(int pointId);
    StatusCode addMLResult(int observationId, const QString& moduleName,const QJsonObject& result);
    StatusCode addRecommendation(int observationId,const QString& text);

    int lastInsertId();


private:
    QSqlDatabase db;

    StatusCode execQuery(QSqlQuery &query, StatusCode errCode = StatusCode::DB_QUERY_FAILED);
    StatusCode initDatabase(); // создаёт таблицы, если их нет
    bool createTable(QSqlQuery &query, const QString &sql, const QString &tableName);

};

#endif // SQLITEDB_H
