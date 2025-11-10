#include "long_term_stability_test.h"
#include <iostream>
#include <memory>
#include <cstdlib>

int main(int argc, char* argv[]) {
    try {
        // 创建长时间稳定性测试实例
        std::unique_ptr<sqlcc::test::LongTermStabilityTest> stability_test = 
            std::make_unique<sqlcc::test::LongTermStabilityTest>();
        
        // 设置输出目录为构建目录
        const char* build_dir = std::getenv("CMAKE_BINARY_DIR");
        if (build_dir) {
            stability_test->SetOutputDirectory(std::string(build_dir) + "/performance_results");
        } else {
            // 默认使用当前目录下的build子目录
            stability_test->SetOutputDirectory("./build/performance_results");
        }
        
        // 配置测试参数
        sqlcc::test::LongTermStabilityTest::TestConfig config;
        
        // 根据命令行参数设置测试时长
        if (argc > 1) {
            int duration_seconds = std::atoi(argv[1]);
            if (duration_seconds > 0) {
                config.test_duration = std::chrono::seconds(duration_seconds);
                std::cout << "Using custom test duration: " << duration_seconds << " seconds" << std::endl;
            }
        } else {
            // 默认测试时长为5分钟（300秒），适合演示
            config.test_duration = std::chrono::seconds(300);
            std::cout << "Using default test duration: 300 seconds (5 minutes)" << std::endl;
        }
        
        // 根据命令行参数设置线程数
        if (argc > 2) {
            int thread_count = std::atoi(argv[2]);
            if (thread_count > 0 && thread_count <= 16) {
                config.thread_count = thread_count;
                std::cout << "Using custom thread count: " << thread_count << std::endl;
            }
        } else {
            // 默认线程数为4
            config.thread_count = 4;
            std::cout << "Using default thread count: 4" << std::endl;
        }
        
        // 设置其他配置参数
        config.warmup_duration_seconds = 30;  // 30秒预热
        config.sampling_interval_seconds = 10; // 每10秒采样一次
        config.output_file = "long_term_stability_results.csv";
        config.enable_memory_monitoring = true;
        config.enable_cpu_monitoring = true;
        config.enable_disk_io_monitoring = true;
        
        // 应用配置
        stability_test->SetConfig(config);
        
        // 运行测试
        stability_test->RunAllTests();
        
        // 清理资源
        stability_test->Cleanup();
        
        std::cout << "Long-term stability test completed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred during stability test" << std::endl;
        return 1;
    }
}