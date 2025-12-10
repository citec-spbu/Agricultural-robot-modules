add_library(Database
        sqlitedb/sqlitedb.cpp
)
target_include_directories(Database PUBLIC
    include/dbinterface
    include/config
    include/status
    sqlitedb
)
target_link_libraries(Database
    Qt6::Core
    Qt6::Sql
)