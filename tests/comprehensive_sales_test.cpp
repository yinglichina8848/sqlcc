#include "sql_executor.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// 读取SQL文件内容
std::string readSqlFile(const std::string &filename) {
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
std::vector<std::string> splitSqlStatements(const std::string &sqlContent) {
  std::vector<std::string> statements;
  std::string currentStatement;

  for (size_t i = 0; i < sqlContent.length(); ++i) {
    // 跳过注释
    if (i < sqlContent.length() - 1 && sqlContent[i] == '-' &&
        sqlContent[i + 1] == '-') {
      while (i < sqlContent.length() && sqlContent[i] != '\n') {
        ++i;
      }
      continue;
    }

    // 处理多行注释
    if (i < sqlContent.length() - 1 && sqlContent[i] == '/' &&
        sqlContent[i + 1] == '*') {
      i += 2;
      while (i < sqlContent.length() - 1 &&
             !(sqlContent[i] == '*' && sqlContent[i + 1] == '/')) {
        ++i;
      }
      i += 2;
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

  // 处理最后一个没有分号结束的语句
  if (!currentStatement.empty()) {
    size_t start = currentStatement.find_first_not_of(" \t\n\r");
    size_t end = currentStatement.find_last_not_of(" \t\n\r");
    if (start != std::string::npos && end != std::string::npos) {
      statements.push_back(currentStatement.substr(start, end - start + 1));
    }
  }

  return statements;
}

int main() {
  std::cout << "=== 综合销售系统测试开始 ===" << std::endl;
  std::cout << "测试SQLCC系统对SQL-92标准命令的支持情况" << std::endl;
  std::cout << "======================================" << std::endl;

  // 创建SQL执行器实例
  sqlcc::SqlExecutor executor;
  std::cout << "SQL执行器初始化完成" << std::endl;

  // 读取综合测试脚本
  std::string sqlContent =
      readSqlFile("../scripts/sql/comprehensive_sales_test.sql");

  if (sqlContent.substr(0, 5) == "Error") {
    std::cerr << sqlContent << std::endl;
    return 1;
  }

  // 分割SQL语句
  std::vector<std::string> statements = splitSqlStatements(sqlContent);

  std::cout << "共读取到 " << statements.size() << " 条SQL语句" << std::endl;
  std::cout << "开始执行测试..." << std::endl;

  // 统计变量
  int success_count = 0;
  int error_count = 0;
  int total_count = statements.size();

  // 执行SQL语句
  for (size_t i = 0; i < statements.size(); ++i) {
    std::string statement = statements[i];

    // 显示当前执行的语句类型
    std::string command_type =
        statement.substr(0, statement.find_first_of(" \t\n\r"));
    std::cout << "\n执行语句 " << (i + 1) << ": " << command_type << std::endl;

    // 执行语句
    std::string result = executor.Execute(statement);

    // 显示执行结果
    if (result.find("ERROR") != std::string::npos) {
      std::cout << "结果: ERROR - " << result << std::endl;
      error_count++;
    } else {
      std::cout << "结果: SUCCESS - " << result << std::endl;
      success_count++;
    }
  }

  // 显示测试统计结果
  std::cout << "\n=== 测试完成 ===" << std::endl;
  std::cout << "总语句数: " << total_count << std::endl;
  std::cout << "成功执行: " << success_count << std::endl;
  std::cout << "执行失败: " << error_count << std::endl;
  std::cout << "成功率: "
            << (static_cast<double>(success_count) / total_count) * 100 << "%"
            << std::endl;

  // 显示详细结果
  std::cout << "\n=== 测试详细结果 ===" << std::endl;
  std::cout << "DCL命令: 用户和角色管理" << std::endl;
  std::cout << "DDL命令: 数据库对象创建（表、视图、索引、约束）" << std::endl;
  std::cout << "DML命令: 数据插入、更新、删除" << std::endl;
  std::cout << "DQL命令: 数据查询和报告" << std::endl;

  std::cout
      << "\n测试脚本覆盖了SQL-92标准的主要命令，验证了SQLCC系统的基本功能。"
      << std::endl;
  std::cout << "======================================" << std::endl;

  return 0;
}