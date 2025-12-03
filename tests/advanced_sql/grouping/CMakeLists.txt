# Grouping and aggregation tests CMakeLists.txt

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/sql
    ${CMAKE_SOURCE_DIR}/include/utils
)

# HAVING clause test
add_executable(having_clause_test having_clause_test.cpp)
target_link_libraries(having_clause_test 
    sqlcc_parser
    sqlcc_core_lib
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
    gtest 
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
)