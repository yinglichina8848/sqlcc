#include "memory_stress_test.h"
#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) {
    // 避免未使用参数警告
    (void)argc;
    (void)argv;
    
    std::cout << "SQLCC Memory Stress Test" << std::endl;
    std::cout << "=========================" << std::endl;
    
    // 创建内存压力测试实例
    sqlcc::test::MemoryStressTest memory_test;
    
    // 设置输出目录为构建目录
    const char* build_dir = std::getenv("CMAKE_BINARY_DIR");
    if (build_dir) {
        memory_test.SetOutputDirectory(std::string(build_dir) + "/performance_results");
    } else {
        // 默认使用当前目录下的build子目录
        memory_test.SetOutputDirectory("./build/performance_results");
    }
    
    // 运行所有内存压力测试
    memory_test.RunAllTests();
    memory_test.Cleanup();
    
    // 创建内存压力测试运行器
    try {
        sqlcc::test::MemoryStressTestRunner stress_test_runner;
        
        // 运行长时间内存压力测试（10秒，2个线程）
        stress_test_runner.RunStressTest(10, 2);
    } catch (const std::exception& e) {
        std::cerr << "Error during stress test: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "All memory stress tests completed successfully!" << std::endl;
    
    return 0;
}