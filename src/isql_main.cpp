/**
 * @file isql_main.cc
 * @brief 交互式SQL命令行工具主程序
 *
 * Why: 需要一个交互式命令行工具来执行SQL语句和脚本
 * What: isql工具支持交互式执行SQL命令，以及通过-f参数执行SQL脚本
 * How: 解析命令行参数，创建存储引擎和SQL执行器，实现交互循环
 */

#include <csignal>
#include <iostream>
#include <memory>
#include <string>

// 条件包含readline库
#ifdef USE_READLINE
#include <readline/history.h>
#include <readline/readline.h>
#endif

#include "../include/config_manager.h"
#include "../include/sql_executor.h"
#include "exception.h"
#include "storage_engine.h"
#include "version.h"

// 使用sqlcc命名空间
using namespace sqlcc;

// 全局标志，用于控制程序是否继续运行
volatile sig_atomic_t g_running = 1;

/**
 * @brief 信号处理函数，处理Ctrl+C信号
 */
void signalHandler(int signal) {
  if (signal == SIGINT) {
    std::cout << "\nInterrupted by user. Type 'exit' or 'quit' to exit."
              << std::endl;
    // 重置readline的输入行（如果可用）
#ifdef USE_READLINE
    rl_replace_line("", 0);
    rl_on_new_line();
    rl_redisplay();
#endif
    g_running = true; // 确保程序继续运行
  }
}

/**
 * @brief 显示帮助信息
 */
void showHelp() {
  std::cout << "Usage: isql [options]" << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "  -f <file>   Execute SQL script from file" << std::endl;
  std::cout << "  -h, --help  Show this help message" << std::endl;
  std::cout << "  -v, --version  Show version information" << std::endl;
  std::cout << std::endl;
  std::cout << "Interactive commands:" << std::endl;
  std::cout << "  exit, quit  Exit the program" << std::endl;
  std::cout << "  help        Show this help message" << std::endl;
  std::cout << "  show tables, .schema  Show all tables" << std::endl;
  std::cout << "  describe <table>, .desc <table>  Show table structure"
            << std::endl;
  std::cout << "  show create table <table>  Show CREATE TABLE statement"
            << std::endl;
  std::cout << "  .exit, .quit  Exit the program" << std::endl;
  std::cout << std::endl;
  std::cout << "Supported SQL commands:" << std::endl;
  std::cout << "  CREATE DATABASE, CREATE TABLE" << std::endl;
  std::cout << "  USE database" << std::endl;
  std::cout << "  INSERT INTO, SELECT, UPDATE, DELETE" << std::endl;
  std::cout << "  ALTER TABLE, DROP TABLE, DROP DATABASE" << std::endl;
}

/**
 * @brief 显示版本信息
 */
void showVersion() {
  std::cout << "isql (SqlCC) 0.6.5" << std::endl;
  std::cout << "Interactive SQL command-line utility" << std::endl;
}

/**
 * @brief 处理交互式命令
 * @param cmd 命令字符串
 * @param executor SQL执行器引用
 * @return 是否应该退出程序
 */
bool handleInteractiveCommand(const std::string &cmd, SqlExecutor &executor) {
  if (cmd == "exit" || cmd == "quit" || cmd == ".exit" || cmd == ".quit") {
    std::cout << "Goodbye!" << std::endl;
    return true;
  }

  if (cmd == "help" || cmd == ".help") {
    showHelp();
    return false;
  }

  if (cmd == ".schema" || cmd == "show tables" || cmd == ".show tables") {
    std::string result = executor.Execute("SHOW TABLES");
    // 临时替代ListTables方法
    std::cout << result << std::endl;
    return false;
  }

  // 处理DESCRIBE TABLE命令
  if (cmd.substr(0, 8) == "describe" || cmd.substr(0, 4) == ".desc" ||
      cmd.substr(0, 7) == "show create table") {
    std::string table_name;
    size_t pos = cmd.find_last_of(' ');
    if (pos != std::string::npos) {
      table_name = cmd.substr(pos + 1);
      // 移除可能的引号
      if (!table_name.empty() &&
          (table_name[0] == '\'' || table_name[0] == '"')) {
        table_name = table_name.substr(1, table_name.length() - 2);
      }
      std::string result = executor.Execute("SHOW CREATE TABLE " + table_name);
      // 临时替代ShowTableSchema方法
      std::cout << result << std::endl;
    } else {
      std::cout << "Error: Table name required" << std::endl;
    }
    return false;
  }

  return false;
}

/**
 * @brief 执行SQL脚本文件
 * @param file_path 文件路径
 * @param executor SQL执行器引用
 * @return 执行是否成功
 */
bool executeScript(const std::string &file_path, SqlExecutor &executor) {
  try {
    std::cout << "Executing script from file: " << file_path << std::endl;
    std::string result = executor.ExecuteFile(file_path);
    std::cout << result << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error executing script: " << e.what() << std::endl;
    return false;
  }
}

/**
 * @brief 主函数
 */
int main(int argc, char *argv[]) {
  // 注册信号处理函数
  std::signal(SIGINT, signalHandler);

  // 解析命令行参数
  std::string script_file;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      showHelp();
      return 0;
    } else if (arg == "-v" || arg == "--version") {
      showVersion();
      return 0;
    } else if (arg == "-f" && i + 1 < argc) {
      script_file = argv[++i];
    } else {
      std::cerr << "Unknown option: " << arg << std::endl;
      showHelp();
      return 1;
    }
  }

  try {
    // 创建配置管理器（使用默认参数）
    auto config_mgr = std::make_shared<sqlcc::ConfigManager>();
    config_mgr->LoadDefaultConfig();

    // 创建临时数据库文件路径
    std::string db_path = "./temp_isql_db";

    // 创建数据库管理器
    // DatabaseManager db_manager(db_path, 1024, 4, 16);

    // 创建SQL执行器
    SqlExecutor executor;

    // 如果指定了脚本文件，则执行脚本
    if (!script_file.empty()) {
      return executeScript(script_file, executor) ? 0 : 1;
    }

    // 设置readline配置（如果可用）
#ifdef USE_READLINE
    rl_bind_key("\t", rl_complete);
    using_history();
#endif

    // 显示欢迎信息
    std::cout << "Welcome to isql (SqlCC 0.6.5)" << std::endl;
    std::cout << "Type 'help' for help, 'exit' or 'quit' to exit." << std::endl;
    std::cout << std::endl;

    // 交互式命令循环
    std::string prompt = "sqlcc> ";
    std::string current_sql;

    while (g_running) {
      std::string input;

      // 显示提示符
      std::cout << prompt;
      std::cout.flush();

// 读取输入
#ifdef USE_READLINE
      char *line = readline("");

      // 如果用户按下Ctrl+D，退出程序
      if (line == nullptr) {
        std::cout << "\nGoodbye!" << std::endl;
        break;
      }

      input = line;
      free(line);

      // 保存到历史记录
      if (!input.empty()) {
        add_history(input.c_str());
      }
#else
      // 使用标准输入
      if (!std::getline(std::cin, input)) {
        // 处理EOF或错误
        std::cout << "\nGoodbye!" << std::endl;
        break;
      }
#endif

      // 检查是否是交互式命令（以.开头或特殊命令）
      if (!input.empty() && (input[0] == '.' || input == "exit" ||
                             input == "quit" || input == "help")) {
        if (handleInteractiveCommand(input, executor)) {
          break;
        }
        continue;
      }

      // 处理SQL语句
      current_sql += input;

      // 检查语句是否结束（以分号结尾）
      if (current_sql.find_last_of(';') != std::string::npos) {
        // 执行SQL语句
        std::string result = executor.Execute(current_sql);
        std::cout << result << std::endl;

        // 重置当前SQL语句
        current_sql.clear();
      } else {
        // SQL语句未结束，继续输入
        prompt = "...> ";
        continue;
      }

      // 重置提示符
      prompt = "sqlcc> ";
    }

    // 清理资源
    // 在模拟模式下，不需要清理存储引擎资源

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}