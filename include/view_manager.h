#ifndef VIEW_MANAGER_H
#define VIEW_MANAGER_H

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace sqlcc {

// 视图数据结构
struct View {
  std::string view_name;
  std::string schema_name;
  std::string definition;
  std::string owner;
  std::string created_at;
  bool is_updatable;
};

// 视图管理器类
class ViewManager {
public:
  ViewManager();
  ~ViewManager();

  // 创建视图
  bool CreateView(const std::string &view_name, const std::string &schema_name,
                  const std::string &definition, const std::string &owner,
                  bool is_updatable = false);

  // 删除视图
  bool DropView(const std::string &view_name,
                const std::string &schema_name = "public");

  // 修改视图
  bool AlterView(const std::string &view_name, const std::string &schema_name,
                 const std::string &new_definition);

  // 获取视图信息
  View GetView(const std::string &view_name,
               const std::string &schema_name = "public") const;

  // 列出所有视图
  std::vector<View> ListViews(const std::string &schema_name = "") const;

  // 检查视图是否存在
  bool ViewExists(const std::string &view_name,
                  const std::string &schema_name = "public") const;

  // 获取视图定义
  std::string
  GetViewDefinition(const std::string &view_name,
                    const std::string &schema_name = "public") const;

  // 获取最后一次错误信息
  const std::string &GetLastError() const;

private:
  // 获取当前时间字符串
  std::string GetCurrentTimeString();

  // 生成视图的完整名称（schema.view）
  std::string GetFullViewName(const std::string &view_name,
                              const std::string &schema_name) const;

  // 成员变量
  std::unordered_map<std::string, View> views_; // 完整视图名 -> 视图信息
  mutable std::mutex mutex_;                    // 互斥锁，保护并发访问
  std::string last_error_;                      // 最后一次错误信息
};

} // namespace sqlcc

#endif // VIEW_MANAGER_H