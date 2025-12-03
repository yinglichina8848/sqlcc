# Advanced SQL integration tests CMakeLists.txt

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/network
    ${CMAKE_SOURCE_DIR}/include/sql
    ${CMAKE_SOURCE_DIR}/include/utils
)

# Advanced SQL Integration Tests
add_executable(comprehensive_test comprehensive_test.cpp)
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

add_executable(isql_integration_test isql_integration_test.cpp)
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

add_executable(sql_executor_integration_test sql_executor_integration_test.cpp)
target_link_libraries(sql_executor_integration_test
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