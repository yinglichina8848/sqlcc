#include "../../../include/sql_executor/window_function_executor.h"
#include "../../../include/sql_parser/window_function.h"
#include "../../../include/sql_parser/window_function_node.h"
#include <iostream>
#include <memory>
#include <vector>

int main() {
    using namespace sqlcc;
    using namespace sqlcc::sql_parser;
    
    // 创建一个模拟的DatabaseManager（简化版）
    std::shared_ptr<DatabaseManager> db_manager = nullptr;
    
    // 创建窗口函数执行器
    WindowFunctionExecutor executor(db_manager);
    
    // 创建测试数据
    std::vector<std::vector<std::string>> test_data = {
        {"1", "Alice", "Engineering", "70000"},
        {"2", "Bob", "Engineering", "80000"},
        {"3", "Charlie", "Sales", "60000"},
        {"4", "David", "Sales", "65000"},
        {"5", "Eve", "Engineering", "90000"}
    };
    
    // 测试ROW_NUMBER函数
    WindowFunctionNode rowNumberNode(WindowFunctionNode::ROW_NUMBER);
    rowNumberNode.setAlias("row_num");
    rowNumberNode.addPartitionByColumn("department");
    rowNumberNode.addOrderByColumn("salary");
    rowNumberNode.setOrderDirection("DESC");
    
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
    WindowFunctionNode rankNode(WindowFunctionNode::RANK);
    rankNode.setAlias("rank_val");
    rankNode.addPartitionByColumn("department");
    rankNode.addOrderByColumn("salary");
    rankNode.setOrderDirection("DESC");
    
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
    WindowFunctionNode denseRankNode(WindowFunctionNode::DENSE_RANK);
    denseRankNode.setAlias("dense_rank_val");
    denseRankNode.addPartitionByColumn("department");
    denseRankNode.addOrderByColumn("salary");
    denseRankNode.setOrderDirection("DESC");
    
    auto denseRankResult = executor.executeWindowFunction(denseRankNode, "employees", test_data);
    
    if (denseRankResult.success) {
        std::cout << "\nDENSE_RANK函数执行成功！" << std::endl;
        for (size_t i = 0; i < denseRankResult.values.size(); ++i) {
            std::cout << "行 " << i << ": " << denseRankResult.values[i] << std::endl;
        }
    } else {
        std::cout << "DENSE_RANK函数执行失败: " << denseRankResult.error_message << std::endl;
    }
    
    return 0;
}