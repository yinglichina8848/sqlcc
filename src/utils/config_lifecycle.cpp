/**
 * @file config_lifecycle.cpp
 * @brief RAII配置生命周期管理器实现文件
 *
 * Why: 实现RAII模式的配置生命周期管理，确保资源自动清理
 * What: 提供ConfigLifecycleManager类和相关的RAII配置访问功能的实现
 * How: 使用RAII模式管理配置资源的获取和释放
 */

#include "include/utils/config_lifecycle.h"
#include <sstream>
#include <iomanip>
#include <chrono>

namespace sqlcc {

// ConfigLifecycleManager 实现已在头文件中提供（inline函数）
// 工具函数实现已在config_snapshot.cpp中提供

std::string FormatConfigValue(const ConfigValue& value) {
    return std::visit([](const auto& val) -> std::string {
        if constexpr (std::is_same_v<std::decay_t<decltype(val)>, bool>) {
            return val ? "true" : "false";
        } else if constexpr (std::is_same_v<std::decay_t<decltype(val)>, std::string>) {
            return "\"" + val + "\"";
        } else {
            return std::to_string(val);
        }
    }, value);
}

bool ParseConfigValue(const std::string& str, ConfigValue& value) {
    std::string trimmed_str = str;
    
    // 去除前后空格
    trimmed_str.erase(0, trimmed_str.find_first_not_of(" \t\r\n"));
    trimmed_str.erase(trimmed_str.find_last_not_of(" \t\r\n") + 1);
    
    // 处理布尔值
    if (trimmed_str == "true" || trimmed_str == "TRUE" || trimmed_str == "True") {
        value = true;
        return true;
    } else if (trimmed_str == "false" || trimmed_str == "FALSE" || trimmed_str == "False") {
        value = false;
        return true;
    }
    
    // 处理整数
    try {
        size_t pos;
        int int_val = std::stoi(trimmed_str, &pos);
        if (pos == trimmed_str.length()) {
            value = int_val;
            return true;
        }
    } catch (...) {
        // 不是整数，继续尝试其他类型
    }
    
    // 处理浮点数
    try {
        size_t pos;
        double double_val = std::stod(trimmed_str, &pos);
        if (pos == trimmed_str.length()) {
            value = double_val;
            return true;
        }
    } catch (...) {
        // 不是浮点数，作为字符串处理
    }
    
    // 作为字符串处理
    value = trimmed_str;
    return true;
}

}  // namespace sqlcc
