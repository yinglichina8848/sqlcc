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

#ifndef STORAGE_ENGINE_INDEX_MANAGER_SMART_INDEX_CACHE_H_
#define STORAGE_ENGINE_INDEX_MANAGER_SMART_INDEX_CACHE_H_

#include <memory>
#include <unordered_map>
#include <mutex>
#include <string>

namespace sqlcc {

class IndexCacheEntry {
public:
    explicit IndexCacheEntry(const std::string& key);
    virtual ~IndexCacheEntry() = default;

    const std::string& get_key() const { return key_; }
    virtual size_t get_size() const = 0;

private:
    std::string key_;
};

class SmartIndexCache {
public:
    explicit SmartIndexCache(size_t max_size);
    ~SmartIndexCache();

    // Cache operations
    bool put(const std::string& key, std::shared_ptr<IndexCacheEntry> entry);
    std::shared_ptr<IndexCacheEntry> get(const std::string& key);
    bool remove(const std::string& key);
    void clear();

    // Cache statistics
    size_t get_size() const;
    size_t get_max_size() const;
    double get_hit_rate() const;

private:
    mutable std::mutex mutex_;
    size_t max_size_;
    size_t current_size_;
    std::unordered_map<std::string, std::shared_ptr<IndexCacheEntry>> cache_;

    // Statistics
    size_t hits_;
    size_t misses_;
};

} // namespace sqlcc

#endif // STORAGE_ENGINE_INDEX_MANAGER_SMART_INDEX_CACHE_H_
