#ifndef SCHEMA_MANAGER_H
#define SCHEMA_MANAGER_H

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace sqlcc {

// 模式数据结构
struct Schema {
  std::string schema_name;
  std::string owner;
  std::string created_at;
};

// 模式管理器类
class SchemaManager {
public:
  SchemaManager();
  ~SchemaManager();

  // 创建模式
  bool CreateSchema(const std::string &schema_name, const std::string &owner);

  // 删除模式
  bool DropSchema(const std::string &schema_name);

  // 修改模式
  bool AlterSchema(const std::string &schema_name,
                   const std::string &new_schema_name);

  // 获取模式信息
  Schema GetSchema(const std::string &schema_name) const;

  // 列出所有模式
  std::vector<Schema> ListSchemas() const;

  // 检查模式是否存在
  bool SchemaExists(const std::string &schema_name) const;

  // 获取最后一次错误信息
  const std::string &GetLastError() const;

private:
  // 获取当前时间字符串
  std::string GetCurrentTimeString();

  // 成员变量
  std::unordered_map<std::string, Schema> schemas_; // 模式名 -> 模式信息
  mutable std::mutex mutex_;                        // 互斥锁，保护并发访问
  std::string last_error_;                          // 最后一次错误信息
};

} // namespace sqlcc

#endif // SCHEMA_MANAGER_H