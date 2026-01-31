#include "page_raii.h"
#include "../storage_engine.h"
#include "../../logger/logger.h"
#include <stdexcept>

namespace sqlcc {
namespace storage_engine {
namespace table_storage {

/**
 * @class PageRAII
 * @brief 页面资源管理器 - 实现缓冲池页面的自动锁定与释放机制
 *
 * WHY层 - 设计意图：
 *   在 Buffer Pool 中，被获取的页面必须显式地进行“取消固定（Unpin）”，
 *   否则该页面将永远留在内存中无法被替换，导致缓存泄漏。
 *   PageRAII 通过 C++ 的析构函数机制，确保无论业务逻辑是否发生异常（Exception），
 *   页面都能被正确地归还给存储引擎，极大增强了系统的鲁棒性。
 *
 * WHAT层 - 功能说明：
 *   持有页面的指针及其元数据（page_id）。
 *   提供数据缓冲区的访问接口（GetData）。
 *   管理 pinned_ 状态，在对象生命周期结束时自动调用 UnpinPage。
 *   支持移动语义（Move Semantics）以避免不必要的 Pin 计数波动。
 *
 * HOW层 - 实现机制：
 *   1. RAII 封装：构造函数记录 page 和 storage_engine，并将 pinned_ 设为 true。
 *   2. 析构回收：~PageRAII 自动执行 Unpin，默认标记为干净页（除非手动调用了 Unpin(true)）。
 *   3. 异常防御：在析构过程中使用 try-catch 包裹，防止清理时的次生异常导致程序崩溃。
 *   4. 无所有权转移：仅管理“访问权”，不负责页面对象的物理内存销毁（由 BufferPool 负责）。
 */
PageRAII::PageRAII(Page* page, std::shared_ptr<StorageEngine> storage_engine, int32_t page_id)
    : page_(page), storage_engine_(std::move(storage_engine)), page_id_(page_id), pinned_(true) {
    if (!page_) {
        throw std::invalid_argument("Page cannot be null");
    }
    if (!storage_engine_) {
        throw std::invalid_argument("StorageEngine cannot be null");
    }
    if (page_id < 0) {
        throw std::invalid_argument("Page ID must be non-negative");
    }
}

PageRAII::~PageRAII() {
    if (pinned_ && storage_engine_ && page_id_ >= 0) {
        try {
            storage_engine_->UnpinPage(page_id_, false);
        } catch (const std::exception& e) {
            SQLCC_LOG_ERROR("Failed to unpin page " + std::to_string(page_id_) +
                           " during RAII cleanup: " + std::string(e.what()));
        }
    }
}

PageRAII::PageRAII(PageRAII&& other) noexcept
    : page_(other.page_), storage_engine_(std::move(other.storage_engine_)),
      page_id_(other.page_id_), pinned_(other.pinned_) {
    other.page_ = nullptr;
    other.pinned_ = false;
}

PageRAII& PageRAII::operator=(PageRAII&& other) noexcept {
    if (this != &other) {
        // 清理当前资源
        if (pinned_ && storage_engine_ && page_id_ >= 0) {
            storage_engine_->UnpinPage(page_id_, false);
        }

        // 移动新资源
        page_ = other.page_;
        storage_engine_ = std::move(other.storage_engine_);
        page_id_ = other.page_id_;
        pinned_ = other.pinned_;

        other.page_ = nullptr;
        other.pinned_ = false;
    }
    return *this;
}

char* PageRAII::GetData() {
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

const char* PageRAII::GetData() const {
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

void PageRAII::Unpin(bool is_dirty) {
    if (pinned_ && storage_engine_ && page_id_ >= 0) {
        storage_engine_->UnpinPage(page_id_, is_dirty);
        pinned_ = false;
    }
}

} // namespace table_storage
} // namespace storage_engine
} // namespace sqlcc
