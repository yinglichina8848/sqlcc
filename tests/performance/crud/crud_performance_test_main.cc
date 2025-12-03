#include "crud_performance_test.h"
#include <iostream>
#include <string>
#include <cstdlib>

/**
 * CRUD性能测试主程序
 * 一键运行所有CRUD操作性能测试，验证1-10万行数据下的性能要求
 * 要求：单操作耗时<5ms (SSD)
 */
int main(int argc, char** argv) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "CRUD PERFORMANCE TEST SUITE" << std::endl;
    std::cout << "SQLCC Database System" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    // 显示测试配置
    std::cout << "\nTest Configuration:" << std::endl;
    std::cout << "- Data Sizes: 1K, 10K, 50K, 100K records" << std::endl;
    std::cout << "- Operations: INSERT, SELECT (point/range), UPDATE, DELETE" << std::endl;
    std::cout << "- Performance Requirement: <5ms per operation (SSD)" << std::endl;
    std::cout << "- Test Environment: Single-threaded" << std::endl;
    
    // 检查命令行参数
    bool quick_mode = false;
    bool verbose_mode = false;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--quick" || arg == "-q") {
            quick_mode = true;
            std::cout << "\nQuick mode enabled: Reduced test iterations" << std::endl;
        } else if (arg == "--verbose" || arg == "-v") {
            verbose_mode = true;
            std::cout << "\nVerbose mode enabled: Detailed logging" << std::endl;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "\nUsage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --quick, -q     Run in quick mode (reduced iterations)" << std::endl;
            std::cout << "  --verbose, -v   Enable verbose logging" << std::endl;
            std::cout << "  --help, -h      Show this help message" << std::endl;
            return 0;
        }
    }
    
    std::cout << "\nStarting CRUD performance tests..." << std::endl;
    std::cout << std::string(40, '-') << std::endl;
    
    try {
        // 创建测试实例
        sqlcc::test::CRUDPerformanceTest test;
        
        // 设置输出目录
        test.SetOutputDirectory("./test_reports");
        
        // 记录开始时间
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // 运行所有测试
        test.RunAllTests();
        
        // 记录结束时间
        auto end_time = std::chrono::high_resolution_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
        
        std::cout << "\n" << std::string(40, '-') << std::endl;
        std::cout << "All tests completed in " << total_duration.count() << " seconds" << std::endl;
        
        // 检查系统环境
        std::cout << "\nSystem Environment Check:" << std::endl;
        std::cout << "- Storage Type: " << (system("cat /sys/block/sda/queue/rotational 2>/dev/null | grep -q '0' && echo 'SSD' || echo 'HDD'") == 0 ? "SSD" : "HDD") << std::endl;
        
        // 显示性能要求验证结果
        std::cout << "\nPerformance Requirement Verification:" << std::endl;
        std::cout << "- Target: Single operation latency < 5ms (SSD)" << std::endl;
        std::cout << "- Status: See detailed report above" << std::endl;
        
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "CRUD PERFORMANCE TEST COMPLETED" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\nERROR: Test execution failed: " << e.what() << std::endl;
        std::cerr << "Please check if the database system is properly built and configured." << std::endl;
        return 1;
    }
}