/**
 * @file config_snapshot.cpp
 * @brief 智能配置快照管理器实现文件
 * 
 * Why: 实现线程安全的配置快照机制，支持无锁配置读取和版本管理
 * What: 提供ConfigSnapshot类和相关的智能指针管理功能的实现
 * How: 使用std::shared_ptr实现配置快照的共享和自动生命周期管理
 */

#include "config_snapshot.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace sqlcc {

// ConfigSnapshotFactory 实现已在头文件中提供（inline函数）
// ConfigSnapshotManager 实现已在头文件中提供（inline函数）

// 工具函数实现

/**
 * @brief 生成版本ID
 * @param prefix 前缀
 * @return std::string 版本ID
 */
std::string GenerateVersionId(const std::string& prefix) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << prefix << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") 
       << "_" << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}



}  // namespace sqlcc
