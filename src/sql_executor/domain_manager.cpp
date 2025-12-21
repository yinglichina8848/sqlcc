#include "sql_executor/domain_manager.h"
#include <stdexcept>

namespace sqlcc {
namespace sql_executor {

// DomainManager 实现
DomainManager& DomainManager::getInstance() {
    static DomainManager instance;
    return instance;
}

bool DomainManager::createDomain(const sql_parser::CreateDomainStatement& stmt) {
    const auto& domainDef = stmt.getDomainDefinition();
    std::string domainName = domainDef.getName();

    if (domainExists(domainName)) {
        return false; // 域已存在
    }

    DomainInfo info;
    info.name = domainName;
    info.baseType = domainDef.getBaseType();
    info.characterLength = domainDef.getCharacterLength();
    info.precision = domainDef.getPrecision();
    info.scale = domainDef.getScale();
    info.defaultValue = domainDef.getDefaultValue();
    info.checkConstraint = domainDef.getCheckConstraint();
    info.notNull = domainDef.isNotNull();
    info.owner = "current_user"; // TODO: 从上下文获取当前用户
    info.created_time = time(nullptr);

    domains_[domainName] = info;
    return true;
}

bool DomainManager::alterDomain(const sql_parser::AlterDomainStatement& stmt) {
    // 简化实现：这里应该实现域修改逻辑
    (void)stmt; // 避免未使用参数警告
    return true;
}

bool DomainManager::dropDomain(const std::string& domainName, bool ifExists) {
    if (!domainExists(domainName)) {
        return ifExists; // 如果指定了IF EXISTS且域不存在，返回true
    }

    domains_.erase(domainName);
    return true;
}

bool DomainManager::domainExists(const std::string& domainName) const {
    return domains_.find(domainName) != domains_.end();
}

bool DomainManager::validateValue(const std::string& domainName, const std::string& value) const {
    auto it = domains_.find(domainName);
    if (it == domains_.end()) {
        return false;
    }

    const DomainInfo& info = it->second;

    // 检查NOT NULL约束
    if (info.notNull && value.empty()) {
        return false;
    }

    // 检查长度约束
    if (info.baseType == sql_parser::DomainDefinition::VARCHAR ||
        info.baseType == sql_parser::DomainDefinition::CHARACTER) {
        if (value.length() > static_cast<size_t>(info.characterLength)) {
            return false;
        }
    }

    // 检查精度约束（对于数值类型）
    if (info.baseType == sql_parser::DomainDefinition::DECIMAL ||
        info.baseType == sql_parser::DomainDefinition::NUMERIC) {
        // 简化实现：这里应该检查数值精度和范围
        (void)info.precision; // 避免未使用参数警告
        (void)info.scale; // 避免未使用参数警告
    }

    // 检查检查约束
    if (!info.checkConstraint.empty()) {
        // 简化实现：这里应该执行检查约束
        (void)info.checkConstraint; // 避免未使用参数警告
    }

    return true;
}

std::string DomainManager::getDomainType(const std::string& domainName) const {
    auto it = domains_.find(domainName);
    if (it == domains_.end()) {
        return "";
    }

    const DomainInfo& info = it->second;
    switch (info.baseType) {
        case sql_parser::DomainDefinition::INTEGER:
            return "INTEGER";
        case sql_parser::DomainDefinition::CHARACTER:
            return "CHARACTER";
        case sql_parser::DomainDefinition::VARCHAR:
            return "VARCHAR";
        case sql_parser::DomainDefinition::DECIMAL:
            return "DECIMAL";
        case sql_parser::DomainDefinition::NUMERIC:
            return "NUMERIC";
        case sql_parser::DomainDefinition::BOOLEAN:
            return "BOOLEAN";
        default:
            return "UNKNOWN";
    }
}

std::string DomainManager::getDomainInfo(const std::string& domainName) const {
    auto it = domains_.find(domainName);
    if (it == domains_.end()) {
        return "Domain not found: " + domainName;
    }

    const DomainInfo& info = it->second;
    std::string result = "Domain: " + info.name + "\n";
    result += "Type: " + getDomainType(domainName) + "\n";
    result += "Owner: " + info.owner + "\n";
    result += "Created: " + std::to_string(info.created_time) + "\n";

    if (!info.defaultValue.empty()) {
        result += "Default: " + info.defaultValue + "\n";
    }

    if (info.notNull) {
        result += "NOT NULL: true\n";
    }

    if (!info.dependent_columns.empty()) {
        result += "Used by columns: ";
        for (size_t i = 0; i < info.dependent_columns.size(); ++i) {
            if (i > 0) result += ", ";
            result += info.dependent_columns[i];
        }
        result += "\n";
    }

    return result;
}

std::vector<std::string> DomainManager::listDomains() const {
    std::vector<std::string> result;
    for (const auto& pair : domains_) {
        result.push_back(pair.first);
    }
    return result;
}

} // namespace sql_executor
} // namespace sqlcc
