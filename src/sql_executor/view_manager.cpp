#include "view_manager.h"
#include <algorithm>
#include <iostream>

namespace sqlcc {

ViewManager::ViewManager() {
  // 初始化视图管理器
}

ViewManager::~ViewManager() = default;

// 获取当前时间字符串
std::string ViewManager::GetCurrentTimeString() {
  auto now = std::chrono::system_clock::now();
  auto now_time_t = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm = *std::localtime(&now_time_t);

  char buffer[20];
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &now_tm);
  return std::string(buffer);
}

// 生成视图的完整名称（schema.view）
std::string ViewManager::GetFullViewName(const std::string &view_name,
                                         const std::string &schema_name) const {
  return schema_name + "." + view_name;
}

// 创建视图
bool ViewManager::CreateView(const std::string &view_name,
                             const std::string &schema_name,
                             const std::string &definition,
                             const std::string &owner, bool is_updatable) {
  std::lock_guard<std::mutex> lock(mutex_);

  // 生成完整视图名
  std::string full_view_name = GetFullViewName(view_name, schema_name);

  // 检查视图是否已存在
  if (views_.find(full_view_name) != views_.end()) {
    last_error_ = "View already exists: " + full_view_name;
    return false;
  }

  // 创建视图
  View new_view;
  new_view.view_name = view_name;
  new_view.schema_name = schema_name;
  new_view.definition = definition;
  new_view.owner = owner;
  new_view.created_at = GetCurrentTimeString();
  new_view.is_updatable = is_updatable;

  views_[full_view_name] = new_view;
  return true;
}

// 删除视图
bool ViewManager::DropView(const std::string &view_name,
                           const std::string &schema_name) {
  std::lock_guard<std::mutex> lock(mutex_);

  // 生成完整视图名
  std::string full_view_name = GetFullViewName(view_name, schema_name);

  // 检查视图是否存在
  if (views_.find(full_view_name) == views_.end()) {
    last_error_ = "View not found: " + full_view_name;
    return false;
  }

  // 删除视图
  views_.erase(full_view_name);
  return true;
}

// 修改视图
bool ViewManager::AlterView(const std::string &view_name,
                            const std::string &schema_name,
                            const std::string &new_definition) {
  std::lock_guard<std::mutex> lock(mutex_);

  // 生成完整视图名
  std::string full_view_name = GetFullViewName(view_name, schema_name);

  // 检查视图是否存在
  auto view_it = views_.find(full_view_name);
  if (view_it == views_.end()) {
    last_error_ = "View not found: " + full_view_name;
    return false;
  }

  // 更新视图定义
  view_it->second.definition = new_definition;
  return true;
}

// 获取视图信息
View ViewManager::GetView(const std::string &view_name,
                          const std::string &schema_name) const {
  std::lock_guard<std::mutex> lock(mutex_);

  // 生成完整视图名
  std::string full_view_name = GetFullViewName(view_name, schema_name);

  auto view_it = views_.find(full_view_name);
  if (view_it != views_.end()) {
    return view_it->second;
  }

  // 返回空视图
  return View();
}

// 列出所有视图
std::vector<View> ViewManager::ListViews(const std::string &schema_name) const {
  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<View> view_list;
  for (const auto &pair : views_) {
    if (schema_name.empty() || pair.second.schema_name == schema_name) {
      view_list.push_back(pair.second);
    }
  }

  return view_list;
}

// 检查视图是否存在
bool ViewManager::ViewExists(const std::string &view_name,
                             const std::string &schema_name) const {
  std::lock_guard<std::mutex> lock(mutex_);

  // 生成完整视图名
  std::string full_view_name = GetFullViewName(view_name, schema_name);
  return views_.find(full_view_name) != views_.end();
}

// 获取视图定义
std::string
ViewManager::GetViewDefinition(const std::string &view_name,
                               const std::string &schema_name) const {
  std::lock_guard<std::mutex> lock(mutex_);

  // 生成完整视图名
  std::string full_view_name = GetFullViewName(view_name, schema_name);

  auto view_it = views_.find(full_view_name);
  if (view_it != views_.end()) {
    return view_it->second.definition;
  }

  last_error_ = "View not found: " + full_view_name;
  return "";
}

// 获取最后一次错误信息
const std::string &ViewManager::GetLastError() const { return last_error_; }

} // namespace sqlcc