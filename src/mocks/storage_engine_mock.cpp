#include "storage_engine_mock.h"
#include "../page/page.h"
#include <sstream>

namespace sqlcc {
namespace mocks {

StorageEngineMock::StorageEngineMock(ConfigManager &config_manager, const std::string& db_path)
    : StorageEngine(config_manager, db_path) {
    RecordCall("StorageEngineMock", {db_path});
}

StorageEngineMock::~StorageEngineMock() {
    RecordCall("~StorageEngineMock");
}

void StorageEngineMock::SetNewPageResult(bool success, int32_t page_id) {
    new_page_success_ = success;
    new_page_id_ = page_id;
}

void StorageEngineMock::SetFetchPageResult(std::shared_ptr<Page> page) {
    fetch_page_result_ = page;
}

void StorageEngineMock::SetUnpinPageResult(bool success) {
    unpin_page_success_ = success;
}

void StorageEngineMock::SetFlushPageResult(bool success) {
    flush_page_success_ = success;
}

void StorageEngineMock::SetDeletePageResult(bool success) {
    delete_page_success_ = success;
}

void StorageEngineMock::SetFlushAllPagesResult(bool success) {
    flush_all_success_ = success;
}

void StorageEngineMock::SetGetStatsResult(const std::string& stats) {
    stats_result_ = stats;
}

void StorageEngineMock::InitializeIndexManager() {
    RecordCall("InitializeIndexManager");
    // Mock实现：不执行实际初始化
}

std::unique_ptr<Page> StorageEngineMock::NewPage(int32_t *page_id) {
    RecordCall("NewPage", {page_id ? std::to_string(*page_id) : "nullptr"});

    if (new_page_success_) {
        if (page_id) {
            *page_id = new_page_id_;
        }
        // 创建一个模拟的Page对象
        return std::make_unique<Page>(new_page_id_++);
    }
    return nullptr;
}

std::shared_ptr<Page> StorageEngineMock::FetchPage(int32_t page_id) {
    RecordCall("FetchPage", {std::to_string(page_id)});
    return fetch_page_result_;
}

bool StorageEngineMock::UnpinPage(int32_t page_id, bool is_dirty) {
    RecordCall("UnpinPage", {std::to_string(page_id), is_dirty ? "true" : "false"});
    return unpin_page_success_;
}

bool StorageEngineMock::FlushPage(int32_t page_id) {
    RecordCall("FlushPage", {std::to_string(page_id)});
    return flush_page_success_;
}

bool StorageEngineMock::DeletePage(int32_t page_id) {
    RecordCall("DeletePage", {std::to_string(page_id)});
    return delete_page_success_;
}

void StorageEngineMock::FlushAllPages() {
    RecordCall("FlushAllPages");
    // Mock实现：记录调用但不执行实际操作
}

std::string StorageEngineMock::GetStats() const {
    // Use const_cast to call non-const method from const context
    // This is safe because RecordCall doesn't modify observable state in this context
    const_cast<StorageEngineMock*>(this)->RecordCall("GetStats");
    return stats_result_;
}

void StorageEngineMock::RecordCall(const std::string& method, const std::vector<std::string>& args) {
    CallRecord record;
    record.method_name = method;

    std::stringstream ss;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << args[i];
    }
    record.args = {ss.str()};

    call_history_.push_back(record);
}

} // namespace mocks
} // namespace sqlcc