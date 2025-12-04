#include "include/sql_executor.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "开始构造SqlExecutor..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    
    sqlcc::SqlExecutor executor;
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "SqlExecutor构造完成，耗时: " << duration.count() << "ms" << std::endl;
    
    return 0;
}
