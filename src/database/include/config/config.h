#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

namespace Config {
// Имя подключения для SQLite
const QString DB_CONNECTION_NAME = "agro_connection";

// Путь к схеме БД
const QString DB_SCHEMA_PATH = "D:/QtProjects/AgroDB/schema.sql";

// Имя файла БД
const QString DB_FILE_PATH = "D:/QtProjects/AgroDB/agro.db";

// Тестовая база данных
const QString DB_TEST_FILE_PATH = "D:/QtProjects/AgroDB/agro_test.db";
}



#endif // CONFIG_H
