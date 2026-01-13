
# execution_context_test.cpp
add_executable(execution_context_test execution_context_test.cpp)
target_link_libraries(execution_context_test
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
add_test(NAME execution_context_test COMMAND execution_context_test)
