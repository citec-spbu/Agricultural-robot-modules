# Настройка сборки модуля Manager и модуля базы данных (SQLite)

cmake_minimum_required(VERSION 3.16)
project(AgroScout LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Sql)

# Основной исполняемый файл 
add_executable(AgroScout
    database/sqlitedb/sqlitedb.cpp
    manager/manager.cpp
)

# Пути к заголовочным файлам
target_include_directories(AgroScout PRIVATE
    database/include/dbinterface
    database/include/config
    database/include/status
    manager
    database/sqlitedb
)

target_link_libraries(AgroScout
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql
)

# Установка основного исполняемого файла
include(GNUInstallDirs)
install(TARGETS AgroScout
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
