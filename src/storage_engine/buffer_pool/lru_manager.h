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

#ifndef STORAGE_ENGINE_BUFFER_POOL_LRU_MANAGER_H_
#define STORAGE_ENGINE_BUFFER_POOL_LRU_MANAGER_H_

#include <list>
#include <unordered_map>
#include <memory>

namespace sqlcc {

class BufferFrame;

class LRUManager {
public:
    explicit LRUManager(size_t capacity);
    ~LRUManager();

    // Access a buffer frame
    void access(BufferFrame* frame);

    // Remove a buffer frame
    void remove(BufferFrame* frame);

    // Get victim frame for eviction
    BufferFrame* get_victim();

    // Check if frame is in LRU
    bool contains(BufferFrame* frame) const;

    // Get current size
    size_t size() const;

    // Get capacity
    size_t capacity() const;

private:
    size_t capacity_;
    std::list<BufferFrame*> lru_list_;
    std::unordered_map<BufferFrame*, std::list<BufferFrame*>::iterator> lru_map_;
};

} // namespace sqlcc

#endif // STORAGE_ENGINE_BUFFER_POOL_LRU_MANAGER_H_
