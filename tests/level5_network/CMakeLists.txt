# tests/level6_network CMakeLists.txt
# 自动生成于 2026年 01月 12日 星期一 19:49:33 CST

# 包含上级目录的CMake设置
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests/framework
)

# 添加gtest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# 子目录测试
# 注意: 请根据实际测试文件添加executable和测试定义

