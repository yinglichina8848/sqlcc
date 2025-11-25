cmake_minimum_required(VERSION 3.10)
project(sqlcc_cpu_intensive_test)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)

# 查找线程库
find_package(Threads REQUIRED)

# 源文件
set(SOURCES
    cpu_intensive_test_runner.cc
    cpu_intensive_performance_test.cc
    ../performance_test_base.cc
)

# 创建可执行文件
add_executable(cpu_intensive_test ${SOURCES})

# 链接所需库
target_link_libraries(cpu_intensive_test
    PRIVATE
    sqlcc_executor
    sqlcc_core
    sqlcc_config_manager
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

# 设置编译选项
target_compile_options(cpu_intensive_test PRIVATE -O2 -Wall -Wextra)

# 设置输出目录
set_target_properties(cpu_intensive_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)