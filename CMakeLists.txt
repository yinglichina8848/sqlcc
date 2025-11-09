cmake_minimum_required(VERSION 3.16)
project(sqlcc LANGUAGES C CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 编译选项
add_compile_options(-Wall -Wextra -pedantic)

# 检查是否启用覆盖率测试
option(ENABLE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_COVERAGE)
    # 添加覆盖率编译选项
    add_compile_options(-fprofile-arcs -ftest-coverage)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
endif()

# 添加源码目录
add_subdirectory(src)

# 查找Google Test库（可选）
find_package(GTest QUIET)

# 如果找到GTest，则添加测试目录
if(GTest_FOUND)
    enable_testing()
    add_subdirectory(tests)
else()
    message(WARNING "GTest not found. Tests will not be built.")
endif()

# 检查是否启用性能测试
option(ENABLE_PERFORMANCE_TESTS "Enable performance tests" OFF)

# 如果启用性能测试，则添加性能测试目录
if(ENABLE_PERFORMANCE_TESTS)
    add_subdirectory(tests/performance)
endif()