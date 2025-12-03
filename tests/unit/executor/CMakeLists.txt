# Executor unit tests CMakeLists.txt

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/sql
    ${CMAKE_SOURCE_DIR}/include/utils
)

# Executor Tests
add_executable(execution_engine_test execution_engine_test.cpp)
target_link_libraries(execution_engine_test
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

add_executable(join_executor_test join_executor_test.cpp)
target_link_libraries(join_executor_test 
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

add_executable(set_operation_test set_operation_test.cpp)
target_link_libraries(set_operation_test 
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

add_executable(subquery_executor_test subquery_executor_test.cpp)
target_link_libraries(subquery_executor_test 
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

add_executable(window_function_executor_test window_function_executor_test.cpp)
target_link_libraries(window_function_executor_test 
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

add_executable(system_database_test system_database_test.cpp)
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

add_executable(unified_executor_test unified_executor_test.cpp)
target_link_libraries(unified_executor_test 
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

add_executable(permission_validation_test permission_validation_test.cpp)
target_link_libraries(permission_validation_test 
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

add_executable(permission_validator_test permission_validator_test.cpp)
target_link_libraries(permission_validator_test 
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