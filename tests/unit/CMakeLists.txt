# Unit tests CMakeLists.txt

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/network
    ${CMAKE_SOURCE_DIR}/include/sql
    ${CMAKE_SOURCE_DIR}/include/utils
)

# Add subdirectories
add_subdirectory(parser)
add_subdirectory(executor)
add_subdirectory(storage)
add_subdirectory(transaction)
add_subdirectory(config)
add_subdirectory(core)
add_subdirectory(network)

# Google Test Basic Test
add_executable(test_gtest test_gtest.cpp)
target_link_libraries(test_gtest
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
)