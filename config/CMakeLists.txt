# SQLCC项目的主CMakeLists.txt文件

# 最低CMake版本要求
cmake_minimum_required(VERSION 3.10)
project(SQLCC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找所需的库
find_package(Threads REQUIRED)
find_package(Readline)
find_package(OpenSSL REQUIRED)

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# 选项控制是否使用spdlog
option(USE_SPDLOG "Use spdlog for logging" ON)

# 查找第三方依赖
# 如果系统没有安装spdlog，则使用本地版本
find_package(spdlog QUIET)
if(spdlog_FOUND)
    message(STATUS "Found system spdlog")
    set(USE_SPDLOG ON)
else()
    message(STATUS "spdlog not found, using bundled version")
    include_directories(${CMAKE_SOURCE_DIR}/third_party/spdlog/include)
    set(USE_SPDLOG ON)
endif()

# 检查是否启用JSON支持
option(ENABLE_JSON "Enable JSON support" ON)
if(ENABLE_JSON)
    find_package(nlohmann_json QUIET)
    if(NOT nlohmann_json_FOUND)
        # 如果未在系统中找到nlohmann_json，则使用本地副本
        include_directories(${CMAKE_SOURCE_DIR}/third_party/nlohmann_json/include)
    endif()
    message(STATUS "JSON support enabled")
    add_definitions(-DENABLE_JSON)
else()
    message(STATUS "JSON support disabled")
endif()

# 检查是否启用覆盖率测试支持
option(ENABLE_COVERAGE "Enable coverage testing support" ON)
if(ENABLE_COVERAGE)
    message(STATUS "启用覆盖率测试支持")
    # 启用覆盖率编译选项
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
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
    message(WARNING "覆盖率测试已禁用")
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

# 检查Readline库
if(READLINE_FOUND)
    include_directories(${READLINE_INCLUDE_DIRS})
    message(STATUS "Found Readline library")
    add_definitions(-DHAVE_READLINE)
else()
    message(WARNING "Readline library not found, using basic command line input")
endif()

# 设置包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/core)
include_directories(${CMAKE_SOURCE_DIR}/include/network)
include_directories(${CMAKE_SOURCE_DIR}/include/sql)
include_directories(${CMAKE_SOURCE_DIR}/include/sql_parser)
include_directories(${CMAKE_SOURCE_DIR}/include/storage)
include_directories(${CMAKE_SOURCE_DIR}/include/execution)
include_directories(${CMAKE_SOURCE_DIR}/include/utils)

# 明确设置源码目录和测试目录的关系
# 确保源码和测试代码分离
# 1. 源码放在src目录
# 2. 测试代码放在tests目录

# 添加源码目录（必须先于tests添加，因为tests可能依赖src中的库）
add_subdirectory(${CMAKE_SOURCE_DIR}/src)

# 添加测试目录
add_subdirectory(${CMAKE_SOURCE_DIR}/tests)

# 查找Google Test库
find_package(GTest REQUIRED)

# 确保GTest被正确配置
if(GTest_FOUND)
    include_directories(${GTEST_INCLUDE_DIRS})
    message(STATUS "Found Google Test: ${GTEST_VERSION}")
else()
    message(FATAL_ERROR "Google Test not found, tests cannot be built")
endif()

# 确保源码和测试代码分离的策略
# - 源码文件不会被包含在测试目录中
# - 测试文件不会被包含在源码目录中
# - 测试通过依赖关系访问源码，而不是直接包含源码文件
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
    Threads::Threads
)

# 添加isql可执行文件
add_executable(isql src/bin/isql_main.cpp)
target_link_libraries(isql 
    PRIVATE
    sqlcc_core
    sqlcc_executor
    ${READLINE_LIBRARIES}
    Threads::Threads
)

# 添加加密网络服务器可执行文件
add_executable(sqlcc_server src/sqlcc_server/server_main.cpp)
target_link_libraries(sqlcc_server
    PRIVATE
    sqlcc_network
    sqlcc_executor
    sqlcc_core
    Threads::Threads
    OpenSSL::Crypto
)
set_target_properties(sqlcc_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 添加加密网络客户端可执行文件
add_executable(isql_network src/isql_network/client_main.cpp)
target_link_libraries(isql_network
    PRIVATE
    sqlcc_network
    Threads::Threads
    OpenSSL::Crypto
)
set_target_properties(isql_network PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# B+树调试程序已移除，因为源文件不存在

# 添加测试可执行文件 - 已移动到 tests/legacy/
add_executable(test_table_storage tests/legacy/test_table_storage.cpp)
add_executable(test_db_persistence tests/legacy/test_db_persistence.cpp)
add_executable(simple_db_test tests/legacy/simple_db_test.cpp)
add_executable(pure_fs_test tests/legacy/pure_fs_test.cpp)
add_executable(test_dcl_ddl_persistence tests/legacy/test_dcl_ddl_persistence.cpp)
add_executable(simple_dcl_ddl_test tests/legacy/simple_dcl_ddl_test.cpp)
add_executable(persistence_check tests/legacy/persistence_check.cpp)
add_executable(dcl_ddl_persistence_test tests/legacy/dcl_ddl_persistence_test.cpp)
add_executable(user_persistence_test tests/legacy/user_persistence_test.cpp)

# 链接所需的库
target_link_libraries(test_table_storage 
    sqlcc_core_lib
    sqlcc_storage_engine
    sqlcc_config_manager
    sqlcc_parser
    sqlcc_executor
    sqlcc_transaction_manager
    sqlcc_network
    ${READLINE_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(test_db_persistence 
    sqlcc_core_lib
    sqlcc_storage_engine
    sqlcc_config_manager
    sqlcc_parser
    sqlcc_executor
    sqlcc_transaction_manager
    sqlcc_network
    ${READLINE_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(simple_db_test 
    sqlcc_core_lib
    sqlcc_storage_engine
    sqlcc_config_manager
    sqlcc_parser
    sqlcc_executor
    sqlcc_transaction_manager
    sqlcc_network
    ${READLINE_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(pure_fs_test 
    ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(test_dcl_ddl_persistence 
    sqlcc_core_lib
    sqlcc_storage_engine
    sqlcc_config_manager
    sqlcc_parser
    sqlcc_executor
    sqlcc_transaction_manager
    sqlcc_network
    ${READLINE_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(simple_dcl_ddl_test 
    sqlcc_core_lib
    sqlcc_storage_engine
    sqlcc_config_manager
    sqlcc_parser
    sqlcc_executor
    sqlcc_transaction_manager
    sqlcc_network
    ${READLINE_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(persistence_check 
    ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(dcl_ddl_persistence_test 
    sqlcc_core_lib
    sqlcc_storage_engine
    sqlcc_config_manager
    sqlcc_parser
    sqlcc_executor
    sqlcc_transaction_manager
    sqlcc_network
    ${READLINE_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(user_persistence_test 
    sqlcc_core_lib
    sqlcc_storage_engine
    sqlcc_config_manager
    sqlcc_parser
    sqlcc_executor
    sqlcc_transaction_manager
    sqlcc_network
    ${READLINE_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

# 设置输出目录
set_target_properties(sqlcc isql PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 定义主要的库目标，方便其他项目引用

# 查找Google Test库
find_package(GTest)

# 启用测试
enable_testing()

# 添加GoogleTest
find_package(GTest REQUIRED)
if(GTEST_FOUND)
    message(STATUS "Found Google Test")
endif()

# 检查是否启用性能测试
option(ENABLE_PERFORMANCE_TESTS "Enable performance tests" OFF)

# 性能测试目录将在tests/CMakeLists.txt中添加，这里不再重复添加

# 创建单元测试目标
add_custom_target(unit
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run unit tests"
)

# 创建更详细的测试分类目标
add_custom_target(unit_tests
    COMMAND ctest -R "^test_" --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run all unit tests"
)

add_custom_target(integration_tests
    COMMAND ${CMAKE_BINARY_DIR}/tests/comprehensive_test
    COMMAND ${CMAKE_BINARY_DIR}/tests/isql_integration_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run integration tests"
)

add_custom_target(performance_tests
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR} ${CMAKE_MAKE_PROGRAM} run_performance_suite
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run performance tests"
)

# 统一测试目标 - 一键运行所有测试
add_custom_target(all_tests
    COMMAND echo "=== 运行所有测试 ==="
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target unit_tests
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target integration_tests
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target performance_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run all tests (unit, integration, performance)"
)

# 添加覆盖率报告目标
if(ENABLE_COVERAGE)
    find_program(GCOVR_PATH gcovr)
    if(GCOVR_PATH)
        message(STATUS "Found gcovr: ${GCOVR_PATH}")
        add_custom_target(coverage
            COMMAND ${GCOVR_PATH} --exclude-throw-branches --exclude-unreachable-branches --html-details -o coverage.html .
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating HTML coverage report"
        )
    else()
        message(WARNING "gcovr not found, coverage report generation skipped")
    endif()
endif()

# 创建性能测试目标
add_custom_target(performance
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target run_performance_suite
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run comprehensive performance test suite"
)

# 创建完整的测试套件目标（按分类运行所有测试）
# 重命名以避免冲突
add_custom_target(run_all_tests
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

# 创建统一报告生成目标
add_custom_target(test_report
    COMMAND echo "=== 生成测试报告 ==="
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all_tests
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target coverage
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/test_reports
    # 生成统一HTML报告
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/generate_test_report.py
        --results-dir ${CMAKE_BINARY_DIR}
        --output ${CMAKE_BINARY_DIR}/test_reports/index.html
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generate comprehensive test report"
)
