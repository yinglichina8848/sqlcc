#include "concurrency_performance_test.h"
#include "sharded_buffer_pool_concurrent_test.h"
#include <iostream>
#include <cstdlib>
#include <gtest/gtest.h>

int main(int argc, char** argv) {
    std::cout << "SQLCC 并发性能测试" << std::endl;
    std::cout << "=================" << std::endl;
    
    try {
        // 运行传统并发性能测试
        std::cout << "\n运行传统并发性能测试..." << std::endl;
        sqlcc::test::ConcurrencyPerformanceTest concurrency_test;
        
        // 设置输出目录为构建目录
        const char* build_dir = std::getenv("CMAKE_BINARY_DIR");
        std::string output_dir;
        if (build_dir) {
            output_dir = std::string(build_dir) + "/performance_results";
        } else {
            // 默认使用当前目录下的build子目录
            output_dir = "./build/performance_results";
        }
        
        concurrency_test.SetOutputDirectory(output_dir);
        concurrency_test.RunAllTests();
        
        std::cout << "\n传统并发性能测试完成！" << std::endl;
        std::cout << "结果已保存到 " << output_dir << "/concurrency_performance_results.csv" << std::endl;
        
        // 运行分片缓冲池并发测试
        std::cout << "\n运行分片缓冲池并发测试..." << std::endl;
        ::testing::InitGoogleTest(&argc, argv);
        int gtest_result = RUN_ALL_TESTS();
        
        std::cout << "\n分片缓冲池并发测试完成！" << std::endl;
        
        return gtest_result;
    } catch (const std::exception& e) {
        std::cerr << "测试失败: " << e.what() << std::endl;
        return 1;
    }
}