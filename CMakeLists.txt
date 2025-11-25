# SQLCC项目的主CMakeLists.txt文件

# 最低CMake版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(sqlcc VERSION 1.0.0 LANGUAGES CXX)

# 设置编译选项
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 禁止在源目录构建
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Please build in a separate directory: mkdir -p build && cd build && cmake ..")
endif()

# 设置构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter")

# 设置源文件扩展名支持
set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS cpp cc)

# 设置默认源文件扩展名首选项为.cc
set(CMAKE_CXX_CREATE_WIN32_RESOURCE cxx)

# 代码覆盖率支持选项
option(ENABLE_COVERAGE "Enable code coverage testing" ON)

# 查找覆盖率测试工具
find_program(GCov gcov)
find_program(LCOV lcov)
find_program(GENHTML genhtml)

# 根据构建类型和覆盖率选项设置编译标志
if(ENABLE_COVERAGE AND GCov AND LCOV AND GENHTML)
    message(STATUS "启用覆盖率测试支持")
    # 覆盖率测试强制使用Debug模式的编译选项
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    # 确保不使用NDEBUG
    string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
    # 正常的构建类型设置
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
    endif()
    message(WARNING "未找到覆盖率测试工具(gcov/lcov/genhtml)，禁用覆盖率测试")
    set(ENABLE_COVERAGE OFF)
endif()

# 查找GCOVR作为备用工具
find_program(GCOVR gcovr)
if(GCOVR)
    message(STATUS "Found gcovr: ${GCOVR}")
else()
    message(WARNING "gcovr not found, may use lcov for coverage reports instead")
endif()

# 查找必要的依赖
find_package(Threads REQUIRED)
find_package(PkgConfig)

# 尝试查找readline库（可选）
if(PkgConfig_FOUND)
    pkg_check_modules(READLINE QUIET readline)
    if(READLINE_FOUND)
        include_directories(${READLINE_INCLUDE_DIRS})
        link_directories(${READLINE_LIBRARY_DIRS})
        add_definitions(-DHAVE_READLINE)
    else()
        message(WARNING "Readline library not found, using basic command line input")
    endif()
else()
    message(WARNING "PkgConfig not found, cannot search for readline")
endif()

# 设置包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/core)
include_directories(${CMAKE_SOURCE_DIR}/include/network)
include_directories(${CMAKE_SOURCE_DIR}/include/sql)
include_directories(${CMAKE_SOURCE_DIR}/include/utils)

# 添加源码目录（必须先于tests添加，因为tests可能依赖src中的库）
add_subdirectory(src)

# 添加子目录
add_subdirectory(tests)

# 查找Google Test库
find_package(GTest)
if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
endif()

# 启用测试
enable_testing()

# 注意：覆盖率标志已在前面设置

# 添加主可执行文件
add_executable(sqlcc src/main.cpp)
target_link_libraries(sqlcc
    PRIVATE
    sqlcc_core
    sqlcc_executor
    ${READLINE_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

# 添加isql可执行文件
add_executable(isql src/isql_main.cpp)
target_link_libraries(isql
    PRIVATE
    sqlcc_core
    sqlcc_executor
    ${READLINE_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

# 构建DCL测试
add_executable(dcl_test src/dcl_test.cpp)
target_link_libraries(dcl_test sqlcc_executor)

# 构建DDL测试
add_executable(ddl_test src/ddl_test.cpp)
target_link_libraries(ddl_test sqlcc_executor)

# 设置输出目录
set_target_properties(sqlcc isql PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 定义主要的库目标，方便其他项目引用

# 查找Google Test库
find_package(GTest)

# 如果找到GTest，则启用测试
if(GTest_FOUND)
    enable_testing()
else()
    message(WARNING "GTest not found. Tests will not be built.")
endif()

# 检查是否启用性能测试
option(ENABLE_PERFORMANCE_TESTS "Enable performance tests" ON)

# 性能测试目录将在tests/CMakeLists.txt中添加，这里不再重复添加

# 创建单元测试目标
add_custom_target(unit
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run unit tests"
)

# 创建覆盖率测试目标
if(ENABLE_COVERAGE AND GCov AND LCOV AND GENHTML)
    add_custom_target(coverage
        COMMAND echo "=== 运行覆盖率测试 ==="
        COMMAND mkdir -p ${CMAKE_BINARY_DIR}/coverage
        # 先清理旧的覆盖率数据
        COMMAND ${LCOV} --directory ${CMAKE_BINARY_DIR} --zerocounters
        # 运行所有测试
        COMMAND ${CMAKE_BINARY_DIR}/dcl_test || echo "DCL测试失败，但继续生成覆盖率报告"
        COMMAND ${CMAKE_BINARY_DIR}/ddl_test || echo "DDL测试失败，但继续生成覆盖率报告"
        COMMAND ${CMAKE_BINARY_DIR}/dml_test || echo "DML测试失败，但继续生成覆盖率报告"
        COMMAND ${CMAKE_BINARY_DIR}/comprehensive_test || echo "综合测试失败，但继续生成覆盖率报告"
        # 捕获覆盖率数据
        COMMAND ${LCOV} --capture --directory ${CMAKE_BINARY_DIR} --output-file ${CMAKE_BINARY_DIR}/coverage/coverage.info
        # 排除不需要的文件，忽略未使用的模式错误
        COMMAND ${LCOV} --remove ${CMAKE_BINARY_DIR}/coverage/coverage.info "${CMAKE_SOURCE_DIR}/tests/*" "${CMAKE_SOURCE_DIR}/build/*" "*test_*.cpp" --output-file ${CMAKE_BINARY_DIR}/coverage/coverage_clean.info --ignore-errors unused
        # 生成HTML报告
        COMMAND ${GENHTML} --output-directory ${CMAKE_BINARY_DIR}/coverage ${CMAKE_BINARY_DIR}/coverage/coverage_clean.info
        # 显示文本摘要
        COMMAND ${LCOV} --list ${CMAKE_BINARY_DIR}/coverage/coverage_clean.info
        COMMAND echo "=== 覆盖率报告生成完成 ==="
        COMMAND echo "=== HTML报告路径: ${CMAKE_BINARY_DIR}/coverage/index.html ==="
        DEPENDS dcl_test ddl_test dml_test comprehensive_test
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "生成代码覆盖率报告"
    )
endif()

# 创建性能测试目标
add_custom_target(performance
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target run_performance_suite
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run comprehensive performance test suite"
)

# 创建完整的测试套件目标（按分类运行所有测试）
add_custom_target(test_all
    COMMAND echo "=== 运行单元测试 ==="
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test
    COMMAND echo "=== 运行性能测试 ==="
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target run_performance_suite
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run all tests by category: unit tests and performance tests"
)

# 创建清理目标（删除build目录外的所有中间文件）
add_custom_target(clean_external_files
    COMMAND ${CMAKE_COMMAND} -E echo "=== 清理build目录外的中间文件 ==="
    COMMAND ${CMAKE_SOURCE_DIR}/clean_external_files.sh
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Clean all intermediate files outside build directory"
)

# 创建检查目标（检查build目录外的中间文件）
add_custom_target(check_external_files
    COMMAND ${CMAKE_COMMAND} -E echo "=== 检查build目录外的中间文件 ==="
    COMMAND ${CMAKE_SOURCE_DIR}/check_external_files.sh
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Check for intermediate files outside build directory"
)
