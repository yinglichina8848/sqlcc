#ifndef SQLCC_CONSTRAINT_EXECUTOR_H
#define SQLCC_CONSTRAINT_EXECUTOR_H

#include "execution_engine.h"
#include <memory>
#include <string>

namespace sqlcc {

/**
 * @brief 约束执行器
 *
 * 负责执行数据库约束验证，包括主键约束、唯一约束、NOT NULL约束等
 */
class ConstraintExecutor {
public:
    explicit ConstraintExecutor(std::shared_ptr<DatabaseManager> db_manager);
    ~ConstraintExecutor() = default;

    /**
     * @brief 验证主键约束
     */
    bool ValidatePrimaryKey(const std::string& table_name,
                           const std::vector<std::string>& record);

    /**
     * @brief 验证唯一约束
     */
    bool ValidateUnique(const std::string& table_name,
                       const std::string& column_name,
                       const std::string& value);

    /**
     * @brief 验证NOT NULL约束
     */
    bool ValidateNotNull(const std::string& table_name,
                        const std::vector<std::string>& record);

    /**
     * @brief 验证CHECK约束
     */
    bool ValidateCheck(const std::string& table_name,
                      const std::vector<std::string>& record);

private:
    std::shared_ptr<DatabaseManager> db_manager_;
};

} // namespace sqlcc

#endif // SQLCC_CONSTRAINT_EXECUTOR_H
