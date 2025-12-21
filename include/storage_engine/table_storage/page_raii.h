#pragma once

#include <memory>
#include <stdexcept>

namespace sqlcc {
namespace storage_engine {
namespace table_storage {

// Forward declarations
class Page;
class StorageEngine;

/**
 * @brief Page RAII (Resource Acquisition Is Initialization) Manager
 *
 * 实现安全的页面生命周期管理，确保页面在不再需要时自动解除固定。
 * 遵循RAII原则，防止内存泄漏和资源泄漏。
 */
class PageRAII {
public:
    /**
     * @brief 构造函数
     * @param page 页面指针
     * @param storage_engine 存储引擎指针
     * @param page_id 页面ID
     */
    PageRAII(Page* page, std::shared_ptr<StorageEngine> storage_engine, int32_t page_id);

    /**
     * @brief 析构函数
     * 自动解除页面固定
     */
    ~PageRAII();

    // 禁止拷贝，允许移动
    PageRAII(const PageRAII&) = delete;
    PageRAII& operator=(const PageRAII&) = delete;

    /**
     * @brief 移动构造函数
     */
    PageRAII(PageRAII&& other) noexcept;

    /**
     * @brief 移动赋值操作符
     */
    PageRAII& operator=(PageRAII&& other) noexcept;

    /**
     * @brief 获取页面指针
     * @return 页面指针
     */
    Page* Get() const { return page_; }

    /**
     * @brief 获取页面引用
     * @return 页面引用
     */
    Page& operator*() const { return *page_; }

    /**
     * @brief 获取页面指针（通过操作符）
     * @return 页面指针
     */
    Page* operator->() const { return page_; }

    /**
     * @brief 获取页面ID
     * @return 页面ID
     */
    int32_t GetPageId() const { return page_id_; }

    /**
     * @brief 安全的数据访问
     * @return 页面数据指针
     */
    char* GetData();

    /**
     * @brief 安全的数据访问（常量版本）
     * @return 页面数据指针
     */
    const char* GetData() const;

    /**
     * @brief 手动解除页面固定
     * @param is_dirty 是否为脏页
     */
    void Unpin(bool is_dirty = false);

private:
    Page* page_;
    std::shared_ptr<StorageEngine> storage_engine_;
    int32_t page_id_;
    bool pinned_;
};

} // namespace table_storage
} // namespace storage_engine
} // namespace sqlcc
