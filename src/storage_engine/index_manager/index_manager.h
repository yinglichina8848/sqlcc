#pragma once

#include <memory>
#include <string>
#include "../../utils/config_manager.h"

namespace sqlcc {

class StorageEngine;

namespace storage_engine {

namespace index_manager {

class IndexManager {
public:
    IndexManager(std::shared_ptr<StorageEngine> storage_engine,
                 ConfigManager &config);
    ~IndexManager();

    bool CreateIndex(const std::string& index_name,
                     const std::string& table_name,
                     const std::string& column_name,
                     bool unique = false);
    bool DropIndex(const std::string& index_name,
                   const std::string& table_name);

private:
    std::shared_ptr<StorageEngine> storage_engine_;
};

} // namespace index_manager
} // namespace storage_engine
} // namespace sqlcc
