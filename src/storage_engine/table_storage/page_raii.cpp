#include "include/storage_engine/table_storage/page_raii.h"
#include "include/storage_engine.h"
#include "src/utils/logger.h"
#include <stdexcept>

namespace sqlcc {
namespace storage_engine {
namespace table_storage {

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
