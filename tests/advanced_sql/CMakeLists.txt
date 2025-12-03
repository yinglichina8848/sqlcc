# Advanced SQL tests CMakeLists.txt

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/network
    ${CMAKE_SOURCE_DIR}/include/sql
    ${CMAKE_SOURCE_DIR}/include/utils
)

# Add subdirectories
add_subdirectory(join)
add_subdirectory(subquery)
add_subdirectory(window)
add_subdirectory(grouping)
add_subdirectory(set_operation)

# HAVING clause test is in grouping subdirectory
# add_executable(having_clause_test having_clause_test.cpp)
# target_link_libraries(having_clause_test 
#     sqlcc_parser
#     sqlcc_core_lib
#     sqlcc_config_manager
#     sqlcc_storage_engine
#     sqlcc_transaction_manager
#     sqlcc_executor
#     gtest 
#     gtest_main
#     pthread
#     ${CMAKE_DL_LIBS}
# )