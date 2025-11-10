/**
 * @file batch_prefetch_performance_test.cc
 * @brief 批量预取性能测试实现文件
 * 
 * Why: 需要测试批量页面读取和预取操作的性能，评估不同批量大小和访问模式下的性能表现
 * What: 实现BatchPrefetchPerformanceTest类，提供多种性能测试场景，包括单页读取、批量读取、预取等
 * How: 使用磁盘管理器和缓冲池模拟实际数据库操作，测量不同场景下的延迟和吞吐量
 */

#include "batch_prefetch_performance_test.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <filesystem>

namespace sqlcc {
namespace test {

<<<<<<< Updated upstream
/**
 * @brief 构造函数
 * 
 * Why: 需要初始化性能测试对象，设置随机数生成器
 * What: BatchPrefetchPerformanceTest构造函数初始化随机数生成器
 * How: 使用std::random_device创建随机数生成器
 */
=======

>>>>>>> Stashed changes
BatchPrefetchPerformanceTest::BatchPrefetchPerformanceTest() 
    : rng_(std::random_device{}()) {
}

<<<<<<< Updated upstream
/**
 * @brief 析构函数
 * 
 * Why: 需要清理测试过程中创建的资源，如测试数据库文件
 * What: BatchPrefetchPerformanceTest析构函数调用Cleanup方法清理资源
 * How: 调用Cleanup方法删除测试数据库文件
 */
=======

>>>>>>> Stashed changes
BatchPrefetchPerformanceTest::~BatchPrefetchPerformanceTest() {
    Cleanup();
}

<<<<<<< Updated upstream
/**
 * @brief 运行所有性能测试
 * 
 * Why: 需要执行所有相关的性能测试，全面评估批量读取和预取的性能
 * What: RunAllTests方法执行所有测试场景，包括单页读取、批量读取、预取等
 * How: 依次调用各个测试方法，并输出测试开始和结束信息
 */
=======

>>>>>>> Stashed changes
void BatchPrefetchPerformanceTest::RunAllTests() {
    std::cout << "\n=====================================" << std::endl;
    std::cout << "Running Batch & Prefetch Performance Tests" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    // 设置测试环境
    // Why: 需要初始化测试环境，包括磁盘管理器、缓冲池和测试数据
    // What: 调用SetupTestEnvironment方法初始化测试环境
    // How: 直接调用SetupTestEnvironment方法
    SetupTestEnvironment();
    
    // 运行各种测试
    // Why: 需要执行所有测试场景，评估不同访问模式的性能
    // What: 依次调用各个测试方法，包括单页读取、批量读取、预取等
    // How: 直接调用各个测试方法
    RunSinglePageReadTest();
    RunBatchPageReadTest();
    RunSinglePagePrefetchTest();
    RunBatchPrefetchTest();
    RunMixedAccessPatternTest();
    RunVaryingBatchSizeTest();
    
    std::cout << "\n=====================================" << std::endl;
    std::cout << "Batch & Prefetch Performance Tests Completed" << std::endl;
    std::cout << "=====================================" << std::endl;
}

/**
 * @brief 清理测试环境
 * 
 * Why: 需要清理测试过程中创建的资源，如测试数据库文件
 * What: Cleanup方法删除测试数据库文件
 * How: 使用filesystem库检查文件是否存在并删除
 */
void BatchPrefetchPerformanceTest::Cleanup() {
    // 清理测试数据库文件
    // Why: 需要删除测试过程中创建的数据库文件
    // What: 检查测试数据库文件是否存在，存在则删除
    // How: 使用filesystem::exists检查文件存在性，使用filesystem::remove删除文件
    if (std::filesystem::exists(test_db_file_)) {
        std::filesystem::remove(test_db_file_);
    }
}

/**
 * @brief 设置测试环境
 * 
 * Why: 需要初始化测试环境，包括磁盘管理器、缓冲池和测试数据
 * What: SetupTestEnvironment方法创建磁盘管理器和缓冲池，初始化测试页面
 * How: 创建磁盘管理器和缓冲池对象，写入测试页面到磁盘
 */
void BatchPrefetchPerformanceTest::SetupTestEnvironment() {
    std::cout << "Setting up test environment..." << std::endl;
    
<<<<<<< Updated upstream
    // 获取配置管理器单例实例
    // Why: 需要配置管理器来管理测试配置
    // What: 获取ConfigManager单例实例
    // How: 调用GetInstance()方法
    sqlcc::ConfigManager& config_manager = sqlcc::ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    // Why: 需要磁盘管理器来管理测试数据库文件
    // What: 创建DiskManager对象，管理测试数据库文件
    // How: 使用make_unique创建DiskManager对象，传入数据库文件名和配置管理器
    disk_manager_ = std::make_unique<sqlcc::DiskManager>(test_db_file_, config_manager);
    
    // 创建缓冲池
    // Why: 需要缓冲池来模拟数据库的页面缓存机制
    // What: 创建BufferPool对象，使用磁盘管理器、缓冲池大小和配置管理器
    // How: 使用make_unique创建BufferPool对象
    buffer_pool_ = std::make_unique<sqlcc::BufferPool>(disk_manager_.get(), pool_size_, config_manager);
=======
    // 创建磁盘管理器（需要ConfigManager参数）
    disk_manager_ = std::make_unique<sqlcc::DiskManager>(test_db_file_, sqlcc::ConfigManager::GetInstance());
    
    // 创建缓冲池（使用配置管理器单例）
    buffer_pool_ = std::make_unique<sqlcc::BufferPool>(disk_manager_.get(), pool_size_, sqlcc::ConfigManager::GetInstance());
>>>>>>> Stashed changes
    
    // 初始化一些页面到磁盘
    // Why: 需要创建测试页面，模拟数据库的实际数据
    // What: 创建指定数量的页面并写入磁盘
    // How: 循环创建页面并调用WritePage方法写入磁盘
    for (int32_t i = 0; i < working_set_size_; ++i) {
        sqlcc::Page page(i);
        disk_manager_->WritePage(i, page.GetData());
    }
    
    std::cout << "Test environment setup completed." << std::endl;
}

<<<<<<< Updated upstream
/**
 * @brief 运行单页读取测试
 * 
 * Why: 需要测试单个页面读取的性能，作为基准测试
 * What: RunSinglePageReadTest方法生成随机访问序列并执行单页读取测试
 * How: 生成随机访问序列，执行单页读取操作，计算性能指标并保存结果
 */
=======

>>>>>>> Stashed changes
void BatchPrefetchPerformanceTest::RunSinglePageReadTest() {
    std::cout << "\nRunning Single Page Read Test..." << std::endl;
    
    std::vector<TestResult> results;
    
    // 生成随机访问序列
    // Why: 需要生成随机访问序列，模拟实际数据库的随机访问模式
    // What: 调用GenerateRandomAccess方法生成随机访问序列
    // How: 传入访问数量和工作集大小参数
    std::vector<int32_t> page_ids;
    GenerateRandomAccess(page_ids, access_count_, working_set_size_);
    
    // 执行测试
    // Why: 需要执行单页读取操作并测量延迟
    // What: 调用ExecuteSinglePageAccesses方法执行单页读取操作
    // How: 传入页面ID序列和延迟向量
    std::vector<double> latencies;
    auto start_time = GetCurrentTime();
    ExecuteSinglePageAccesses(page_ids, latencies);
    auto end_time = GetCurrentTime();
    
    // 计算结果
    // Why: 需要计算测试的性能指标，包括持续时间、吞吐量和延迟
    // What: 计算测试结果并保存到TestResult结构中
    // How: 调用相应的计算方法计算性能指标
    TestResult result;
    result.test_name = "Single Page Read";
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = access_count_;
    result.throughput = CalculateThroughput(access_count_, result.duration);
    
    double avg, p95, p99;
    CalculateLatencies(latencies, avg, p95, p99);
    result.avg_latency = avg;
    result.p95_latency = p95;
    result.p99_latency = p99;
    
    // 打印和保存结果
    // Why: 需要输出测试结果并保存到文件
    // What: 调用PrintResult和SaveResultsToFile方法输出和保存结果
    // How: 直接调用相应方法
    PrintResult(result);
    results.push_back(result);
    SaveResultsToFile(results, "single_page_read.csv");
}

<<<<<<< Updated upstream
/**
 * @brief 运行批量页面读取测试
 * 
 * Why: 需要测试不同批量大小下的批量页面读取性能
 * What: RunBatchPageReadTest方法测试不同批量大小的批量读取性能
 * How: 遍历不同批量大小，生成随机访问序列，执行批量读取操作，计算性能指标
 */
=======

>>>>>>> Stashed changes
void BatchPrefetchPerformanceTest::RunBatchPageReadTest() {
    std::cout << "\nRunning Batch Page Read Test..." << std::endl;
    
    std::vector<TestResult> results;
    
    // 测试不同的批量大小
    // Why: 需要评估不同批量大小对性能的影响
    // What: 遍历预定义的批量大小列表，为每个批量大小执行测试
    // How: 使用范围for循环遍历batch_sizes_列表
    for (size_t batch_size : batch_sizes_) {
        if (batch_size == 1) continue; // 跳过单个页面，因为已经在单页测试中测试过
        
        // 生成随机访问序列
        // Why: 需要生成随机访问序列，模拟实际数据库的随机访问模式
        // What: 调用GenerateRandomAccess方法生成随机访问序列
        // How: 传入访问数量和工作集大小参数
        std::vector<int32_t> page_ids;
        GenerateRandomAccess(page_ids, access_count_, working_set_size_);
        
        // 执行测试
        // Why: 需要执行批量页面读取操作并测量延迟
        // What: 调用ExecuteBatchPageAccesses方法执行批量页面读取操作
        // How: 传入页面ID序列、延迟向量和批量大小
        std::vector<double> latencies;
        auto start_time = GetCurrentTime();
        ExecuteBatchPageAccesses(page_ids, latencies, batch_size);
        auto end_time = GetCurrentTime();
        
        // 计算结果
        // Why: 需要计算测试的性能指标，包括持续时间、吞吐量和延迟
        // What: 计算测试结果并保存到TestResult结构中
        // How: 调用相应的计算方法计算性能指标
        TestResult result;
        result.test_name = "Batch Page Read (size=" + std::to_string(batch_size) + ")";
        result.duration = CalculateDuration(start_time, end_time);
        result.operations_completed = access_count_;
        result.throughput = CalculateThroughput(access_count_, result.duration);
        
        double avg, p95, p99;
        CalculateLatencies(latencies, avg, p95, p99);
        result.avg_latency = avg;
        result.p95_latency = p95;
        result.p99_latency = p99;
        
        result.custom_metrics["batch_size"] = std::to_string(batch_size);
        
        // 打印结果
        // Why: 需要输出测试结果
        // What: 调用PrintResult方法输出结果
        // How: 直接调用PrintResult方法
        PrintResult(result);
        results.push_back(result);
    }
    
    // 保存结果
    // Why: 需要将测试结果保存到文件
    // What: 调用SaveResultsToFile方法保存结果
    // How: 传入结果列表和文件名
    SaveResultsToFile(results, "batch_page_read.csv");
}
<<<<<<< Updated upstream

/**
 * @brief 运行单页预取测试
 * 
 * Why: 需要测试单页预取的性能，评估预取对顺序访问模式的性能提升
 * What: RunSinglePagePrefetchTest方法生成顺序访问序列并执行单页预取测试
 * How: 生成顺序访问序列，执行预取和读取操作，计算性能指标并保存结果
 */
=======
>>>>>>> Stashed changes
void BatchPrefetchPerformanceTest::RunSinglePagePrefetchTest() {
    std::cout << "\nRunning Single Page Prefetch Test..." << std::endl;
    
    std::vector<TestResult> results;
    
    // 生成顺序访问序列（适合预取）
    // Why: 需要生成顺序访问序列，模拟适合预取的访问模式
    // What: 调用GenerateSequentialAccess方法生成顺序访问序列
    // How: 传入访问数量参数
    std::vector<int32_t> page_ids;
    GenerateSequentialAccess(page_ids, access_count_);
    
    // 执行测试
    // Why: 需要执行单页预取操作并测量延迟
    // What: 调用ExecutePrefetchOperations方法执行预取操作
    // How: 传入页面ID序列和延迟向量
    std::vector<double> latencies;
    auto start_time = GetCurrentTime();
    ExecutePrefetchOperations(page_ids, latencies);
    auto end_time = GetCurrentTime();
    
    // 计算结果
    // Why: 需要计算测试的性能指标，包括持续时间、吞吐量和延迟
    // What: 计算测试结果并保存到TestResult结构中
    // How: 调用相应的计算方法计算性能指标
    TestResult result;
    result.test_name = "Single Page Prefetch";
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = access_count_;
    result.throughput = CalculateThroughput(access_count_, result.duration);
    
    double avg, p95, p99;
    CalculateLatencies(latencies, avg, p95, p99);
    result.avg_latency = avg;
    result.p95_latency = p95;
    result.p99_latency = p99;
    
    // 打印和保存结果
    // Why: 需要输出测试结果并保存到文件
    // What: 调用PrintResult和SaveResultsToFile方法输出和保存结果
    // How: 直接调用相应方法
    PrintResult(result);
    results.push_back(result);
    SaveResultsToFile(results, "single_page_prefetch.csv");
}

/**
 * @brief 运行批量预取测试
 * 
 * Why: 需要测试不同批量大小下的批量预取性能
 * What: RunBatchPrefetchTest方法测试不同批量大小的批量预取性能
 * How: 遍历不同批量大小，生成顺序访问序列，执行批量预取操作，计算性能指标
 */
void BatchPrefetchPerformanceTest::RunBatchPrefetchTest() {
    std::cout << "\nRunning Batch Prefetch Test..." << std::endl;
    
    std::vector<TestResult> results;
    
    // 测试不同的批量大小
    // Why: 需要评估不同批量大小对预取性能的影响
    // What: 遍历预定义的批量大小列表，为每个批量大小执行测试
    // How: 使用范围for循环遍历batch_sizes_列表
    for (size_t batch_size : batch_sizes_) {
        if (batch_size == 1) continue; // 跳过单个页面，因为已经在单页测试中测试过
        
        // 生成顺序访问序列（适合预取）
        // Why: 需要生成顺序访问序列，模拟适合预取的访问模式
        // What: 调用GenerateSequentialAccess方法生成顺序访问序列
        // How: 传入访问数量参数
        std::vector<int32_t> page_ids;
        GenerateSequentialAccess(page_ids, access_count_);
        
        // 执行测试
        // Why: 需要执行批量预取操作并测量延迟
        // What: 调用ExecuteBatchPrefetchOperations方法执行批量预取操作
        // How: 传入页面ID序列、延迟向量和批量大小
        std::vector<double> latencies;
        auto start_time = GetCurrentTime();
        ExecuteBatchPrefetchOperations(page_ids, latencies, batch_size);
        auto end_time = GetCurrentTime();
        
        // 计算结果
        // Why: 需要计算测试的性能指标，包括持续时间、吞吐量和延迟
        // What: 计算测试结果并保存到TestResult结构中
        // How: 调用相应的计算方法计算性能指标
        TestResult result;
        result.test_name = "Batch Prefetch (size=" + std::to_string(batch_size) + ")";
        result.duration = CalculateDuration(start_time, end_time);
        result.operations_completed = access_count_;
        result.throughput = CalculateThroughput(access_count_, result.duration);
        
        double avg, p95, p99;
        CalculateLatencies(latencies, avg, p95, p99);
        result.avg_latency = avg;
        result.p95_latency = p95;
        result.p99_latency = p99;
        
        result.custom_metrics["batch_size"] = std::to_string(batch_size);
        
        // 打印结果
        // Why: 需要输出测试结果
        // What: 调用PrintResult方法输出结果
        // How: 直接调用PrintResult方法
        PrintResult(result);
        results.push_back(result);
    }
    
    // 保存结果
    // Why: 需要将测试结果保存到文件
    // What: 调用SaveResultsToFile方法保存结果
    // How: 传入结果列表和文件名
    SaveResultsToFile(results, "batch_prefetch.csv");
}
<<<<<<< Updated upstream

/**
 * @brief 运行混合访问模式测试
 * 
 * Why: 需要测试在具有局部性的混合访问模式下不同方法的性能
 * What: RunMixedAccessPatternTest方法比较单页读取、批量读取和预取在混合访问模式下的性能
 * How: 生成具有局部性的访问序列，分别使用三种方法执行测试，比较性能差异
 */
=======
>>>>>>> Stashed changes
void BatchPrefetchPerformanceTest::RunMixedAccessPatternTest() {
    std::cout << "\nRunning Mixed Access Pattern Test..." << std::endl;
    
    std::vector<TestResult> results;
    
    // 生成具有局部性的访问序列
    // Why: 需要生成具有局部性的访问序列，模拟实际数据库的访问模式
    // What: 调用GenerateLocalityAccess方法生成具有局部性的访问序列
    // How: 传入访问数量、工作集大小和局部性范围参数
    std::vector<int32_t> page_ids;
    GenerateLocalityAccess(page_ids, access_count_, working_set_size_ / 4);
    
    // 测试不同的方法
    // 1. 单个页面读取
    // Why: 需要测试单页读取在混合访问模式下的性能
    // What: 执行单页读取操作并计算性能指标
    // How: 调用ExecuteSinglePageAccesses方法执行测试
    std::vector<double> latencies1;
    auto start_time1 = GetCurrentTime();
    ExecuteSinglePageAccesses(page_ids, latencies1);
    auto end_time1 = GetCurrentTime();
    
    TestResult result1;
    result1.test_name = "Mixed Pattern - Single Page Read";
    result1.duration = CalculateDuration(start_time1, end_time1);
    result1.operations_completed = access_count_;
    result1.throughput = CalculateThroughput(access_count_, result1.duration);
    
    double avg1, p95_1, p99_1;
    CalculateLatencies(latencies1, avg1, p95_1, p99_1);
    result1.avg_latency = avg1;
    result1.p95_latency = p95_1;
    result1.p99_latency = p99_1;
    
    PrintResult(result1);
    results.push_back(result1);
    
    // 2. 批量页面读取（批量大小为8）
    // Why: 需要测试批量读取在混合访问模式下的性能
    // What: 执行批量读取操作并计算性能指标
    // How: 调用ExecuteBatchPageAccesses方法执行测试，批量大小为8
    std::vector<double> latencies2;
    auto start_time2 = GetCurrentTime();
    ExecuteBatchPageAccesses(page_ids, latencies2, 8);
    auto end_time2 = GetCurrentTime();
    
    TestResult result2;
    result2.test_name = "Mixed Pattern - Batch Read (size=8)";
    result2.duration = CalculateDuration(start_time2, end_time2);
    result2.operations_completed = access_count_;
    result2.throughput = CalculateThroughput(access_count_, result2.duration);
    
    double avg2, p95_2, p99_2;
    CalculateLatencies(latencies2, avg2, p95_2, p99_2);
    result2.avg_latency = avg2;
    result2.p95_latency = p95_2;
    result2.p99_latency = p99_2;
    
    result2.custom_metrics["batch_size"] = "8";
    
    PrintResult(result2);
    results.push_back(result2);
    
    // 3. 预取操作
    // Why: 需要测试预取操作在混合访问模式下的性能
    // What: 执行预取操作并计算性能指标
    // How: 调用ExecutePrefetchOperations方法执行测试
    std::vector<double> latencies3;
    auto start_time3 = GetCurrentTime();
    ExecutePrefetchOperations(page_ids, latencies3);
    auto end_time3 = GetCurrentTime();
    
    TestResult result3;
    result3.test_name = "Mixed Pattern - Prefetch";
    result3.duration = CalculateDuration(start_time3, end_time3);
    result3.operations_completed = access_count_;
    result3.throughput = CalculateThroughput(access_count_, result3.duration);
    
    double avg3, p95_3, p99_3;
    CalculateLatencies(latencies3, avg3, p95_3, p99_3);
    result3.avg_latency = avg3;
    result3.p95_latency = p95_3;
    result3.p99_latency = p99_3;
    
    PrintResult(result3);
    results.push_back(result3);
    
    // 保存结果
    // Why: 需要将测试结果保存到文件
    // What: 调用SaveResultsToFile方法保存结果
    // How: 传入结果列表和文件名
    SaveResultsToFile(results, "mixed_access_pattern.csv");
}
<<<<<<< Updated upstream

/**
 * @brief 运行不同批量大小测试
 * 
 * Why: 需要测试不同批量大小对性能的影响，找出最优批量大小
 * What: RunVaryingBatchSizeTest方法测试不同批量大小下的批量读取性能
 * How: 生成随机访问序列，遍历不同批量大小，执行批量读取操作，比较性能差异
 */
=======
>>>>>>> Stashed changes
void BatchPrefetchPerformanceTest::RunVaryingBatchSizeTest() {
    std::cout << "\nRunning Varying Batch Size Test..." << std::endl;
    
    std::vector<TestResult> results;
    
    // 生成随机访问序列
    // Why: 需要生成随机访问序列，模拟实际数据库的随机访问模式
    // What: 调用GenerateRandomAccess方法生成随机访问序列
    // How: 传入访问数量和工作集大小参数
    std::vector<int32_t> page_ids;
    GenerateRandomAccess(page_ids, access_count_, working_set_size_);
    
    // 测试不同的批量大小
    // Why: 需要评估不同批量大小对性能的影响
    // What: 遍历预定义的批量大小列表，为每个批量大小执行测试
    // How: 使用范围for循环遍历batch_sizes_列表
    for (size_t batch_size : batch_sizes_) {
        // 执行测试
        // Why: 需要执行批量页面读取操作并测量延迟
        // What: 调用ExecuteBatchPageAccesses方法执行批量页面读取操作
        // How: 传入页面ID序列、延迟向量和批量大小
        std::vector<double> latencies;
        auto start_time = GetCurrentTime();
        ExecuteBatchPageAccesses(page_ids, latencies, batch_size);
        auto end_time = GetCurrentTime();
        
        // 计算结果
        // Why: 需要计算测试的性能指标，包括持续时间、吞吐量和延迟
        // What: 计算测试结果并保存到TestResult结构中
        // How: 调用相应的计算方法计算性能指标
        TestResult result;
        result.test_name = "Varying Batch Size";
        result.duration = CalculateDuration(start_time, end_time);
        result.operations_completed = access_count_;
        result.throughput = CalculateThroughput(access_count_, result.duration);
        
        double avg, p95, p99;
        CalculateLatencies(latencies, avg, p95, p99);
        result.avg_latency = avg;
        result.p95_latency = p95;
        result.p99_latency = p99;
        
        result.custom_metrics["batch_size"] = std::to_string(batch_size);
        
        // 打印结果
        // Why: 需要输出测试结果
        // What: 调用PrintResult方法输出结果
        // How: 直接调用PrintResult方法
        PrintResult(result);
        results.push_back(result);
    }
    
    // 保存结果
    // Why: 需要将测试结果保存到文件
    // What: 调用SaveResultsToFile方法保存结果
    // How: 传入结果列表和文件名
    SaveResultsToFile(results, "varying_batch_size.csv");
}
<<<<<<< Updated upstream

/**
 * @brief 生成顺序访问序列
 * @param page_ids 输出的页面ID序列
 * @param count 需要生成的页面数量
 * 
 * Why: 需要生成顺序访问序列，模拟适合预取的访问模式
 * What: GenerateSequentialAccess方法生成从0开始的顺序访问序列
 * How: 循环生成页面ID，使用模运算确保ID在工作集范围内
 */
=======
>>>>>>> Stashed changes
void BatchPrefetchPerformanceTest::GenerateSequentialAccess(std::vector<int32_t>& page_ids, size_t count) {
    page_ids.clear();
    page_ids.reserve(count);
    
    // 生成顺序访问序列
    // Why: 需要生成从0开始的顺序访问序列
    // What: 循环生成页面ID，使用模运算确保ID在工作集范围内
    // How: 使用for循环和模运算生成页面ID
    for (size_t i = 0; i < count; ++i) {
        page_ids.push_back(i % working_set_size_);
    }
}

/**
 * @brief 生成随机访问序列
 * @param page_ids 输出的页面ID序列
 * @param count 需要生成的页面数量
 * @param max_page_id 最大页面ID
 * 
 * Why: 需要生成随机访问序列，模拟实际数据库的随机访问模式
 * What: GenerateRandomAccess方法生成指定范围内的随机访问序列
 * How: 使用随机数生成器生成指定范围内的随机页面ID
 */
void BatchPrefetchPerformanceTest::GenerateRandomAccess(std::vector<int32_t>& page_ids, size_t count, size_t max_page_id) {
    page_ids.clear();
    page_ids.reserve(count);
    
    // 创建均匀分布
    // Why: 需要创建均匀分布的随机数生成器
    // What: 使用std::uniform_int_distribution创建均匀分布
    // How: 指定分布范围为0到max_page_id-1
    std::uniform_int_distribution<int32_t> dist(0, max_page_id - 1);
    
    // 生成随机访问序列
    // Why: 需要生成随机页面ID序列
    // What: 使用随机数生成器生成随机页面ID
    // How: 循环调用dist(rng_)生成随机页面ID
    for (size_t i = 0; i < count; ++i) {
        page_ids.push_back(dist(rng_));
    }
}

/**
 * @brief 生成具有局部性的访问序列
 * @param page_ids 输出的页面ID序列
 * @param count 需要生成的页面数量
 * @param working_set 局部性工作集大小
 * 
 * Why: 需要生成具有局部性的访问序列，模拟实际数据库的访问模式
 * What: GenerateLocalityAccess方法生成具有局部性的访问序列
 * How: 将工作集划分为多个区域，随机选择区域并在区域内随机选择页面
 */
void BatchPrefetchPerformanceTest::GenerateLocalityAccess(std::vector<int32_t>& page_ids, size_t count, size_t working_set) {
    page_ids.clear();
    page_ids.reserve(count);
    
    // 创建均匀分布
    // Why: 需要创建均匀分布的随机数生成器
    // What: 使用std::uniform_int_distribution创建两个均匀分布
    // How: 一个用于区域选择，一个用于区域内页面选择
    std::uniform_int_distribution<int32_t> dist(0, working_set - 1);
    std::uniform_int_distribution<int32_t> region_dist(0, (working_set_size_ / working_set) - 1);
    
    // 生成具有局部性的访问序列
    // Why: 需要生成具有局部性的页面ID序列
    // What: 随机选择区域，然后在区域内随机选择页面
    // How: 使用区域分布选择区域，使用页面分布选择页面ID
    for (size_t i = 0; i < count; ++i) {
        int32_t region = region_dist(rng_);
        int32_t offset = dist(rng_);
        page_ids.push_back(region * working_set + offset);
    }
}

<<<<<<< Updated upstream
/**
 * @brief 执行单页访问操作
 * @param page_ids 页面ID序列
 * @param latencies 输出的延迟序列
 * 
 * Why: 需要执行单页访问操作并测量每个操作的延迟
 * What: ExecuteSinglePageAccesses方法遍历页面ID序列，执行单页读取操作并测量延迟
 * How: 对每个页面ID调用FetchPage方法，测量操作时间，然后调用UnpinPage释放页面
 */
=======

>>>>>>> Stashed changes
void BatchPrefetchPerformanceTest::ExecuteSinglePageAccesses(const std::vector<int32_t>& page_ids, 
                                                             std::vector<double>& latencies) {
    latencies.clear();
    latencies.reserve(page_ids.size());
    
    // 遍历页面ID序列
    // Why: 需要对每个页面ID执行单页读取操作
    // What: 遍历页面ID序列，对每个ID执行读取操作
    // How: 使用范围for循环遍历页面ID
    for (int32_t page_id : page_ids) {
        // 测量页面获取时间
        // Why: 需要测量页面获取操作的延迟
        // What: 调用FetchPage方法获取页面并测量时间
        // How: 使用GetCurrentTime获取开始和结束时间
        auto start = GetCurrentTime();
        auto page = buffer_pool_->FetchPage(page_id);
        auto end = GetCurrentTime();
        
        // 计算延迟（毫秒）
        // Why: 需要将时间差转换为毫秒并保存
        // What: 调用CalculateDuration计算时间差，转换为毫秒
        // How: 调用CalculateDuration和count方法
        auto duration = CalculateDuration(start, end);
        latencies.push_back(duration.count());
        
        // 模拟页面使用
        // Why: 需要模拟页面使用后释放页面，避免缓冲池溢出
        // What: 调用UnpinPage方法释放页面
        // How: 直接调用UnpinPage方法，false表示页面未被修改
        buffer_pool_->UnpinPage(page_id, false);
    }
}

<<<<<<< Updated upstream
/**
 * @brief 执行批量页面访问操作
 * @param page_ids 页面ID序列
 * @param latencies 输出的延迟序列
 * @param batch_size 批量大小
 * 
 * Why: 需要执行批量页面访问操作并测量每个操作的延迟
 * What: ExecuteBatchPageAccesses方法将页面ID序列分组，执行批量读取操作并测量延迟
 * How: 将页面ID序列按批量大小分组，对每组调用BatchFetchPages方法，测量操作时间
 */
=======

>>>>>>> Stashed changes
void BatchPrefetchPerformanceTest::ExecuteBatchPageAccesses(const std::vector<int32_t>& page_ids, 
                                                            std::vector<double>& latencies, 
                                                            size_t batch_size) {
    latencies.clear();
    latencies.reserve(page_ids.size());
    
    // 按批量大小处理页面ID序列
    // Why: 需要将页面ID序列按批量大小分组处理
    // What: 使用步长为batch_size的循环处理页面ID序列
    // How: 使用for循环，步长为batch_size
    for (size_t i = 0; i < page_ids.size(); i += batch_size) {
        // 确定当前批次的页面ID
        // Why: 需要提取当前批次的页面ID
        // What: 从页面ID序列中提取当前批次的页面ID
        // How: 使用内层循环提取页面ID
        std::vector<int32_t> batch_ids;
        for (size_t j = 0; j < batch_size && (i + j) < page_ids.size(); ++j) {
            batch_ids.push_back(page_ids[i + j]);
        }
        
        // 执行批量读取
        // Why: 需要执行批量页面读取操作并测量时间
        // What: 调用BatchFetchPages方法执行批量读取
        // How: 传入批次页面ID列表，测量操作时间
        auto start = GetCurrentTime();
        auto pages = buffer_pool_->BatchFetchPages(batch_ids);
        auto end = GetCurrentTime();
        
        // 计算延迟（毫秒）
        // Why: 需要计算平均延迟并分配给每个页面
        // What: 计算总延迟除以页面数量得到平均延迟
        // How: 调用CalculateDuration计算时间差，除以页面数量
        auto duration = CalculateDuration(start, end);
        double avg_latency = static_cast<double>(duration.count()) / batch_ids.size();
        
        // 为每个页面记录相同的延迟
        // Why: 需要为每个页面记录延迟，以便后续分析
        // What: 将平均延迟分配给批次中的每个页面
        // How: 使用循环将平均延迟添加到延迟向量
        for (size_t j = 0; j < batch_ids.size(); ++j) {
            latencies.push_back(avg_latency);
        }
        
        // 模拟页面使用
        // Why: 需要模拟页面使用后释放页面，避免缓冲池溢出
        // What: 调用UnpinPage方法释放每个页面
        // How: 遍历批次页面ID，调用UnpinPage方法
        for (int32_t page_id : batch_ids) {
            buffer_pool_->UnpinPage(page_id, false);
        }
    }
}

<<<<<<< Updated upstream
/**
 * @brief 执行预取操作
 * @param page_ids 页面ID序列
 * @param latencies 输出的延迟序列
 * 
 * Why: 需要执行预取操作并测量每个操作的延迟
 * What: ExecutePrefetchOperations方法对每个页面ID执行预取和读取操作并测量延迟
 * How: 对每个页面ID，先预取下一个页面，然后读取当前页面，测量读取操作时间
 */
=======

>>>>>>> Stashed changes
void BatchPrefetchPerformanceTest::ExecutePrefetchOperations(const std::vector<int32_t>& page_ids, 
                                                            std::vector<double>& latencies) {
    latencies.clear();
    latencies.reserve(page_ids.size());
    
    // 遍历页面ID序列
    // Why: 需要对每个页面ID执行预取和读取操作
    // What: 遍历页面ID序列，对每个ID执行预取和读取操作
    // How: 使用范围for循环遍历页面ID
    for (size_t i = 0; i < page_ids.size(); ++i) {
        // 预取下一个页面
        // Why: 需要预取下一个页面，提高后续访问的性能
        // What: 调用PrefetchPage方法预取下一个页面
        // How: 检查是否有下一个页面，有则调用PrefetchPage
        if (i + 1 < page_ids.size()) {
            buffer_pool_->PrefetchPage(page_ids[i + 1]);
        }
        
        // 读取当前页面
        // Why: 需要读取当前页面并测量延迟
        // What: 调用FetchPage方法获取页面并测量时间
        // How: 使用GetCurrentTime获取开始和结束时间
        auto start = GetCurrentTime();
        auto page = buffer_pool_->FetchPage(page_ids[i]);
        auto end = GetCurrentTime();
        
        // 计算延迟（毫秒）
        // Why: 需要将时间差转换为毫秒并保存
        // What: 调用CalculateDuration计算时间差，转换为毫秒
        // How: 调用CalculateDuration和count方法
        auto duration = CalculateDuration(start, end);
        latencies.push_back(duration.count());
        
        // 模拟页面使用
        // Why: 需要模拟页面使用后释放页面，避免缓冲池溢出
        // What: 调用UnpinPage方法释放页面
        // How: 直接调用UnpinPage方法，false表示页面未被修改
        buffer_pool_->UnpinPage(page_ids[i], false);
    }
}

<<<<<<< Updated upstream
/**
 * @brief 执行批量预取操作
 * @param page_ids 页面ID序列
 * @param latencies 输出的延迟序列
 * @param batch_size 批量大小
 * 
 * Why: 需要执行批量预取操作并测量每个操作的延迟
 * What: ExecuteBatchPrefetchOperations方法将页面ID序列分组，执行批量预取和读取操作并测量延迟
 * How: 将页面ID序列按批量大小分组，对每组先执行批量预取，然后逐个读取页面，测量读取操作时间
 */
=======

>>>>>>> Stashed changes
void BatchPrefetchPerformanceTest::ExecuteBatchPrefetchOperations(const std::vector<int32_t>& page_ids, 
                                                                 std::vector<double>& latencies, 
                                                                 size_t batch_size) {
    latencies.clear();
    latencies.reserve(page_ids.size());
    
    // 按批量大小处理页面ID序列
    // Why: 需要将页面ID序列按批量大小分组处理
    // What: 使用步长为batch_size的循环处理页面ID序列
    // How: 使用for循环，步长为batch_size
    for (size_t i = 0; i < page_ids.size(); i += batch_size) {
        // 确定当前批次的页面ID
        // Why: 需要提取当前批次的页面ID
        // What: 从页面ID序列中提取当前批次的页面ID
        // How: 使用内层循环提取页面ID
        std::vector<int32_t> batch_ids;
        for (size_t j = 0; j < batch_size && (i + j) < page_ids.size(); ++j) {
            batch_ids.push_back(page_ids[i + j]);
        }
        
        // 批量预取
        // Why: 需要批量预取当前批次的页面，提高后续访问的性能
        // What: 调用BatchPrefetchPages方法批量预取页面
        // How: 直接调用BatchPrefetchPages方法
        buffer_pool_->BatchPrefetchPages(batch_ids);
        
        // 读取当前批次的页面
        // Why: 需要读取当前批次的页面并测量延迟
        // What: 遍历批次页面ID，逐个读取页面并测量时间
        // How: 使用范围for循环遍历批次页面ID
        for (int32_t page_id : batch_ids) {
            auto start = GetCurrentTime();
            auto page = buffer_pool_->FetchPage(page_id);
            auto end = GetCurrentTime();
            
            // 计算延迟（毫秒）
            // Why: 需要将时间差转换为毫秒并保存
            // What: 调用CalculateDuration计算时间差，转换为毫秒
            // How: 调用CalculateDuration和count方法
            auto duration = CalculateDuration(start, end);
            latencies.push_back(duration.count());
            
            // 模拟页面使用
            // Why: 需要模拟页面使用后释放页面，避免缓冲池溢出
            // What: 调用UnpinPage方法释放页面
            // How: 直接调用UnpinPage方法，false表示页面未被修改
            buffer_pool_->UnpinPage(page_id, false);
        }
    }
}

}  // namespace test
}  // namespace sqlcc