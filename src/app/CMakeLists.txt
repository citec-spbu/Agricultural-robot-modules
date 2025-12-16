add_executable(AgroDesktop
    main.cpp
    Application.cc
    RobotService.h
    widget.cc
    manager/manager.cpp
)
target_include_directories(AgroDesktop PRIVATE
    manager
)
target_link_libraries(AgroDesktop PRIVATE
        Qt6::Core
        Qt6::Widgets
        network
        Database
        MapLib)