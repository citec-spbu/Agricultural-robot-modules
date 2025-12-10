#ifndef LOGMESSAGES_H
#define LOGMESSAGES_H

#include <QString>

namespace LogMsg {
// Подключение
const QString DB_CONNECTED       = "Подключение к базе данных успешно.";
const QString DB_CONNECT_FAILED  = "Ошибка подключения к базе данных.";
const QString DB_DISCONNECTED    = "Соединение с базой данных закрыто.";

// Инициализация
const QString DB_INIT_SUCCESS    = "Инициализация БД выполнена успешно.";
const QString DB_INIT_FAILED     = "Не удалось инициализировать базу данных.";

// SQL
const QString DB_QUERY_FAILED    = "Ошибка выполнения SQL-запроса.";
const QString DB_TABLE_CREATE_FAILED = "Ошибка создания таблицы.";

// Файлы
const QString FILE_NOT_FOUND     = "Файл не найден.";

// Общее
const QString UNKNOWN_ERROR      = "Неизвестная ошибка.";
}

#endif // LOGMESSAGES_H
