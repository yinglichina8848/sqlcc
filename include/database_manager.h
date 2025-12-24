#ifndef SQLCC_DATABASE_MANAGER_H
#define SQLCC_DATABASE_MANAGER_H

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "page.h"
#include "storage/table_storage.h"
#include "transaction_manager.h"

namespace sqlcc {

// 前向声明
class ConfigManager;
class StorageEngine;
class BufferPoolSharded;
class TransactionManager;
class TableStorage;
class IndexManager;  // 前向声明IndexManager



} // namespace sqlcc

#endif // SQLCC_DATABASE_MANAGER_H
