#pragma once

#include "disk_manager.h"
#include "buffer_pool.h"
#include "page.h"

namespace sqlcc {

/**
 * @brief 存储引擎类，封装磁盘管理和缓冲池功能
 */
class StorageEngine {
public:
    /**
     * @brief 构造函数
     * @param db_filename 数据库文件名
     * @param pool_size 缓冲池大小（页面数量）
     */
    explicit StorageEngine(const std::string& db_filename, size_t pool_size = 64);

    /**
     * @brief 析构函数
     */
    ~StorageEngine();

    /**
     * @brief 禁止拷贝构造
     */
    StorageEngine(const StorageEngine&) = delete;

    /**
     * @brief 禁止赋值操作
     */
    StorageEngine& operator=(const StorageEngine&) = delete;

    /**
     * @brief 创建新页面
     * @param[out] page_id 新页面的ID
     * @return 页面指针，如果失败返回nullptr
     */
    Page* NewPage(int32_t* page_id);

    /**
     * @brief 获取页面
     * @param page_id 页面ID
     * @return 页面指针，如果失败返回nullptr
     */
    Page* FetchPage(int32_t page_id);

    /**
     * @brief 取消固定页面
     * @param page_id 页面ID
     * @param is_dirty 是否标记为脏页
     * @return 操作是否成功
     */
    bool UnpinPage(int32_t page_id, bool is_dirty);

    /**
     * @brief 刷新页面到磁盘
     * @param page_id 页面ID
     * @return 操作是否成功
     */
    bool FlushPage(int32_t page_id);

    /**
     * @brief 删除页面
     * @param page_id 页面ID
     * @return 操作是否成功
     */
    bool DeletePage(int32_t page_id);

    /**
     * @brief 刷新所有页面到磁盘
     */
    void FlushAllPages();

private:
    /// 磁盘管理器
    std::unique_ptr<DiskManager> disk_manager_;

    /// 缓冲池管理器
    std::unique_ptr<BufferPool> buffer_pool_;
};

}  // namespace sqlcc