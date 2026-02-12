/**
 * @file core_interfaces.h
 * @brief SQLCC Core 模块接口汇总头文件
 * @author SQLCC Team
 * @date 2026-02-11
 * @copyright Copyright (c) 2026
 *
 * 文件用途说明：
 * 本文件汇总了 Core 模块的所有公共接口，便于其他模块统一引用。
 * 使用此头文件而非直接引用具体实现头文件，可有效降低编译依赖。
 *
 * 使用示例：
 * @code
 * #include "src/core/interfaces/core_interfaces.h"
 *
 * void SomeFunction(sqlcc::core::interfaces::IDatabaseManager* db_mgr) {
 *     db_mgr->CreateDatabase("test_db");
 * }
 * @endcode
 */

#pragma once

// Core 模块公共接口
#include "i_database_manager.h"
#include "i_execution_context.h"
#include "i_user_manager.h"

/**
 * @namespace sqlcc::core::interfaces
 * @brief SQLCC Core 模块接口命名空间
 *
 * 此命名空间包含 Core 模块的所有抽象接口定义：
 * - IDatabaseManager: 数据库管理器接口
 * - IExecutionContext: 执行上下文接口
 * - IUserManager: 用户管理器接口
 * - IStorageEngine: 存储引擎接口（预留）
 * - IIndexManager: 索引管理器接口（预留）
 * - ITransactionManager: 事务管理器接口（预留）
 * - IConfigManager: 配置管理器接口（预留）
 *
 * 设计原则：
 * 1. 所有接口都是纯虚类
 * 2. 接口不暴露实现细节
 * 3. 使用智能指针进行资源管理
 * 4. 返回值使用 bool 表示成功/失败
 */
namespace sqlcc {
namespace core {
namespace interfaces {

// 接口版本信息
constexpr int kCoreInterfaceVersionMajor = 1;
constexpr int kCoreInterfaceVersionMinor = 0;
constexpr int kCoreInterfaceVersionPatch = 0;

/**
 * @brief 获取接口版本字符串
 * @return 版本字符串，格式为 "major.minor.patch"
 */
inline std::string GetCoreInterfaceVersion() {
    return std::to_string(kCoreInterfaceVersionMajor) + "." +
           std::to_string(kCoreInterfaceVersionMinor) + "." +
           std::to_string(kCoreInterfaceVersionPatch);
}

} // namespace interfaces
} // namespace core
} // namespace sqlcc
