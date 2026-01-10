#include "mocks/index_manager_mock.h"
#include <sstream>

namespace sqlcc {
namespace mocks {

IndexManagerMock::IndexManagerMock(std::shared_ptr<StorageEngine> storage_engine, ConfigManager &config_manager)
    : IndexManager(storage_engine, config_manager) {
    RecordCall("IndexManagerMock", {storage_engine ? "storage_engine" : "nullptr"});
}

IndexManagerMock::~IndexManagerMock() {
    RecordCall("~IndexManagerMock");
}

// Mock配置方法实现
void IndexManagerMock::SetCreateIndexResult(bool success) {
    create_index_success_ = success;
}

void IndexManagerMock::SetCreateCompositeIndexResult(bool success) {
    create_composite_index_success_ = success;
}

void IndexManagerMock::SetDropIndexResult(bool success) {
    drop_index_success_ = success;
}

void IndexManagerMock::SetIndexExistsResult(bool exists) {
    index_exists_result_ = exists;
}

void IndexManagerMock::SetGetIndexResult(BPlusTreeIndex* index) {
    get_index_result_ = index;
}

void IndexManagerMock::SetGetTableIndexesResult(const std::vector<BPlusTreeIndex*>& indexes) {
    get_table_indexes_result_ = indexes;
}

void IndexManagerMock::SetGetIndexedColumnsResult(const std::vector<std::string>& columns) {
    get_indexed_columns_result_ = columns;
}

void IndexManagerMock::SetGetCompositeIndexedColumnsResult(const std::vector<std::vector<std::string>>& columns) {
    get_composite_indexed_columns_result_ = columns;
}

void IndexManagerMock::SetGetIndexNameResult(const std::string& name) {
    get_index_name_result_ = name;
}

void IndexManagerMock::SetGetCompositeIndexNameResult(const std::string& name) {
    get_composite_index_name_result_ = name;
}

// 重写IndexManager接口方法
bool IndexManagerMock::CreateIndex(const std::string &index_name, const std::string &table_name,
                                  const std::string &column_name, bool unique) {
    RecordCall("CreateIndex", {index_name, table_name, column_name, unique ? "true" : "false"});
    return create_index_success_;
}

bool IndexManagerMock::CreateCompositeIndex(const std::string &index_name,
                                           const std::string &table_name,
                                           const std::vector<std::string> &columns,
                                           bool unique) {
    std::string columns_str = "[";
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) columns_str += ",";
        columns_str += columns[i];
    }
    columns_str += "]";

    RecordCall("CreateCompositeIndex", {index_name, table_name, columns_str, unique ? "true" : "false"});
    return create_composite_index_success_;
}

bool IndexManagerMock::DropIndex(const std::string &index_name, const std::string &table_name) {
    RecordCall("DropIndex", {index_name, table_name});
    return drop_index_success_;
}

bool IndexManagerMock::IndexExists(const std::string &index_name, const std::string &table_name) const {
    RecordCall("IndexExists", {index_name, table_name});
    return index_exists_result_;
}

BPlusTreeIndex *IndexManagerMock::GetIndex(const std::string &index_name, const std::string &table_name) {
    RecordCall("GetIndex", {index_name, table_name});
    return get_index_result_;
}

std::vector<BPlusTreeIndex *> IndexManagerMock::GetTableIndexes(const std::string &table_name) const {
    RecordCall("GetTableIndexes", {table_name});
    return get_table_indexes_result_;
}

std::vector<std::string> IndexManagerMock::GetIndexedColumns(const std::string &table_name) const {
    RecordCall("GetIndexedColumns", {table_name});
    return get_indexed_columns_result_;
}

std::vector<std::vector<std::string>> IndexManagerMock::GetCompositeIndexedColumns(const std::string &table_name) const {
    RecordCall("GetCompositeIndexedColumns", {table_name});
    return get_composite_indexed_columns_result_;
}

std::string IndexManagerMock::GetIndexName(const std::string &table_name, const std::string &column_name) const {
    RecordCall("GetIndexName", {table_name, column_name});
    return get_index_name_result_;
}

std::string IndexManagerMock::GetCompositeIndexName(const std::string &table_name,
                                                   const std::vector<std::string> &columns) const {
    std::string columns_str = "[";
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) columns_str += ",";
        columns_str += columns[i];
    }
    columns_str += "]";

    RecordCall("GetCompositeIndexName", {table_name, columns_str});
    return get_composite_index_name_result_;
}

void IndexManagerMock::RecordCall(const std::string& method, const std::vector<std::string>& args) {
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