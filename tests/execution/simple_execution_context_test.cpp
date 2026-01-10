#include "core/execution_context.h"
#include "core/core_database_manager.h"
#include <iostream>

int main() {
    std::shared_ptr<sqlcc::DatabaseManager> db_manager = nullptr;
    std::shared_ptr<sqlcc::ExecutionContext> context = std::make_shared<sqlcc::ExecutionContext>(db_manager, nullptr, nullptr);
    std::cout << "ExecutionContext created successfully!" << std::endl;
    return 0;
}
