#include "../../storage_engine.h"
#include "../../../logger/logger.h"
#include "../../../storage/b_plus_tree_nodes.h"

namespace sqlcc {

/**
 * @class BPlusTreeNode
 * @brief B+ 树节点基类 - 实现磁盘页面与索引逻辑结构的映射
 *
 * WHY层 - 设计意图：
 *   B+ 树是数据库索引的核心数据结构。为了支持磁盘持久化，每个树节点必须对应一个物理 8KB 页面。
 *   该基类抽象了节点与 Buffer Pool 页面的交互逻辑（Pin/Unpin），
 *   并区分了“叶子节点”（存数据）与“内部节点”（存导航键）的通用属性。
 *
 * WHAT层 - 功能说明：
 *   维护与 StorageEngine 的连接，实现页面的按需加载和自动写回。
 *   存储节点的核心元数据：page_id, parent_page_id, is_leaf 标志。
 *   提供底层数据缓冲区的访问接口（GetData）。
 *
 * HOW层 - 实现机制：
 *   1. RAII 模式：构造时调用 FetchPage 增加 Pin 计数，析构时调用 UnpinPage 减小计数并标记 Dirty。
 *   2. 页面头管理：保留前 24 字节作为 PAGE_HEADER，存储树结构链接（如 Prev/Next 指针）。
 *   3. 类型感知：基于 is_leaf_ 标志动态分发后续的索引查找和分裂逻辑。
 */
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
