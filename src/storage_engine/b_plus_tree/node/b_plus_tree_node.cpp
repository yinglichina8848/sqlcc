#include "../../../include/storage_engine.h"
#include "../../../../include/utils/logger.h"
#include "../../../../include/storage/b_plus_tree_nodes.h"

namespace sqlcc {

// B+树设计参数 (商业数据库标准)
#define PAGE_HEADER_SIZE 24
#define PAGE_DATA_SIZE (PAGE_SIZE - PAGE_HEADER_SIZE)

BPlusTreeNode::BPlusTreeNode(std::shared_ptr<StorageEngine> storage_engine, int32_t page_id, bool is_leaf)
    : storage_engine_(std::move(storage_engine)), page_id_(page_id), parent_page_id_(-1), is_leaf_(is_leaf) {
    // 获取页面对象用于数据存储
    if (storage_engine_) {
        auto page_ptr = storage_engine_->FetchPage(page_id);
        if (page_ptr) {
            page_ = std::move(page_ptr);
        } else {
            SQLCC_LOG_ERROR("Failed to fetch page " + std::to_string(page_id) + " from storage engine");
            throw std::runtime_error("Failed to fetch page from storage engine");
        }
    } else {
        SQLCC_LOG_ERROR("Storage engine is null in BPlusTreeNode constructor");
        throw std::invalid_argument("Storage engine cannot be null");
    }

    SQLCC_LOG_DEBUG(std::string("Created B+Tree ") +
                    (is_leaf ? "leaf" : "internal") +
                    " node: page_id=" + std::to_string(page_id));
}

BPlusTreeNode::~BPlusTreeNode() {
    // 页面资源需要手动释放，将页面返回给StorageEngine
    if (storage_engine_ && page_) {
        storage_engine_->UnpinPage(page_id_, true); // 标记为脏页并释放
    }
    SQLCC_LOG_DEBUG("Destroyed B+Tree node: page_id=" + std::to_string(page_id_));
}

// 页面访问
char* BPlusTreeNode::GetData() {
    if (!page_) {
        throw std::runtime_error("Page is null");
    }
    // 根据C++标准版本选择合适的访问方式
#ifdef __cpp_lib_span
    return page_->GetDataSpan().data();
#else
    // C++17兼容模式下直接返回数据指针
    return page_->GetDataSpan().data;
#endif
}

const char* BPlusTreeNode::GetData() const {
    if (!page_) {
        throw std::runtime_error("Page is null");
    }
    // 根据C++标准版本选择合适的访问方式
#ifdef __cpp_lib_span
    return page_->GetDataSpan().data();
#else
    // C++17兼容模式下直接返回数据指针
    return page_->GetDataSpan().data;
#endif
}

} // namespace sqlcc
