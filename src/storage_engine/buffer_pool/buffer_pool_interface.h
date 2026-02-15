#ifndef SQLCC_STORAGE_BUFFER_POOL_INTERFACE_H
#define SQLCC_STORAGE_BUFFER_POOL_INTERFACE_H

#include <cstdint>
#include <memory>
#include <string>

namespace sqlcc {
namespace storage {

class Page;
// PageId 保持与现有实现一致
using PageId = int32_t;

/**
 * IBufferPool - 缓冲区管理接口
 * 
 * What: 管理数据库页面的缓存
 * Why: 为 DatabaseManager 提供缓冲区功能，不暴露具体实现
 * How: 定义 6 个抽象方法，由 BufferPoolManager 实现
 */
class IBufferPool {
public:
    virtual ~IBufferPool() = default;
    
    // ==================== 页面管理 ====================
    
    /**
     * FetchPage - 获取页面
     * @param page_id 页面 ID
     * @return 页面指针，失败返回 nullptr
     */
    virtual std::unique_ptr<Page> FetchPage(PageId page_id) = 0;
    
    /**
     * UnpinPage - 释放页面
     * @param page_id 页面 ID
     * @param is_dirty 页面是否被修改
     * @return 成功返回 true
     */
    virtual bool UnpinPage(PageId page_id, bool is_dirty) = 0;
    
    /**
     * AllocatePage - 分配新页面
     * @return 新页面 ID
     */
    virtual PageId AllocatePage() = 0;
    
    /**
     * DeallocatePage - 释放页面
     * @param page_id 页面 ID
     * @return 成功返回 true
     */
    virtual bool DeallocatePage(PageId page_id) = 0;
    
    // ==================== 生命周期 ====================
    
    /**
     * FlushAll - 刷新所有脏页
     * @return 全部成功返回 true
     */
    virtual bool FlushAll() = 0;
    
    /**
     * Shutdown - 关闭缓冲区
     */
    virtual void Shutdown() = 0;
};

}  // namespace storage
}  // namespace sqlcc

#endif // SQLCC_STORAGE_BUFFER_POOL_INTERFACE_H
