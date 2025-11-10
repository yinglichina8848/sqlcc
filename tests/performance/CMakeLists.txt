# 性能测试CMakeLists.txt

# 查找必要的库
find_package(Threads REQUIRED)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

<<<<<<< Updated upstream
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
=======
# 添加子目录
add_subdirectory(concurrency_test)
add_subdirectory(cpu_test)
add_subdirectory(memory_stress_test)
add_subdirectory(stability_test)

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

# 创建性能测试帮助目标
add_custom_target(performance_help
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --help
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Show performance test help"
)

# 创建运行所有性能测试的目标
add_custom_target(run_all_performance_tests
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --all --verbose --output-dir ${CMAKE_BINARY_DIR}/performance_results
>>>>>>> Stashed changes
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run all performance tests with verbose output"
)

<<<<<<< Updated upstream
# 创建运行缓冲池性能测试的目标
add_custom_target(run_buffer_pool_tests
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --buffer-pool --verbose
=======
# 创建运行缓冲池测试的目标
add_custom_target(run_buffer_pool_tests
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --buffer-pool --verbose --output-dir ${CMAKE_BINARY_DIR}/performance_results
>>>>>>> Stashed changes
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run buffer pool performance tests"
)

<<<<<<< Updated upstream
# 创建运行磁盘I/O性能测试的目标
add_custom_target(run_disk_io_tests
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --disk-io --verbose
=======
# 创建运行磁盘I/O测试的目标
add_custom_target(run_disk_io_tests
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --disk-io --verbose --output-dir ${CMAKE_BINARY_DIR}/performance_results
>>>>>>> Stashed changes
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run disk I/O performance tests"
)

# 创建运行混合工作负载性能测试的目标
add_custom_target(run_mixed_workload_tests
<<<<<<< Updated upstream
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --mixed-workload --verbose
=======
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --mixed-workload --verbose --output-dir ${CMAKE_BINARY_DIR}/performance_results
>>>>>>> Stashed changes
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run mixed workload performance tests"
)

# 创建运行批量预取性能测试的目标
add_custom_target(run_batch_prefetch_tests
<<<<<<< Updated upstream
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --batch-prefetch --verbose
=======
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --batch-prefetch --verbose --output-dir ${CMAKE_BINARY_DIR}/performance_results
>>>>>>> Stashed changes
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run batch & prefetch performance tests"
)

# 创建运行100万INSERT性能测试的目标
add_custom_target(run_million_insert_tests
<<<<<<< Updated upstream
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --million-insert --verbose
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run million INSERT performance tests"
)
=======
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --million-insert --verbose --output-dir ${CMAKE_BINARY_DIR}/performance_results
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run million INSERT performance tests"
)

# 创建运行并发测试的目标
add_custom_target(run_concurrency_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/concurrency_test
    DEPENDS concurrency_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run concurrency test"
)

# 创建运行CPU密集测试的目标
add_custom_target(run_cpu_intensive_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/cpu_intensive_test
    DEPENDS cpu_intensive_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run CPU intensive test"
)

# 创建运行内存压力测试的目标
add_custom_target(run_memory_stress_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/memory_stress_test
    DEPENDS memory_stress_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run memory stress test"
)

# 创建运行稳定性测试的目标
add_custom_target(run_stability_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/stability_test
    DEPENDS stability_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run stability test"
)

# 创建性能测试分类目标
# 1. 核心性能测试目标（推荐）
add_custom_target(performance_core
    # 确保依赖已编译
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target concurrency_test
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target cpu_intensive_test
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target memory_stress_test
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target stability_test
    
    # 运行核心测试
    COMMAND echo "=== 运行核心性能测试 ==="
    COMMAND echo "1. 并发性能测试..."
    COMMAND ${CMAKE_BINARY_DIR}/bin/concurrency_test
    COMMAND echo "2. CPU密集型测试..."
    COMMAND ${CMAKE_BINARY_DIR}/bin/cpu_intensive_test
    COMMAND echo "3. 内存压力测试..."
    COMMAND ${CMAKE_BINARY_DIR}/bin/memory_stress_test
    COMMAND echo "4. 稳定性测试..."
    COMMAND ${CMAKE_BINARY_DIR}/bin/stability_test
    COMMAND echo "=== 核心性能测试完成 ==="
    
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run core performance tests: concurrency, CPU, memory, and stability"
)

# 2. 综合性能测试目标（包含所有性能测试）
add_custom_target(performance_all
    # 编译所有依赖
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target concurrency_test
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target cpu_intensive_test
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target memory_stress_test
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target stability_test
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target performance_test
    
    # 运行所有测试
    COMMAND echo "=== 开始运行所有性能测试 ==="
    COMMAND echo "1. 并发性能测试..."
    COMMAND ${CMAKE_BINARY_DIR}/bin/concurrency_test
    COMMAND echo "2. CPU密集型测试..."
    COMMAND ${CMAKE_BINARY_DIR}/bin/cpu_intensive_test
    COMMAND echo "3. 内存压力测试..."
    COMMAND ${CMAKE_BINARY_DIR}/bin/memory_stress_test
    COMMAND echo "4. 稳定性测试..."
    COMMAND ${CMAKE_BINARY_DIR}/bin/stability_test
    COMMAND echo "5. 综合性能测试..."
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --all --verbose --output-dir ${CMAKE_BINARY_DIR}/performance_results
    COMMAND echo "=== 所有性能测试完成 ==="
    
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run all performance tests including detailed benchmarks"
)

# 3. 保持向后兼容的旧目标名称
add_custom_target(run_performance_suite DEPENDS performance_core)
add_custom_target(run_all_tests DEPENDS performance_all)
>>>>>>> Stashed changes
