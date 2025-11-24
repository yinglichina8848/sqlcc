include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/network
    ${CMAKE_SOURCE_DIR}/include/sql
    ${CMAKE_SOURCE_DIR}/include/utils
)

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
)

# 注释掉缺少的测试文件
# 创建dcl_operations_test测试可执行文件
# add_executable(dcl_operations_test network/dcl_operations_test.cc)
# 创建simple_connection_test测试可执行文件
# add_executable(simple_connection_test network/simple_connection_test.cc)

# 确保所有测试都能正确处理.cc和.cpp后缀文件
set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS cc cpp)

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
