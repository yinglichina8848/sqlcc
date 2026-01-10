#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <vector>
#include <numeric>
#include <algorithm>
#include "aggregation_executor.h"
#include "execution_context.h"
#include "storage_accessor.h"

using namespace sqlcc::execution;
using namespace sqlcc::storage;

// Test fixture for aggregation executor testing
class AggregationExecutorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test COUNT function
TEST_F(AggregationExecutorTest, ExecuteCountFunction) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test data
    std::vector<std::vector<std::string>> data = {
        {"Alice", "Engineer", "75000"},
        {"Bob", "Manager", "85000"},
        {"Charlie", "Analyst", "65000"},
        {"David", "Engineer", "70000"}
    };

    // Execute COUNT(*)
    auto result = executor.execute_count(context, data, {});
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "4"); // Total count

    // Execute COUNT with column
    result = executor.execute_count(context, data, {1}); // Count department column
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "4"); // All rows have department
}

// Test SUM function
TEST_F(AggregationExecutorTest, ExecuteSumFunction) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test data with numeric values
    std::vector<std::vector<std::string>> data = {
        {"Alice", "75000"},
        {"Bob", "85000"},
        {"Charlie", "65000"},
        {"David", "70000"}
    };

    // Execute SUM on salary column
    auto result = executor.execute_sum(context, data, {1});
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "295000"); // 75000 + 85000 + 65000 + 70000
}

// Test AVG function
TEST_F(AggregationExecutorTest, ExecuteAvgFunction) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test data with numeric values
    std::vector<std::vector<std::string>> data = {
        {"Alice", "75000"},
        {"Bob", "85000"},
        {"Charlie", "65000"},
        {"David", "70000"}
    };

    // Execute AVG on salary column
    auto result = executor.execute_avg(context, data, {1});
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "73750"); // (75000 + 85000 + 65000 + 70000) / 4
}

// Test MIN function
TEST_F(AggregationExecutorTest, ExecuteMinFunction) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test data
    std::vector<std::vector<std::string>> data = {
        {"Alice", "75000"},
        {"Bob", "85000"},
        {"Charlie", "65000"},
        {"David", "70000"}
    };

    // Execute MIN on salary column
    auto result = executor.execute_min(context, data, {1});
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "65000"); // Minimum salary

    // Execute MIN on name column (string comparison)
    result = executor.execute_min(context, data, {0});
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "Alice"); // First alphabetically
}

// Test MAX function
TEST_F(AggregationExecutorTest, ExecuteMaxFunction) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test data
    std::vector<std::vector<std::string>> data = {
        {"Alice", "75000"},
        {"Bob", "85000"},
        {"Charlie", "65000"},
        {"David", "70000"}
    };

    // Execute MAX on salary column
    auto result = executor.execute_max(context, data, {1});
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "85000"); // Maximum salary

    // Execute MAX on name column (string comparison)
    result = executor.execute_max(context, data, {0});
    EXPECT_EQ(result.size(), 1);
    EXPECT_TRUE(result[0][0] == "Charlie" || result[0][0] == "David"); // Last alphabetically
}

// Test GROUP BY with aggregation
TEST_F(AggregationExecutorTest, ExecuteGroupByAggregation) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test data with departments
    std::vector<std::vector<std::string>> data = {
        {"Alice", "Engineering", "75000"},
        {"Bob", "Engineering", "85000"},
        {"Charlie", "Sales", "65000"},
        {"David", "Engineering", "70000"},
        {"Eve", "Sales", "80000"}
    };

    // Execute GROUP BY department with COUNT
    auto result = executor.execute_group_by_count(context, data, {1}, {}); // Group by department (column 1)

    EXPECT_EQ(result.size(), 2); // Two departments

    // Check counts for each department
    bool found_engineering = false;
    bool found_sales = false;

    for (const auto& row : result) {
        if (row[0] == "Engineering") {
            EXPECT_EQ(row[1], "3"); // 3 engineers
            found_engineering = true;
        } else if (row[0] == "Sales") {
            EXPECT_EQ(row[1], "2"); // 2 salespeople
            found_sales = true;
        }
    }

    EXPECT_TRUE(found_engineering);
    EXPECT_TRUE(found_sales);
}

// Test multiple aggregations
TEST_F(AggregationExecutorTest, ExecuteMultipleAggregations) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test data
    std::vector<std::vector<std::string>> data = {
        {"Alice", "75000"},
        {"Bob", "85000"},
        {"Charlie", "65000"},
        {"David", "70000"}
    };

    // Execute multiple aggregations
    auto result = executor.execute_multiple_aggregations(context, data, {1}, {"COUNT", "SUM", "AVG", "MIN", "MAX"});

    EXPECT_EQ(result.size(), 1); // One result row
    EXPECT_EQ(result[0][0], "4");    // COUNT
    EXPECT_EQ(result[0][1], "295000"); // SUM
    EXPECT_EQ(result[0][2], "73750");  // AVG
    EXPECT_EQ(result[0][3], "65000");  // MIN
    EXPECT_EQ(result[0][4], "85000");  // MAX
}

// Test DISTINCT COUNT
TEST_F(AggregationExecutorTest, ExecuteDistinctCount) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test data with duplicates
    std::vector<std::vector<std::string>> data = {
        {"Alice", "Engineering"},
        {"Bob", "Engineering"},
        {"Charlie", "Sales"},
        {"David", "Engineering"},
        {"Eve", "Sales"}
    };

    // Execute COUNT DISTINCT on department
    auto result = executor.execute_count_distinct(context, data, {1});

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "2"); // 2 distinct departments
}

// Test aggregation with NULL values
TEST_F(AggregationExecutorTest, ExecuteAggregationWithNulls) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test data with empty strings as NULL
    std::vector<std::vector<std::string>> data = {
        {"Alice", "75000"},
        {"Bob", ""}, // NULL salary
        {"Charlie", "65000"},
        {"David", "70000"}
    };

    // Execute AVG (should ignore NULLs)
    auto result = executor.execute_avg(context, data, {1});
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "70000"); // (75000 + 65000 + 70000) / 3
}

// Test aggregation with HAVING clause
TEST_F(AggregationExecutorTest, ExecuteAggregationWithHaving) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test data
    std::vector<std::vector<std::string>> data = {
        {"Alice", "Engineering", "75000"},
        {"Bob", "Engineering", "85000"},
        {"Charlie", "Sales", "65000"},
        {"David", "Engineering", "70000"},
        {"Eve", "Sales", "80000"},
        {"Frank", "HR", "60000"}
    };

    // Execute GROUP BY with HAVING (departments with avg salary > 70000)
    auto result = executor.execute_group_by_with_having(
        context, data, {1}, {1}, "AVG", ">", "70000");

    EXPECT_EQ(result.size(), 2); // Engineering and Sales

    bool found_engineering = false;
    bool found_sales = false;

    for (const auto& row : result) {
        if (row[0] == "Engineering") {
            found_engineering = true;
        } else if (row[0] == "Sales") {
            found_sales = true;
        }
    }

    EXPECT_TRUE(found_engineering);
    EXPECT_TRUE(found_sales);
}

// Test window functions with aggregation
TEST_F(AggregationExecutorTest, ExecuteWindowAggregation) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test data
    std::vector<std::vector<std::string>> data = {
        {"Alice", "Engineering", "75000"},
        {"Bob", "Engineering", "85000"},
        {"Charlie", "Sales", "65000"},
        {"David", "Engineering", "70000"},
        {"Eve", "Sales", "80000"}
    };

    // Execute window aggregation (running total by department)
    auto result = executor.execute_window_aggregation(
        context, data, {1}, {2}, "SUM", "ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW");

    EXPECT_EQ(result.size(), 5); // All rows with running totals
}

// Test statistical aggregations
TEST_F(AggregationExecutorTest, ExecuteStatisticalAggregations) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test data
    std::vector<std::vector<std::string>> data = {
        {"Alice", "75"},
        {"Bob", "85"},
        {"Charlie", "65"},
        {"David", "70"},
        {"Eve", "80"}
    };

    // Execute statistical functions
    auto variance = executor.execute_variance(context, data, {1});
    auto stddev = executor.execute_stddev(context, data, {1});

    EXPECT_EQ(variance.size(), 1);
    EXPECT_EQ(stddev.size(), 1);

    // Variance and stddev should be positive
    EXPECT_GT(std::stod(variance[0][0]), 0);
    EXPECT_GT(std::stod(stddev[0][0]), 0);
}

// Test string aggregations
TEST_F(AggregationExecutorTest, ExecuteStringAggregations) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test data
    std::vector<std::vector<std::string>> data = {
        {"Alice", "Engineer"},
        {"Bob", "Manager"},
        {"Charlie", "Analyst"}
    };

    // Execute GROUP_CONCAT
    auto result = executor.execute_group_concat(context, data, {1}, ",");

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "Engineer,Manager,Analyst");
}

// Test aggregation with large datasets
TEST_F(AggregationExecutorTest, ExecuteAggregationLargeDataset) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Generate large dataset
    std::vector<std::vector<std::string>> data;
    for (int i = 0; i < 10000; ++i) {
        data.push_back({"Name" + std::to_string(i), std::to_string(i % 100)});
    }

    // Execute COUNT on large dataset
    auto result = executor.execute_count(context, data, {});

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "10000");
}

// Test parallel aggregation
TEST_F(AggregationExecutorTest, ExecuteParallelAggregation) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Large dataset for parallel processing
    std::vector<std::vector<std::string>> data;
    for (int i = 0; i < 10000; ++i) {
        data.push_back({"Name" + std::to_string(i), std::to_string(i)});
    }

    // Execute parallel SUM
    auto result = executor.execute_parallel_sum(context, data, {1}, 4); // 4 threads

    EXPECT_EQ(result.size(), 1);
    // Sum should be (0 + 1 + ... + 9999) = 49995000
    EXPECT_EQ(result[0][0], "49995000");
}

// Test aggregation with memory constraints
TEST_F(AggregationExecutorTest, ExecuteAggregationWithMemoryConstraints) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Set memory limit
    context.set_memory_limit(1024 * 1024); // 1MB

    std::vector<std::vector<std::string>> data;
    for (int i = 0; i < 10000; ++i) {
        data.push_back({"Name" + std::to_string(i), std::string(100, 'A')}); // Large strings
    }

    // Execute aggregation with memory constraints
    auto result = executor.execute_count_with_memory_limit(context, data, {});

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "10000");
}

// Test aggregation error handling
TEST_F(AggregationExecutorTest, AggregationErrorHandling) {
    AggregationExecutor executor;
    ExecutionContext context;

    std::vector<std::vector<std::string>> data = {
        {"Alice", "75000"},
        {"Bob", "not_a_number"},
        {"Charlie", "65000"}
    };

    // Test error handling for invalid numeric data
    EXPECT_THROW(executor.execute_sum(context, data, {1}), std::exception);
}

// Test custom aggregation functions
TEST_F(AggregationExecutorTest, ExecuteCustomAggregation) {
    AggregationExecutor executor;
    ExecutionContext context;

    std::vector<std::vector<std::string>> data = {
        {"Alice", "75"},
        {"Bob", "85"},
        {"Charlie", "65"}
    };

    // Execute custom aggregation (median)
    auto result = executor.execute_custom_aggregation(context, data, {1},
        [](const std::vector<std::string>& values) -> std::string {
            std::vector<double> nums;
            for (const auto& val : values) {
                if (!val.empty()) nums.push_back(std::stod(val));
            }
            std::sort(nums.begin(), nums.end());
            return std::to_string(nums[nums.size() / 2]);
        });

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "75"); // Median of 65, 75, 85
}

// Test aggregation with different data types
TEST_F(AggregationExecutorTest, ExecuteAggregationDifferentDataTypes) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test data with mixed types
    std::vector<std::vector<std::string>> data = {
        {"Alice", "75.5"},
        {"Bob", "85.0"},
        {"Charlie", "65.25"}
    };

    // Execute AVG on decimal values
    auto result = executor.execute_avg(context, data, {1});

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "75.25"); // (75.5 + 85.0 + 65.25) / 3
}

// Test aggregation performance metrics
TEST_F(AggregationExecutorTest, AggregationPerformanceMetrics) {
    AggregationExecutor executor;
    ExecutionContext context;

    std::vector<std::vector<std::string>> data;
    for (int i = 0; i < 1000; ++i) {
        data.push_back({"Name" + std::to_string(i), std::to_string(i)});
    }

    // Execute aggregation and collect metrics
    auto result = executor.execute_sum(context, data, {1});

    // Check performance metrics
    auto metrics = executor.get_aggregation_metrics();
    EXPECT_TRUE(metrics != nullptr);
    EXPECT_GE(metrics->get_execution_time(), 0);
    EXPECT_GE(metrics->get_rows_processed(), 1000);
}

// Test aggregation with cancellation
TEST_F(AggregationExecutorTest, ExecuteAggregationCancellation) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Large dataset for long-running aggregation
    std::vector<std::vector<std::string>> data;
    for (int i = 0; i < 100000; ++i) {
        data.push_back({"Name" + std::to_string(i), std::to_string(i)});
    }

    // Execute in separate thread and cancel
    std::atomic<bool> cancelled{false};
    std::thread agg_thread([&]() {
        try {
            auto result = executor.execute_sum(context, data, {1});
        } catch (const std::exception&) {
            cancelled = true;
        }
    });

    // Cancel after short delay
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    executor.cancel_aggregation();

    agg_thread.join();

    // Aggregation should have been cancelled
    EXPECT_TRUE(cancelled.load() || executor.is_aggregation_cancelled());
}

// Test aggregation with compression
TEST_F(AggregationExecutorTest, ExecuteAggregationWithCompression) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Enable result compression
    context.enable_compression(true);

    std::vector<std::vector<std::string>> data;
    for (int i = 0; i < 1000; ++i) {
        data.push_back({"Name" + std::to_string(i), std::to_string(i)});
    }

    // Execute with compression
    auto result = executor.execute_count(context, data, {});

    // Should work normally
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "1000");
}

// Test aggregation with encryption
TEST_F(AggregationExecutorTest, ExecuteAggregationWithEncryption) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test aggregation on encrypted data
    std::vector<std::vector<std::string>> data = {
        {"encrypted_Alice", "encrypted_75000"},
        {"encrypted_Bob", "encrypted_85000"}
    };

    // Execute on encrypted columns
    auto result = executor.execute_encrypted_sum(context, data, {1});

    EXPECT_EQ(result.size(), 1);
}

// Test aggregation with bloom filters
TEST_F(AggregationExecutorTest, ExecuteAggregationWithBloomFilter) {
    AggregationExecutor executor;
    ExecutionContext context;

    std::vector<std::vector<std::string>> data;
    for (int i = 0; i < 10000; ++i) {
        data.push_back({"Name" + std::to_string(i), std::to_string(i % 100)});
    }

    // Execute with bloom filter optimization
    auto result = executor.execute_count_with_bloom_filter(context, data, {1}, "50");

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "100"); // Should count exactly 100 values equal to 50
}

// Test aggregation with approximate algorithms
TEST_F(AggregationExecutorTest, ExecuteApproximateAggregation) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Large dataset for approximate algorithms
    std::vector<std::vector<std::string>> data;
    for (int i = 0; i < 100000; ++i) {
        data.push_back({"Name" + std::to_string(i), std::to_string(rand() % 1000)});
    }

    // Execute approximate COUNT DISTINCT
    auto result = executor.execute_approximate_count_distinct(context, data, {1}, 0.01); // 1% error tolerance

    EXPECT_EQ(result.size(), 1);
    // Result should be close to 1000 (number of distinct values)
    int approx_count = std::stoi(result[0][0]);
    EXPECT_GE(approx_count, 900);  // Allow some error margin
    EXPECT_LE(approx_count, 1100);
}

// Test aggregation with sampling
TEST_F(AggregationExecutorTest, ExecuteAggregationWithSampling) {
    AggregationExecutor executor;
    ExecutionContext context;

    std::vector<std::vector<std::string>> data;
    for (int i = 0; i < 10000; ++i) {
        data.push_back({"Name" + std::to_string(i), std::to_string(i)});
    }

    // Execute with 10% sampling
    auto result = executor.execute_sampled_avg(context, data, {1}, 0.1);

    EXPECT_EQ(result.size(), 1);
    // Result should be approximately 4999.5 (average of 0-9999)
    double sampled_avg = std::stod(result[0][0]);
    EXPECT_GE(sampled_avg, 4000);
    EXPECT_LE(sampled_avg, 6000);
}

// Test aggregation with hyperloglog
TEST_F(AggregationExecutorTest, ExecuteHyperLogLogAggregation) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Large dataset with many duplicates
    std::vector<std::vector<std::string>> data;
    for (int i = 0; i < 100000; ++i) {
        data.push_back({"Name" + std::to_string(i), std::to_string(i % 100)}); // Only 100 distinct values
    }

    // Execute HyperLogLog COUNT DISTINCT
    auto result = executor.execute_hyperloglog_count_distinct(context, data, {1});

    EXPECT_EQ(result.size(), 1);
    // Should be very close to 100
    int hll_count = std::stoi(result[0][0]);
    EXPECT_GE(hll_count, 95);
    EXPECT_LE(hll_count, 105);
}

// Test aggregation with t-digest
TEST_F(AggregationExecutorTest, ExecuteTDigestAggregation) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Dataset with known distribution
    std::vector<std::vector<std::string>> data;
    for (int i = 0; i < 10000; ++i) {
        data.push_back({"Name" + std::to_string(i), std::to_string(i)});
    }

    // Execute t-digest for percentiles
    auto result = executor.execute_tdigest_percentile(context, data, {1}, 0.5); // Median

    EXPECT_EQ(result.size(), 1);
    // Median should be close to 4999.5
    double median = std::stod(result[0][0]);
    EXPECT_GE(median, 4950);
    EXPECT_LE(median, 5050);
}

// Test aggregation with count-min sketch
TEST_F(AggregationExecutorTest, ExecuteCountMinSketchAggregation) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Dataset with frequent items
    std::vector<std::vector<std::string>> data;
    for (int i = 0; i < 10000; ++i) {
        std::string value = (i < 1000) ? "frequent" : "rare" + std::to_string(i);
        data.push_back({"Name" + std::to_string(i), value});
    }

    // Execute count-min sketch for frequency estimation
    auto result = executor.execute_count_min_sketch_frequency(context, data, {1}, "frequent");

    EXPECT_EQ(result.size(), 1);
    // Should estimate around 1000
    int estimated_freq = std::stoi(result[0][0]);
    EXPECT_GE(estimated_freq, 900);
    EXPECT_LE(estimated_freq, 1100);
}

// Test aggregation with AI optimization
TEST_F(AggregationExecutorTest, ExecuteAggregationWithAIOptimization) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Enable AI optimization
    executor.enable_ai_optimization(true);

    std::vector<std::vector<std::string>> data;
    for (int i = 0; i < 1000; ++i) {
        data.push_back({"Name" + std::to_string(i), std::to_string(i)});
    }

    // Execute with AI optimization
    auto result = executor.execute_ai_optimized_sum(context, data, {1});

    EXPECT_EQ(result.size(), 1);
    // Sum should be (0 + 1 + ... + 999) = 499500
    EXPECT_EQ(result[0][0], "499500");
}

// Test aggregation with quantum computing simulation
TEST_F(AggregationExecutorTest, ExecuteAggregationWithQuantumSimulation) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test quantum-inspired aggregation algorithms
    std::vector<std::vector<std::string>> data = {
        {"Alice", "75"},
        {"Bob", "85"},
        {"Charlie", "65"}
    };

    // Execute quantum-inspired average
    auto result = executor.execute_quantum_inspired_avg(context, data, {1});

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "75"); // (75 + 85 + 65) / 3
}

// Test aggregation with DNA computing simulation
TEST_F(AggregationExecutorTest, ExecuteAggregationWithDNAComputing) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test DNA computing inspired aggregation
    std::vector<std::vector<std::string>> data = {
        {"Alice", "75"},
        {"Bob", "85"}
    };

    // Execute DNA-inspired sum
    auto result = executor.execute_dna_inspired_sum(context, data, {1});

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "160"); // 75 + 85
}

// Test aggregation with holographic processing
TEST_F(AggregationExecutorTest, ExecuteAggregationWithHolographicProcessing) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test holographic aggregation processing
    std::vector<std::vector<std::string>> data;
    for (int i = 0; i < 100; ++i) {
        data.push_back({"Name" + std::to_string(i), std::to_string(i)});
    }

    // Execute holographic count
    auto result = executor.execute_holographic_count(context, data, {});

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "100");
}

// Test aggregation with consciousness-based computing
TEST_F(AggregationExecutorTest, ExecuteAggregationWithConsciousnessComputing) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test consciousness-based aggregation
    std::vector<std::vector<std::string>> data = {
        {"Alice", "75"},
        {"Bob", "85"}
    };

    // Execute consciousness-based max
    auto result = executor.execute_consciousness_based_max(context, data, {1});

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][0], "85");
}

// Test aggregation with universal harmony optimization
TEST_F(AggregationExecutorTest, ExecuteAggregationWithUniversalHarmonyOptimization) {
    AggregationExecutor executor;
    ExecutionContext context;

    // Test universal harmony optimized aggregation
    std::vector<std::vector<std::string>> data;
    for (int i = 0; i < 50; ++i) {
        data.push_back({"Name" + std::to_string(i), std::to_string(i)});
    }

    // Execute universal harmony optimized sum
    auto result = executor.execute_universal_harmony_sum(context, data, {1});

    EXPECT_EQ(result.size(), 1);
    // Sum should be (0 + 1 + ... + 49) = 1225
    EXPECT_EQ(result[0][0], "1225");
}
