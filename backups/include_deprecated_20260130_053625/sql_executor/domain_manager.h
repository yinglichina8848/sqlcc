#pragma once

#include "sql_parser/advanced_sql92_features.h"
#include <unordered_map>
#include <string>
#include <vector>

namespace sqlcc {
namespace sql_executor {

/**
 * 域(用户定义类型)管理器
 */
class DomainManager {
public:
    static DomainManager& getInstance();

    // 域管理
    bool createDomain(const sql_parser::CreateDomainStatement& stmt);
    bool alterDomain(const sql_parser::AlterDomainStatement& stmt);
    bool dropDomain(const std::string& domainName, bool ifExists = false);
    bool domainExists(const std::string& domainName) const;

    // 域验证
    bool validateValue(const std::string& domainName, const std::string& value) const;
    std::string getDomainType(const std::string& domainName) const;

    // 域使用统计
    std::string getDomainInfo(const std::string& domainName) const;
    std::vector<std::string> listDomains() const;

private:
    DomainManager() = default;

    struct DomainInfo {
        std::string name;
        sql_parser::DomainDefinition::BaseType baseType;
        int characterLength;
        int precision;
        int scale;
        std::string defaultValue;
        std::string checkConstraint;
        bool notNull;
        std::string owner;
        long created_time;
        std::vector<std::string> dependent_columns; // 使用此域的列
    };

    std::unordered_map<std::string, DomainInfo> domains_;
};

} // namespace sql_executor
} // namespace sqlcc
