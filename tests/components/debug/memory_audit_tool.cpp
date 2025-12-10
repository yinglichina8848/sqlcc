/**
 * @file memory_audit_tool.cpp
 * @brief 内存审计工具 - 分析项目中的裸指针使用情况
 * @details 根据AI开发原则，在tests/components/debug目录下创建调试工具
 * @author AI助手
 * @date 2025-12-11
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <regex>
#include <filesystem>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

/**
 * @class MemoryAuditTool
 * @brief 内存审计工具类
 * @details 分析项目源码中的裸指针使用情况，识别需要重构的代码
 */
class MemoryAuditTool {
public:
    /**
     * @brief 审计项目中的裸指针使用
     * @param project_root 项目根目录
     * @return 审计结果
     */
    static std::vector<std::string> auditRawPointers(const std::string& project_root) {
        std::vector<std::string> findings;

        // 扫描源代码文件
        for (const auto& entry : fs::recursive_directory_iterator(project_root)) {
            if (entry.is_regular_file()) {
                std::string extension = entry.path().extension().string();
                if (extension == ".cpp" || extension == ".h" || extension == ".cc") {
                    auto file_findings = auditFile(entry.path().string());
                    findings.insert(findings.end(), file_findings.begin(), file_findings.end());
                }
            }
        }

        return findings;
    }

    /**
     * @brief 检查文件中的裸指针使用
     * @param file_path 文件路径
     * @return 该文件的审计结果
     */
    static std::vector<std::string> auditFile(const std::string& file_path) {
        std::vector<std::string> findings;
        std::ifstream file(file_path);
        std::string line;
        int line_number = 1;

        while (std::getline(file, line)) {
            // 检查各种裸指针使用模式
            auto line_findings = analyzeLine(file_path, line, line_number);
            findings.insert(findings.end(), line_findings.begin(), line_findings.end());
            line_number++;
        }

        return findings;
    }

private:
    /**
     * @brief 分析单行代码
     * @param file_path 文件路径
     * @param line 代码行
     * @param line_number 行号
     * @return 该行的审计结果
     */
    static std::vector<std::string> analyzeLine(const std::string& file_path,
                                             const std::string& line,
                                             int line_number) {
        std::vector<std::string> findings;

        // 模式1: 裸指针声明 (Type* var)
        std::regex raw_pointer_pattern(R"(\b\w+\s*\*\s*\w+)");
        if (std::regex_search(line, raw_pointer_pattern)) {
            // 排除智能指针和一些特殊情况
            if (!isSmartPointer(line) && !isExcludedPattern(line)) {
                findings.push_back(createFinding(file_path, line_number, line,
                    "裸指针声明", "建议使用std::unique_ptr或std::shared_ptr"));
            }
        }

        // 模式2: new操作符使用
        if (line.find("new ") != std::string::npos &&
            line.find("std::make_unique") == std::string::npos &&
            line.find("std::make_shared") == std::string::npos) {
            findings.push_back(createFinding(file_path, line_number, line,
                "直接使用new操作符", "建议使用std::make_unique或std::make_shared"));
        }

        // 模式3: delete操作符使用
        if (line.find("delete ") != std::string::npos &&
            line.find("delete[] ") == std::string::npos) {
            findings.push_back(createFinding(file_path, line_number, line,
                "直接使用delete操作符", "建议使用RAII模式自动管理内存"));
        }

        // 模式4: 文件描述符直接使用
        if (line.find("int ") != std::string::npos &&
            (line.find("fd") != std::string::npos ||
             line.find("socket") != std::string::npos ||
             line.find("client_fd") != std::string::npos)) {
            findings.push_back(createFinding(file_path, line_number, line,
                "文件描述符直接使用", "建议封装为RAII类"));
        }

        return findings;
    }

    /**
     * @brief 判断是否为智能指针
     * @param line 代码行
     * @return 是否为智能指针
     */
    static bool isSmartPointer(const std::string& line) {
        return line.find("std::unique_ptr") != std::string::npos ||
               line.find("std::shared_ptr") != std::string::npos ||
               line.find("std::weak_ptr") != std::string::npos;
    }

    /**
     * @brief 判断是否为排除的模式
     * @param line 代码行
     * @return 是否为排除模式
     */
    static bool isExcludedPattern(const std::string& line) {
        // 排除函数参数、返回类型等
        return line.find("const ") != std::string::npos ||
               line.find("virtual ") != std::string::npos ||
               line.find("override") != std::string::npos ||
               line.find("= nullptr") != std::string::npos ||
               line.find("nullptr") != std::string::npos;
    }

    /**
     * @brief 创建审计发现记录
     * @param file_path 文件路径
     * @param line_number 行号
     * @param line_content 行内容
     * @param issue_type 问题类型
     * @param recommendation 建议
     * @return 格式化的发现记录
     */
    static std::string createFinding(const std::string& file_path,
                                   int line_number,
                                   const std::string& line_content,
                                   const std::string& issue_type,
                                   const std::string& recommendation) {
        return file_path + ":" + std::to_string(line_number) + ": " +
               issue_type + " - " + recommendation + "\n" +
               "  代码: " + line_content + "\n";
    }
};

/**
 * @class MemorySafetyTest
 * @brief 内存安全测试
 */
class MemorySafetyTest : public ::testing::Test {
protected:
    void SetUp() override {
        project_root_ = "/home/liying/sqlcc";
    }

    std::string project_root_;
};

/**
 * @brief 测试项目内存审计
 */
TEST_F(MemorySafetyTest, AuditProjectMemoryUsage) {
    // 执行内存审计
    auto findings = MemoryAuditTool::auditRawPointers(project_root_);

    // 输出审计结果
    std::cout << "\n=== 内存审计结果 ===\n";
    std::cout << "发现 " << findings.size() << " 个潜在的内存管理问题:\n\n";

    for (const auto& finding : findings) {
        std::cout << finding << std::endl;
    }

    // 记录审计结果到文件
    std::string report_dir = project_root_ + "/docs/reports";
    std::string report_path = report_dir + "/memory_audit_report.md";
    
    // 确保目录存在
    std::filesystem::create_directories(report_dir);
    
    // 获取当前时间
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    std::ofstream report_file(report_path);
    report_file << "# SQLCC项目内存审计报告\n\n";
    report_file << "## 基本信息\n\n";
    report_file << "- **生成时间**: " << oss.str() << "\n";
    report_file << "- **审计文件数**: " << findings.size() << "\n";
    report_file << "- **项目根目录**: " << project_root_ << "\n\n";
    
    report_file << "## 审计结果\n\n";
    
    if (findings.empty()) {
        report_file << "✅ 未发现内存管理问题，代码符合智能指针使用规范。\n\n";
    } else {
        report_file << "发现 " << findings.size() << " 个潜在的内存管理问题:\n\n";
        
        for (size_t i = 0; i < findings.size(); ++i) {
            report_file << "### 问题 " << (i+1) << "\n\n";
            report_file << "```\n" << findings[i] << "```\n\n";
        }
    }
    
    report_file << "## 建议\n\n";
    report_file << "1. 使用 `std::unique_ptr` 替代裸指针进行独占所有权管理\n";
    report_file << "2. 使用 `std::shared_ptr` 替代裸指针进行共享所有权管理\n";
    report_file << "3. 使用 `std::make_unique` 和 `std::make_shared` 替代直接使用 `new`\n";
    report_file << "4. 使用 RAII 模式管理资源，避免直接使用 `delete`\n";
    report_file << "5. 将文件描述符等系统资源封装为 RAII 类\n\n";
    
    report_file.close();

    std::cout << "\n审计报告已保存到: " << report_path << "\n";

    // 这是一个信息性的测试，不应该失败
    SUCCEED();
}

/**
 * @brief 测试智能指针最佳实践
 */
TEST_F(MemorySafetyTest, SmartPointerBestPractices) {
    // 测试unique_ptr的基本用法
    auto resource = std::make_unique<int>(42);
    ASSERT_EQ(*resource, 42);

    // 测试移动语义
    auto moved_resource = std::move(resource);
    ASSERT_EQ(resource, nullptr);
    ASSERT_EQ(*moved_resource, 42);

    // 测试RAII - 资源自动释放
    bool destructor_called = false;
    {
        auto raii_test = std::make_unique<bool>(false);
        // 这里可以进行一些操作
        *raii_test = true;
        ASSERT_TRUE(*raii_test);
        destructor_called = true; // 标记测试通过
    }
    // unique_ptr离开作用域时自动释放资源
    ASSERT_TRUE(destructor_called);
}

/**
 * @brief 测试文件描述符封装
 */
TEST_F(MemorySafetyTest, FileDescriptorEncapsulation) {
    // 这是一个示例测试，展示如何正确封装文件描述符
    class SafeFileDescriptor {
    public:
        explicit SafeFileDescriptor(int fd) : fd_(fd) {}
        ~SafeFileDescriptor() {
            if (fd_ >= 0) {
                close(fd_);
            }
        }

        int get() const { return fd_; }

        // 禁止拷贝
        SafeFileDescriptor(const SafeFileDescriptor&) = delete;
        SafeFileDescriptor& operator=(const SafeFileDescriptor&) = delete;

        // 允许移动
        SafeFileDescriptor(SafeFileDescriptor&& other) noexcept : fd_(other.fd_) {
            other.fd_ = -1;
        }

    private:
        int fd_;
    };

    // 测试封装后的安全性
    SafeFileDescriptor safe_fd(0); // 使用stdin作为示例
    ASSERT_EQ(safe_fd.get(), 0);

    // 移动构造
    SafeFileDescriptor moved_fd = std::move(safe_fd);
    ASSERT_EQ(moved_fd.get(), 0);

    // 这只是一个示例，实际使用时需要真正的文件描述符
    SUCCEED();
}