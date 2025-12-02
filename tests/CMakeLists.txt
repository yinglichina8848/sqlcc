include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/network
    ${CMAKE_SOURCE_DIR}/include/sql
    ${CMAKE_SOURCE_DIR}/include/utils
)

# Add performance test directory (controlled by ENABLE_PERFORMANCE_TESTS)
if(ENABLE_PERFORMANCE_TESTS)
    add_subdirectory(performance)
endif()

# 定义测试源文件列表
set(TEST_SOURCES
    storage_engine_test.cpp
    buffer_pool_test.cpp
    storage_engine_newpage_test.cpp
    disk_manager_test.cpp
    deadlock_fix_test.cpp
    lock_timeout_test.cpp
    sql_executor/constraint_executor_test.cpp
    sql_executor/record_test.cpp
    sql_executor/sql_executor_test.cpp
    sql_parser/sql_parser_test.cpp
    sql_parser/lexer_test.cpp
    sql_parser/test_parser.cpp
)

# 定义增强测试源文件列表
set(ENHANCED_TEST_SOURCES
    unit/b_plus_tree_core_test.cpp
    unit/config_manager_test.cpp
    unit/config_manager_enhanced_test.cpp
    unit/page_enhanced_test.cpp
    unit/disk_manager_enhanced_test.cpp
    unit/storage_engine_enhanced_test.cpp
    unit/sql_parser_comprehensive_test.cpp
    unit/transaction_manager_test.cpp
    unit/transaction_functional_test.cpp
    unit/network_test.cpp
)

# Add unit tests
set(UNIT_TESTS
    storage_engine_test.cpp
    buffer_pool_test.cpp
    storage_engine_newpage_test.cpp
    disk_manager_test.cpp
    deadlock_fix_test.cpp
    lock_timeout_test.cpp
    unit/b_plus_tree_test.cpp
    unit/b_plus_tree_core_test.cpp
    unit/b_plus_tree_performance_test.cpp
    unit/buffer_pool_new_test.cpp
    unit/config_manager_test.cpp
    unit/config_manager_enhanced_test.cpp
    unit/disk_manager_enhanced_test.cpp
    unit/page_enhanced_test.cpp
    unit/storage_engine_enhanced_test.cpp
    unit/transaction_functional_test.cpp
    unit/transaction_manager_test.cpp
    unit/ast_nodes_comprehensive_test.cpp
    unit/sql_parser_comprehensive_test.cpp
    unit/network_test.cpp
    unit/network_integration_test.cpp
    sql_executor/constraint_executor_test.cpp
    sql_executor/record_test.cpp
    sql_executor/sql_executor_test.cpp
    sql_parser/sql_parser_test.cpp
    sql_parser/test_parser.cpp
    network/sql_network_test.cpp
)

# 创建SQL网络测试可执行文件
add_executable(sql_network_test network/sql_network_test.cpp)
add_executable(tls_e2e_test network/tls_e2e_test.cc)
add_executable(aes_encryption_test network/aes_encryption_test.cc)

# 创建SQL解析器测试可执行文件
add_executable(sql_parser_test sql_parser/sql_parser_test.cpp)

# 创建Lexer测试可执行文件
add_executable(lexer_test sql_parser/lexer_test.cpp)

# 创建SQL执行器测试可执行文件 - 暂时禁用，因为SqlExecutor实现不完整
# add_executable(sql_executor_test sql_executor/sql_executor_test.cpp sql_executor/constraint_executor_test.cpp sql_executor/record_test.cpp)

# 注释掉缺少的测试文件
# 创建dcl_operations_test测试可执行文件
# add_executable(dcl_operations_test network/dcl_operations_test.cc)
# 创建simple_connection_test测试可执行文件
# add_executable(simple_connection_test network/simple_connection_test.cc)

# 确保所有测试都能正确处理.cc和.cpp后缀文件
set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS cc cpp)

# 链接必要的库
 target_link_libraries(sql_network_test
     PRIVATE
     gtest
     gtest_main
     pthread
     ${CMAKE_DL_LIBS}
     sqlcc_network
     sqlcc_core_lib
 )

 target_link_libraries(tls_e2e_test
     PRIVATE
     gtest
     gtest_main
     pthread
     ${CMAKE_DL_LIBS}
     sqlcc_network
     sqlcc_executor
 )

 target_link_libraries(aes_encryption_test
     PRIVATE
     gtest
     gtest_main
     pthread
     ${CMAKE_DL_LIBS}
     sqlcc_network
 )

# 链接SQL解析器测试库
target_link_libraries(sql_parser_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_parser
    sqlcc_core_lib
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

# 链接Lexer测试库
target_link_libraries(lexer_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_parser
    sqlcc_core_lib
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

# 链接SQL执行器测试库 - 暂时禁用
# target_link_libraries(sql_executor_test
#     PRIVATE
#     gtest
#     gtest_main
#     pthread
#     ${CMAKE_DL_LIBS}
#     sqlcc_executor
#     sqlcc_storage_engine
#     sqlcc_core_lib
#     sqlcc_transaction_manager
#     sqlcc_parser
# )

# 注释掉缺少测试文件的链接库部分
# 链接必要的库
# target_link_libraries(dcl_operations_test
#     PRIVATE
#     gtest
#     gtest_main
#     pthread
#     ${CMAKE_DL_LIBS}
# )

# 链接必要的库
# target_link_libraries(simple_connection_test
#     PRIVATE
#     gtest
#     gtest_main
#     pthread
#     ${CMAKE_DL_LIBS}
# )

# 将SQL网络测试添加到CTest
add_test(
    NAME sql_network_test
    COMMAND sql_network_test
)

add_test(
    NAME tls_e2e_test
    COMMAND tls_e2e_test
)

add_test(
    NAME aes_encryption_test
    COMMAND aes_encryption_test
)

# 将SQL解析器测试添加到CTest
add_test(
    NAME sql_parser_test
    COMMAND sql_parser_test
)

# 将Lexer测试添加到CTest
add_test(
    NAME lexer_test
    COMMAND lexer_test
)

# 将SQL执行器测试添加到CTest - 暂时禁用
# add_test(
#     NAME sql_executor_test
#     COMMAND sql_executor_test
# )

# 注释掉缺少测试文件的CTest注册
# 将测试添加到CTest
# add_test(
#     NAME dcl_operations_test
#     COMMAND dcl_operations_test
# )

# 注释掉缺少测试文件的CTest注册
# 将测试添加到CTest
# add_test(
#     NAME simple_connection_test
#     COMMAND simple_connection_test
# )

# 添加新移动的测试文件

# 创建comprehensive_test可执行文件
add_executable(comprehensive_test integration/comprehensive_test.cpp)

target_link_libraries(comprehensive_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

# 创建DCL相关测试可执行文件
add_executable(dcl_test unit/dcl_test.cpp)

target_link_libraries(dcl_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

add_executable(dcl_test_advanced unit/dcl_test_advanced.cpp)

target_link_libraries(dcl_test_advanced
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

# DCL Parser Tests
add_executable(dcl_parser_test
    unit/dcl_parser_test.cpp
)
target_link_libraries(dcl_parser_test 
    sqlcc_parser
    gtest 
    gtest_main
)
add_test(NAME dcl_parser_test COMMAND dcl_parser_test)

# 创建DDL和DML测试可执行文件
add_executable(ddl_test unit/ddl_test.cpp)

target_link_libraries(ddl_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

add_executable(dml_test unit/dml_test.cpp)

target_link_libraries(dml_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

# DML执行器集成测试
add_executable(dml_executor_integration_test unit/dml_executor_integration_test.cpp)

target_link_libraries(dml_executor_integration_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

add_test(NAME dml_executor_integration_test COMMAND dml_executor_integration_test)

# 约束验证测试
add_executable(constraint_validation_test unit/constraint_validation_test.cpp)

target_link_libraries(constraint_validation_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

add_test(NAME constraint_validation_test COMMAND constraint_validation_test)

# 索引维护测试
add_executable(index_maintenance_test unit/index_maintenance_test.cpp)

target_link_libraries(index_maintenance_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

add_test(NAME index_maintenance_test COMMAND index_maintenance_test)

# WHERE条件优化测试
add_executable(where_clause_optimization_test unit/where_clause_optimization_test.cpp)

target_link_libraries(where_clause_optimization_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

add_test(NAME where_clause_optimization_test COMMAND where_clause_optimization_test)

# DML改进测试
add_executable(dml_improvement_test unit/dml_improvement_test.cpp)

target_link_libraries(dml_improvement_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

add_test(NAME dml_improvement_test COMMAND dml_improvement_test)

# 创建 simple_test可执行文件
add_executable(simple_test unit/simple_test.cpp)

target_link_libraries(simple_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

# 创建database_manager_test可执行文件 - 暂时禁用，因为依赖未实现的事务方法
# add_executable(database_manager_test unit/database_manager_test.cpp)
# 
# target_link_libraries(database_manager_test
#     PRIVATE
#     gtest
#     gtest_main
#     pthread
#     ${CMAKE_DL_LIBS}
#     sqlcc_core_lib
#     sqlcc_parser
#     sqlcc_config_manager
#     sqlcc_storage_engine
#     sqlcc_transaction_manager
#     sqlcc_executor
# )

# 创建transaction_manager_test可执行文件
add_executable(transaction_manager_test unit/transaction_manager_test.cpp)

target_link_libraries(transaction_manager_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

# 创建storage_engine相关测试可执行文件
add_executable(disk_manager_test unit/storage_engine/disk_manager_test.cpp)

target_link_libraries(disk_manager_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

add_executable(buffer_pool_test unit/storage_engine/buffer_pool_test.cpp)

target_link_libraries(buffer_pool_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

add_executable(b_plus_tree_test unit/storage_engine/b_plus_tree_test.cpp)

target_link_libraries(b_plus_tree_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

# 创建SQL执行器测试可执行文件
add_executable(sql_executor_comprehensive_test sql_executor/sql_executor_comprehensive_test.cpp)

target_link_libraries(sql_executor_comprehensive_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

add_executable(sql_executor_minimal_test sql_executor/sql_executor_minimal_test.cpp)

target_link_libraries(sql_executor_minimal_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

add_executable(sql_executor_targeted_test sql_executor/sql_executor_targeted_test.cpp)

target_link_libraries(sql_executor_targeted_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

add_executable(sql_executor_unit_test sql_executor/sql_executor_unit_test.cpp)

target_link_libraries(sql_executor_unit_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

add_executable(test_sql_executor sql_executor/test_sql_executor.cpp)

target_link_libraries(test_sql_executor
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

# 将新测试添加到CTest
add_test(NAME comprehensive_test COMMAND comprehensive_test)
add_test(NAME dcl_test COMMAND dcl_test)
add_test(NAME dcl_test_advanced COMMAND dcl_test_advanced)
add_test(NAME ddl_test COMMAND ddl_test)
add_test(NAME dml_test COMMAND dml_test)
add_test(NAME simple_test COMMAND simple_test)
# add_test(NAME database_manager_test COMMAND database_manager_test) # 暂时禁用
add_test(NAME transaction_manager_test COMMAND transaction_manager_test)
add_test(NAME sql_executor_comprehensive_test COMMAND sql_executor_comprehensive_test)
add_test(NAME sql_executor_minimal_test COMMAND sql_executor_minimal_test)
add_test(NAME sql_executor_targeted_test COMMAND sql_executor_targeted_test)
add_test(NAME sql_executor_unit_test COMMAND sql_executor_unit_test)
add_test(NAME test_sql_executor COMMAND test_sql_executor)

# 添加客户机-服务器测试
# 创建客户机-服务器集成测试可执行文件
add_executable(client_server_integration_test client_server/client_server_integration_test.cpp client_server/server_manager.cpp client_server/client_test.cpp)

target_link_libraries(client_server_integration_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
    sqlcc_network
)

# 将客户机-服务器测试添加到CTest
add_test(NAME client_server_integration_test COMMAND client_server_integration_test)

# 将storage_engine相关测试添加到CTest
add_test(NAME disk_manager_test COMMAND disk_manager_test)
add_test(NAME buffer_pool_test COMMAND buffer_pool_test)
add_test(NAME b_plus_tree_test COMMAND b_plus_tree_test)

# 创建network_unit_test可执行文件
add_executable(network_unit_test unit/network/network_unit_test.cpp)

target_link_libraries(network_unit_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
    sqlcc_network
)

# 将network_unit_test添加到CTest
add_test(NAME network_unit_test COMMAND network_unit_test)

# 创建客户机-服务器测试目标
add_custom_target(client_server_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "client_server_" --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run client-server integration tests"
)

# 添加isql集成测试
add_executable(isql_integration_test integration/isql_integration_test.cpp)

target_link_libraries(isql_integration_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
    sqlcc_network
)

# 将isql集成测试添加到CTest
add_test(NAME isql_integration_test COMMAND isql_integration_test)

# 创建isql集成测试目标
add_custom_target(isql_integration_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "isql_integration_" --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run isql integration tests"
)

# 添加新的解析器验证测试
add_executable(parser_verification_test 
    unit/parser_verification_test.cpp
)

target_link_libraries(parser_verification_test 
    sqlcc_parser
    sqlcc_core_lib
)

add_test(NAME parser_verification_test 
         COMMAND parser_verification_test)

# 添加SystemDatabase元数据操作测试
add_executable(system_database_test 
    unit/system_database_test.cpp
)

target_link_libraries(system_database_test 
    sqlcc_executor
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    gtest 
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
)

add_test(NAME system_database_test 
         COMMAND system_database_test)

# 添加REVOKE持久化测试
add_executable(revoke_persistence_test 
    unit/revoke_persistence_test.cpp
)

target_link_libraries(revoke_persistence_test 
    sqlcc_executor
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    gtest 
    gtest_main
    pthread
    stdc++fs
    ${CMAKE_DL_LIBS}
)

add_test(NAME revoke_persistence_test 
         COMMAND revoke_persistence_test)
