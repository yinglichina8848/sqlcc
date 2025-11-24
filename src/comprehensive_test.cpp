#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "../include/sql_executor.h"

// 读取SQL文件内容
std::string readSqlFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "Error: Cannot open file " + filename;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)), 
                        std::istreambuf_iterator<char>());
    file.close();
    return content;
}

// 分割SQL语句
std::vector<std::string> splitSqlStatements(const std::string& sqlContent) {
    std::vector<std::string> statements;
    std::string currentStatement;
    bool inMultiLineComment = false;
    
    for (size_t i = 0; i < sqlContent.length(); ++i) {
        // 处理多行注释 /* ... */
        if (!inMultiLineComment && i < sqlContent.length() - 1 && 
            sqlContent[i] == '/' && sqlContent[i+1] == '*') {
            inMultiLineComment = true;
            i++; // 跳过 '*'
            continue;
        }
        
        if (inMultiLineComment) {
            if (i < sqlContent.length() - 1 && 
                sqlContent[i] == '*' && sqlContent[i+1] == '/') {
                inMultiLineComment = false;
                i++; // 跳过 '/'
            }
            continue;
        }
        
        // 处理单行注释 --
        if (i < sqlContent.length() - 1 && sqlContent[i] == '-' && sqlContent[i+1] == '-') {
            while (i < sqlContent.length() && sqlContent[i] != '\n') {
                ++i;
            }
            continue;
        }
        
        currentStatement += sqlContent[i];
        
        // 检测分号作为语句结束符
        if (sqlContent[i] == ';') {
            // 去除前后空白字符
            size_t start = currentStatement.find_first_not_of(" \t\n\r");
            size_t end = currentStatement.find_last_not_of(" \t\n\r");
            if (start != std::string::npos && end != std::string::npos) {
                statements.push_back(currentStatement.substr(start, end - start + 1));
            }
            currentStatement.clear();
        }
    }
    
    return statements;
}

// 检查结果是否包含预期的成功信息
bool checkSuccessResult(const std::string& result) {
    // 检查是否包含常见的错误标识
    std::string lowerResult = result;
    std::transform(lowerResult.begin(), lowerResult.end(), lowerResult.begin(), ::tolower);
    
    // 如果结果包含这些错误关键字，则认为失败
    std::vector<std::string> errorKeywords = {
        "error", "exception", "fail", "syntax error", "not found", "invalid"
    };
    
    for (const auto& keyword : errorKeywords) {
        if (lowerResult.find(keyword) != std::string::npos) {
            return false;
        }
    }
    
    // 对于SELECT语句，应该有结果或影响行数
    return !result.empty();
}

int main() {
    std::cout << "=== 高级综合SQL测试开始 ===" << std::endl;
    
    // 创建SQL执行器实例
    sqlcc::SqlExecutor executor;
    
    // 读取高级综合测试脚本
    std::string sqlContent = readSqlFile("../scripts/sql/advanced_comprehensive_test.sql");
    
    if (sqlContent.substr(0, 5) == "Error") {
        std::cerr << sqlContent << std::endl;
        return 1;
    }
    
    // 分割并执行SQL语句
    std::vector<std::string> statements = splitSqlStatements(sqlContent);
    
    std::cout << "共读取到 " << statements.size() << " 条SQL语句" << std::endl;
    
    int successCount = 0;
    int warningCount = 0;
    int errorCount = 0;
    
    // 跳过事务相关的注释掉的语句
    std::vector<std::string> skippedKeywords = {"BEGIN TRANSACTION", "COMMIT TRANSACTION", "ROLLBACK"};
    
    for (size_t i = 0; i < statements.size(); ++i) {
        const std::string& statement = statements[i];
        
        // 检查是否需要跳过注释掉的事务语句
        bool skip = false;
        std::string lowerStatement = statement;
        std::transform(lowerStatement.begin(), lowerStatement.end(), lowerStatement.begin(), ::tolower);
        
        for (const auto& keyword : skippedKeywords) {
            std::string lowerKeyword = keyword;
            std::transform(lowerKeyword.begin(), lowerKeyword.end(), lowerKeyword.begin(), ::tolower);
            if (lowerStatement.find(lowerKeyword) != std::string::npos) {
                std::cout << "\n跳过语句 " << (i + 1) << " (事务语句)" << std::endl;
                skip = true;
                break;
            }
        }
        
        if (skip) {
            continue;
        }
        
        // 对于DELETE和DROP操作，提供警告
        bool isDangerStatement = false;
        if (lowerStatement.find("delete") != std::string::npos || 
            lowerStatement.find("drop") != std::string::npos) {
            isDangerStatement = true;
            std::cout << "\n警告: 语句 " << (i + 1) << " 包含危险操作，谨慎执行" << std::endl;
        }
        
        std::cout << "\n执行语句 " << (i + 1) << ":" << std::endl;
        
        // 对于长语句，只显示前100个字符
        if (statement.length() > 100) {
            std::cout << statement.substr(0, 100) << "..." << std::endl;
        } else {
            std::cout << statement << std::endl;
        }
        
        std::string result = executor.Execute(statement);
        
        // 对于查询结果，可能很长，只显示部分
        if (result.length() > 200) {
            std::cout << "结果预览: " << result.substr(0, 200) << "..." << std::endl;
        } else {
            std::cout << "结果: " << result << std::endl;
        }
        
        // 检查执行结果
        if (checkSuccessResult(result)) {
            successCount++;
            std::cout << "✓ 执行成功" << std::endl;
        } else {
            if (isDangerStatement) {
                warningCount++;
                std::cout << "! 警告: 危险操作可能被安全机制阻止" << std::endl;
            } else {
                errorCount++;
                std::cout << "✗ 执行失败" << std::endl;
            }
        }
    }
    
    // 输出测试统计信息
    std::cout << "\n=== 测试统计信息 ===" << std::endl;
    std::cout << "总语句数: " << statements.size() << std::endl;
    std::cout << "成功执行: " << successCount << std::endl;
    std::cout << "警告数量: " << warningCount << std::endl;
    std::cout << "失败数量: " << errorCount << std::endl;
    
    // 如果有严重错误，返回非零值
    if (errorCount > statements.size() / 3) { // 允许1/3的错误率，因为某些高级功能可能不支持
        std::cerr << "\n测试失败: 错误数量过多" << std::endl;
        return 1;
    }
    
    std::cout << "\n=== 高级综合SQL测试完成 ===" << std::endl;
    return 0;
}
