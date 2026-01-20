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

#ifndef STORAGE_ENGINE_BUFFER_POOL_STATISTICS_COLLECTOR_H_
#define STORAGE_ENGINE_BUFFER_POOL_STATISTICS_COLLECTOR_H_

#include <atomic>
#include <string>
#include <memory>

namespace sqlcc {

class StatisticsCollector {
public:
    explicit StatisticsCollector(const std::string& name);
    ~StatisticsCollector();

    // Hit/miss statistics
    void record_hit();
    void record_miss();
    void record_eviction();

    // Access statistics
    void record_read(size_t bytes);
    void record_write(size_t bytes);

    // Performance metrics
    size_t get_hit_count() const;
    size_t get_miss_count() const;
    size_t get_eviction_count() const;
    double get_hit_rate() const;

    // I/O statistics
    size_t get_total_reads() const;
    size_t get_total_writes() const;
    size_t get_total_bytes_read() const;
    size_t get_total_bytes_written() const;

    // Reset statistics
    void reset();

    // Get statistics summary
    std::string get_summary() const;

private:
    std::string name_;

    // Cache statistics
    std::atomic<size_t> hits_;
    std::atomic<size_t> misses_;
    std::atomic<size_t> evictions_;

    // I/O statistics
    std::atomic<size_t> total_reads_;
    std::atomic<size_t> total_writes_;
    std::atomic<size_t> total_bytes_read_;
    std::atomic<size_t> total_bytes_written_;
};

} // namespace sqlcc

#endif // STORAGE_ENGINE_BUFFER_POOL_STATISTICS_COLLECTOR_H_
