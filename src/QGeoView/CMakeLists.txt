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




add_library(MapLib
        Map/GeoViewWidget.h
        Map/GeoViewWidget.cpp
        Map/GeoPolyline.h
        Map/GeoPolyline.cpp
        Map/Contour.h
        Map/Contour.cpp
        Map/ObservationPoint.h Map/ObservationPoint.cpp
        Map/ObservaionsLayer.h Map/ObservaionsLayer.cpp
        Map/TestJson.h
        Map/ContourPreviewDialog.h Map/ContourPreviewDialog.cpp
        Map/resources.qrc
)
target_include_directories(MapLib PUBLIC Map Map/robot_icons)
target_link_libraries(MapLib
        PUBLIC
        Qt6::Core
        Qt6::Network
        Qt6::Gui
        Qt6::Widgets
        QGeoView
        qgeoview-samples-shared
)

