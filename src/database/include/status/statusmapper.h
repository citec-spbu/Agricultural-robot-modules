#ifndef STATUSMAPPER_H
#define STATUSMAPPER_H

#include "statuscodes.h"
#include "logmessages.h"
#include <QString>

inline QString statusToMessage(StatusCode code) {
    using namespace LogMsg;
    switch (code) {
    case StatusCode::DB_CONNECTION_FAILED:   return DB_CONNECT_FAILED;
    case StatusCode::DB_INIT_FAILED:         return DB_INIT_FAILED;
    case StatusCode::DB_QUERY_FAILED:        return DB_QUERY_FAILED;
    case StatusCode::DB_TABLE_CREATE_FAILED: return DB_TABLE_CREATE_FAILED;
    case StatusCode::FILE_NOT_FOUND:         return FILE_NOT_FOUND;
    case StatusCode::SUCCESS:                return "Операция успешно выполнена.";
    default:                                 return UNKNOWN_ERROR;
    }
}

#endif // STATUSMAPPER_H
