#pragma once

#include "storage_engine.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

namespace sqlcc {
namespace mocks {

/**
 * @brief StorageEngine Mock类，用于单元测试
 *
 * 提供可配置的StorageEngine接口实现，支持：
 * - 模拟页面操作的成功/失败
 * - 记录方法调用历史
 * - 自定义返回值
 * - 验证调用参数
 */
class StorageEngineMock : public StorageEngine {
public:
    /**
     * @brief 构造函数
     * @param config_manager 配置管理器引用
     * @param db_path 数据库路径
     */
    explicit StorageEngineMock(ConfigManager &config_manager, const std::string& db_path = "./data");

    /**
     * @brief 析构函数
     */
    ~StorageEngineMock() override;

    // 禁止拷贝
    StorageEngineMock(const StorageEngineMock&) = delete;
    StorageEngineMock& operator=(const StorageEngineMock&) = delete;

    // Mock配置方法
    void SetNewPageResult(bool success, int32_t page_id = -1);
    void SetFetchPageResult(std::shared_ptr<Page> page);
    void SetUnpinPageResult(bool success);
    void SetFlushPageResult(bool success);
    void SetDeletePageResult(bool success);
    void SetFlushAllPagesResult(bool success);
    void SetGetStatsResult(const std::string& stats);

    // 调用历史记录
    struct CallRecord {
        std::string method_name;
        std::vector<std::string> args;
    };

    const std::vector<CallRecord>& GetCallHistory() const { return call_history_; }
    void ClearCallHistory() { call_history_.clear(); }

    // 重写StorageEngine接口方法
    void InitializeIndexManager() override;
    std::unique_ptr<Page> NewPage(int32_t *page_id = nullptr) override;
    std::shared_ptr<Page> FetchPage(int32_t page_id) override;
    bool UnpinPage(int32_t page_id, bool is_dirty = false) override;
    bool FlushPage(int32_t page_id) override;
    bool DeletePage(int32_t page_id) override;
    void FlushAllPages() override;
    std::string GetStats() const override;

private:
    void RecordCall(const std::string& method, const std::vector<std::string>& args = {});

    // Mock配置
    bool new_page_success_ = true;
    int32_t new_page_id_ = 1;
    std::shared_ptr<Page> fetch_page_result_;
    bool unpin_page_success_ = true;
    bool flush_page_success_ = true;
    bool delete_page_success_ = true;
    bool flush_all_success_ = true;
    std::string stats_result_ = "Mock Storage Engine Stats";

    // 调用历史
    std::vector<CallRecord> call_history_;
};

} // namespace mocks
} // namespace sqlcc

#endif // SQLCC_MOCKS_STORAGE_ENGINE_MOCK_H