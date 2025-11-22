# 性能测试CMake配置

# 启用测试
enable_testing()

# 查找Google Test库
find_package(GTest REQUIRED)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 添加子目录
add_subdirectory(concurrency_test)
add_subdirectory(cpu_test)
add_subdirectory(memory_stress_test)
add_subdirectory(stability_test)

# Add performance tests
set(PERFORMANCE_TESTS
    performance_test.cc
    buffer_pool_performance_test.cc
    disk_io_performance_test.cc
    index_performance_test.cc
    mixed_workload_test.cc
    million_insert_test.cc
    cpu_test/cpu_intensive_performance_test.cc
    concurrency_test/concurrency_performance_test.cc
    large_scale_index_constraint_test.cc
    index_constraint_benchmark.cc
    batch_prefetch_performance_test.cc
    network_performance_test.cc  # 添加网络性能测试
    performance_test_base.cc
)

# 创建性能测试可执行文件
add_executable(performance_test ${PERFORMANCE_TESTS})

# 链接必要的库
target_link_libraries(performance_test 
    sqlcc_core
    sqlcc_parser
    sqlcc_executor
    sqlcc_storage_engine
    sqlcc_config_manager
    sqlcc_transaction_manager
    sqlcc_network
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

# 确保性能结果目录在build目录中
add_custom_target(ensure_performance_results_dir
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/performance_results
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Ensure performance results directory exists"
)

# 创建运行所有性能测试的目标
add_custom_target(run_all_performance_tests
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --all --verbose --output-dir ${CMAKE_BINARY_DIR}/performance_results
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run all performance tests with verbose output"
)

# 创建运行缓冲池测试的目标
add_custom_target(run_buffer_pool_tests
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --buffer-pool --verbose --output-dir ${CMAKE_BINARY_DIR}/performance_results
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run buffer pool performance tests"
)

# 创建运行磁盘I/O测试的目标
add_custom_target(run_disk_io_tests
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --disk-io --verbose --output-dir ${CMAKE_BINARY_DIR}/performance_results
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run disk I/O performance tests"
)

# 创建运行混合工作负载性能测试的目标
add_custom_target(run_mixed_workload_tests
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --mixed-workload --verbose --output-dir ${CMAKE_BINARY_DIR}/performance_results
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run mixed workload performance tests"
)

# 创建运行批量预取性能测试的目标
add_custom_target(run_batch_prefetch_tests
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --batch-prefetch --verbose --output-dir ${CMAKE_BINARY_DIR}/performance_results
    DEPENDS performance_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run batch & prefetch performance tests"
)

# 创建运行100万INSERT性能测试的目标
add_custom_target(run_million_insert_tests
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
    DEPENDS performance_test concurrency_test cpu_intensive_test memory_stress_test stability_test
    
    # 运行核心性能测试
    COMMAND ${CMAKE_COMMAND} -E echo "=== Running Core Performance Tests ==="
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --buffer-pool --disk-io --mixed-workload --output-dir ${CMAKE_BINARY_DIR}/performance_results
    COMMAND ${CMAKE_BINARY_DIR}/bin/concurrency_test --output-dir ${CMAKE_BINARY_DIR}/performance_results
    COMMAND ${CMAKE_BINARY_DIR}/bin/cpu_intensive_test --output-dir ${CMAKE_BINARY_DIR}/performance_results
    COMMAND ${CMAKE_BINARY_DIR}/bin/memory_stress_test --output-dir ${CMAKE_BINARY_DIR}/performance_results
    COMMAND ${CMAKE_BINARY_DIR}/bin/stability_test --output-dir ${CMAKE_BINARY_DIR}/performance_results
    
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run core performance tests (recommended)"
)

# 2. 全面性能测试目标（包含所有测试）
add_custom_target(performance_full
    # 确保依赖已编译
    DEPENDS performance_test concurrency_test cpu_intensive_test memory_stress_test stability_test
    
    # 运行所有性能测试
    COMMAND ${CMAKE_COMMAND} -E echo "=== Running Full Performance Test Suite ==="
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --all --verbose --output-dir ${CMAKE_BINARY_DIR}/performance_results
    COMMAND ${CMAKE_BINARY_DIR}/bin/concurrency_test --verbose --output-dir ${CMAKE_BINARY_DIR}/performance_results
    COMMAND ${CMAKE_BINARY_DIR}/bin/cpu_intensive_test --verbose --output-dir ${CMAKE_BINARY_DIR}/performance_results
    COMMAND ${CMAKE_BINARY_DIR}/bin/memory_stress_test --verbose --output-dir ${CMAKE_BINARY_DIR}/performance_results
    COMMAND ${CMAKE_BINARY_DIR}/bin/stability_test --verbose --output-dir ${CMAKE_BINARY_DIR}/performance_results
    
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run full performance test suite"
)

# 3. 快速性能测试目标（只运行核心测试，时间较短）
add_custom_target(performance_quick
    # 确保依赖已编译
    DEPENDS performance_test
    
    # 运行快速性能测试
    COMMAND ${CMAKE_COMMAND} -E echo "=== Running Quick Performance Tests ==="
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --buffer-pool --disk-io --output-dir ${CMAKE_BINARY_DIR}/performance_results
    
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run quick performance tests"
)

# 4. 压力测试目标（重点测试稳定性和性能极限）
add_custom_target(performance_stress
    # 确保依赖已编译
    DEPENDS performance_test memory_stress_test stability_test
    
    # 运行压力测试
    COMMAND ${CMAKE_COMMAND} -E echo "=== Running Stress Tests ==="
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --million-insert --mixed-workload --batch-prefetch --output-dir ${CMAKE_BINARY_DIR}/performance_results
    COMMAND ${CMAKE_BINARY_DIR}/bin/memory_stress_test --stress-mode --output-dir ${CMAKE_BINARY_DIR}/performance_results
    COMMAND ${CMAKE_BINARY_DIR}/bin/stability_test --stress-mode --output-dir ${CMAKE_BINARY_DIR}/performance_results
    
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run stress tests"
)

# 5. 基准测试目标（生成基准性能数据）
add_custom_target(performance_benchmark
    # 确保依赖已编译
    DEPENDS performance_test
    
    # 运行基准测试
    COMMAND ${CMAKE_COMMAND} -E echo "=== Running Benchmark Tests ==="
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --all --benchmark-mode --output-dir ${CMAKE_BINARY_DIR}/performance_results
    
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run benchmark tests"
)

# 创建清理性能测试结果的目标
add_custom_target(clean_performance_results
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/performance_results
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/performance_results
    COMMENT "Clean performance test results"
)

# 创建查看性能测试结果的目标
add_custom_target(view_performance_results
    COMMAND ${CMAKE_COMMAND} -E echo "Performance test results location: ${CMAKE_BINARY_DIR}/performance_results"
    COMMAND ${CMAKE_COMMAND} -E ls ${CMAKE_BINARY_DIR}/performance_results
    COMMENT "View performance test results"
)

# 添加大规模索引约束性能测试 - 暂时禁用编译
# add_executable(large_scale_index_constraint_test large_scale_index_constraint_test.cc)
# target_link_libraries(large_scale_index_constraint_test
#     GTest::gtest
#     GTest::gtest_main
#     sqlcc_executor
#     sqlcc_parser
#     sqlcc_core
#     sqlcc_storage_engine
#     sqlcc_transaction_manager
#     Threads::Threads
# )
# 
# # 设置大规模性能测试编译选项
# target_compile_options(large_scale_index_constraint_test PRIVATE
#     $<$<CONFIG:Debug>:-O0 -g>
#     $<$<CONFIG:Release>:-O3 -DNDEBUG>
# )
# 
# # 设置输出目录
# set_target_properties(large_scale_index_constraint_test PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
# )

# 创建大规模索引约束性能测试目标 - 暂时禁用
add_custom_target(run_large_scale_test
    # 暂时禁用对不存在目标的依赖
    # COMMAND ${CMAKE_BINARY_DIR}/bin/large_scale_index_constraint_test
    # DEPENDS large_scale_index_constraint_test
    COMMAND ${CMAKE_COMMAND} -E echo "=== Large Scale Index & Constraint Tests Disabled ==="
    COMMAND ${CMAKE_COMMAND} -E echo "These tests are temporarily disabled for build purposes."
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Large scale index and constraint tests temporarily disabled"
)

# 创建索引基准测试目标 - 暂时禁用
add_custom_target(run_index_benchmark
    # 暂时禁用对不存在目标的依赖
    # COMMAND ${CMAKE_BINARY_DIR}/bin/large_scale_index_constraint_test --gtest_filter=LargeScaleIndexConstraintTest.IndexPerformance100KRecords
    # DEPENDS large_scale_index_constraint_test
    COMMAND ${CMAKE_COMMAND} -E echo "=== Index Benchmark Tests Disabled ==="
    COMMAND ${CMAKE_COMMAND} -E echo "These tests are temporarily disabled for build purposes."
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Index benchmark test temporarily disabled"
)

# 创建约束基准测试目标 - 暂时禁用
add_custom_target(run_constraint_benchmark
    # 暂时禁用对不存在目标的依赖
    # COMMAND ${CMAKE_BINARY_DIR}/bin/large_scale_index_constraint_test --gtest_filter=LargeScaleIndexConstraintTest.ConstraintValidationPerformance
    # DEPENDS large_scale_index_constraint_test
    COMMAND ${CMAKE_COMMAND} -E echo "=== Constraint Benchmark Tests Disabled ==="
    COMMAND ${CMAKE_COMMAND} -E echo "These tests are temporarily disabled for build purposes."
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Constraint validation benchmark test temporarily disabled"
)

# 创建主要的性能测试套件目标（兼容性目标）
add_custom_target(run_performance_suite
    # 确保依赖已编译
    DEPENDS performance_test

    # 运行主要的性能测试套件
    COMMAND ${CMAKE_COMMAND} -E echo "=== Running SQLCC Performance Test Suite ==="
    COMMAND ${CMAKE_BINARY_DIR}/bin/performance_test --all --output-dir ${CMAKE_BINARY_DIR}/performance_results
    # 暂时禁用大规模索引约束测试
    # COMMAND ${CMAKE_COMMAND} -E echo "=== Running Large Scale Index & Constraint Tests ==="
    # COMMAND ${CMAKE_BINARY_DIR}/bin/large_scale_index_constraint_test

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run comprehensive performance test suite (large scale tests temporarily disabled)"
)
