cmake_minimum_required(VERSION 3.16)
project(sqlcc LANGUAGES C CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 基础编译选项
add_compile_options(-Wall -Wextra -pedantic)

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
