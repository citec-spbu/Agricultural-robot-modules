#ifndef STATUSCODES_H
#define STATUSCODES_H

enum class StatusCode {
    SUCCESS = 0,               // операция выполнена успешно

    // Ошибки подключения
    DB_CONNECTION_FAILED = 1001,
    DB_INIT_FAILED = 1002,

    // Ошибки SQL
    DB_QUERY_FAILED = 1101,
    DB_TABLE_CREATE_FAILED = 1102,

    // Ошибки файлов
    FILE_NOT_FOUND = 1201,

    // Неизвестная ошибка
    UNKNOWN_ERROR = 1999
};

#endif // STATUSCODES_H
