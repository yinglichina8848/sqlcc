/**
 * @file statistics_collector.cpp
 * @brief 缓冲池统计收集器实现
 */

#include "statistics_collector.h"
#include <sstream>

namespace sqlcc {
namespace storage {

StatisticsCollector::StatisticsCollector()
    : total_accesses_(0), total_hits_(0), replacement_count_(0), flush_count_(0) {
}

StatisticsCollector::~StatisticsCollector() {
}

void StatisticsCollector::RecordPageAccess(int32_t page_id) {
  total_accesses_.fetch_add(1, std::memory_order_relaxed);

  std::lock_guard<std::mutex> lock(access_stats_mutex_);
  access_frequency_[page_id]++;
}

void StatisticsCollector::RecordPageHit() {
  total_hits_.fetch_add(1, std::memory_order_relaxed);
}

void StatisticsCollector::RecordPageMiss() {
  // Page miss is implicitly recorded by total_accesses_ - total_hits_
}

void StatisticsCollector::RecordPageReplacement() {
  replacement_count_.fetch_add(1, std::memory_order_relaxed);
}

void StatisticsCollector::RecordPageFlush() {
  flush_count_.fetch_add(1, std::memory_order_relaxed);
}

uint64_t StatisticsCollector::GetTotalAccesses() const {
  return total_accesses_.load(std::memory_order_relaxed);
}

uint64_t StatisticsCollector::GetTotalHits() const {
  return total_hits_.load(std::memory_order_relaxed);
}

double StatisticsCollector::GetHitRate() const {
  uint64_t accesses = GetTotalAccesses();
  if (accesses == 0) {
    return 0.0;
  }
  return static_cast<double>(GetTotalHits()) / accesses;
}

std::unordered_map<int32_t, uint64_t> StatisticsCollector::GetAccessFrequency() const {
  std::lock_guard<std::mutex> lock(access_stats_mutex_);
  return access_frequency_;
}

uint64_t StatisticsCollector::GetReplacementCount() const {
  return replacement_count_.load(std::memory_order_relaxed);
}

uint64_t StatisticsCollector::GetFlushCount() const {
  return flush_count_.load(std::memory_order_relaxed);
}

void StatisticsCollector::Reset() {
  total_accesses_.store(0, std::memory_order_relaxed);
  total_hits_.store(0, std::memory_order_relaxed);
  replacement_count_.store(0, std::memory_order_relaxed);
  flush_count_.store(0, std::memory_order_relaxed);

  std::lock_guard<std::mutex> lock(access_stats_mutex_);
  access_frequency_.clear();
}

std::string StatisticsCollector::GetStatisticsString() const {
  std::stringstream ss;
  ss << "BufferPool Statistics:\n";
  ss << "  Total Accesses: " << GetTotalAccesses() << "\n";
  ss << "  Total Hits: " << GetTotalHits() << "\n";
  ss << "  Hit Rate: " << (GetHitRate() * 100.0) << "%\n";
  ss << "  Replacements: " << GetReplacementCount() << "\n";
  ss << "  Flushes: " << GetFlushCount() << "\n";

  auto access_freq = GetAccessFrequency();
  if (!access_freq.empty()) {
    ss << "  Top 5 Most Accessed Pages:\n";
    // Simple sorting by access count (in a real implementation, this could be optimized)
    std::vector<std::pair<int32_t, uint64_t>> sorted_freq(access_freq.begin(), access_freq.end());
    std::sort(sorted_freq.begin(), sorted_freq.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    size_t count = 0;
    for (const auto& pair : sorted_freq) {
      if (count >= 5) break;
      ss << "    Page " << pair.first << ": " << pair.second << " accesses\n";
      count++;
    }
  }

  return ss.str();
}

} // namespace storage
} // namespace sqlcc
