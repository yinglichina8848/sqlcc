/**
 * @file main.cc
 * @brief 数据库系统主程序入口
 * 
 * Why: 需要一个程序入口点来启动数据库系统，展示系统版本信息
 * What: 主程序负责初始化数据库系统，显示版本信息，然后正常退出
 * How: 包含必要的头文件，调用main函数，输出版本信息后返回0
 */

// Why: 需要使用标准输入输出流来显示版本信息
// What: 包含iostream头文件，提供std::cout功能
// How: 使用#include预处理指令包含标准库头文件
#include <iostream>

// Why: 需要获取数据库系统的版本信息
// What: 包含version.h头文件，提供SQLCC_VERSION宏定义
// How: 使用#include预处理指令包含项目头文件
#include "version.h"

/**
 * @brief 主函数入口
 * 
 * Why: 需要一个程序入口点来启动数据库系统
 * What: 主函数负责显示数据库系统版本信息，然后正常退出
 * How: 使用std::cout输出版本信息，然后返回0表示程序正常结束
 * 
 * @return int 程序退出状态码，0表示正常退出
 */
int main() {
    // Why: 需要显示数据库系统的版本信息，让用户了解当前运行的版本
    // What: 使用std::cout输出数据库系统名称和版本号
    // How: 使用流插入运算符<<输出字符串和SQLCC_VERSION宏
    std::cout << "SqlCC " << SQLCC_VERSION << " startup!" << std::endl;
    
    // Why: 需要返回一个状态码表示程序正常结束
    // What: 返回0表示程序成功执行完毕
    // How: 使用return语句返回0
    return 0;
}

