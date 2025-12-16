#include "../../../include/sql_parser/ast_nodes.h"
#include "../../../include/sql_parser/window_function.h"
#include "../../../include/sql_parser/window_function_node.h"
#include "../../../include/sql_parser/parser_new.h"
#include <iostream>
#include <memory>
#include <vector>

int main() {
    using namespace sqlcc;
    using namespace sqlcc::sql_parser;
    
    try {
        // 创建一个简单的SELECT语句，包含窗口函数
        std::string sql = "SELECT id, name, salary, ROW_NUMBER() OVER (PARTITION BY department ORDER BY salary DESC) as row_num FROM employees";
        
        // 解析SQL语句
        ParserNew parser(sql);
        auto statements = parser.parse();
        
        if (statements.empty()) {
            std::cout << "解析结果为空" << std::endl;
            return 1;
        }
        
        std::cout << "SQL解析成功！" << std::endl;
        
        // 简单测试窗口函数节点的创建
        WindowFunctionNode rowNumberNode(WindowFunctionNode::ROW_NUMBER);
        std::cout << "窗口函数节点创建成功！" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}