#include "buffer_pool.h"
#include "exception.h"
#include "logger.h"
#include <iostream>
#include <algorithm>

namespace sqlcc {

BufferPool::BufferPool(DiskManager* disk_manager, size_t pool_size)
    : disk_manager_(disk_manager), pool_size_(pool_size) {
    SQLCC_LOG_INFO("Initializing BufferPool with pool size: " + std::to_string(pool_size_));
}

BufferPool::~BufferPool() {
    SQLCC_LOG_INFO("Destroying BufferPool");
    // 析构时刷新所有脏页
    FlushAllPages();
}

Page* BufferPool::FetchPage(int32_t page_id) {
    SQLCC_LOG_DEBUG("Fetching page ID: " + std::to_string(page_id));
    
    // 检查页面是否已经在缓冲池中
    auto it = page_table_.find(page_id);
    if (it != page_table_.end()) {
        // 页面已在缓冲池中，增加引用计数
        page_refs_[page_id]++;
        // 将页面移到LRU链表头部
        MoveToHead(page_id);
        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " found in buffer pool");
        return it->second.get();
    }

    // 页面不在缓冲池中，需要从磁盘读取或创建新页面
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " not found in buffer pool, reading from disk");
    
    // 如果缓冲池已满，需要替换页面
    if (page_table_.size() >= pool_size_) {
        SQLCC_LOG_DEBUG("Buffer pool is full, replacing page");
        int32_t replaced_page_id = ReplacePage();
        if (replaced_page_id == -1) {
            // 无法替换页面
            std::string error_msg = "Failed to replace page in buffer pool";
            SQLCC_LOG_ERROR(error_msg);
            throw BufferPoolException(error_msg);
        }
    }

    // 创建新页面
    auto page = std::make_unique<Page>(page_id);
    
    // 从磁盘读取页面数据
    if (!disk_manager_->ReadPage(page_id, page.get())) {
        // 如果读取失败，说明页面不存在，返回nullptr
        SQLCC_LOG_DEBUG("Failed to read page ID " + std::to_string(page_id) + " from disk, page does not exist");
        return nullptr;
    }

    // 添加到页面表
    Page* page_ptr = page.get();
    page_table_[page_id] = std::move(page);
    
    // 初始化引用计数
    page_refs_[page_id] = 1;
    
    // 初始化脏页标记
    dirty_pages_[page_id] = false;
    
    // 添加到LRU链表头部
    lru_list_.push_front(page_id);
    lru_map_[page_id] = lru_list_.begin();
    
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " loaded into buffer pool");

    return page_ptr;
}

bool BufferPool::UnpinPage(int32_t page_id, bool is_dirty) {
    SQLCC_LOG_DEBUG("Unpinning page ID: " + std::to_string(page_id) + ", is_dirty: " + std::to_string(is_dirty));
    
    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        // 页面不在缓冲池中
        std::string error_msg = "Page ID " + std::to_string(page_id) + " not found in buffer pool";
        SQLCC_LOG_WARN(error_msg);
        return false;
    }

    // 减少引用计数
    auto ref_it = page_refs_.find(page_id);
    if (ref_it == page_refs_.end() || ref_it->second <= 0) {
        // 引用计数错误
        std::string error_msg = "Invalid reference count for page ID " + std::to_string(page_id);
        SQLCC_LOG_ERROR(error_msg);
        return false;
    }
    
    ref_it->second--;
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " reference count decreased to " + std::to_string(ref_it->second));
    
    // 如果页面被修改，标记为脏页
    if (is_dirty) {
        dirty_pages_[page_id] = true;
        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " marked as dirty");
    }

    return true;
}

bool BufferPool::FlushPage(int32_t page_id) {
    SQLCC_LOG_DEBUG("Flushing page ID: " + std::to_string(page_id));
    
    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        // 页面不在缓冲池中
        std::string error_msg = "Page ID " + std::to_string(page_id) + " not found in buffer pool";
        SQLCC_LOG_WARN(error_msg);
        return false;
    }

    // 如果不是脏页，无需刷新
    auto dirty_it = dirty_pages_.find(page_id);
    if (dirty_it == dirty_pages_.end() || !dirty_it->second) {
        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " is not dirty, no need to flush");
        return true;
    }

    // 将页面写入磁盘
    if (!disk_manager_->WritePage(*it->second)) {
        std::string error_msg = "Failed to write page ID " + std::to_string(page_id) + " to disk";
        SQLCC_LOG_ERROR(error_msg);
        return false;
    }

    // 清除脏页标记
    dirty_it->second = false;
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " flushed to disk and marked as clean");
    return true;
}

Page* BufferPool::NewPage(int32_t* page_id) {
    SQLCC_LOG_DEBUG("Creating new page");
    
    // 分配新页面ID
    *page_id = disk_manager_->AllocatePage();
    SQLCC_LOG_DEBUG("Allocated new page ID: " + std::to_string(*page_id));
    
    // 如果缓冲池已满，需要替换页面
    if (page_table_.size() >= pool_size_) {
        SQLCC_LOG_DEBUG("Buffer pool is full, replacing page");
        int32_t replaced_page_id = ReplacePage();
        if (replaced_page_id == -1) {
            // 无法替换页面
            std::string error_msg = "Failed to replace page in buffer pool";
            SQLCC_LOG_ERROR(error_msg);
            throw BufferPoolException(error_msg);
        }
    }

    // 创建新页面
    auto page = std::make_unique<Page>(*page_id);
    
    // 添加到页面表
    Page* page_ptr = page.get();
    page_table_[*page_id] = std::move(page);
    
    // 初始化引用计数
    page_refs_[*page_id] = 1;
    
    // 初始化脏页标记
    dirty_pages_[*page_id] = true;  // 新页面标记为脏页
    
    // 添加到LRU链表头部
    lru_list_.push_front(*page_id);
    lru_map_[*page_id] = lru_list_.begin();
    
    SQLCC_LOG_DEBUG("New page ID " + std::to_string(*page_id) + " created and loaded into buffer pool");

    return page_ptr;
}

bool BufferPool::DeletePage(int32_t page_id) {
    SQLCC_LOG_DEBUG("Deleting page ID: " + std::to_string(page_id));
    
    // 检查页面是否在缓冲池中
    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        // 页面不在缓冲池中
        std::string error_msg = "Page ID " + std::to_string(page_id) + " not found in buffer pool";
        SQLCC_LOG_WARN(error_msg);
        return false;
    }

    // 检查页面引用计数
    auto ref_it = page_refs_.find(page_id);
    if (ref_it != page_refs_.end() && ref_it->second > 0) {
        // 页面正在被使用，无法删除
        std::string error_msg = "Page ID " + std::to_string(page_id) + " is in use, cannot delete";
        SQLCC_LOG_ERROR(error_msg);
        return false;
    }

    // 从缓冲池中移除页面
    page_table_.erase(it);
    
    // 从脏页表中移除
    dirty_pages_.erase(page_id);
    
    // 从引用计数表中移除
    page_refs_.erase(page_id);
    
    // 从LRU链表中移除
    auto lru_it = lru_map_.find(page_id);
    if (lru_it != lru_map_.end()) {
        lru_list_.erase(lru_it->second);
        lru_map_.erase(lru_it);
    }
    
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " deleted from buffer pool");

    return true;
}

void BufferPool::FlushAllPages() {
    SQLCC_LOG_INFO("Flushing all pages in buffer pool");
    
    // 遍历所有页面，刷新脏页到磁盘
    for (auto& pair : page_table_) {
        int32_t page_id = pair.first;
        auto dirty_it = dirty_pages_.find(page_id);
        if (dirty_it != dirty_pages_.end() && dirty_it->second) {
            // 是脏页，需要刷新到磁盘
            if (!disk_manager_->WritePage(*pair.second)) {
                std::string error_msg = "Failed to write page ID " + std::to_string(page_id) + " to disk during flush all";
                SQLCC_LOG_ERROR(error_msg);
                // 继续刷新其他页面，不中断整个过程
                continue;
            }
            // 清除脏页标记
            dirty_it->second = false;
            SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " flushed to disk and marked as clean");
        }
    }
    
    SQLCC_LOG_INFO("All pages flushed to disk");
}

int32_t BufferPool::ReplacePage() {
    SQLCC_LOG_DEBUG("Replacing page using LRU algorithm");
    
    // 使用LRU算法查找最久未使用的页面
    while (!lru_list_.empty()) {
        // 获取LRU链表尾部的页面ID（最久未使用）
        int32_t page_id = lru_list_.back();
        
        // 检查页面引用计数
        auto ref_it = page_refs_.find(page_id);
        if (ref_it != page_refs_.end() && ref_it->second > 0) {
            // 页面正在被使用，不能替换，移到头部
            SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " is in use, moving to head of LRU list");
            MoveToHead(page_id);
            continue;
        }
        
        // 检查是否为脏页，如果是则需要刷新到磁盘
        auto dirty_it = dirty_pages_.find(page_id);
        if (dirty_it != dirty_pages_.end() && dirty_it->second) {
            auto page_it = page_table_.find(page_id);
            if (page_it != page_table_.end()) {
                // 刷新脏页到磁盘
                SQLCC_LOG_DEBUG("Flushing dirty page ID " + std::to_string(page_id) + " to disk before replacement");
                if (!disk_manager_->WritePage(*page_it->second)) {
                    std::string error_msg = "Failed to write dirty page ID " + std::to_string(page_id) + " to disk";
                    SQLCC_LOG_ERROR(error_msg);
                    // 无法刷新脏页，不能替换此页面
                    continue;
                }
            }
        }
        
        // 从缓冲池中移除页面
        page_table_.erase(page_id);
        
        // 从脏页表中移除
        if (dirty_it != dirty_pages_.end()) {
            dirty_pages_.erase(dirty_it);
        }
        
        // 从引用计数表中移除
        if (ref_it != page_refs_.end()) {
            page_refs_.erase(ref_it);
        }
        
        // 从LRU链表中移除
        lru_list_.pop_back();
        lru_map_.erase(page_id);
        
        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " replaced");
        return page_id;
    }
    
    // 无法找到可替换的页面
    SQLCC_LOG_WARN("No page can be replaced in buffer pool");
    return -1;
}

void BufferPool::MoveToHead(int32_t page_id) {
    auto it = lru_map_.find(page_id);
    if (it == lru_map_.end()) {
        return;
    }
    
    // 从当前位置移除
    lru_list_.erase(it->second);
    
    // 添加到头部
    lru_list_.push_front(page_id);
    lru_map_[page_id] = lru_list_.begin();
    
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " moved to head of LRU list");
}

}  // namespace sqlcc