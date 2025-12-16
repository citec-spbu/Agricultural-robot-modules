set(CMAKE_CXX_STANDARD 11)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set the QT version
find_package(Qt6 COMPONENTS Core QUIET)
if (NOT Qt6_FOUND)
    set(QT_VERSION 5 CACHE STRING "Qt version for QGeoView")
else()
    set(QT_VERSION 6 CACHE STRING "Qt version for QGeoView")
endif()

find_package(Qt${QT_VERSION} REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
)

add_executable(Map
    main.cpp
    GeoViewWidget.h
    GeoViewWidget.cpp
    GeoPolyline.h
    GeoPolyline.cpp
    Contour.h
    Contour.cpp
    ObservationPoint.h ObservationPoint.cpp
    ObservaionsLayer.h ObservaionsLayer.cpp
    TestJson.h
    ContourPreviewDialog.h ContourPreviewDialog.cpp
    resources.qrc
)

target_link_libraries(Map
    PRIVATE
    Qt${QT_VERSION}::Core
    Qt${QT_VERSION}::Network
    Qt${QT_VERSION}::Gui
    Qt${QT_VERSION}::Widgets
    QGeoView
    qgeoview-samples-shared
)
