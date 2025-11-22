include_directories(${CMAKE_SOURCE_DIR}/include)

# 定义测试源文件列表
set(TEST_SOURCES
    storage_engine_test.cc
    buffer_pool_test.cc
    storage_engine_newpage_test.cc
    disk_manager_test.cc
    deadlock_fix_test.cc
    lock_timeout_test.cc
    sql_executor/constraint_executor_test.cpp
    sql_executor/record_test.cpp
    sql_executor/sql_executor_test.cpp
    sql_parser/sql_parser_test.cpp
    sql_parser/test_parser.cpp
)

# 定义增强测试源文件列表
set(ENHANCED_TEST_SOURCES
    unit/b_plus_tree_core_test.cc
    unit/config_manager_test.cc
    unit/config_manager_enhanced_test.cc
    unit/page_enhanced_test.cc
    unit/disk_manager_enhanced_test.cc
    unit/storage_engine_enhanced_test.cc
    unit/sql_parser_comprehensive_test.cc
    unit/transaction_manager_test.cc
    unit/transaction_functional_test.cc
    unit/network_test.cc
)

# Add unit tests
set(UNIT_TESTS
    storage_engine_test.cc
    buffer_pool_test.cc
    storage_engine_newpage_test.cc
    disk_manager_test.cc
    deadlock_fix_test.cc
    lock_timeout_test.cc
    unit/b_plus_tree_test.cc
    unit/b_plus_tree_core_test.cc
    unit/b_plus_tree_performance_test.cc
    unit/buffer_pool_new_test.cc
    unit/config_manager_test.cc
    unit/config_manager_enhanced_test.cc
    unit/disk_manager_enhanced_test.cc
    unit/page_enhanced_test.cc
    unit/storage_engine_enhanced_test.cc
    unit/transaction_functional_test.cc
    unit/transaction_manager_test.cc
    unit/ast_nodes_comprehensive_test.cc
    unit/sql_parser_comprehensive_test.cc
    unit/network_test.cc
    unit/network_integration_test.cc
    sql_executor/constraint_executor_test.cpp
    sql_executor/record_test.cpp
    sql_executor/sql_executor_test.cpp
    sql_parser/sql_parser_test.cpp
    sql_parser/test_parser.cpp
)

# Create test executables
foreach(TEST_FILE ${UNIT_TESTS})
    # Get the name of the test without the path and extension
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    
    # Create the executable
    add_executable(${TEST_NAME} ${TEST_FILE})
    
    # Link with required libraries
    target_link_libraries(${TEST_NAME} 
        PRIVATE 
        gtest 
        gtest_main 
        sqlcc_core
        sqlcc_parser
        sqlcc_executor
        pthread
        ${CMAKE_DL_LIBS}
    )
    
    # Add test to CTest
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

# Add performance tests
add_subdirectory(performance)
