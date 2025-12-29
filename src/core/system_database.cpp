#include "core/system_database.h"

namespace sqlcc {

SystemDatabase::SystemDatabase(std::shared_ptr<DatabaseManager> db_manager)
    : db_manager_(db_manager), is_initialized_(false) {}

SystemDatabase::~SystemDatabase() {
    // 清理资源
}

bool SystemDatabase::Initialize() {
    if (is_initialized_) {
        return true;
    }

    // 这里可以初始化系统数据库
    // 目前简化为设置初始化标志
    is_initialized_ = true;
    return true;
}

std::shared_ptr<DatabaseManager> SystemDatabase::GetDatabaseManager() {
    return db_manager_;
}

bool SystemDatabase::IsInitialized() const {
    return is_initialized_;
}

std::string SystemDatabase::GetLastError() const {
    return last_error_;
}

} // namespace sqlcc
