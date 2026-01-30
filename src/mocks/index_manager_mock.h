#pragma once

#include "storage/index_manager.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace sqlcc {
namespace mocks {

/**
 * @brief IndexManager Mock类，用于单元测试
 *
 * 提供可配置的IndexManager接口实现，支持：
 * - 模拟索引操作的成功/失败
 * - 记录方法调用历史
 * - 自定义返回值
 * - 验证调用参数
 */
class IndexManagerMock : public IndexManager {
public:
    /**
     * @brief 构造函数
     * @param storage_engine 存储引擎智能指针
     * @param config_manager 配置管理器引用
     */
    IndexManagerMock(std::shared_ptr<StorageEngine> storage_engine, ConfigManager &config_manager);

    /**
     * @brief 析构函数
     */
    ~IndexManagerMock();

    // 禁止拷贝
    IndexManagerMock(const IndexManagerMock&) = delete;
    IndexManagerMock& operator=(const IndexManagerMock&) = delete;

    // Mock配置方法
    void SetCreateIndexResult(bool success);
    void SetCreateCompositeIndexResult(bool success);
    void SetDropIndexResult(bool success);
    void SetIndexExistsResult(bool exists);
    void SetGetIndexResult(BPlusTreeIndex* index);
    void SetGetTableIndexesResult(const std::vector<BPlusTreeIndex*>& indexes);
    void SetGetIndexedColumnsResult(const std::vector<std::string>& columns);
    void SetGetCompositeIndexedColumnsResult(const std::vector<std::vector<std::string>>& columns);
    void SetGetIndexNameResult(const std::string& name);
    void SetGetCompositeIndexNameResult(const std::string& name);

    // 调用历史记录
    struct CallRecord {
        std::string method_name;
        std::vector<std::string> args;
    };

    const std::vector<CallRecord>& GetCallHistory() const { return call_history_; }
    void ClearCallHistory() { call_history_.clear(); }

    // 重写IndexManager接口方法
    bool CreateIndex(const std::string &index_name, const std::string &table_name,
                    const std::string &column_name, bool unique = false);
    bool CreateCompositeIndex(const std::string &index_name,
                             const std::string &table_name,
                             const std::vector<std::string> &columns,
                             bool unique = false);
    bool DropIndex(const std::string &index_name, const std::string &table_name);
    bool IndexExists(const std::string &index_name,
                    const std::string &table_name) const;
    BPlusTreeIndex *GetIndex(const std::string &index_name,
                            const std::string &table_name);
    std::vector<BPlusTreeIndex *> GetTableIndexes(const std::string &table_name) const;
    std::vector<std::string> GetIndexedColumns(const std::string &table_name) const;
    std::vector<std::vector<std::string>> GetCompositeIndexedColumns(const std::string &table_name) const;
    std::string GetIndexName(const std::string &table_name,
                            const std::string &column_name) const;
    std::string GetCompositeIndexName(const std::string &table_name,
                                     const std::vector<std::string> &columns) const;

private:
    void RecordCall(const std::string& method, const std::vector<std::string>& args = {}) const;

    // Mock配置
    bool create_index_success_ = true;
    bool create_composite_index_success_ = true;
    bool drop_index_success_ = true;
    bool index_exists_result_ = false;
    BPlusTreeIndex* get_index_result_ = nullptr;
    std::vector<BPlusTreeIndex*> get_table_indexes_result_;
    std::vector<std::string> get_indexed_columns_result_;
    std::vector<std::vector<std::string>> get_composite_indexed_columns_result_;
    std::string get_index_name_result_ = "mock_index_name";
    std::string get_composite_index_name_result_ = "mock_composite_index_name";

    // 调用历史
    mutable std::vector<CallRecord> call_history_;
};

} // namespace mocks
} // namespace sqlcc

#endif // SQLCC_MOCKS_INDEX_MANAGER_MOCK_H