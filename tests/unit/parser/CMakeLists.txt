# Parser unit tests CMakeLists.txt

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/sql
    ${CMAKE_SOURCE_DIR}/include/utils
)

# Parser Tests
add_executable(dcl_parser_test dcl_parser_test.cpp)
target_link_libraries(dcl_parser_test 
    sqlcc_parser
    gtest 
    gtest_main
)

add_executable(parser_verification_test parser_verification_test.cpp)
target_link_libraries(parser_verification_test 
    sqlcc_parser
    sqlcc_core_lib
)