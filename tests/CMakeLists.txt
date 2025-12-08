# ===== 代码覆盖率配置 =====
option(ENABLE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_COVERAGE)
    # 检查编译器支持
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "启用代码覆盖率支持")
        # 添加覆盖率编译标志
        set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} --coverage -fprofile-arcs")
        set(CMAKE_C_FLAGS_COVERAGE "${CMAKE_C_FLAGS_DEBUG} --coverage -fprofile-arcs")
        set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "--coverage")
        set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "--coverage")
        
        # 替换构建类型
        set(CMAKE_BUILD_TYPE Coverage)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_COVERAGE}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_COVERAGE}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS_COVERAGE}")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS_COVERAGE}")
        
        # 检查gcov工具是否可用
        find_program(GCOV_EXECUTABLE gcov)
        if(GCOV_EXECUTABLE)
            message(STATUS "找到gcov工具: ${GCOV_EXECUTABLE}")
        else()
            message(WARNING "未找到gcov工具，可能需要安装build-essential包")
        endif()
        
        # 确保链接器包含gcov运行时
        find_library(GCOV_LIBRARY NAMES gcov)
        if(GCOV_LIBRARY)
            message(STATUS "找到gcov运行时库: ${GCOV_LIBRARY}")
        else()
            message(STATUS "未找到gcov运行时库，将使用编译器内置支持")
        endif()
    else()
        message(WARNING "当前编译器不支持代码覆盖率")
    endif()
endif()

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

# 定义测试源文件列表 - 移除旧Parser测试
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
    # sql_parser/sql_parser_test.cpp  - 禁用旧Parser测试
    # sql_parser/lexer_test.cpp       - 禁用旧Lexer测试
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
    # sql_parser/sql_parser_test.cpp  - 禁用旧Parser测试
    sql_parser/test_parser.cpp
    network/sql_network_test.cpp
)

# 创建SQL网络测试可执行文件
add_executable(sql_network_test network/sql_network_test.cpp)
add_executable(tls_e2e_test network/tls_e2e_test.cc)
add_executable(aes_encryption_test network/aes_encryption_test.cc)

# 创建SQL解析器测试可执行文件 - 禁用旧Parser，启用新Parser
# add_executable(sql_parser_test sql_parser/sql_parser_test.cpp)

# 创建Lexer测试可执行文件 - 禁用旧Lexer，启用新Lexer
# add_executable(lexer_test sql_parser/lexer_test.cpp)

# 添加新Parser测试
add_executable(parser_new_unit_test ../tests/sql_parser/parser_new_unit_test.cpp)
add_executable(lexer_new_unit_test ../tests/sql_parser/lexer_new_unit_test.cpp)
add_executable(token_new_unit_test ../tests/sql_parser/token_new_unit_test.cpp)
add_executable(parser_new_integration_test ../tests/sql_parser/parser_new_integration_test.cpp)

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

# 链接新Parser测试库
target_link_libraries(parser_new_unit_test
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

target_link_libraries(lexer_new_unit_test
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

target_link_libraries(token_new_unit_test
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

target_link_libraries(parser_new_integration_test
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

# 将新Parser测试添加到CTest
add_test(
    NAME parser_new_unit_test
    COMMAND parser_new_unit_test
)

add_test(
    NAME lexer_new_unit_test
    COMMAND lexer_new_unit_test
)

add_test(
    NAME token_new_unit_test
    COMMAND token_new_unit_test
)

add_test(
    NAME parser_new_integration_test
    COMMAND parser_new_integration_test
)

# 将SQL解析器测试添加到CTest - 禁用旧Parser测试
# add_test(
#     NAME sql_parser_test
#     COMMAND sql_parser_test
# )

# 将Lexer测试添加到CTest - 禁用旧Lexer测试
# add_test(
#     NAME lexer_test
#     COMMAND lexer_test
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
add_executable(dcl_test unit/basic/dcl_test.cpp)

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

# DCL Parser Tests
add_executable(dcl_parser_test
    unit/basic/dcl_parser_test.cpp
)
target_link_libraries(dcl_parser_test 
    sqlcc_parser
    gtest 
    gtest_main
)
add_test(NAME dcl_parser_test COMMAND dcl_parser_test)

# 创建DDL和DML测试可执行文件
add_executable(ddl_test unit/basic/ddl_test.cpp)

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

# DML执行器集成测试
add_executable(dml_executor_integration_test unit/basic/dml_executor_integration_test.cpp)

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

# 索引系统集成测试
add_executable(index_system_integration_test unit/basic/index_system_integration_test.cpp)

target_link_libraries(index_system_integration_test
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

# 定义约束验证测试源文件
set(CONSTRAINT_VALIDATION_TEST_SOURCES
    unit/basic/constraint_validation_test.cpp
)

# 约束验证测试
add_executable(constraint_validation_test unit/basic/constraint_validation_test.cpp)

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


# CompareValues单元测试
add_executable(compare_values_test unit/basic/compare_values_test.cpp)

target_link_libraries(compare_values_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
)



# 创建 simple_test可执行文件
add_executable(simple_test unit/basic/simple_test.cpp)

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

# 创建transaction_manager_test可执行文件
add_executable(transaction_manager_test unit/basic/transaction_manager_test.cpp)

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

# 将新测试添加到CTest
# add_test(NAME dcl_test COMMAND dcl_test)  # 已在下文重复注册
# add_test(NAME ddl_test COMMAND ddl_test)  # 已在下文重复注册
# add_test(NAME dml_executor_integration_test COMMAND dml_executor_integration_test)  # 已在下文重复注册
# add_test(NAME index_system_integration_test COMMAND index_system_integration_test)  # 已在下文重复注册
# add_test(NAME constraint_validation_test COMMAND constraint_validation_test)  # 已在下文重复注册
# add_test(NAME compare_values_test COMMAND compare_values_test)  # 已在下文重复注册

# 创建storage_engine相关测试可执行文件
add_executable(disk_manager_test unit/basic/disk_manager_test.cpp)

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

add_executable(buffer_pool_test unit/basic/buffer_pool_test.cpp)

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

# 注释掉buffer_pool_v3_test，因为它使用旧版本的BufferPool接口
# add_executable(buffer_pool_v3_test storage_engine/buffer_pool_v3_test.cpp)
# 
# target_link_libraries(buffer_pool_v3_test
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

add_executable(b_plus_tree_test unit/basic/b_plus_tree_test.cpp)

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

# 将新测试添加到CTest
add_test(NAME comprehensive_test COMMAND comprehensive_test)
add_test(NAME dcl_test COMMAND dcl_test)
add_test(NAME ddl_test COMMAND ddl_test)
add_test(NAME dml_executor_integration_test COMMAND dml_executor_integration_test)
add_test(NAME index_system_integration_test COMMAND index_system_integration_test)
add_test(NAME simple_test COMMAND simple_test)
add_test(NAME transaction_manager_test COMMAND transaction_manager_test)
add_test(NAME sql_executor_comprehensive_test COMMAND sql_executor_comprehensive_test)
add_test(NAME sql_executor_minimal_test COMMAND sql_executor_minimal_test)
add_test(NAME constraint_validation_test COMMAND constraint_validation_test)
add_test(NAME compare_values_test COMMAND compare_values_test)

# 将storage_engine相关测试添加到CTest
add_test(NAME disk_manager_test COMMAND disk_manager_test)
add_test(NAME buffer_pool_test COMMAND buffer_pool_test)
# add_test(NAME buffer_pool_v3_test COMMAND buffer_pool_v3_test)  # 注释掉，因为buffer_pool_v3_test已被注释
add_test(NAME b_plus_tree_test COMMAND b_plus_tree_test)

# 约束验证测试已在前面定义，此处注释掉重复定义
# add_executable(constraint_validation_test unit/constraint_validation_test.cpp)
# 
# target_link_libraries(constraint_validation_test
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

# 创建network_unit_test可执行文件
add_executable(network_unit_test unit/basic/network_unit_test.cpp)

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

# 设置测试目标后缀，避免与主项目目标冲突
if(NOT TEST_SUFFIX)
    set(TEST_SUFFIX "sqlcc_test")
endif()

# 使用已定义的UNIT_TESTS中的路径，避免重复定义
# 缓冲池和B+树测试已包含在UNIT_TESTS中，无需单独定义

# 缓冲池测试和B+树测试已在前面定义，此处注释掉重复定义
# add_executable(buffer_pool_test_${TEST_SUFFIX} ${BUFFER_POOL_TEST_SOURCES})
# add_executable(b_plus_tree_test_${TEST_SUFFIX} ${B_PLUS_TREE_TEST_SOURCES})
# 客户端-服务器测试
add_custom_target(client_server_tests_${TEST_SUFFIX} DEPENDS ${CLIENT_SERVER_TEST_TARGETS})
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

# 添加新的解析器验证测试
# add_executable(parser_verification_test 
#     unit/parser_verification_test.cpp
# )
# 
# target_link_libraries(parser_verification_test 
#     sqlcc_parser
#     sqlcc_core_lib
# )
# 
# add_test(NAME parser_verification_test 
#          COMMAND parser_verification_test)

# 添加SystemDatabase元数据操作测试
# add_executable(system_database_test 
#     unit/system_database_test.cpp
# )
# 
# target_link_libraries(system_database_test 
#     sqlcc_executor
#     sqlcc_core_lib
#     sqlcc_parser
#     sqlcc_config_manager
#     sqlcc_storage_engine
#     sqlcc_transaction_manager
#     gtest 
#     gtest_main
#     pthread
#     ${CMAKE_DL_LIBS}
# )
# 
# add_test(NAME system_database_test 
#          COMMAND system_database_test)

# 添加REVOKE持久化测试
# add_executable(revoke_persistence_test 
#     unit/revoke_persistence_test.cpp
# )
# 
# target_link_libraries(revoke_persistence_test 
#     sqlcc_executor
#     sqlcc_core_lib
#     sqlcc_parser
#     sqlcc_config_manager
#     sqlcc_storage_engine
#     sqlcc_transaction_manager
#     gtest 
#     gtest_main
#     pthread
#     stdc++fs
#     ${CMAKE_DL_LIBS}
# )
# 
# add_test(NAME revoke_persistence_test 
#          COMMAND revoke_persistence_test)

# 添加SQL-92综合测试
# add_executable(sql_92_comprehensive_test 
#     integration/sql_92_comprehensive_test.cpp
# )
# 
# target_link_libraries(sql_92_comprehensive_test 
#     sqlcc_executor
#     sqlcc_core_lib
#     sqlcc_parser
#     sqlcc_config_manager
#     sqlcc_storage_engine
#     sqlcc_transaction_manager
#     gtest 
#     gtest_main
#     pthread
#     stdc++fs
#     ${CMAKE_DL_LIBS}
# )
# 
# add_test(NAME sql_92_comprehensive_test 
#          COMMAND sql_92_comprehensive_test)

# ================================
# SQLCC v1.1.1 测试框架独立配置  
# ================================

# 使用独立的测试框架配置，避免与CMakeLists_integrated.txt冲突
# 集成测试目标已在主CMakeLists.txt中定义
