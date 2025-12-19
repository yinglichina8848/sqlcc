/**
 * @file migration_script.cpp
 * @brief 配置管理器自动迁移脚本
 * 
 * Why: 自动化传统配置管理器到智能配置管理器的代码迁移
 * What: 提供代码搜索、替换和验证功能
 * How: 使用正则表达式和文件操作批量更新代码
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include <chrono>
#include <iomanip>

namespace fs = std::filesystem;

class ConfigMigrationScript {
private:
    struct MigrationRule {
        std::string name;
        std::regex pattern;
        std::string replacement;
        std::string description;
        bool critical;
    };
    
    std::vector<MigrationRule> rules_;
    std::vector<std::string> processed_files_;
    std::vector<std::string> errors_;
    int changes_count_ = 0;
    
public:
    ConfigMigrationScript() {
        InitializeRules();
    }
    
    void RunMigration(const std::string& source_dir) {
        std::cout << "🔧 开始配置管理器迁移..." << std::endl;
        std::cout << "📁 源目录: " << source_dir << std::endl;
        
        auto start_time = std::chrono::steady_clock::now();
        
        // 1. 扫描文件
        std::vector<fs::path> cpp_files = ScanCppFiles(source_dir);
        std::cout << "📊 发现 " << cpp_files.size() << " 个C++文件" << std::endl;
        
        // 2. 处理每个文件
        for (const auto& file : cpp_files) {
            ProcessFile(file);
        }
        
        // 3. 生成报告
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        GenerateReport(duration);
    }
    
private:
    void InitializeRules() {
        // 头文件引用替换
        rules_.push_back({
            "Header Include",
            std::regex(R"(#include\s*["<]utils/config_manager\.h[">])"),
            "#include \"utils/smart_config_manager.h\"",
            "替换配置管理器头文件",
            true
        });
        
        // 获取实例替换
        rules_.push_back({
            "Get Instance",
            std::regex(R"(ConfigManager\s*\*\s*(\w+)\s*=\s*ConfigManager::GetInstance\(\))"),
            "auto $1 = SmartConfigManager::GetInstance()",
            "替换获取实例的调用",
            true
        });
        
        // 配置访问方法替换
        rules_.push_back({
            "GetBool Method",
            std::regex(R"((\w+)->GetBool\(([^,]+),\s*([^)]+)\))"),
            "$1->GetBoolConfig($2, $3)",
            "替换布尔配置访问方法",
            true
        });
        
        rules_.push_back({
            "GetInt Method",
            std::regex(R"((\w+)->GetInt\(([^,]+),\s*([^)]+)\))"),
            "$1->GetIntConfig($2, $3)",
            "替换整数配置访问方法",
            true
        });
        
        rules_.push_back({
            "GetDouble Method",
            std::regex(R"((\w+)->GetDouble\(([^,]+),\s*([^)]+)\))"),
            "$1->GetDoubleConfig($2, $3)",
            "替换双精度配置访问方法",
            true
        });
        
        rules_.push_back({
            "GetString Method",
            std::regex(R"((\w+)->GetString\(([^,]+),\s*([^)]+)\))"),
            "$1->GetStringConfig($2, $3)",
            "替换字符串配置访问方法",
            true
        });
        
        // 高级功能添加（非关键）
        rules_.push_back({
            "Add Hot Reload",
            std::regex(R"(//\s*TODO:\s*添加热更新功能)"),
            "config->EnableHotReload(std::chrono::milliseconds(5000));",
            "添加热更新功能",
            false
        });
        
        // 错误处理增强
        rules_.push_back({
            "Add Exception Handling",
            std::regex(R"(//\s*TODO:\s*添加异常处理)"),
            R"(try {
    // 配置操作
} catch (const ConfigLifecycleException& e) {
    std::cerr << "Config error: " << e.what() << std::endl;
})",
            "添加异常处理",
            false
        });
    }
    
    std::vector<fs::path> ScanCppFiles(const std::string& dir) {
        std::vector<fs::path> cpp_files;
        
        try {
            for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string();
                    if (ext == ".cpp" || ext == ".h" || ext == ".hpp") {
                        cpp_files.push_back(entry.path());
                    }
                }
            }
        } catch (const std::exception& e) {
            errors_.push_back("扫描文件时出错: " + std::string(e.what()));
        }
        
        return cpp_files;
    }
    
    void ProcessFile(const fs::path& file_path) {
        try {
            // 读取文件内容
            std::ifstream file(file_path);
            if (!file.is_open()) {
                errors_.push_back("无法打开文件: " + file_path.string());
                return;
            }
            
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();
            file.close();
            
            std::string original_content = content;
            std::vector<std::string> applied_rules;
            
            // 应用迁移规则
            for (const auto& rule : rules_) {
                std::string new_content = std::regex_replace(content, rule.pattern, rule.replacement);
                if (new_content != content) {
                    content = new_content;
                    applied_rules.push_back(rule.name + " - " + rule.description);
                    changes_count_++;
                }
            }
            
            // 如果有变更，写回文件
            if (content != original_content) {
                std::ofstream out_file(file_path);
                if (!out_file.is_open()) {
                    errors_.push_back("无法写入文件: " + file_path.string());
                    return;
                }
                
                out_file << content;
                out_file.close();
                
                processed_files_.push_back(file_path.string());
                
                // 记录应用的规则
                std::cout << "✅ 处理文件: " << file_path.filename() << std::endl;
                for (const auto& rule : applied_rules) {
                    std::cout << "   - " << rule << std::endl;
                }
            }
            
        } catch (const std::exception& e) {
            errors_.push_back("处理文件 " + file_path.string() + " 时出错: " + e.what());
        }
    }
    
    void GenerateReport(std::chrono::milliseconds duration) {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "📋 迁移报告" << std::endl;
        std::cout << std::string(60, '-') << std::endl;
        
        std::cout << "⏱️  处理时间: " << duration.count() << "ms" << std::endl;
        std::cout << "📁 处理文件数: " << processed_files_.size() << std::endl;
        std::cout << "🔧 总变更数: " << changes_count_ << std::endl;
        
        if (!processed_files_.empty()) {
            std::cout << "\n📄 已处理的文件:" << std::endl;
            for (const auto& file : processed_files_) {
                std::cout << "   - " << fs::path(file).filename() << std::endl;
            }
        }
        
        if (!errors_.empty()) {
            std::cout << "\n❌ 错误:" << std::endl;
            for (const auto& error : errors_) {
                std::cout << "   - " << error << std::endl;
            }
        }
        
        std::cout << "\n🎯 下一步建议:" << std::endl;
        std::cout << "1. 运行编译测试: make clean && make" << std::endl;
        std::cout << "2. 执行单元测试: ./tests/test_smart_config_manager" << std::endl;
        std::cout << "3. 验证功能正常后，提交代码变更" << std::endl;
        
        std::cout << "\n⚠️  重要提醒:" << std::endl;
        std::cout << "- 请仔细检查自动迁移的结果" << std::endl;
        std::cout << "- 运行完整的测试套件确保功能正常" << std::endl;
        std::cout << "- 备份原始代码以防需要回滚" << std::endl;
        
        std::cout << std::string(60, '=') << std::endl;
        
        // 生成详细日志文件
        GenerateLogFile();
    }
    
    void GenerateLogFile() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
        
        std::string log_filename = "migration_log_" + ss.str() + ".txt";
        std::ofstream log_file(log_filename);
        
        if (log_file.is_open()) {
            log_file << "SQLCC Config Manager Migration Log\n";
            log_file << "Generated: " << std::ctime(&time_t);
            log_file << "=====================================\n\n";
            
            log_file << "Migration Rules Applied:\n";
            for (const auto& rule : rules_) {
                log_file << "- " << rule.name << ": " << rule.description << "\n";
            }
            
            log_file << "\nProcessed Files:\n";
            for (const auto& file : processed_files_) {
                log_file << "- " << file << "\n";
            }
            
            if (!errors_.empty()) {
                log_file << "\nErrors:\n";
                for (const auto& error : errors_) {
                    log_file << "- " << error << "\n";
                }
            }
            
            log_file.close();
            std::cout << "\n📝 详细日志已保存到: " << log_filename << std::endl;
        }
    }
};

// 主函数
int main(int argc, char* argv[]) {
    std::cout << "SQLCC 智能配置管理器迁移脚本 v1.0" << std::endl;
    std::cout << "==================================" << std::endl;
    
    std::string source_dir = ".";  // 默认当前目录
    
    if (argc > 1) {
        source_dir = argv[1];
    }
    
    try {
        ConfigMigrationScript migration;
        migration.RunMigration(source_dir);
        
        std::cout << "\n🎉 迁移完成！" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 迁移失败: " << e.what() << std::endl;
        return 1;
    }
}

/**
 * 使用示例:
 * 
 * 1. 基本使用（处理当前目录）:
 *    ./migration_script
 * 
 * 2. 指定源目录:
 *    ./migration_script /path/to/source/code
 * 
 * 3. 处理特定模块:
 *    ./migration_script src/config
 * 
 * 4. 生成报告后验证:
 *    make clean && make
 *    ./tests/test_smart_config_manager
 * 
 * 注意事项:
 * - 迁移前请备份代码
 * - 仔细检查自动迁移结果
 * - 运行完整测试套件
 * - 保留迁移日志文件
 */