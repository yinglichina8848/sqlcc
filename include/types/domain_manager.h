#ifndef SQLCC_TYPES_DOMAIN_MANAGER_H
#define SQLCC_TYPES_DOMAIN_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace sqlcc {

/**
 * @brief 变量值类型
 */
class Value {
public:
    enum Type { INTEGER, DOUBLE, STRING, BOOLEAN, NULL_VALUE };

    Value() : type_(NULL_VALUE) {}
    Value(int int_val) : type_(INTEGER), int_value_(int_val) {}
    Value(double double_val) : type_(DOUBLE), double_value_(double_val) {}
    Value(const std::string& str_val) : type_(STRING), string_value_(str_val) {}
    Value(bool bool_val) : type_(BOOLEAN), bool_value_(bool_val) {}

    Type getType() const { return type_; }

    int asInteger() const;
    double asDouble() const;
    const std::string& asString() const;
    bool asBoolean() const;

    std::string toString() const;

private:
    Type type_;
    int int_value_;
    double double_value_;
    std::string string_value_;
    bool bool_value_;
};

/**
 * @brief 域定义
 *
 * SQL-92 DOMAIN定义，允许用户创建自定义数据类型
 */
class DomainDefinition {
public:
    DomainDefinition(const std::string& name, const std::string& base_type);

    ~DomainDefinition();

    // Getters
    const std::string& getName() const { return name_; }
    const std::string& getBaseType() const { return base_type_; }
    const std::string& getCheckConstraint() const { return check_constraint_; }
    const std::string& getDefaultValue() const { return default_value_; }
    bool isNullable() const { return nullable_; }

    // Setters
    void setCheckConstraint(const std::string& constraint);
    void setDefaultValue(const std::string& default_value);
    void setNullable(bool nullable);

    // 验证值是否符合域定义
    bool validateValue(const Value& value) const;

private:
    std::string name_;
    std::string base_type_;
    std::string check_constraint_;
    std::string default_value_;
    bool nullable_;

    // 检查约束表达式求值（简化实现）
    bool evaluateCheckConstraint(const Value& value) const;
};

/**
 * @brief 域管理器
 *
 * 负责管理SQL-92 DOMAIN定义的创建、使用和验证
 */
class DomainManager {
public:
    static DomainManager& getInstance();

    /**
     * 创建域
     * @param domain 域定义
     * @return 是否成功
     */
    bool createDomain(std::unique_ptr<DomainDefinition> domain);

    /**
     * 删除域
     * @param domain_name 域名
     * @return 是否成功
     */
    bool dropDomain(const std::string& domain_name);

    /**
     * 获取域定义
     * @param domain_name 域名
     * @return 域定义指针，如果不存在返回nullptr
     */
    std::shared_ptr<const DomainDefinition> getDomain(const std::string& domain_name) const;

    /**
     * 检查域是否存在
     * @param domain_name 域名
     * @return 是否存在
     */
    bool domainExists(const std::string& domain_name) const;

    /**
     * 验证值是否符合域定义
     * @param domain_name 域名
     * @param value 要验证的值
     * @return 是否有效
     */
    bool validateDomainValue(const std::string& domain_name, const Value& value) const;

    /**
     * 获取所有域名
     * @return 域名列表
     */
    std::vector<std::string> getAllDomainNames() const;

    /**
     * 获取域的默认值
     * @param domain_name 域名
     * @return 默认值，如果没有返回空字符串
     */
    std::string getDomainDefaultValue(const std::string& domain_name) const;

    /**
     * 检查域是否可空
     * @param domain_name 域名
     * @return 是否可空
     */
    bool isDomainNullable(const std::string& domain_name) const;

    /**
     * 获取最后错误信息
     */
    const std::string& getLastError() const;

private:
    DomainManager();
    ~DomainManager();

    // 禁用拷贝
    DomainManager(const DomainManager&) = delete;
    DomainManager& operator=(const DomainManager&) = delete;

    std::unordered_map<std::string, std::unique_ptr<DomainDefinition>> domains_;
    mutable std::string last_error_;
};

/**
 * @brief DOMAIN语法节点
 */
class DomainDefinitionNode {
public:
    DomainDefinitionNode(const std::string& name, const std::string& base_type);
    ~DomainDefinitionNode();

    const std::string& getName() const { return name_; }
    const std::string& getBaseType() const { return base_type_; }
    const std::string& getCheckConstraint() const { return check_constraint_; }
    const std::string& getDefaultValue() const { return default_value_; }
    bool isNullable() const { return nullable_; }

    void setCheckConstraint(const std::string& constraint);
    void setDefaultValue(const std::string& default_value);
    void setNullable(bool nullable);

private:
    std::string name_;
    std::string base_type_;
    std::string check_constraint_;
    std::string default_value_;
    bool nullable_;
};

} // namespace sqlcc

#endif // SQLCC_TYPES_DOMAIN_MANAGER_H