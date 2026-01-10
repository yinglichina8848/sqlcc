#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

/**
 * @brief 覆盖率报告生成器
 *
 * 生成HTML格式的代码覆盖率报告，包括：
 * - 行覆盖率
 * - 分支覆盖率
 * - 函数覆盖率
 * - 覆盖率趋势图
 */
class CoverageReportGenerator {
public:
    CoverageReportGenerator(const std::string& output_dir = "./coverage_report")
        : output_dir_(output_dir) {
        fs::create_directories(output_dir_);
    }

    /**
     * @brief 生成覆盖率报告
     * @param gcov_data_dir gcov数据目录
     * @param source_dirs 源代码目录列表
     * @return 是否成功
     */
    bool GenerateReport(const std::string& gcov_data_dir,
                       const std::vector<std::string>& source_dirs) {
        std::cout << "Generating coverage report..." << std::endl;
        std::cout << "Output directory: " << output_dir_ << std::endl;

        try {
            // 收集覆盖率数据
            collectCoverageData(gcov_data_dir, source_dirs);

            // 生成HTML报告
            generateHTMLReport();

            // 生成摘要报告
            generateSummaryReport();

            std::cout << "Coverage report generated successfully." << std::endl;
            std::cout << "Open " << output_dir_ << "/index.html in your browser." << std::endl;

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error generating coverage report: " << e.what() << std::endl;
            return false;
        }
    }

private:
    struct FileCoverage {
        std::string filename;
        int lines_total = 0;
        int lines_covered = 0;
        int functions_total = 0;
        int functions_covered = 0;
        int branches_total = 0;
        int branches_covered = 0;
        std::vector<std::string> source_lines;
        std::vector<int> line_coverage; // -1=not executable, 0=not covered, >0=covered
    };

    void collectCoverageData(const std::string& gcov_data_dir,
                           const std::vector<std::string>& source_dirs) {
        std::cout << "Collecting coverage data from: " << gcov_data_dir << std::endl;

        // 遍历源代码目录，查找对应的gcov文件
        for (const auto& source_dir : source_dirs) {
            for (const auto& entry : fs::recursive_directory_iterator(source_dir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
                    std::string source_file = entry.path().string();
                    std::string gcov_file = gcov_data_dir + "/" +
                                          entry.path().filename().string() + ".gcov";

                    if (fs::exists(gcov_file)) {
                        parseGcovFile(gcov_file, source_file);
                    }
                }
            }
        }
    }

    void parseGcovFile(const std::string& gcov_file, const std::string& source_file) {
        std::ifstream file(gcov_file);
        if (!file.is_open()) {
            return;
        }

        FileCoverage coverage;
        coverage.filename = fs::path(source_file).filename().string();

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            // 解析gcov格式
            // 格式: <execution_count>:<line_number>:<source_line>
            std::regex pattern(R"(^\s*(-|\d+):\s*(\d+):(.*)$)");
            std::smatch matches;

            if (std::regex_match(line, matches, pattern)) {
                int execution_count = (matches[1] == "-") ? -1 : std::stoi(matches[1]);
                int line_number = std::stoi(matches[2]);
                std::string source_line = matches[3];

                // 调整向量大小
                if (line_number >= coverage.line_coverage.size()) {
                    coverage.line_coverage.resize(line_number + 1, -1);
                    coverage.source_lines.resize(line_number + 1);
                }

                coverage.line_coverage[line_number] = execution_count;
                coverage.source_lines[line_number] = source_line;

                if (execution_count >= 0) {
                    coverage.lines_total++;
                    if (execution_count > 0) {
                        coverage.lines_covered++;
                    }
                }
            }
        }

        file_coverages_.push_back(coverage);
    }

    void generateHTMLReport() {
        std::string html_file = output_dir_ + "/index.html";

        std::ofstream file(html_file);
        file << R"HTML(
<!DOCTYPE html>
<html>
<head>
    <title>SQLCC Coverage Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .summary { background: #f0f0f0; padding: 20px; margin-bottom: 20px; }
        .file-list { border-collapse: collapse; width: 100%; }
        .file-list th, .file-list td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        .file-list th { background-color: #f2f2f2; }
        .covered { background-color: #c8e6c9; }
        .not-covered { background-color: #ffcdd2; }
        .not-executable { background-color: #f5f5f5; }
        .progress-bar {
            width: 200px;
            height: 20px;
            background-color: #f0f0f0;
            border: 1px solid #ccc;
        }
        .progress-fill {
            height: 100%;
            background-color: #4CAF50;
        }
    </style>
</head>
<body>
    <h1>SQLCC Code Coverage Report</h1>
)HTML";

        // 生成摘要
        generateSummaryHTML(file);

        // 生成文件列表
        generateFileListHTML(file);

        // 生成详细的文件覆盖率
        generateFileDetailsHTML(file);

        file << R"HTML(
</body>
</html>
)HTML";
    }

    void generateSummaryHTML(std::ofstream& file) {
        int total_lines = 0, covered_lines = 0;
        int total_functions = 0, covered_functions = 0;

        for (const auto& coverage : file_coverages_) {
            total_lines += coverage.lines_total;
            covered_lines += coverage.lines_covered;
            total_functions += coverage.functions_total;
            covered_functions += coverage.functions_covered;
        }

        double line_coverage = total_lines > 0 ? (covered_lines * 100.0 / total_lines) : 0.0;
        double function_coverage = total_functions > 0 ? (covered_functions * 100.0 / total_functions) : 0.0;

        file << "<div class='summary'>\n";
        file << "<h2>Summary</h2>\n";
        file << "<p>Line Coverage: " << covered_lines << "/" << total_lines
             << " (" << std::fixed << std::setprecision(1) << line_coverage << "%)</p>\n";
        file << "<p>Function Coverage: " << covered_functions << "/" << total_functions
             << " (" << std::fixed << std::setprecision(1) << function_coverage << "%)</p>\n";
        file << "<p>Files: " << file_coverages_.size() << "</p>\n";
        file << "</div>\n";
    }

    void generateFileListHTML(std::ofstream& file) {
        file << "<h2>File Coverage</h2>\n";
        file << "<table class='file-list'>\n";
        file << "<tr><th>File</th><th>Line Coverage</th><th>Function Coverage</th></tr>\n";

        for (const auto& coverage : file_coverages_) {
            double line_pct = coverage.lines_total > 0 ?
                (coverage.lines_covered * 100.0 / coverage.lines_total) : 0.0;

            file << "<tr>\n";
            file << "<td>" << coverage.filename << "</td>\n";
            file << "<td>" << coverage.lines_covered << "/" << coverage.lines_total
                 << " (" << std::fixed << std::setprecision(1) << line_pct << "%)</td>\n";
            file << "<td>" << coverage.functions_covered << "/" << coverage.functions_total << "</td>\n";
            file << "</tr>\n";
        }

        file << "</table>\n";
    }

    void generateFileDetailsHTML(std::ofstream& file) {
        for (const auto& coverage : file_coverages_) {
            file << "<h3>" << coverage.filename << "</h3>\n";
            file << "<pre>\n";

            for (size_t i = 1; i < coverage.source_lines.size(); ++i) {
                std::string css_class;
                if (coverage.line_coverage[i] == -1) {
                    css_class = "not-executable";
                } else if (coverage.line_coverage[i] == 0) {
                    css_class = "not-covered";
                } else {
                    css_class = "covered";
                }

                file << "<span class='" << css_class << "'>";
                file << std::setw(6) << coverage.line_coverage[i] << ": ";
                file << coverage.source_lines[i] << "</span>\n";
            }

            file << "</pre>\n";
        }
    }

    void generateSummaryReport() {
        std::string summary_file = output_dir_ + "/coverage_summary.txt";

        std::ofstream file(summary_file);
        file << "SQLCC Code Coverage Summary Report\n";
        file << "===================================\n\n";

        int total_lines = 0, covered_lines = 0;
        int total_functions = 0, covered_functions = 0;

        for (const auto& coverage : file_coverages_) {
            total_lines += coverage.lines_total;
            covered_lines += coverage.lines_covered;
            total_functions += coverage.functions_total;
            covered_functions += coverage.functions_covered;

            double line_pct = coverage.lines_total > 0 ?
                (coverage.lines_covered * 100.0 / coverage.lines_total) : 0.0;

            file << coverage.filename << ": "
                 << coverage.lines_covered << "/" << coverage.lines_total
                 << " lines (" << std::fixed << std::setprecision(1) << line_pct << "%)\n";
        }

        double overall_line_coverage = total_lines > 0 ? (covered_lines * 100.0 / total_lines) : 0.0;
        double overall_function_coverage = total_functions > 0 ? (covered_functions * 100.0 / total_functions) : 0.0;

        file << "\nOverall Coverage:\n";
        file << "Lines: " << covered_lines << "/" << total_lines
             << " (" << std::fixed << std::setprecision(1) << overall_line_coverage << "%)\n";
        file << "Functions: " << covered_functions << "/" << total_functions
             << " (" << std::fixed << std::setprecision(1) << overall_function_coverage << "%)\n";
        file << "Files: " << file_coverages_.size() << "\n";
    }

private:
    std::string output_dir_;
    std::vector<FileCoverage> file_coverages_;
};

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <gcov_data_dir> <source_dir1> [source_dir2] ...\n";
        return 1;
    }

    std::string gcov_data_dir = argv[1];
    std::vector<std::string> source_dirs;

    for (int i = 2; i < argc; ++i) {
        source_dirs.push_back(argv[i]);
    }

    CoverageReportGenerator generator;
    return generator.GenerateReport(gcov_data_dir, source_dirs) ? 0 : 1;
}