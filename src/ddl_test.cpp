#include <iostream>
#include <fstream>
#include <string>
#include <vector>
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
    
    for (size_t i = 0; i < sqlContent.length(); ++i) {
        // 跳过注释
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

int main() {
    std::cout << "=== DDL（数据定义语言）测试开始 ===" << std::endl;
    
    // 创建SQL执行器实例
    sqlcc::SqlExecutor executor;
    
    // 读取DDL测试脚本
    std::string sqlContent = readSqlFile("../scripts/sql/ddl_test_script.sql");
    
    if (sqlContent.substr(0, 5) == "Error") {
        std::cerr << sqlContent << std::endl;
        return 1;
    }
    
    // 分割并执行SQL语句
    std::vector<std::string> statements = splitSqlStatements(sqlContent);
    
    std::cout << "共读取到 " << statements.size() << " 条SQL语句" << std::endl;
    
    for (size_t i = 0; i < statements.size(); ++i) {
        std::cout << "\n执行语句 " << (i + 1) << ":" << std::endl;
        // 对于长语句，只显示前100个字符
        if (statements[i].length() > 100) {
            std::cout << statements[i].substr(0, 100) << "..." << std::endl;
        } else {
            std::cout << statements[i] << std::endl;
        }
        
        std::string result = executor.Execute(statements[i]);
        std::cout << "结果: " << result << std::endl;
        
        // 检查是否有错误
        if (result.find("Error") != std::string::npos && result.find("Error: Cannot open file") == std::string::npos) {
            std::cout << "警告: 语句执行可能存在问题" << std::endl;
        }
    }
    
    std::cout << "\n=== DDL测试完成 ===" << std::endl;
    return 0;
}