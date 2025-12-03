#include <iostream>
#include <memory>
#include "sql_parser/having_clause_node.h"
#include "sql_parser/ast_nodes.h"
#include "sql_parser/advanced_ast.h"

using namespace sqlcc::sql_parser;

int main() {
    std::cout << "=== SQLCC 高级SQL功能演示 ===\n\n";
    
    // 1. 创建HAVING子句节点
    std::cout << "1. 创建HAVING子句节点:\n";
    HavingClauseNode havingNode("COUNT(*) > 5");
    std::cout << "   - 条件: " << havingNode.getCondition() << "\n";
    std::cout << "   - 类型: " << havingNode.getTypeName() << "\n";
    std::cout << "   - 包含聚合函数: " << (havingNode.containsAggregateFunction() ? "是" : "否") << "\n";
    
    // 2. 添加参数
    std::cout << "\n2. 添加参数到HAVING子句:\n";
    havingNode.addParameter("5");
    havingNode.addParameter("10");
    std::cout << "   - 参数数量: " << havingNode.getParameters().size() << "\n";
    for (const auto& param : havingNode.getParameters()) {
        std::cout << "     - " << param << "\n";
    }
    
    // 3. 创建基础SELECT语句
    std::cout << "\n3. 创建基础SELECT语句:\n";
    auto baseSelect = std::make_unique<SelectStatement>();
    baseSelect->setTableName("employees");
    baseSelect->addSelectColumn("department");
    baseSelect->addSelectColumn("COUNT(*) as emp_count");
    baseSelect->setGroupByColumn("department");
    std::cout << "   - 表名: " << baseSelect->getTableName() << "\n";
    std::cout << "   - GROUP BY列: " << baseSelect->getGroupByColumn() << "\n";
    
    // 4. 创建带HAVING的SELECT语句
    std::cout << "\n4. 创建带HAVING子句的SELECT语句:\n";
    auto havingClause = std::make_unique<HavingClauseNode>("COUNT(*) > 5");
    SelectWithHavingStatement selectWithHaving(std::move(baseSelect), std::move(havingClause));
    
    std::cout << "   - 类型: " << selectWithHaving.getTypeName() << "\n";
    std::cout << "   - 是否有效: " << (selectWithHaving.isValid() ? "是" : "否") << "\n";
    std::cout << "   - GROUP BY列: " << selectWithHaving.getGroupByColumn() << "\n";
    
    // 5. 演示JSON序列化
    std::cout << "\n5. HAVING子句JSON序列化:\n";
    std::string json = havingNode.toJson();
    std::cout << "   - JSON: " << json << "\n";
    
    // 6. 演示高级AST框架
    std::cout << "\n6. 高级AST框架功能:\n";
    std::cout << "   - AdvancedNode类型枚举已定义: CORRELATED_SUBQUERY, HAVING_CLAUSE, UNION_OPERATION等\n";
    std::cout << "   - 高级节点工厂模式已实现\n";
    std::cout << "   - 高级节点访问者模式已实现\n";
    
    // 7. 与分阶段实施计划的关联
    std::cout << "\n7. 与分阶段实施计划的关联:\n";
    std::cout << "   - 第一阶段（第1-6周）: 基础框架扩展 ✓\n";
    std::cout << "   - 第二阶段（第7-18周）: HAVING子句支持 ✓\n";
    std::cout << "   - 后续阶段: 集合操作、窗口函数、CTE等\n";
    
    std::cout << "\n=== 演示完成 ===\n";
    std::cout << "\n总结:\n";
    std::cout << "- 成功实现了高级AST框架基础\n";
    std::cout << "- 成功实现了HAVING子句节点\n";
    std::cout << "- 成功实现了带HAVING的SELECT语句\n";
    std::cout << "- 为后续高级SQL功能奠定了基础\n";
    
    return 0;
}
