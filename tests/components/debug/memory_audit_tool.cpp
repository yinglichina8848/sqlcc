/**
 * @file memory_audit_tool.cpp
 * @brief 内存审计工具 - 分析项目中的裸指针使用情况
 * @details 根据AI开发原则，在tests/components/debug目录下创建调试工具
 * @author AI助手
 * @date 2025-12-11
 */

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
#include <map>

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
 * @brief 主函数 - 执行内存审计
 */
int main(int argc, char* argv[]) {
    std::string project_root = "/home/liying/sqlcc";

    // 如果提供了命令行参数，使用第一个参数作为项目根目录
    if (argc > 1) {
        project_root = argv[1];
    }

    std::cout << "开始内存审计...\n";
    std::cout << "项目根目录: " << project_root << "\n\n";

    // 执行内存审计
    auto findings = MemoryAuditTool::auditRawPointers(project_root);

    // 输出审计结果
    std::cout << "\n=== 内存审计结果 ===\n";
    std::cout << "发现 " << findings.size() << " 个潜在的内存管理问题:\n\n";

    // 按文件分组显示结果
    std::map<std::string, std::vector<std::string>> findings_by_file;
    for (const auto& finding : findings) {
        // 提取文件路径（在第一个冒号之前）
        size_t colon_pos = finding.find(':');
        if (colon_pos != std::string::npos) {
            std::string file_path = finding.substr(0, colon_pos);
            findings_by_file[file_path].push_back(finding);
        }
    }

    for (const auto& [file_path, file_findings] : findings_by_file) {
        std::cout << "📁 " << file_path << " (" << file_findings.size() << " 个问题)\n";
        for (const auto& finding : file_findings) {
            std::cout << "  " << finding << std::endl;
        }
        std::cout << std::endl;
    }

    // 记录审计结果到文件
    std::string report_dir = project_root + "/docs/reports";
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
    report_file << "- **审计文件总数**: " << findings.size() << "\n";
    report_file << "- **影响文件数**: " << findings_by_file.size() << "\n";
    report_file << "- **项目根目录**: " << project_root << "\n\n";

    report_file << "## 审计结果\n\n";

    if (findings.empty()) {
        report_file << "✅ 未发现内存管理问题，代码符合智能指针使用规范。\n\n";
    } else {
        report_file << "### 问题统计\n\n";
        report_file << "| 文件路径 | 问题数量 | 主要问题类型 |\n";
        report_file << "|---------|---------|-------------|\n";

        for (const auto& [file_path, file_findings] : findings_by_file) {
            // 统计问题类型
            int raw_pointer_count = 0;
            int new_delete_count = 0;
            int fd_count = 0;

            for (const auto& finding : file_findings) {
                if (finding.find("裸指针声明") != std::string::npos) raw_pointer_count++;
                else if (finding.find("new操作符") != std::string::npos || finding.find("delete操作符") != std::string::npos) new_delete_count++;
                else if (finding.find("文件描述符") != std::string::npos) fd_count++;
            }

            std::string main_issues;
            if (raw_pointer_count > 0) main_issues += "裸指针(" + std::to_string(raw_pointer_count) + ") ";
            if (new_delete_count > 0) main_issues += "new/delete(" + std::to_string(new_delete_count) + ") ";
            if (fd_count > 0) main_issues += "文件描述符(" + std::to_string(fd_count) + ") ";

            report_file << "| `" << file_path << "` | " << file_findings.size() << " | " << main_issues << " |\n";
        }

        report_file << "\n### 详细问题列表\n\n";

        for (const auto& [file_path, file_findings] : findings_by_file) {
            report_file << "#### 📁 " << file_path << "\n\n";
            report_file << "**问题数量**: " << file_findings.size() << "\n\n";

            for (size_t i = 0; i < file_findings.size(); ++i) {
                report_file << "**问题 " << (i+1) << "**:\n\n";
                report_file << "```\n" << file_findings[i] << "```\n\n";
            }
        }
    }

    report_file << "## 改进建议\n\n";
    report_file << "### 1. 智能指针使用原则\n";
    report_file << "- 使用 `std::unique_ptr` 替代裸指针进行独占所有权管理\n";
    report_file << "- 使用 `std::shared_ptr` 替代裸指针进行共享所有权管理\n";
    report_file << "- 使用 `std::make_unique` 和 `std::make_shared` 替代直接使用 `new`\n\n";

    report_file << "### 2. RAII资源管理\n";
    report_file << "- 使用 RAII 模式管理资源，避免直接使用 `delete`\n";
    report_file << "- 将文件描述符等系统资源封装为 RAII 类\n";
    report_file << "- 实现异常安全的资源管理\n\n";

    report_file << "### 3. 重构优先级\n";
    report_file << "1. **高优先级**: 裸指针成员变量（内存泄漏风险）\n";
    report_file << "2. **中优先级**: 文件描述符直接使用（资源泄漏风险）\n";
    report_file << "3. **低优先级**: 函数参数裸指针（接口兼容性考虑）\n\n";

    report_file << "### 4. 验证机制\n";
    report_file << "- 启用 ASan/LSan 进行运行时内存检查\n";
    report_file << "- 定期运行内存审计工具监控改进效果\n";
    report_file << "- 建立自动化测试确保重构质量\n\n";

    report_file.close();

    std::cout << "\n审计报告已保存到: " << report_path << "\n";
    std::cout << "总共发现 " << findings.size() << " 个问题，影响 " << findings_by_file.size() << " 个文件\n";

    if (findings.empty()) {
        std::cout << "🎉 恭喜！项目代码符合内存安全规范！\n";
        return 0;
    } else {
        std::cout << "⚠️  发现内存安全问题，请参考上述建议进行改进。\n";
        return 1; // 返回非零表示发现问题
    }
}
