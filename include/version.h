/**
 * @file version.h
 * @brief 数据库系统版本信息定义
 * 
 * Why: 需要一个统一的位置来定义和管理数据库系统的版本信息
 * What: 定义了SQLCC_VERSION宏，包含当前数据库系统的版本号
 * How: 使用#define预处理指令定义版本号字符串常量
 */

// Why: 防止头文件被多次包含，避免编译错误
// What: 使用#pragma once指令确保头文件只被编译一次
// How: 在文件开头添加#pragma once预处理指令
#pragma once

<<<<<<< Updated upstream
// Why: 定义数据库系统的版本号，便于版本管理和兼容性检查
// What: 定义SQLCC_VERSION宏，包含当前数据库系统的版本号字符串"0.2.5"
// How: 使用#define预处理指令定义版本号字符串常量
#define SQLCC_VERSION "0.2.5"

=======
/** @brief SQLCC项目版本号 */
#define SQLCC_VERSION "0.3.5"
>>>>>>> Stashed changes
