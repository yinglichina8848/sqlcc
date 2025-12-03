# Test framework CMakeLists.txt

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/network
    ${CMAKE_SOURCE_DIR}/include/sql
    ${CMAKE_SOURCE_DIR}/include/utils
)

# TODO: Add test framework utilities and tools
# add_library(test_framework test_framework.cpp)
# target_link_libraries(test_framework
#     gtest
#     gtest_main
#     pthread
#     ${CMAKE_DL_LIBS}
# )