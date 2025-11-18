#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace sqlcc {
namespace test {

/**
 * @brief 索引与约束性能基准测试
 *
 * 这个测试直接比较在相同数据上，有索引和无索引情况下的查询性能
 * 不依赖复杂的存储引擎，只在内存中进行操作
 */
class IndexConstraintBenchmark {
public:
  struct TestResult {
    std::string test_name;
    double duration_ms;
    size_t operations;
    double throughput_ops_per_sec;
    std::string description;
  };

  IndexConstraintBenchmark() : data_size_(100000), lookups_(10000) {}

  void runAllTests() {
    std::cout << "=== SQLCC 索引与约束性能基准测试 ===\n";
    std::cout << "测试数据量: " << data_size_ << " 条记录\n";
    std::cout << "测试查询数: " << lookups_ << " 次查找\n\n";

    testSequentialInsert();
    testRandomLookup();
    testRangeQuery();
    testConstraintValidation();
    testMixedWorkload();

    printSummary();
  }

private:
  void testSequentialInsert() {
    std::cout << "1. 顺序插入性能测试\n";

    // 无索引插入
    std::vector<std::pair<int, std::string>> data;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < data_size_; ++i) {
      data.emplace_back(i, "record_" + std::to_string(i));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto no_index_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    // 有索引插入 (using std::map for simplicity)
    std::map<int, std::string> indexed_data;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < data_size_; ++i) {
      indexed_data[i] = "record_" + std::to_string(i);
    }
    end = std::chrono::high_resolution_clock::now();
    auto indexed_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    results_.push_back({"Sequential Insert", indexed_time, size_t(data_size_),
                        data_size_ * 1000.0 / indexed_time,
                        "std::map vs std::vector sequential insert"});

    double overhead = (indexed_time - no_index_time) * 100.0 / no_index_time;
    std::cout << "   无索引插入时间: " << no_index_time << " ms\n";
    std::cout << "   有索引插入时间: " << indexed_time << " ms\n";
    std::cout << "   索引维护开销: " << overhead << "%\n\n";
  }

  void testRandomLookup() {
    std::cout << "2. 随机查找性能测试\n";

    // 准备测试数据
    std::vector<std::pair<int, std::string>> vector_data;
    std::map<int, std::string> map_data;
    std::unordered_map<int, std::string> hash_data;

    for (int i = 0; i < data_size_; ++i) {
      vector_data.emplace_back(i, "record_" + std::to_string(i));
      map_data[i] = "record_" + std::to_string(i);
      hash_data[i] = "record_" + std::to_string(i);
    }

    // 生成随机查询键
    std::vector<int> query_keys;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, data_size_ - 1);

    for (int i = 0; i < lookups_; ++i) {
      query_keys.push_back(dis(gen));
    }

    // Vector linear search (无索引)
    auto start = std::chrono::high_resolution_clock::now();
    int vector_hits = 0;
    for (int key : query_keys) {
      for (const auto &pair : vector_data) {
        if (pair.first == key) {
          vector_hits++;
          break;
        }
      }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto vector_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    // std::map indexed search (B树样式索引)
    start = std::chrono::high_resolution_clock::now();
    int map_hits = 0;
    for (int key : query_keys) {
      auto it = map_data.find(key);
      if (it != map_data.end()) {
        map_hits++;
      }
    }
    end = std::chrono::high_resolution_clock::now();
    auto map_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    // std::unordered_map hash search (哈希索引)
    start = std::chrono::high_resolution_clock::now();
    int hash_hits = 0;
    for (int key : query_keys) {
      auto it = hash_data.find(key);
      if (it != hash_data.end()) {
        hash_hits++;
      }
    }
    end = std::chrono::high_resolution_clock::now();
    auto hash_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    results_.push_back({"Random Lookup - BTree Index", map_time,
                        size_t(lookups_), lookups_ * 1000.0 / map_time,
                        "std::map logarithmic search - simulates B+ tree"});

    results_.push_back({"Random Lookup - Hash Index", hash_time,
                        size_t(lookups_), lookups_ * 1000.0 / hash_time,
                        "std::unordered_map O(1) lookup - ideal hash index"});

    std::cout << "   线性查找 (无索引): " << vector_time << " ms ("
              << vector_hits << " hits)\n";
    std::cout << "   B树查找 (std::map): " << map_time << " ms (" << map_hits
              << " hits)\n";
    std::cout << "   哈希查找: " << hash_time << " ms (" << hash_hits
              << " hits)\n";

    if (map_time > 0 && vector_time > 0) {
      double speedup = static_cast<double>(vector_time) / map_time;
      std::cout << "   B树索引加速比: " << speedup << "x\n";
    }

    if (hash_time > 0 && vector_time > 0) {
      double speedup = static_cast<double>(vector_time) / hash_time;
      std::cout << "   哈希索引加速比 (理想情况): " << speedup << "x\n\n";
    }
  }

  void testRangeQuery() {
    std::cout << "3. 范围查询性能测试\n";

    // 准备测试数据
    std::vector<std::pair<int, std::string>> vector_data;
    std::map<int, std::string> map_data;

    for (int i = 0; i < data_size_; ++i) {
      vector_data.emplace_back(i, "record_" + std::to_string(i));
      map_data[i] = "record_" + std::to_string(i);
    }

    // 范围查询测试
    int range_start = data_size_ / 4;
    int range_end = 3 * data_size_ / 4;
    const int num_ranges = 100;
    constexpr int range_width = 1000;

    // Vector range query (linear scan)
    auto start = std::chrono::high_resolution_clock::now();
    long long vector_results = 0;
    for (int i = 0; i < num_ranges; ++i) {
      int start_pos = (i * (range_width * 2)) % data_size_;
      int end_pos = start_pos + range_width;
      if (end_pos > data_size_)
        end_pos = data_size_;

      for (const auto &pair : vector_data) {
        if (pair.first >= start_pos && pair.first < end_pos) {
          vector_results++;
        }
      }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto vector_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    // Map range query (using lower_bound/upper_bound)
    start = std::chrono::high_resolution_clock::now();
    long long map_results = 0;
    for (int i = 0; i < num_ranges; ++i) {
      int start_pos = (i * (range_width * 2)) % data_size_;
      int end_pos = start_pos + range_width;
      if (end_pos > data_size_)
        end_pos = data_size_;

      auto it = map_data.lower_bound(start_pos);
      auto end_it = map_data.lower_bound(end_pos);
      while (it != end_it) {
        map_results++;
        ++it;
      }
    }
    end = std::chrono::high_resolution_clock::now();
    auto map_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    results_.push_back({"Range Query - BTree Index", map_time,
                        size_t(num_ranges), num_ranges * 1000.0 / map_time,
                        "Range queries using std::map iterators"});

    std::cout << "   线性范围查询: " << vector_time << " ms (" << vector_results
              << " results)\n";
    std::cout << "   B树范围查询: " << map_time << " ms (" << map_results
              << " results)\n";

    if (map_time > 0 && vector_time > 0) {
      double speedup = static_cast<double>(vector_time) / map_time;
      std::cout << "   范围查询加速比: " << speedup << "x\n\n";
    }
  }

  void testConstraintValidation() {
    std::cout << "4. 约束验证性能测试\n";

    // 模拟主键约束检查
    std::unordered_set<int> existing_primary_keys;

    // 填充现有数据
    for (int i = 0; i < data_size_; ++i) {
      existing_primary_keys.insert(i);
    }

    std::vector<int> new_keys;
    for (int i = data_size_; i < data_size_ + lookups_; ++i) {
      new_keys.push_back(i); // 不会冲突的新主键
    }

    // 模拟外键约束检查 (半数会失败)
    std::unordered_set<int> referenced_keys_set;
    for (int i = 0; i < data_size_; ++i) {
      if (i % 2 == 0)
        referenced_keys_set.insert(i); // 只有偶数主键被引用
    }

    std::vector<int> foreign_keys;
    for (int i = 0; i < lookups_; ++i) {
      foreign_keys.push_back(i % 2 == 0 ? i
                                        : i + data_size_); // 半数外键引用无效
    }

    // 主键约束验证
    auto start = std::chrono::high_resolution_clock::now();
    int pk_violations = 0;
    for (int key : new_keys) {
      if (existing_primary_keys.count(key) > 0) {
        pk_violations++;
      }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto pk_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    // 外键约束验证
    start = std::chrono::high_resolution_clock::now();
    int fk_violations = 0;
    for (int key : foreign_keys) {
      if (referenced_keys_set.count(key) == 0) {
        fk_violations++;
      }
    }
    end = std::chrono::high_resolution_clock::now();
    auto fk_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    results_.push_back({"Constraint Validation - Primary Key", pk_time,
                        size_t(lookups_), lookups_ * 1000.0 / pk_time,
                        "Primary key uniqueness validation"});

    results_.push_back({"Constraint Validation - Foreign Key", fk_time,
                        size_t(lookups_), lookups_ * 1000.0 / fk_time,
                        "Foreign key referential integrity validation"});

    std::cout << "   主键约束验证: " << pk_time << " ms (" << pk_violations
              << " violations)\n";
    std::cout << "   外键约束验证: " << fk_time << " ms (" << fk_violations
              << " violations)\n";

    double total_constraint_time = pk_time + fk_time;
    double per_record_overhead_us = (total_constraint_time * 1000.0) / lookups_;
    std::cout << "   约束验证平均每记录开销: " << per_record_overhead_us
              << " μs\n\n";
  }

  void testMixedWorkload() {
    std::cout << "5. 混合工作负载测试\n";

    // 综合测试：70%读，30%写，其中读操作包含索引查询和全表扫描
    const int total_operations = data_size_ / 10; // 减少操作数以控制测试时间
    const int read_ratio = 70;

    std::map<int, std::string> indexed_data;
    std::vector<std::pair<int, std::string>> table_scan_data;

    // 初始数据填充
    for (int i = 0; i < data_size_; ++i) {
      std::string value = "data_" + std::to_string(i);
      indexed_data[i] = value;
      table_scan_data.emplace_back(i, value);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> action_dist(0, 99);
    std::uniform_int_distribution<> key_dist(0, data_size_ - 1);

    int reads_with_index = 0, reads_table_scan = 0, writes = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < total_operations; ++i) {
      int action = action_dist(gen);

      if (action < read_ratio) {
        // 读操作 - 由两种查询混合
        int key = key_dist(gen);

        if (action_dist(gen) < 50) {
          // 索引查询 (efficient)
          auto it = indexed_data.find(key);
          if (it != indexed_data.end()) {
            reads_with_index++;
          }
        } else {
          // 表扫描查询 (inefficient but realistic)
          for (const auto &pair : table_scan_data) {
            if (pair.first == key) {
              reads_table_scan++;
              break;
            }
          }
        }
      } else {
        // 写操作 - 插入或更新
        int key = key_dist(gen);
        std::string new_value = "updated_" + std::to_string(i);

        if (key_dist(gen) % 2 == 0) {
          // 插入新记录 (真实的数据库中这会导致表增长)
          int new_key = data_size_ + i;
          indexed_data[new_key] = new_value;
          table_scan_data.emplace_back(new_key, new_value);
        } else {
          // 更新现有记录
          indexed_data[key] = new_value;
          // 表扫描数据也需要更新 (在实际系统中这会很昂贵)
          for (auto &pair : table_scan_data) {
            if (pair.first == key) {
              pair.second = new_value;
              break;
            }
          }
        }
        writes++;
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto total_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    int total_reads = reads_with_index + reads_table_scan;
    results_.push_back({"Mixed Workload - Indexed Reads",
                        total_time, // 这个时间包含了所有操作
                        size_t(total_reads + writes),
                        (total_reads + writes) * 1000.0 / total_time,
                        "70% reads (50% indexed, 50% table scan), 30% writes"});

    std::cout << "   索引查询读取: " << reads_with_index << " 次\n";
    std::cout << "   表扫描读取: " << reads_table_scan << " 次\n";
    std::cout << "   写操作: " << writes << " 次\n";
    std::cout << "   总时间: " << total_time << " ms\n";

    if (total_time > 0) {
      double ops_per_sec = (total_reads + writes) * 1000.0 / total_time;
      std::cout << "   混合工作负载吞吐量: " << ops_per_sec << " ops/sec\n\n";
    }
  }

  void printSummary() {
    std::cout << "=== 性能测试总结 ===\n";
    std::cout << std::left;
    std::cout << std::setw(35) << "测试项目" << std::setw(15) << "时间(ms)"
              << std::setw(12) << "操作数" << std::setw(18) << "吞吐量(ops/s)"
              << "描述\n";
    std::cout << std::string(80, '-') << "\n";

    for (const auto &result : results_) {
      std::cout << std::setw(35) << result.test_name.substr(0, 34)
                << std::setw(15) << result.duration_ms << std::setw(12)
                << result.operations << std::setw(18) << std::fixed
                << std::setprecision(1) << result.throughput_ops_per_sec
                << result.description << "\n";
    }

    std::cout << "\n关键性能洞察:\n";
    std::cout << "1. 索引查询通常比线性查找快10-1000倍\n";
    std::cout << "2. B树索引在范围查询中特别有效\n";
    std::cout << "3. 约束验证的性能开销相对较低 (< 1μs per record)\n";
    std::cout << "4. 混合工作负载中索引可以显著提升读性能\n";
    std::cout << "5. 索引维护会增加写操作的开销\n\n";

    std::cout << "注意：此测试在内存中进行，真实的磁盘I/O会影响性能表现。\n";
    std::cout << "实际数据库系统中的索引性能取决于多种因素：数据分布、工作负载"
                 "模式、缓冲池大小等。\n";
  }

private:
  std::vector<TestResult> results_;
  const int data_size_;
  const int lookups_;
};

} // namespace test
} // namespace sqlcc

int main() {
  sqlcc::test::IndexConstraintBenchmark benchmark;
  benchmark.runAllTests();
  return 0;
}
