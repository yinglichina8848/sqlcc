# SQLCC v1.1.1 项目根CMakeLists.txt - 集成测试框架版本
# 生成时间: 2025-12-08 04:53:08

# 最低CMake版本要求
cmake_minimum_required(VERSION 3.10)
project(SQLCC)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用测试
enable_testing()

# 查找必要的依赖
find_package(Threads REQUIRED)
find_package(GTest QUIET)

# 如果没有找到GTest，尝试获取
if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    set(GTest_FOUND TRUE)
endif()

# 如果找到GTest
if(GTest_FOUND)
    include_directories(${GTEST_INCLUDE_DIRS})
    message(STATUS "Found Google Test: ${GTEST_VERSION}")
endif()

# 基本包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/network
    ${CMAKE_SOURCE_DIR}/include/sql
    ${CMAKE_SOURCE_DIR}/include/utils
)

# 包含主要的源码目录
add_subdirectory(${CMAKE_SOURCE_DIR}/src)

# 包含tests目录以启用测试框架
# add_subdirectory(${CMAKE_SOURCE_DIR}/tests)  # 暂时禁用测试，因为缺少测试文件

# ===== 集成测试套件定义 =====

# 核心测试套件 - 包含所有基本测试
add_custom_target(core_suite
    COMMAND echo "=== 运行核心测试套件 ==="
    COMMAND ctest -R "^(test_revoke_persistence|test_grant_revoke|comprehensive_dcl_test|dcl_test|dcl_test_advanced|test_show_grants)$" --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "运行核心DCL功能测试"
)

# 快速测试套件 - 包含基础测试
add_custom_target(quick_suite
    COMMAND echo "=== 运行快速测试套件 ==="
    COMMAND ctest -R "^(basic|lexer)" --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "运行快速基础测试"
)

# 集成测试套件 - 包含高级和集成测试
add_custom_target(integration_suite
    COMMAND echo "=== 运行集成测试套件 ==="
    COMMAND ctest -R "^(advanced|integration|executor)" --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "运行集成和高级测试"
)

# 完整测试套件 - 运行所有测试
add_custom_target(full_suite
    COMMAND echo "=== 运行完整测试套件 ==="
    COMMAND ctest --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "运行所有集成测试"
)

# 并行测试套件 - 支持并行执行
add_custom_target(parallel_suite
    COMMAND echo "=== 运行并行测试套件 ==="
    COMMAND ctest -j4 --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "并行运行测试以提高效率"
)

# ===== 测试统计和报告 =====

# 生成测试统计信息
add_custom_target(test_stats
    COMMAND echo "=== 测试统计信息 ==="
    COMMAND ctest -N
    COMMAND echo "=== 统计完成 ==="
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "显示所有可用测试的统计信息"
)

# 生成测试报告
add_custom_target(test_report
    COMMAND echo "=== 生成测试报告 ==="
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/test_reports
    COMMAND ctest -o test_reports/results.xml
    COMMAND echo "测试报告已生成到 test_reports/ 目录"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "生成详细测试报告"
)

# ===== 验证和诊断 =====

# 验证测试环境
add_custom_target(verify_environment
    COMMAND echo "=== 验证测试环境 ==="
    COMMAND echo "CMake版本: ${CMAKE_VERSION}"
    COMMAND echo "C++编译器: ${CMAKE_CXX_COMPILER}"
    COMMAND echo "构建类型: ${CMAKE_BUILD_TYPE}"
    COMMAND echo "项目根目录: ${CMAKE_SOURCE_DIR}"
    COMMAND echo "构建目录: ${CMAKE_BINARY_DIR}"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "验证测试环境的配置"
)

# 诊断测试问题
add_custom_target(diagnose_tests
    COMMAND echo "=== 诊断测试问题 ==="
    COMMAND echo "可用测试列表:"
    COMMAND ctest -N
    COMMAND echo ""
    COMMAND echo "测试配置检查:"
    COMMAND echo "测试目录存在: $([ -d tests ] && echo '是' || echo '否')"
    COMMAND echo "测试文件统计: $(find tests -name '*.cpp' | wc -l) 个文件"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "诊断测试配置和问题"
)

# ===== 集成命令 =====

# 一键测试 - 运行核心测试套件
add_custom_target(test_core
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target core_suite
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "一键运行核心功能测试"
)

# 快速验证 - 运行快速测试套件
add_custom_target(test_quick
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target quick_suite
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "快速验证基本功能"
)

# 完整验证 - 运行完整测试套件
add_custom_target(test_full
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target full_suite
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "完整功能验证"
)

# 性能测试 - 运行性能测试套件
add_custom_target(test_performance
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target integration_suite
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "性能和集成测试"
)

# 默认目标 - 运行核心测试套件
add_custom_target(run_tests
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target core_suite
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "默认测试目标 - 运行核心测试套件"
)