cmake_minimum_required(VERSION 3.30)
project(robot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Network Widgets Sql)


add_subdirectory(src/network)

add_subdirectory(src/database)

add_subdirectory(src/QGeoView)

add_subdirectory(src/app)


