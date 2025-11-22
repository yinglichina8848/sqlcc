# 创建SQL parser独立库
add_library(sqlcc_parser STATIC
    sql_parser/token.cpp
    sql_parser/ast_node.cpp
    sql_parser/ast_nodes.cpp
    sql_parser/lexer.cpp
    sql_parser/parser.cpp
)

# 设置SQL parser库的包含目录
target_include_directories(sqlcc_parser PUBLIC ${CMAKE_SOURCE_DIR}/include)

# 创建config_manager库
add_library(sqlcc_config_manager STATIC
    config_manager/config_manager.cpp
)

# 设置config_manager库的包含目录
target_include_directories(sqlcc_config_manager PUBLIC ${CMAKE_SOURCE_DIR}/include)

# 创建storage_engine库
add_library(sqlcc_storage_engine STATIC
    storage_engine/buffer_pool.cpp
    storage_engine/buffer_pool_new.cpp
    storage_engine/buffer_pool_sharded.cpp
    storage_engine/disk_manager.cpp
    storage_engine/page.cpp
    storage_engine/storage_engine.cpp
    storage_engine/b_plus_tree.cpp
)

# 设置storage_engine库的包含目录
target_include_directories(sqlcc_storage_engine PUBLIC ${CMAKE_SOURCE_DIR}/include)

# 创建transaction_manager库
add_library(sqlcc_transaction_manager STATIC
    transaction_manager/transaction_manager.cpp
    transaction_manager/wal_manager.cpp
)

# 设置transaction_manager库的包含目录
target_include_directories(sqlcc_transaction_manager PUBLIC ${CMAKE_SOURCE_DIR}/include)

# 创建network库
add_library(sqlcc_network STATIC
    network/network.cpp
)

# 设置network库的包含目录
target_include_directories(sqlcc_network PUBLIC ${CMAKE_SOURCE_DIR}/include)

# 创建SQL执行器库
add_library(sqlcc_executor STATIC
    sql_executor/sql_executor.cpp
    sql_executor/constraint_executor.cpp
    sql_executor/data_type.cpp
    sql_executor/index_manager.cpp
)

# 设置SQL执行器库的包含目录
target_include_directories(sqlcc_executor PUBLIC ${CMAKE_SOURCE_DIR}/include)

# 添加必要的链接依赖
find_package(Threads)
if(Threads_FOUND)
    target_link_libraries(sqlcc_parser PUBLIC Threads::Threads)
    target_link_libraries(sqlcc_config_manager PUBLIC Threads::Threads)
    target_link_libraries(sqlcc_storage_engine PUBLIC Threads::Threads)
    target_link_libraries(sqlcc_transaction_manager PUBLIC Threads::Threads)
endif()

# 创建核心库定义（作为一个元库，链接所有子库）
add_library(sqlcc_core INTERFACE)
target_link_libraries(sqlcc_core INTERFACE
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_network
)

# SQL执行器库链接到核心库、parser库和storage_engine库
target_link_libraries(sqlcc_executor PRIVATE sqlcc_parser sqlcc_core sqlcc_storage_engine)

# 添加dml_test可执行文件
add_executable(dml_test
    dml_test.cpp
)

# 链接所需的库
target_link_libraries(dml_test
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

# 添加编译选项
target_compile_options(dml_test PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wno-unused-parameter
)
