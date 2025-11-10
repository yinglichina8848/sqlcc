#include "cpu_intensive_performance_test.h"
#include <iostream>
#include <cstdlib>

int main() {
    std::cout << "SQLCC CPU密集型性能测试" << std::endl;
    std::cout << "=======================" << std::endl;
    
    try {
        sqlcc::test::CpuIntensivePerformanceTest cpu_test;
        
        // 设置输出目录为构建目录
        const char* build_dir = std::getenv("CMAKE_BINARY_DIR");
        if (build_dir) {
            cpu_test.SetOutputDirectory(std::string(build_dir) + "/performance_results");
        } else {
            // 默认使用当前目录下的build子目录
            cpu_test.SetOutputDirectory("./build/performance_results");
        }
        
        cpu_test.RunAllTests();
        
        std::cout << "\nCPU密集型性能测试完成！" << std::endl;
        std::cout << "结果已保存到 cpu_intensive_performance_results.csv" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "测试失败: " << e.what() << std::endl;
        return 1;
    }
}