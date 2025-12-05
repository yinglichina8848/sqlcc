# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(sqlcc_src)

# 创建core库
add_library(sqlcc_core_lib STATIC
    core/database_manager.cpp
)

# 设置core库的包含目录
target_include_directories(sqlcc_core_lib PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/storage
)

# 创建SQL parser库
set(SQL_PARSER_SOURCES
    sql_parser/ast_node.cpp
    sql_parser/ast/ast_node.cpp
    sql_parser/ast/source_location.cpp
    sql_parser/ast_nodes.cpp
    sql_parser/parser_new.cpp
    sql_parser/lexer_new.cpp
    sql_parser/token_new.cpp
    sql_parser/errors/error_core.cpp
    sql_parser/parser_set_operations.cpp
    sql_parser/set_operation_node.cpp
    sql_parser/parser.cpp
    sql_parser/lexer.cpp
    sql_parser/token.cpp
)

add_library(sqlcc_parser STATIC
    ${SQL_PARSER_SOURCES}
)

# 设置SQL parser库的包含目录
target_include_directories(sqlcc_parser PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/sql
)

# 创建config_manager库
add_library(sqlcc_config_manager STATIC
    config_manager/config_manager.cpp
)

# 设置config_manager库的包含目录
target_include_directories(sqlcc_config_manager PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/utils
)

# 创建storage_engine库
set(SQLCC_STORAGE_ENGINE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/storage_engine/disk_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/storage_engine/buffer_pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/storage_engine/buffer_pool_sharded.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/storage_engine/storage_engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/storage_engine/b_plus_tree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/storage_engine/table_storage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/storage_engine/page.cpp
)
add_library(sqlcc_storage_engine STATIC
    ${SQLCC_STORAGE_ENGINE_SOURCES}
)

# 设置storage_engine库的包含目录
target_include_directories(sqlcc_storage_engine PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/storage
)

# 链接storage_engine库与config_manager库
target_link_libraries(sqlcc_storage_engine PUBLIC sqlcc_config_manager)

# 创建transaction_manager库
add_library(sqlcc_transaction_manager STATIC
    transaction_manager/transaction_manager.cpp
    transaction_manager/wal_manager.cpp
)

# 设置transaction_manager库的包含目录
target_include_directories(sqlcc_transaction_manager PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
)

# 创建network库
add_library(sqlcc_network STATIC
    network/network.cpp
    network/encryption.cpp
)

# 设置network库的包含目录
target_include_directories(sqlcc_network PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/network
)

# 查找OpenSSL库
find_package(OpenSSL REQUIRED)
# 链接加密与TLS库
target_link_libraries(sqlcc_network PUBLIC OpenSSL::Crypto)
target_link_libraries(sqlcc_network PUBLIC OpenSSL::SSL)
target_link_libraries(sqlcc_network PUBLIC Threads::Threads)

# 创建SQL执行器库
set(SQL_EXECUTOR_SOURCES
    sql_executor/sql_executor.cpp
    sql_executor/data_type.cpp
    sql_executor/index_manager.cpp
    sql_executor/user_manager.cpp
    sql_executor/system_database.cpp
    execution_engine.cpp
    unified_query_plan.cpp
    error_handler.cpp
    permission_validator.cpp
    unified_executor.cpp
    execution_context.cpp
    execution/set_operation_executor.cpp
    execution/join_executor.cpp
    execution/subquery_executor.cpp
)

add_library(sqlcc_executor STATIC
    ${SQL_EXECUTOR_SOURCES}
)

# 设置SQL执行器库的包含目录
target_include_directories(sqlcc_executor PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/sql
    ${CMAKE_SOURCE_DIR}/include/core
)

# 设置编译选项，启用覆盖率测试
target_compile_options(sqlcc_executor PRIVATE -g -O0 --coverage)
target_link_options(sqlcc_executor PRIVATE --coverage)

# Link executor with its dependencies to resolve static symbols
target_link_libraries(sqlcc_executor PUBLIC
    sqlcc_parser
    sqlcc_core_lib
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_config_manager
)

# 添加必要的链接依赖
find_package(Threads REQUIRED)
target_link_libraries(sqlcc_core_lib PUBLIC Threads::Threads)
target_link_libraries(sqlcc_parser PUBLIC Threads::Threads)
target_link_libraries(sqlcc_config_manager PUBLIC Threads::Threads)
target_link_libraries(sqlcc_storage_engine PUBLIC Threads::Threads)
target_link_libraries(sqlcc_transaction_manager PUBLIC Threads::Threads)

# 创建核心库定义（作为一个元库，链接所有子库）
add_library(sqlcc_core INTERFACE)
target_link_libraries(sqlcc_core INTERFACE
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_network
    sqlcc_executor
)
