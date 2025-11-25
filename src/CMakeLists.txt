# 创建core库
add_library(sqlcc_core_lib STATIC
    core/database_manager.cpp
)

# 设置core库的包含目录
target_include_directories(sqlcc_core_lib PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
)

# 创建SQL parser库
set(SQL_PARSER_SOURCES
    sql_parser/ast_node.cpp
    sql_parser/ast_nodes.cpp
    sql_parser/parser.cpp
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
target_include_directories(sqlcc_storage_engine PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
)

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
)

# 设置network库的包含目录
target_include_directories(sqlcc_network PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/network
)

# 创建SQL执行器库
add_library(sqlcc_executor STATIC
    sql_executor/sql_executor.cpp
    sql_executor/constraint_executor.cpp
    sql_executor/data_type.cpp
    sql_executor/index_manager.cpp
    sql_executor/user_manager.cpp
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

# 添加必要的链接依赖
find_package(Threads)
if(Threads_FOUND)
    target_link_libraries(sqlcc_core_lib PUBLIC Threads::Threads)
    target_link_libraries(sqlcc_parser PUBLIC Threads::Threads)
    target_link_libraries(sqlcc_config_manager PUBLIC Threads::Threads)
    target_link_libraries(sqlcc_storage_engine PUBLIC Threads::Threads)
    target_link_libraries(sqlcc_transaction_manager PUBLIC Threads::Threads)
endif()

# 创建核心库定义（作为一个元库，链接所有子库）
add_library(sqlcc_core INTERFACE)
target_link_libraries(sqlcc_core INTERFACE
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_network
)

# SQL执行器库链接到核心库、parser库和storage_engine库
target_link_libraries(sqlcc_executor PRIVATE sqlcc_parser sqlcc_core_lib sqlcc_storage_engine)

# storage_engine库链接到config_manager库，因为buffer_pool.cpp使用了ConfigManager类
target_link_libraries(sqlcc_storage_engine PRIVATE sqlcc_config_manager)

# 添加dml_test可执行文件
add_executable(dml_test
    dml_test.cpp
)

# 链接所需的库
target_link_libraries(dml_test
    sqlcc_core_lib
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
    -Wno-unused-parameter
)

# 添加comprehensive_test可执行文件
add_executable(comprehensive_test
    comprehensive_test.cpp
)

# 链接所需的库
target_link_libraries(comprehensive_test
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

# 添加编译选项
target_compile_options(comprehensive_test PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
)

# 添加dcl_test_advanced可执行文件
add_executable(dcl_test_advanced
    dcl_test_advanced.cpp
)

# 链接所需的库
target_link_libraries(dcl_test_advanced
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

# 添加编译选项
target_compile_options(dcl_test_advanced PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
)

# 添加sql_executor_comprehensive_test可执行文件
add_executable(sql_executor_comprehensive_test
    sql_executor_comprehensive_test.cpp
)

# 链接所需的库
target_link_libraries(sql_executor_comprehensive_test
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

# 添加编译选项
target_compile_options(sql_executor_comprehensive_test PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
)

# 设置覆盖率测试选项
target_compile_options(sql_executor_comprehensive_test PRIVATE -g -O0 --coverage)
target_link_options(sql_executor_comprehensive_test PRIVATE --coverage)

# 添加sql_executor_targeted_test可执行文件
add_executable(sql_executor_targeted_test
    sql_executor_targeted_test.cpp
)

# 链接所需的库
target_link_libraries(sql_executor_targeted_test
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

# 设置编译选项，启用覆盖率测试
target_compile_options(sql_executor_targeted_test PRIVATE -g -O0 --coverage)
target_link_options(sql_executor_targeted_test PRIVATE --coverage)

# 添加SQL执行器精确单元测试
add_executable(sql_executor_unit_test sql_executor_unit_test.cpp)

# 链接所需的库
target_link_libraries(sql_executor_unit_test
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

# 设置编译选项，启用覆盖率测试
target_compile_options(sql_executor_unit_test PRIVATE -g -O0 --coverage)
target_link_options(sql_executor_unit_test PRIVATE --coverage)

# 添加SQL执行器最小化测试
add_executable(sql_executor_minimal_test sql_executor_minimal_test.cpp)

# 链接所需的库
target_link_libraries(sql_executor_minimal_test
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

# 设置编译选项，启用覆盖率测试
target_compile_options(sql_executor_minimal_test PRIVATE -g -O0 --coverage)
target_link_options(sql_executor_minimal_test PRIVATE --coverage)

# 确保所有库都能正确处理.cpp后缀文件
set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS cpp)
