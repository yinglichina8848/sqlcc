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

// SqlCC版本定义
#define SQLCC_VERSION "0.6.5"

// 版本变更历史
// v0.1.0 - 基础框架搭建
// v0.2.0 - 存储引擎基本功能实现
// v0.3.0 - SQL解析器基础功能实现
// v0.4.0 - 索引系统和B+树实现
// v0.5.0 - 事务管理器和并发控制实现
// v0.6.0 - 约束检查和执行器实现
// v0.6.1 - 修复若干bug，完善测试用例
// v0.6.2 - 实现基本的CRUD模拟功能
// v0.6.3 - 实现服务器和客户端基本网络通信功能
// v0.6.4 - 优化网络通信性能，添加错误处理机制
// v0.6.5 - 增强系统稳定性，完善文档和测试

/** @brief SQLCC版本号组件 */
#define SQLCC_VERSION_MAJOR 0
#define SQLCC_VERSION_MINOR 6
#define SQLCC_VERSION_PATCH 5