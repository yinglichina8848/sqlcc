# Security tests CMakeLists.txt

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/network
    ${CMAKE_SOURCE_DIR}/include/sql
    ${CMAKE_SOURCE_DIR}/include/utils
)

# TODO: Add security test executables when implemented
# add_executable(encryption_test encryption_test.cpp)
# target_link_libraries(encryption_test
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
#     sqlcc_network
# )