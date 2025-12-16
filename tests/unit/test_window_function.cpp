#include "sql_parser/window_function.h"
#include "sql_parser/window_function_node.h"
#include <iostream>

int main() {
    using namespace sqlcc::sql_parser;
    
    // 测试创建窗口函数节点
    WindowFunctionNode rowNumberNode(WindowFunctionNode::ROW_NUMBER);
    WindowFunctionNode rankNode(WindowFunctionNode::RANK);
    WindowFunctionNode denseRankNode(WindowFunctionNode::DENSE_RANK);
    
    // 设置别名
    rowNumberNode.setAlias("row_num");
    rankNode.setAlias("rank_val");
    denseRankNode.setAlias("dense_rank_val");
    
    // 添加分区列
    rowNumberNode.addPartitionByColumn("department");
    rankNode.addPartitionByColumn("department");
    denseRankNode.addPartitionByColumn("department");
    
    // 添加排序列
    rowNumberNode.addOrderByColumn("salary");
    rankNode.addOrderByColumn("salary");
    denseRankNode.addOrderByColumn("salary");
    
    // 设置排序方向
    rowNumberNode.setOrderDirection("DESC");
    rankNode.setOrderDirection("DESC");
    denseRankNode.setOrderDirection("DESC");
    
    std::cout << "窗口函数节点创建成功！" << std::endl;
    std::cout << "ROW_NUMBER别名: " << rowNumberNode.getAlias() << std::endl;
    std::cout << "RANK分区列数量: " << rowNumberNode.getPartitionByColumns().size() << std::endl;
    std::cout << "DENSE_RANK排序列数量: " << denseRankNode.getOrderByColumns().size() << std::endl;
    
    return 0;
}