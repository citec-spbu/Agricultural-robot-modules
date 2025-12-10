add_executable(AgroDesktop
    main.cpp
    Application.cc
    widget.cpp
    manager/manager.cpp
)
target_include_directories(AgroDesktop PRIVATE
    manager
)
target_link_libraries(AgroDesktop PRIVATE Qt::Core Qt::Widgets network Database)