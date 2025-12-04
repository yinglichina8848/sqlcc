# SQLCC项目的根CMakeLists.txt文件

# 最低CMake版本要求
cmake_minimum_required(VERSION 3.10)
project(SQLCC)

# 设置源码和测试目录的路径
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(TESTS_DIR ${CMAKE_SOURCE_DIR}/tests)

# 包含config目录的CMakeLists.txt
include(${CMAKE_SOURCE_DIR}/config/CMakeLists.txt)
