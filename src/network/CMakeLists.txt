add_library(network
        protocols/Base.cc
        protocols/Tcp.cc
        protocols/Udp.cc
        protocols/Http.cc
)
target_include_directories(network PUBLIC
        protocols
)
target_link_libraries(network PUBLIC Qt6::Core Qt6::Network)

add_executable(client client.cpp)
target_link_libraries(client PRIVATE network Qt6::Core Qt6::Network)

add_executable(server server.cpp)
target_link_libraries(server PRIVATE network Qt6::Core Qt6::Network)
