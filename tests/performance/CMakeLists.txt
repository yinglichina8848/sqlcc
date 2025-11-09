# 性能测试CMakeLists.txt

# 查找必要的库
find_package(Threads REQUIRED)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 创建性能测试可执行文件
add_executable(performance_test 
    performance_test.cc
    performance_test_base.cc
    buffer_pool_performance_test.cc
    disk_io_performance_test.cc
    mixed_workload_test.cc
    batch_prefetch_performance_test.cc
    million_insert_test.cc
)

# 链接必要的库
target_link_libraries(performance_test 
    sqlcc_core
    Threads::Threads
)

# 设置编译选项
target_compile_options(performance_test PRIVATE
    $<$<CONFIG:Debug>:-O0 -g>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# 设置输出目录
set_target_properties(performance_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 创建性能测试目标
add_custom_target(performance
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --help
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run performance tests"
)

# 创建运行所有性能测试的目标
add_custom_target(run_all_performance_tests
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --all --verbose
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run all performance tests with verbose output"
)

# 创建运行缓冲池性能测试的目标
add_custom_target(run_buffer_pool_tests
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --buffer-pool --verbose
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run buffer pool performance tests"
)

# 创建运行磁盘I/O性能测试的目标
add_custom_target(run_disk_io_tests
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --disk-io --verbose
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run disk I/O performance tests"
)

# 创建运行混合工作负载性能测试的目标
add_custom_target(run_mixed_workload_tests
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --mixed-workload --verbose
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run mixed workload performance tests"
)

# 创建运行批量预取性能测试的目标
add_custom_target(run_batch_prefetch_tests
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --batch-prefetch --verbose
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run batch & prefetch performance tests"
)

# 创建运行100万INSERT性能测试的目标
add_custom_target(run_million_insert_tests
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --million-insert --verbose
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run million INSERT performance tests"
)