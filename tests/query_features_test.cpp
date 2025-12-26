#include "sql_parser/ast_nodes.h"
#include <gtest/gtest.h>
#include "execution/set_operation_executor.h"
#include "execution/window_function_executor.h"
#include "execution/recursive_query_executor.h"
#include "database_manager.h"
#include "core/execution_context.h"
#include "sql_parser/set_operation.h"
#include "sql_parser/window_function.h"
#include "sql_parser/recursive_query.h"

namespace sqlcc {

class QueryFeaturesTest : public ::testing::Test {
protected:
    void SetUp() override {
        db_manager_ = std::make_shared<DatabaseManager>();
        set_executor_ = std::make_unique<SetOperationExecutor>(db_manager_);
        window_executor_ = std::make_unique<WindowFunctionExecutor>(db_manager_);
        recursive_executor_ = std::make_unique<RecursiveQueryExecutor>(db_manager_);
    }

    void TearDown() override {
        set_executor_.reset();
        window_executor_.reset();
        recursive_executor_.reset();
        db_manager_.reset();
    }

    std::shared_ptr<DatabaseManager> db_manager_;
    std::unique_ptr<SetOperationExecutor> set_executor_;
    std::unique_ptr<WindowFunctionExecutor> window_executor_;
    std::unique_ptr<RecursiveQueryExecutor> recursive_executor_;
};

// 测试集合操作 - UNION
TEST_F(QueryFeaturesTest, UnionOperation) {
    // 创建UNION操作
    sql_parser::SelectStatement left_select("employees");
    sql_parser::SelectStatement right_select("departments");

    sql_parser::SetOperation union_op(sql_parser::SetOperationType::UNION,
                                     std::make_unique<sql_parser::SelectStatement>(left_select),
                                     std::make_unique<sql_parser::SelectStatement>(right_select),
                                     false); // UNION (not ALL)

    ExecutionContext context;
    ExecutionResult result = set_executor_->execute(union_op, context);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows_affected, 5); // 3 employees + 2 departments
    EXPECT_EQ(result.column_metadata.size(), 4); // 4 columns
}

// 测试集合操作 - INTERSECT
TEST_F(QueryFeaturesTest, IntersectOperation) {
    // 创建INTERSECT操作
    sql_parser::SelectStatement left_select("employees");
    sql_parser::SelectStatement right_select("departments");

    sql_parser::SetOperation intersect_op(sql_parser::SetOperationType::INTERSECT,
                                         std::make_unique<sql_parser::SelectStatement>(left_select),
                                         std::make_unique<sql_parser::SelectStatement>(right_select),
                                         false);

    ExecutionContext context;
    ExecutionResult result = set_executor_->execute(intersect_op, context);

    EXPECT_TRUE(result.success);
    // 交集应该为空，因为员工和部门表结构不同
    EXPECT_EQ(result.rows_affected, 0);
}

// 测试集合操作 - EXCEPT
TEST_F(QueryFeaturesTest, ExceptOperation) {
    // 创建EXCEPT操作
    sql_parser::SelectStatement left_select("employees");
    sql_parser::SelectStatement right_select("departments");

    sql_parser::SetOperation except_op(sql_parser::SetOperationType::EXCEPT,
                                      std::make_unique<sql_parser::SelectStatement>(left_select),
                                      std::make_unique<sql_parser::SelectStatement>(right_select),
                                      false);

    ExecutionContext context;
    ExecutionResult result = set_executor_->execute(except_op, context);

    EXPECT_TRUE(result.success);
    // 差集应该是所有员工记录
    EXPECT_EQ(result.rows_affected, 3);
}

// 测试集合操作 - UNION ALL
TEST_F(QueryFeaturesTest, UnionAllOperation) {
    // 创建UNION ALL操作
    sql_parser::SelectStatement left_select("employees");
    sql_parser::SelectStatement right_select("employees");

    sql_parser::SetOperation union_all_op(sql_parser::SetOperationType::UNION,
                                         std::make_unique<sql_parser::SelectStatement>(left_select),
                                         std::make_unique<sql_parser::SelectStatement>(right_select),
                                         true); // UNION ALL

    ExecutionContext context;
    ExecutionResult result = set_executor_->execute(union_all_op, context);

    EXPECT_TRUE(result.success);
    // UNION ALL 应该包含所有重复行
    EXPECT_EQ(result.rows_affected, 6); // 3 + 3 employees
}

// 测试集合操作 - INTERSECT ALL
TEST_F(QueryFeaturesTest, IntersectAllOperation) {
    // 创建INTERSECT ALL操作
    sql_parser::SelectStatement left_select("employees");
    sql_parser::SelectStatement right_select("employees");

    sql_parser::SetOperation intersect_all_op(sql_parser::SetOperationType::INTERSECT,
                                             std::make_unique<sql_parser::SelectStatement>(left_select),
                                             std::make_unique<sql_parser::SelectStatement>(right_select),
                                             true);

    ExecutionContext context;
    ExecutionResult result = set_executor_->execute(intersect_all_op, context);

    EXPECT_TRUE(result.success);
    // INTERSECT ALL 应该包含所有共同出现的行
    EXPECT_EQ(result.rows_affected, 3);
}

// 测试窗口函数 - ROW_NUMBER
TEST_F(QueryFeaturesTest, RowNumberWindowFunction) {
    sql_parser::WindowFunction row_number(sql_parser::FunctionType::ROW_NUMBER);

    // 设置窗口规范
    auto window_spec = std::make_unique<sql_parser::WindowSpecification>();
    window_spec->setPartitionBy({"department"});
    window_spec->setOrderBy({"salary"}, {false}); // DESC

    row_number.setWindowSpecification(std::move(window_spec));

    ExecutionContext context;
    ExecutionResult result = window_executor_->execute(row_number, context);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows.size(), 3);
    EXPECT_EQ(result.rows[0].values[0], "1"); // First row number
}

// 测试窗口函数 - RANK
TEST_F(QueryFeaturesTest, RankWindowFunction) {
    sql_parser::WindowFunction rank_func(sql_parser::FunctionType::RANK);

    // 设置窗口规范
    auto window_spec = std::make_unique<sql_parser::WindowSpecification>();
    window_spec->setPartitionBy({"department"});
    window_spec->setOrderBy({"salary"}, {false});

    rank_func.setWindowSpecification(std::move(window_spec));

    ExecutionContext context;
    ExecutionResult result = window_executor_->execute(rank_func, context);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows.size(), 3);
    // 验证排名结果（相同薪资相同排名）
    EXPECT_EQ(result.rows[0].values[0], "1"); // First rank
}

// 测试窗口函数 - DENSE_RANK
TEST_F(QueryFeaturesTest, DenseRankWindowFunction) {
    sql_parser::WindowFunction dense_rank_func(sql_parser::FunctionType::DENSE_RANK);

    // 设置窗口规范
    auto window_spec = std::make_unique<sql_parser::WindowSpecification>();
    window_spec->setPartitionBy({"department"});
    window_spec->setOrderBy({"salary"}, {false});

    dense_rank_func.setWindowSpecification(std::move(window_spec));

    ExecutionContext context;
    ExecutionResult result = window_executor_->execute(dense_rank_func, context);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows.size(), 3);
    // 验证密集排名（连续排名）
    EXPECT_EQ(result.rows[0].values[0], "1");
}

// 测试窗口函数 - SUM
TEST_F(QueryFeaturesTest, SumWindowFunction) {
    sql_parser::WindowFunction sum_func(sql_parser::FunctionType::SUM);

    // 设置表达式（假设salary列）
    auto expr = std::make_unique<sql_parser::ColumnReference>("salary");
    sum_func.setExpression(std::move(expr));

    // 设置窗口规范
    auto window_spec = std::make_unique<sql_parser::WindowSpecification>();
    window_spec->setPartitionBy({"department"});

    sum_func.setWindowSpecification(std::move(window_spec));

    ExecutionContext context;
    ExecutionResult result = window_executor_->execute(sum_func, context);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows.size(), 3);
    // 验证求和结果（按部门分组）
    EXPECT_EQ(result.rows[0].values[0], "150000"); // Engineering department sum
}

// 测试窗口函数 - AVG
TEST_F(QueryFeaturesTest, AvgWindowFunction) {
    sql_parser::WindowFunction avg_func(sql_parser::FunctionType::AVG);

    auto expr = std::make_unique<sql_parser::ColumnReference>("salary");
    avg_func.setExpression(std::move(expr));

    auto window_spec = std::make_unique<sql_parser::WindowSpecification>();
    window_spec->setPartitionBy({"department"});

    avg_func.setWindowSpecification(std::move(window_spec));

    ExecutionContext context;
    ExecutionResult result = window_executor_->execute(avg_func, context);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows.size(), 3);
    EXPECT_EQ(result.rows[0].values[0], "50000"); // Average salary
}

// 测试递归查询 - 基础功能
TEST_F(QueryFeaturesTest, RecursiveQueryBasic) {
    sql_parser::SelectStatement base_query("employees");
    sql_parser::SelectStatement recursive_query("employees");

    // 设置递归查询条件（模拟父子关系）
    sql_parser::WithRecursiveClause recursive_clause("org_hierarchy", base_query, recursive_query);

    ExecutionContext context;
    ExecutionResult result = recursive_executor_->execute(recursive_clause, context);

    EXPECT_TRUE(result.success);
    EXPECT_GT(result.rows_affected, 0);
    EXPECT_TRUE(result.message.find("iterations") != std::string::npos);
}

// 测试递归查询 - 广度优先
TEST_F(QueryFeaturesTest, RecursiveQueryBreadthFirst) {
    sql_parser::SelectStatement base_query("employees");
    sql_parser::SelectStatement recursive_query("employees");

    sql_parser::WithRecursiveClause recursive_clause("org_hierarchy", base_query, recursive_query);

    ExecutionContext context;
    ExecutionResult result = recursive_executor_->executeBreadthFirst(recursive_clause, context);

    EXPECT_TRUE(result.success);
    EXPECT_GT(result.rows_affected, 0);
    EXPECT_TRUE(result.message.find("Breadth-first") != std::string::npos);
}

// 测试递归查询 - 深度优先
TEST_F(QueryFeaturesTest, RecursiveQueryDepthFirst) {
    sql_parser::SelectStatement base_query("employees");
    sql_parser::SelectStatement recursive_query("employees");

    sql_parser::WithRecursiveClause recursive_clause("org_hierarchy", base_query, recursive_query);

    ExecutionContext context;
    ExecutionResult result = recursive_executor_->executeDepthFirst(recursive_clause, context);

    EXPECT_TRUE(result.success);
    EXPECT_GT(result.rows_affected, 0);
    EXPECT_TRUE(result.message.find("Depth-first") != std::string::npos);
}

// 测试递归查询 - 迭代限制
TEST_F(QueryFeaturesTest, RecursiveQueryIterationLimit) {
    sql_parser::SelectStatement base_query("employees");
    sql_parser::SelectStatement recursive_query("employees");

    // 创建会无限递归的查询（模拟）
    sql_parser::WithRecursiveClause recursive_clause("infinite_recursion", base_query, recursive_query);

    ExecutionContext context;

    // 期望在达到迭代限制时停止
    ExecutionResult result = recursive_executor_->execute(recursive_clause, context);

    // 应该成功完成，但可能达到最大迭代次数
    EXPECT_TRUE(result.success || result.message.find("iterations") != std::string::npos);
}

// 测试集合操作 - ORDER BY
TEST_F(QueryFeaturesTest, SetOperationWithOrderBy) {
    sql_parser::SelectStatement left_select("employees");
    sql_parser::SelectStatement right_select("departments");

    sql_parser::SetOperation union_op(sql_parser::SetOperationType::UNION,
                                     std::make_unique<sql_parser::SelectStatement>(left_select),
                                     std::make_unique<sql_parser::SelectStatement>(right_select),
                                     false);

    // 设置ORDER BY
    union_op.setOrderBy({"id"}, {true}); // ASC by id

    ExecutionContext context;
    ExecutionResult result = set_executor_->execute(union_op, context);

    EXPECT_TRUE(result.success);
    // 验证结果是否按ID排序
    if (result.rows.size() >= 2) {
        EXPECT_LE(std::stoi(result.rows[0].values[0]), std::stoi(result.rows[1].values[0]));
    }
}

// 测试集合操作 - LIMIT
TEST_F(QueryFeaturesTest, SetOperationWithLimit) {
    sql_parser::SelectStatement left_select("employees");
    sql_parser::SelectStatement right_select("departments");

    sql_parser::SetOperation union_op(sql_parser::SetOperationType::UNION,
                                     std::make_unique<sql_parser::SelectStatement>(left_select),
                                     std::make_unique<sql_parser::SelectStatement>(right_select),
                                     true); // UNION ALL

    // 设置LIMIT
    union_op.setLimit(3);

    ExecutionContext context;
    ExecutionResult result = set_executor_->execute(union_op, context);

    EXPECT_TRUE(result.success);
    EXPECT_LE(result.rows_affected, 3u); // 不超过限制
}

// 测试窗口函数 - 多窗口函数
TEST_F(QueryFeaturesTest, MultipleWindowFunctions) {
    std::vector<std::unique_ptr<sql_parser::WindowFunction>> window_funcs;

    // ROW_NUMBER
    auto row_number = std::make_unique<sql_parser::WindowFunction>(sql_parser::FunctionType::ROW_NUMBER);
    auto window_spec1 = std::make_unique<sql_parser::WindowSpecification>();
    window_spec1->setPartitionBy({"department"});
    window_spec1->setOrderBy({"salary"}, {false});
    row_number->setWindowSpecification(std::move(window_spec1));
    window_funcs.push_back(std::move(row_number));

    // SUM
    auto sum_func = std::make_unique<sql_parser::WindowFunction>(sql_parser::FunctionType::SUM);
    auto expr = std::make_unique<sql_parser::ColumnReference>("salary");
    sum_func->setExpression(std::move(expr));
    auto window_spec2 = std::make_unique<sql_parser::WindowSpecification>();
    window_spec2->setPartitionBy({"department"});
    sum_func->setWindowSpecification(std::move(window_spec2));
    window_funcs.push_back(std::move(sum_func));

    // 模拟基础查询结果
    ExecutionResult base_result;
    base_result.success = true;
    base_result.rows = {
        {"1", "John", "Engineering", "50000"},
        {"2", "Jane", "Sales", "45000"},
        {"3", "Bob", "Engineering", "55000"}
    };
    base_result.column_metadata = {
        {"id", "INTEGER", true, true, false, ""},
        {"name", "VARCHAR", false, false, false, ""},
        {"department", "VARCHAR", false, false, false, ""},
        {"salary", "INTEGER", false, false, false, ""}
    };

    ExecutionContext context;
    ExecutionResult result = window_executor_->executeWindowFunctions(window_funcs, base_result, context);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows.size(), 3);
    EXPECT_EQ(result.column_metadata.size(), 6); // 4原始列 + 2窗口函数列
}

} // namespace sqlcc
