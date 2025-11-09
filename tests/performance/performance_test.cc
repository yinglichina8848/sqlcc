#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <getopt.h>

#include "performance_test_base.h"
#include "buffer_pool_performance_test.h"
#include "disk_io_performance_test.h"
#include "mixed_workload_test.h"
#include "batch_prefetch_performance_test.h"
#include "million_insert_test.h"

using namespace sqlcc::test;

/**
 * 性能测试主程序
 */
class PerformanceTestRunner {
public:
    /**
     * 构造函数
     */
    PerformanceTestRunner() = default;

    /**
     * 析构函数
     */
    ~PerformanceTestRunner() = default;

    /**
     * 运行性能测试
     */
    int Run(int argc, char* argv[]);

private:
    /**
     * 打印使用帮助
     */
    void PrintUsage() const;

    /**
     * 解析命令行参数
     */
    bool ParseArguments(int argc, char* argv[]);

    /**
     * 运行指定的测试
     */
    void RunTests();

private:
    // 测试选项
    bool run_buffer_pool_tests_ = true;
    bool run_disk_io_tests_ = true;
    bool run_mixed_workload_tests_ = true;
    bool run_batch_prefetch_tests_ = true;
    bool run_million_insert_tests_ = true;
    bool verbose_ = false;
    std::string output_dir_ = "./performance_results";
};

void PerformanceTestRunner::PrintUsage() const {
    std::cout << "SQLCC Performance Test Suite" << std::endl;
    std::cout << "Usage: performance_test [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help                Show this help message" << std::endl;
    std::cout << "  -b, --buffer-pool         Run buffer pool performance tests" << std::endl;
    std::cout << "  -d, --disk-io             Run disk I/O performance tests" << std::endl;
    std::cout << "  -m, --mixed-workload      Run mixed workload performance tests" << std::endl;
    std::cout << "  -p, --batch-prefetch      Run batch & prefetch performance tests" << std::endl;
    std::cout << "  -i, --million-insert      Run million INSERT performance tests" << std::endl;
    std::cout << "  -a, --all                 Run all performance tests (default)" << std::endl;
    std::cout << "  -v, --verbose             Enable verbose output" << std::endl;
    std::cout << "  -o, --output-dir <dir>    Specify output directory for results" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  performance_test                          # Run all tests" << std::endl;
    std::cout << "  performance_test -b                       # Run only buffer pool tests" << std::endl;
    std::cout << "  performance_test -d -m -v                # Run disk I/O and mixed workload tests with verbose output" << std::endl;
    std::cout << "  performance_test -p                       # Run only batch & prefetch tests" << std::endl;
    std::cout << "  performance_test -i                       # Run only million INSERT tests" << std::endl;
    std::cout << "  performance_test -o /tmp/results          # Run all tests and save results to /tmp/results" << std::endl;
}

bool PerformanceTestRunner::ParseArguments(int argc, char* argv[]) {
    int opt;
    
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"buffer-pool", no_argument, 0, 'b'},
        {"disk-io", no_argument, 0, 'd'},
        {"mixed-workload", no_argument, 0, 'm'},
        {"batch-prefetch", no_argument, 0, 'p'},
        {"million-insert", no_argument, 0, 'i'},
        {"all", no_argument, 0, 'a'},
        {"verbose", no_argument, 0, 'v'},
        {"output-dir", required_argument, 0, 'o'},
        {0, 0, 0, 0}
    };
    
    // 默认运行所有测试
    run_buffer_pool_tests_ = true;
    run_disk_io_tests_ = true;
    run_mixed_workload_tests_ = true;
    run_batch_prefetch_tests_ = true;
    run_million_insert_tests_ = true;
    
    while ((opt = getopt_long(argc, argv, "hbdmipaivo:", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'h':
                PrintUsage();
                return false;
                
            case 'b':
                run_buffer_pool_tests_ = true;
                run_disk_io_tests_ = false;
                run_mixed_workload_tests_ = false;
                run_batch_prefetch_tests_ = false;
                run_million_insert_tests_ = false;
                break;
                
            case 'd':
                run_buffer_pool_tests_ = false;
                run_disk_io_tests_ = true;
                run_mixed_workload_tests_ = false;
                run_batch_prefetch_tests_ = false;
                run_million_insert_tests_ = false;
                break;
                
            case 'm':
                run_buffer_pool_tests_ = false;
                run_disk_io_tests_ = false;
                run_mixed_workload_tests_ = true;
                run_batch_prefetch_tests_ = false;
                run_million_insert_tests_ = false;
                break;
                
            case 'p':
                run_buffer_pool_tests_ = false;
                run_disk_io_tests_ = false;
                run_mixed_workload_tests_ = false;
                run_batch_prefetch_tests_ = true;
                run_million_insert_tests_ = false;
                break;
                
            case 'i':
                run_buffer_pool_tests_ = false;
                run_disk_io_tests_ = false;
                run_mixed_workload_tests_ = false;
                run_batch_prefetch_tests_ = false;
                run_million_insert_tests_ = true;
                break;
                
            case 'a':
                run_buffer_pool_tests_ = true;
                run_disk_io_tests_ = true;
                run_mixed_workload_tests_ = true;
                run_batch_prefetch_tests_ = true;
                run_million_insert_tests_ = true;
                break;
                
            case 'v':
                verbose_ = true;
                break;
                
            case 'o':
                output_dir_ = optarg;
                break;
                
            default:
                PrintUsage();
                return false;
        }
    }
    
    return true;
}

void PerformanceTestRunner::RunTests() {
    std::cout << "SQLCC Performance Test Suite" << std::endl;
    std::cout << "Output directory: " << output_dir_ << std::endl;
    
    // 创建输出目录
    std::string mkdir_cmd = "mkdir -p " + output_dir_;
    system(mkdir_cmd.c_str());
    
    // 运行缓冲池性能测试
    if (run_buffer_pool_tests_) {
        std::cout << "\n=====================================" << std::endl;
        std::cout << "Running Buffer Pool Performance Tests" << std::endl;
        std::cout << "=====================================" << std::endl;
        
        auto buffer_pool_test = std::make_unique<BufferPoolPerformanceTest>();
        buffer_pool_test->RunAllTests();
    }
    
    // 运行磁盘I/O性能测试
    if (run_disk_io_tests_) {
        std::cout << "\n=====================================" << std::endl;
        std::cout << "Running Disk I/O Performance Tests" << std::endl;
        std::cout << "=====================================" << std::endl;
        
        auto disk_io_test = std::make_unique<DiskIOPerformanceTest>();
        disk_io_test->RunAllTests();
    }
    
    // 运行混合工作负载性能测试
    if (run_mixed_workload_tests_) {
        std::cout << "\n=====================================" << std::endl;
        std::cout << "Running Mixed Workload Performance Tests" << std::endl;
        std::cout << "=====================================" << std::endl;
        
        auto mixed_workload_test = std::make_unique<MixedWorkloadTest>();
        mixed_workload_test->RunAllTests();
    }
    
    // 运行批量预取性能测试
    if (run_batch_prefetch_tests_) {
        std::cout << "\n=====================================" << std::endl;
        std::cout << "Running Batch & Prefetch Performance Tests" << std::endl;
        std::cout << "=====================================" << std::endl;
        
        auto batch_prefetch_test = std::make_unique<BatchPrefetchPerformanceTest>();
        batch_prefetch_test->RunAllTests();
    }
    
    // 运行100万INSERT性能测试
    if (run_million_insert_tests_) {
        std::cout << "\n=====================================" << std::endl;
        std::cout << "Running Million INSERT Performance Tests" << std::endl;
        std::cout << "=====================================" << std::endl;
        
        auto million_insert_test = std::make_unique<MillionInsertTest>();
        million_insert_test->RunAllTests();
    }
    
    std::cout << "\n=====================================" << std::endl;
    std::cout << "All Performance Tests Completed" << std::endl;
    std::cout << "Results saved to: " << output_dir_ << std::endl;
    std::cout << "=====================================" << std::endl;
}

int PerformanceTestRunner::Run(int argc, char* argv[]) {
    if (!ParseArguments(argc, argv)) {
        return 1;
    }
    
    try {
        RunTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int main(int argc, char* argv[]) {
    PerformanceTestRunner runner;
    return runner.Run(argc, argv);
}