cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(QGeoView LANGUAGES C CXX)

# Если не найден mingw_64 cmake
# set(CMAKE_PREFIX_PATH "C:/Qt/6.8.2/mingw_64/lib/cmake")

find_package(GDAL CONFIG)

add_subdirectory(lib)
add_subdirectory(shared)
add_subdirectory(Map)

if(GDAL_FOUND)
    add_subdirectory(samples/gdal-shapefile)
    target_link_libraries(Map PRIVATE GDAL::GDAL)
else()
    message(STATUS "GDAL not found, skipping gdal-shapefile example")
endif()
