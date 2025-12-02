#include "schema_manager.h"
#include <algorithm>
#include <iostream>

namespace sqlcc {

SchemaManager::SchemaManager() {
  // 初始化默认模式
  CreateSchema("public", "admin");
}

SchemaManager::~SchemaManager() = default;

// 获取当前时间字符串
std::string SchemaManager::GetCurrentTimeString() {
  auto now = std::chrono::system_clock::now();
  auto now_time_t = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm = *std::localtime(&now_time_t);

  char buffer[20];
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &now_tm);
  return std::string(buffer);
}

// 创建模式
bool SchemaManager::CreateSchema(const std::string &schema_name,
                                 const std::string &owner) {
  std::lock_guard<std::mutex> lock(mutex_);

  // 检查模式是否已存在
  if (schemas_.find(schema_name) != schemas_.end()) {
    last_error_ = "Schema already exists: " + schema_name;
    return false;
  }

  // 创建模式
  Schema new_schema;
  new_schema.schema_name = schema_name;
  new_schema.owner = owner;
  new_schema.created_at = GetCurrentTimeString();

  schemas_[schema_name] = new_schema;
  return true;
}

// 删除模式
bool SchemaManager::DropSchema(const std::string &schema_name) {
  std::lock_guard<std::mutex> lock(mutex_);

  // 检查模式是否存在
  if (schemas_.find(schema_name) == schemas_.end()) {
    last_error_ = "Schema not found: " + schema_name;
    return false;
  }

  // 不允许删除默认模式
  if (schema_name == "public") {
    last_error_ = "Cannot drop default schema 'public'";
    return false;
  }

  // 删除模式
  schemas_.erase(schema_name);
  return true;
}

// 修改模式
bool SchemaManager::AlterSchema(const std::string &schema_name,
                                const std::string &new_schema_name) {
  std::lock_guard<std::mutex> lock(mutex_);

  // 检查原模式是否存在
  auto schema_it = schemas_.find(schema_name);
  if (schema_it == schemas_.end()) {
    last_error_ = "Schema not found: " + schema_name;
    return false;
  }

  // 检查新模式名是否已存在
  if (schemas_.find(new_schema_name) != schemas_.end()) {
    last_error_ = "Schema already exists: " + new_schema_name;
    return false;
  }

  // 不允许修改默认模式
  if (schema_name == "public") {
    last_error_ = "Cannot alter default schema 'public'";
    return false;
  }

  // 创建新模式
  Schema new_schema = schema_it->second;
  new_schema.schema_name = new_schema_name;
  schemas_[new_schema_name] = new_schema;

  // 删除原模式
  schemas_.erase(schema_name);
  return true;
}

// 获取模式信息
Schema SchemaManager::GetSchema(const std::string &schema_name) const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto schema_it = schemas_.find(schema_name);
  if (schema_it != schemas_.end()) {
    return schema_it->second;
  }

  // 返回空模式
  return Schema();
}

// 列出所有模式
std::vector<Schema> SchemaManager::ListSchemas() const {
  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<Schema> schema_list;
  for (const auto &pair : schemas_) {
    schema_list.push_back(pair.second);
  }

  return schema_list;
}

// 检查模式是否存在
bool SchemaManager::SchemaExists(const std::string &schema_name) const {
  std::lock_guard<std::mutex> lock(mutex_);
  return schemas_.find(schema_name) != schemas_.end();
}

// 获取最后一次错误信息
const std::string &SchemaManager::GetLastError() const { return last_error_; }

} // namespace sqlcc