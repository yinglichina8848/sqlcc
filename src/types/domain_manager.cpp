#include "domain_manager.h"
#include <algorithm>
#include <regex>

namespace sqlcc {

// Value implementation
int Value::asInteger() const {
    switch (type_) {
        case INTEGER: return int_value_;
        case DOUBLE: return static_cast<int>(double_value_);
        case STRING: return 0; // 简化实现
        case BOOLEAN: return bool_value_ ? 1 : 0;
        case NULL_VALUE: return 0;
        default: return 0;
    }
}

double Value::asDouble() const {
    switch (type_) {
        case INTEGER: return static_cast<double>(int_value_);
        case DOUBLE: return double_value_;
        case STRING: return 0.0; // 简化实现
        case BOOLEAN: return bool_value_ ? 1.0 : 0.0;
        case NULL_VALUE: return 0.0;
        default: return 0.0;
    }
}

const std::string& Value::asString() const {
    static const std::string empty_string;
    switch (type_) {
        case STRING: return string_value_;
        case INTEGER: 
        case DOUBLE: 
        case BOOLEAN: 
        case NULL_VALUE:
        default: return empty_string;
    }
}

bool Value::asBoolean() const {
    switch (type_) {
        case INTEGER: return int_value_ != 0;
        case DOUBLE: return double_value_ != 0.0;
        case STRING: return !string_value_.empty();
        case BOOLEAN: return bool_value_;
        case NULL_VALUE: return false;
        default: return false;
    }
}

std::string Value::toString() const {
    switch (type_) {
        case INTEGER: return std::to_string(int_value_);
        case DOUBLE: return std::to_string(double_value_);
        case STRING: return string_value_;
        case BOOLEAN: return bool_value_ ? "true" : "false";
        case NULL_VALUE: return "null";
        default: return "unknown";
    }
}

// DomainDefinition implementation
DomainDefinition::DomainDefinition(const std::string& name, const std::string& base_type)
    : name_(name), base_type_(base_type), nullable_(true) {}

DomainDefinition::~DomainDefinition() {}

void DomainDefinition::setCheckConstraint(const std::string& constraint) {
    check_constraint_ = constraint;
}

void DomainDefinition::setDefaultValue(const std::string& default_value) {
    default_value_ = default_value;
}

void DomainDefinition::setNullable(bool nullable) {
    nullable_ = nullable;
}

bool DomainDefinition::validateValue(const Value& value) const {
    // 检查NULL值
    if (value.getType() == Value::NULL_VALUE) {
        return nullable_;
    }

    // 检查类型兼容性（简化实现）
    // 实际应根据base_type进行严格的类型检查
    if (!check_constraint_.empty()) {
        return evaluateCheckConstraint(value);
    }

    return true;
}

bool DomainDefinition::evaluateCheckConstraint(const Value& value) const {
    // 简化的约束检查实现
    // 实际应实现完整的表达式求值器

    // 示例：检查正整数约束
    if (check_constraint_.find("VALUE > 0") != std::string::npos) {
        if (value.getType() == Value::INTEGER) {
            return value.asInteger() > 0;
        }
        return false;
    }

    // 示例：检查邮箱格式约束
    if (check_constraint_.find("LIKE '%@%'") != std::string::npos) {
        if (value.getType() == Value::STRING) {
            const std::string& str = value.asString();
            return str.find('@') != std::string::npos;
        }
        return false;
    }

    // 默认通过（无约束或未识别的约束）
    return true;
}

// DomainDefinitionNode implementation
DomainDefinitionNode::DomainDefinitionNode(const std::string& name, const std::string& base_type)
    : name_(name), base_type_(base_type), nullable_(true) {}

DomainDefinitionNode::~DomainDefinitionNode() {}

void DomainDefinitionNode::setCheckConstraint(const std::string& constraint) {
    check_constraint_ = constraint;
}

void DomainDefinitionNode::setDefaultValue(const std::string& default_value) {
    default_value_ = default_value;
}

void DomainDefinitionNode::setNullable(bool nullable) {
    nullable_ = nullable;
}

// DomainManager implementation
DomainManager& DomainManager::getInstance() {
    static DomainManager instance;
    return instance;
}

DomainManager::DomainManager() = default;

DomainManager::~DomainManager() = default;

bool DomainManager::createDomain(std::unique_ptr<DomainDefinition> domain) {
    if (!domain) {
        last_error_ = "Domain definition is null";
        return false;
    }

    const std::string& name = domain->getName();
    if (domains_.find(name) != domains_.end()) {
        last_error_ = "Domain '" + name + "' already exists";
        return false;
    }

    // 验证基本类型是否支持
    const std::string& base_type = domain->getBaseType();
    std::vector<std::string> supported_types = {"INTEGER", "VARCHAR", "DECIMAL", "DATE", "BOOLEAN"};
    if (std::find(supported_types.begin(), supported_types.end(), base_type) == supported_types.end()) {
        last_error_ = "Unsupported base type: " + base_type;
        return false;
    }

    domains_[name] = std::move(domain);
    return true;
}

bool DomainManager::dropDomain(const std::string& domain_name) {
    auto it = domains_.find(domain_name);
    if (it == domains_.end()) {
        last_error_ = "Domain '" + domain_name + "' does not exist";
        return false;
    }

    // TODO: 检查是否有表正在使用此域
    // 这里应该检查元数据以确保没有表引用此域

    domains_.erase(it);
    return true;
}

std::shared_ptr<const DomainDefinition> DomainManager::getDomain(const std::string& domain_name) const {
    auto it = domains_.find(domain_name);
    if (it != domains_.end()) {
        return std::shared_ptr<const DomainDefinition>(it->second.get(),
            [](const DomainDefinition*) {}); // Empty deleter for const access
    }
    return nullptr;
}

bool DomainManager::domainExists(const std::string& domain_name) const {
    return domains_.find(domain_name) != domains_.end();
}

bool DomainManager::validateDomainValue(const std::string& domain_name, const Value& value) const {
    auto domain = getDomain(domain_name);
    if (!domain) {
        last_error_ = "Domain '" + domain_name + "' does not exist";
        return false;
    }

    return domain->validateValue(value);
}

std::vector<std::string> DomainManager::getAllDomainNames() const {
    std::vector<std::string> names;
    names.reserve(domains_.size());

    for (const auto& pair : domains_) {
        names.push_back(pair.first);
    }

    return names;
}

std::string DomainManager::getDomainDefaultValue(const std::string& domain_name) const {
    auto domain = getDomain(domain_name);
    if (domain) {
        return domain->getDefaultValue();
    }
    return "";
}

bool DomainManager::isDomainNullable(const std::string& domain_name) const {
    auto domain = getDomain(domain_name);
    if (domain) {
        return domain->isNullable();
    }
    return true; // 默认可空
}

const std::string& DomainManager::getLastError() const {
    return last_error_;
}

} // namespace sqlcc