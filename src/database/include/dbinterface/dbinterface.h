#ifndef DBINTERFACE_H
#define DBINTERFACE_H

#include "statuscodes.h"

#include <QString>
#include <QVariant>
#include <QVector>

// Структура для результатов SQL-запроса
struct SQLResult {
    StatusCode code;  // Код ошибки
    QVariant data;
};

class DbInterface
{
public:
    virtual ~DbInterface() {}

    // Подключение к БД
    virtual StatusCode connect(const QString &connectionInfo) = 0;
    virtual SQLResult executeSQL(const QString &query) = 0;

    // Отключиться от базы
    virtual void disconnect() = 0;
};

#endif // DBINTERFACE_H
