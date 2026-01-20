// Copyright 2024 The SQLCC Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef STORAGE_ENGINE_INDEX_MANAGER_SMART_INDEX_FACTORY_H_
#define STORAGE_ENGINE_INDEX_MANAGER_SMART_INDEX_FACTORY_H_

#include <memory>
#include <string>

namespace sqlcc {

class IndexManager;
class SmartIndexCache;

// Factory class for creating index managers with smart caching
class SmartIndexFactory {
public:
    explicit SmartIndexFactory(std::shared_ptr<SmartIndexCache> cache);
    ~SmartIndexFactory();

    // Create index manager with caching support
    std::shared_ptr<IndexManager> create_index_manager(const std::string& name);

    // Get cache statistics
    size_t get_cache_size() const;
    size_t get_cache_hits() const;
    size_t get_cache_misses() const;

private:
    std::shared_ptr<SmartIndexCache> cache_;
};

} // namespace sqlcc

#endif // STORAGE_ENGINE_INDEX_MANAGER_SMART_INDEX_FACTORY_H_
