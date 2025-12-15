#include "crud_performance_test.h"
#include <iostream>
#include <string>
#include <cstring>

// 显示使用说明
void PrintUsage(const char* program_name) {
    std::cout << "SQLCC CRUD性能测试工具" << std::endl;
    std::cout << "用法: " << program_name << " [选项]" << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  --scale=SCALE    测试规模 (small, medium, large, xlarge, all)" << std::endl;
    std::cout << "  --help           显示此帮助信息" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "测试规模说明:" << std::endl;
    std::cout << "  small    - 小规模测试 (1000条记录)" << std::endl;
    std::cout << "  medium   - 中等规模测试 (10000条记录)" << std::endl;
    std::cout << "  large    - 大规模测试 (50000条记录)" << std::endl;
    std::cout << "  xlarge   - 超大规模测试 (100000条记录)" << std::endl;
    std::cout << "  all      - 运行所有规模测试 (默认)" << std::endl;
}

// 解析命令行参数
std::string ParseCommandLine(int argc, char* argv[]) {
    std::string scale = "all"; // 默认运行所有规模
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            PrintUsage(argv[0]);
            exit(0);
        } else if (arg.find("--scale=") == 0) {
            scale = arg.substr(8); // 提取"="后面的值
            
            // 验证规模参数
            if (scale != "small" && scale != "medium" && scale != "large" && 
                scale != "xlarge" && scale != "all") {
                std::cerr << "错误: 无效的测试规模 '" << scale << "'" << std::endl;
                std::cerr << "可用选项: small, medium, large, xlarge, all" << std::endl;
                exit(1);
            }
        } else {
            std::cerr << "错误: 未知参数 '" << arg << "'" << std::endl;
            PrintUsage(argv[0]);
            exit(1);
        }
    }
    
    return scale;
}

int main(int argc, char* argv[]) {
    std::cout << "=== SQLCC CRUD性能测试 ===" << std::endl;
    std::cout << "开始时间: " << sqlcc::test::CRUDPerformanceTest::GetCurrentTime() << std::endl;
    std::cout << std::endl;
    
    // 解析命令行参数
    std::string scale = ParseCommandLine(argc, argv);
    
    std::cout << "测试规模: " << scale << std::endl;
    std::cout << std::endl;
    
    try {
        // 创建性能测试实例
        sqlcc::test::CRUDPerformanceTest test(scale);
        
        // 运行所有测试
        test.RunAllTests();
        
        std::cout << std::endl;
        std::cout << "=== CRUD性能测试完成 ===" << std::endl;
        std::cout << "结束时间: " << sqlcc::test::CRUDPerformanceTest::GetCurrentTime() << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "未知错误发生" << std::endl;
        return 1;
    }
}