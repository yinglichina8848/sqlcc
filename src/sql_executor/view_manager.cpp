#include "src/view_manager.h"
#include <iostream>

namespace sqlcc {

ViewManager::ViewManager() : sql_executor_(nullptr) {}

ViewManager::ViewManager(std::shared_ptr<SqlExecutor> sql_executor)
    : sql_executor_(sql_executor) {}

ViewManager::~ViewManager() {
    // 清理所有视图
    views_.clear();
}

bool ViewManager::CreateView(const std::string& view_name, const std::string& sql_query) {
    // 检查视图是否已存在
    if (views_.find(view_name) != views_.end()) {
        return false; // 视图已存在
    }

    // 这里可以添加SQL查询验证逻辑
    // 目前简化为直接存储
    views_[view_name] = sql_query;
    return true;
}

bool ViewManager::DropView(const std::string& view_name) {
    auto it = views_.find(view_name);
    if (it == views_.end()) {
        return false; // 视图不存在
    }

    views_.erase(it);
    return true;
}

bool ViewManager::ViewExists(const std::string& view_name) {
    return views_.find(view_name) != views_.end();
}

std::string ViewManager::GetViewDefinition(const std::string& view_name) {
    auto it = views_.find(view_name);
    if (it == views_.end()) {
        return ""; // 视图不存在
    }

    return it->second;
}

std::vector<std::string> ViewManager::ListViews() {
    std::vector<std::string> view_names;
    for (const auto& pair : views_) {
        view_names.push_back(pair.first);
    }
    return view_names;
}

bool ViewManager::AlterView(const std::string& view_name, const std::string& new_sql_query) {
    auto it = views_.find(view_name);
    if (it == views_.end()) {
        return false; // 视图不存在
    }

    // 更新视图定义
    it->second = new_sql_query;
    return true;
}

} // namespace sqlcc
