cmake_minimum_required(VERSION 3.10)
project(SqlCC VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# 编译选项
add_compile_options(-Wall -Wextra -Werror -Wno-unused-parameter)

# 查找所需的包
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(GTest REQUIRED)

# 查找Readline库（可选）
pkg_check_modules(READLINE readline)
if(READLINE_FOUND)
    add_definitions(-DHAVE_READLINE)
else()
    message(WARNING "Readline library not found, using basic command line input")
endif()

# 包含目录
include_directories(include)
include_directories(include/config_manager)
include_directories(include/sql_parser)
include_directories(include/storage_engine)
include_directories(include/transaction_manager)
include_directories(include/sql_executor)
include_directories(include/network)

# 添加子目录
add_subdirectory(src/config_manager)
add_subdirectory(src/sql_parser)
add_subdirectory(src/storage_engine)
add_subdirectory(src/transaction_manager)
add_subdirectory(src/sql_executor)

# 网络模块暂时禁用
# add_subdirectory(src/network)

add_subdirectory(tests)

# 添加主可执行文件
add_executable(sqlcc src/main.cc)
target_link_libraries(sqlcc 
    sqlcc_config_manager
    sqlcc_parser
    sqlcc_storage_engine
    sqlcc_transaction_manager
)

# 添加isql可执行文件
add_executable(isql src/isql_main.cc)
target_link_libraries(isql 
    sqlcc_config_manager
    sqlcc_parser
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)

# 添加编译选项
target_compile_options(sqlcc PRIVATE -Wall -Wextra -Werror -Wno-unused-parameter)
target_compile_options(isql PRIVATE -Wall -Wextra -Werror -Wno-unused-parameter)

# 根据构建类型设置优化选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -Werror)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -DNDEBUG -Werror)
elseif(CMAKE_BUILD_TYPE STREQUAL "Coverage")
    add_compile_options(-g -O0 --coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
    set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
    set(CMAKE_C_OUTPUT_EXTENSION_REPLACE ON)
    message(STATUS "启用代码覆盖率编译选项")
else()
    add_compile_options(-Werror)
endif()
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

# 定义主要的库目标，方便其他项目引用
add_library(sqlcc INTERFACE)
target_link_libraries(sqlcc INTERFACE 
    sqlcc_core
    sqlcc_parser
    sqlcc_executor
    sqlcc_storage_engine
    sqlcc_config_manager
    sqlcc_transaction_manager
    sqlcc_network
)

# 查找Google Test库
find_package(GTest)

# 如果找到GTest，则添加测试目录
if(GTest_FOUND)
    enable_testing()
    add_subdirectory(tests)
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
add_custom_target(coverage
    COMMAND echo "=== 配置覆盖率构建 ==="
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Coverage -DENABLE_COVERAGE=ON ${CMAKE_SOURCE_DIR}
    COMMAND echo "=== 构建覆盖率版本 ==="
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --parallel
    COMMAND echo "=== 运行覆盖率测试 ==="
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test
    COMMAND echo "=== 生成覆盖率报告 ==="
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
    COMMAND gcovr --root ${CMAKE_SOURCE_DIR} --html --html-details 
            -o ${CMAKE_BINARY_DIR}/coverage/index.html
            --exclude ${CMAKE_SOURCE_DIR}/tests/ --exclude ${CMAKE_SOURCE_DIR}/build/
            --merge-mode-functions=merge-use-line-0
            --object-directory ${CMAKE_BINARY_DIR} --gcov-object-directory ${CMAKE_BINARY_DIR}
    COMMAND echo "=== 清理临时gcov文件 ==="
    COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcov" -delete
    COMMAND echo "=== 覆盖率测试完成，报告已生成 ==="
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Configure, build and run tests with code coverage"
)

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
