#include "../../../include/sql_parser/window_function.h"
#include "../../../include/sql_parser/window_function_node.h"
#include "../../../include/sql_parser/ast_nodes.h"
#include "../../../include/sql_parser/parser_new.h"
#include "../../../include/sql_executor/window_function_executor.h"
#include <iostream>
#include <memory>
#include <vector>

int main() {
    using namespace sqlcc;
    using namespace sqlcc::sql_parser;
    
    try {
        // 测试窗口函数节点的创建和基本功能
        std::cout << "=== 测试窗口函数节点创建 ===" << std::endl;
        
        WindowFunctionNode rowNumberNode(WindowFunctionNode::ROW_NUMBER);
        rowNumberNode.setAlias("row_num");
        rowNumberNode.addPartitionByColumn("department");
        rowNumberNode.addOrderByColumn("salary");
        rowNumberNode.setOrderDirection("DESC");
        
        WindowFunctionNode rankNode(WindowFunctionNode::RANK);
        rankNode.setAlias("rank_val");
        rankNode.addPartitionByColumn("department");
        rankNode.addOrderByColumn("salary");
        rankNode.setOrderDirection("DESC");
        
        WindowFunctionNode denseRankNode(WindowFunctionNode::DENSE_RANK);
        denseRankNode.setAlias("dense_rank_val");
        denseRankNode.addPartitionByColumn("department");
        denseRankNode.addOrderByColumn("salary");
        denseRankNode.setOrderDirection("DESC");
        
        std::cout << "ROW_NUMBER别名: " << rowNumberNode.getAlias() << std::endl;
        std::cout << "RANK分区列数量: " << rowNumberNode.getPartitionByColumns().size() << std::endl;
        std::cout << "DENSE_RANK排序列数量: " << denseRankNode.getOrderByColumns().size() << std::endl;
        
        // 测试窗口函数执行器
        std::cout << "\n=== 测试窗口函数执行器 ===" << std::endl;
        
        // 创建测试数据
        std::vector<std::vector<std::string>> test_data = {
            {"1", "Alice", "Engineering", "70000"},
            {"2", "Bob", "Engineering", "80000"},
            {"3", "Charlie", "Sales", "60000"},
            {"4", "David", "Sales", "65000"},
            {"5", "Eve", "Engineering", "90000"}
        };
        
        // 创建一个模拟的DatabaseManager（简化版）
        std::shared_ptr<DatabaseManager> db_manager = nullptr;
        
        // 创建窗口函数执行器
        WindowFunctionExecutor executor(db_manager);
        
        // 测试ROW_NUMBER函数
        auto rowNumberResult = executor.executeWindowFunction(rowNumberNode, "employees", test_data);
        
        if (rowNumberResult.success) {
            std::cout << "ROW_NUMBER函数执行成功！" << std::endl;
            for (size_t i = 0; i < rowNumberResult.values.size(); ++i) {
                std::cout << "行 " << i << ": " << rowNumberResult.values[i] << std::endl;
            }
        } else {
            std::cout << "ROW_NUMBER函数执行失败: " << rowNumberResult.error_message << std::endl;
        }
        
        // 测试RANK函数
        auto rankResult = executor.executeWindowFunction(rankNode, "employees", test_data);
        
        if (rankResult.success) {
            std::cout << "\nRANK函数执行成功！" << std::endl;
            for (size_t i = 0; i < rankResult.values.size(); ++i) {
                std::cout << "行 " << i << ": " << rankResult.values[i] << std::endl;
            }
        } else {
            std::cout << "RANK函数执行失败: " << rankResult.error_message << std::endl;
        }
        
        // 测试DENSE_RANK函数
        auto denseRankResult = executor.executeWindowFunction(denseRankNode, "employees", test_data);
        
        if (denseRankResult.success) {
            std::cout << "\nDENSE_RANK函数执行成功！" << std::endl;
            for (size_t i = 0; i < denseRankResult.values.size(); ++i) {
                std::cout << "行 " << i << ": " << denseRankResult.values[i] << std::endl;
            }
        } else {
            std::cout << "DENSE_RANK函数执行失败: " << denseRankResult.error_message << std::endl;
        }
        
        std::cout << "\n所有测试完成！" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}