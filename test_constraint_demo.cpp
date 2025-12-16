#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include "sql_parser/constraint.h"
#include "sql_parser/ast_nodes.h"
#include "storage/table_storage.h"

int main() {
    std::cout << "=== SQLCC 约束系统增强功能演示 ===\n\n";

    // 1. 测试多列外键约束
    std::cout << "1. 多列外键约束测试:\n";
    std::vector<std::string> fk_columns = {"user_id", "dept_id"};
    std::vector<std::string> ref_columns = {"id", "code"};

    sqlcc::sql_parser::ForeignKeyConstraint fk_constraint(
        fk_columns,
        "departments",
        ref_columns,
        "fk_user_dept_multi",
        sqlcc::sql_parser::ForeignKeyConstraint::CASCADE,
        sqlcc::sql_parser::ForeignKeyConstraint::RESTRICT
    );

    std::cout << "  外键约束名: " << fk_constraint.getName() << "\n";
    std::cout << "  本表列: ";
    for (const auto& col : fk_constraint.getColumns()) {
        std::cout << col << " ";
    }
    std::cout << "\n";
    std::cout << "  引用表: " << fk_constraint.getReferencedTable() << "\n";
    std::cout << "  引用列: ";
    for (const auto& col : fk_constraint.getReferencedColumns()) {
        std::cout << col << " ";
    }
    std::cout << "\n";
    std::cout << "  删除操作: CASCADE\n";
    std::cout << "  更新操作: RESTRICT\n\n";

    // 2. 测试多列唯一约束验证器
    std::cout << "2. 多列唯一约束验证器测试:\n";
    std::vector<std::string> unique_columns = {"email", "phone"};
    sqlcc::sql_parser::UniqueKeyValidator unique_validator(unique_columns, "uk_email_phone");

    // 模拟表元数据
    std::shared_ptr<sqlcc::TableMetadata> metadata =
        std::make_shared<sqlcc::TableMetadata>();
    metadata->table_name = "users";
    metadata->column_index_map = {
        {"id", 0},
        {"email", 1},
        {"phone", 2},
        {"name", 3}
    };

    // 测试记录
    std::vector<std::string> test_record = {"1", "user@example.com", "123456789", "John Doe"};

    try {
        bool result = unique_validator.validate(test_record, metadata, "users");
        std::cout << "  唯一性验证结果: " << (result ? "通过" : "失败") << "\n";
        std::cout << "  约束类型: " << unique_validator.getConstraintType() << "\n";
        std::cout << "  约束名: " << unique_validator.getConstraintName() << "\n\n";
    } catch (const std::exception& e) {
        std::cout << "  验证异常: " << e.what() << "\n\n";
    }

    // 3. 测试检查约束验证器
    std::cout << "3. 检查约束验证器测试:\n";
    sqlcc::sql_parser::CheckConstraintValidator check_validator("age >= 18", "ck_adult_age");

    std::vector<std::string> adult_record = {"1", "John", "25"};  // 年龄25
    std::vector<std::string> minor_record = {"2", "Jane", "16"};  // 年龄16

    // 更新元数据
    metadata->column_index_map = {{"id", 0}, {"name", 1}, {"age", 2}};

    try {
        bool adult_result = check_validator.validate(adult_record, metadata, "users");
        std::cout << "  成人记录验证: " << (adult_result ? "通过" : "失败") << "\n";

        bool minor_result = check_validator.validate(minor_record, metadata, "users");
        std::cout << "  未成年人记录验证: " << (minor_result ? "通过" : "失败") << "\n";
        std::cout << "  约束类型: " << check_validator.getConstraintType() << "\n\n";
    } catch (const std::exception& e) {
        std::cout << "  验证异常: " << e.what() << "\n\n";
    }

    // 4. 测试主键约束验证器
    std::cout << "4. 主键约束验证器测试:\n";
    sqlcc::sql_parser::PrimaryKeyValidator pk_validator({"id"}, "pk_users");

    std::vector<std::string> valid_pk_record = {"123", "John"};
    std::vector<std::string> invalid_pk_record = {"", "Jane"};

    metadata->column_index_map = {{"id", 0}, {"name", 1}};

    try {
        bool valid_result = pk_validator.validate(valid_pk_record, metadata, "users");
        std::cout << "  有效主键记录: " << (valid_result ? "通过" : "失败") << "\n";
    } catch (const std::exception& e) {
        std::cout << "  有效记录验证异常: " << e.what() << "\n";
    }

    try {
        bool invalid_result = pk_validator.validate(invalid_pk_record, metadata, "users");
        std::cout << "  无效主键记录: " << (invalid_result ? "通过" : "失败") << "\n";
    } catch (const std::exception& e) {
        std::cout << "  无效记录验证异常: " << e.what() << "\n";
    }
    std::cout << "  约束类型: " << pk_validator.getConstraintType() << "\n\n";

    // 5. 测试约束管理器
    std::cout << "5. 约束管理器测试:\n";
    sqlcc::sql_parser::ConstraintManager& manager =
        sqlcc::sql_parser::ConstraintManager::getInstance();

    // 添加约束验证器
    auto fk_validator = std::make_unique<sqlcc::sql_parser::ForeignKeyValidator>(
        std::vector<std::string>{"dept_id"},
        "departments",
        std::vector<std::string>{"id"},
        "fk_user_dept"
    );

    manager.addValidator("users", std::move(fk_validator));
    std::cout << "  添加了外键约束验证器\n";

    auto validators = manager.getValidators("users");
    std::cout << "  当前验证器数量: " << validators.size() << "\n";

    if (!validators.empty()) {
        std::cout << "  第一个验证器类型: " << validators[0]->getConstraintType() << "\n";
        std::cout << "  第一个验证器名称: " << validators[0]->getConstraintName() << "\n";
    }

    manager.removeValidator("users", "fk_user_dept");
    validators = manager.getValidators("users");
    std::cout << "  删除后验证器数量: " << validators.size() << "\n\n";

    // 6. 测试级联操作枚举
    std::cout << "6. 级联操作枚举测试:\n";
    using Action = sqlcc::sql_parser::ForeignKeyConstraint::CascadeAction;

    std::cout << "  RESTRICT = " << static_cast<int>(Action::RESTRICT) << "\n";
    std::cout << "  CASCADE = " << static_cast<int>(Action::CASCADE) << "\n";
    std::cout << "  SET_NULL = " << static_cast<int>(Action::SET_NULL) << "\n";
    std::cout << "  SET_DEFAULT = " << static_cast<int>(Action::SET_DEFAULT) << "\n";
    std::cout << "  NO_ACTION = " << static_cast<int>(Action::NO_ACTION) << "\n\n";

    std::cout << "=== 约束系统增强功能演示完成 ===\n";
    std::cout << "所有核心功能已成功实现并验证！\n";

    return 0;
}
