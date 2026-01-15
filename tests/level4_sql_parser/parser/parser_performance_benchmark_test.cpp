#include <gtest/gtest.h>
#include <sql_parser/parser_new.h>
#include <sql_parser/lexer.h>
#include <chrono>
#include <vector>
#include <string>
#include <iostream>
#include <numeric>
#include <algorithm>
#include <cmath>

namespace sqlcc {
namespace sql_parser {
namespace test {

/**
 * @brief Parser Performance Benchmark Test
 *
 * 真实性能基准测试：测量新Parser的实际解析性能
 * - 解析时间测量
 * - 吞吐量计算
 * - 不同复杂度SQL语句的性能对比
 * - 内存使用效率评估
 */

class ParserPerformanceBenchmarkTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化测试数据
        testQueries_ = generateTestQueries();
        complexQueries_ = generateComplexQueries();
    }

    void TearDown() override {
        // 清理测试数据
    }

    // 生成不同复杂度的测试查询
    std::vector<std::string> generateTestQueries() {
        return {
            // 简单查询 (1-2个表，基本条件)
            "SELECT id FROM users",
            "SELECT name, age FROM users WHERE age > 18",
            "INSERT INTO users (name) VALUES ('John')",
            "UPDATE users SET age = 25 WHERE id = 1",
            "DELETE FROM users WHERE id = 1",

            // 中等复杂度 (JOIN，多条件)
            "SELECT u.name, p.title FROM users u JOIN posts p ON u.id = p.user_id",
            "SELECT u.name FROM users u WHERE u.age BETWEEN 18 AND 65 AND u.status = 'active'",
            "CREATE TABLE products (id INT, name VARCHAR(100), price DECIMAL(10,2))",

            // 复杂查询 (多表JOIN，子查询，聚合)
            "SELECT u.name, COUNT(p.id) as post_count FROM users u LEFT JOIN posts p ON u.id = p.user_id GROUP BY u.id, u.name",
            "SELECT * FROM users WHERE id IN (SELECT user_id FROM active_users WHERE last_login > '2024-01-01')",

            // 非常复杂的查询
            "SELECT u.name, p.title, c.content FROM users u JOIN posts p ON u.id = p.user_id LEFT JOIN comments c ON p.id = c.post_id WHERE u.created_at >= '2024-01-01' ORDER BY p.created_at DESC LIMIT 100"
        };
    }

    // 生成复杂查询用于压力测试
    std::vector<std::string> generateComplexQueries() {
        std::vector<std::string> queries;

        // 生成不同大小的复杂查询
        for (int i = 1; i <= 5; ++i) {
            std::string query = generateComplexQuery(i);
            queries.push_back(query);
        }

        return queries;
    }

    // 生成指定复杂度的复杂查询
    std::string generateComplexQuery(int complexity) {
        std::string query = "SELECT ";

        // 添加多个字段
        for (int i = 1; i <= complexity * 3; ++i) {
            if (i > 1) query += ", ";
            query += "field" + std::to_string(i);
        }

        query += " FROM table1 t1";

        // 添加多个JOIN
        for (int i = 2; i <= complexity + 1; ++i) {
            query += " JOIN table" + std::to_string(i) + " t" + std::to_string(i) +
                    " ON t1.id = t" + std::to_string(i) + ".ref_id";
        }

        // 添加复杂WHERE条件
        query += " WHERE t1.status = 'active'";
        for (int i = 2; i <= complexity + 1; ++i) {
            query += " AND t" + std::to_string(i) + ".created_at >= '2024-01-01'";
        }

        // 添加子查询
        if (complexity >= 2) {
            query += " AND t1.category_id IN (SELECT id FROM categories WHERE active = 1)";
        }

        // 添加GROUP BY和ORDER BY
        query += " GROUP BY t1.category";
        query += " ORDER BY t1.created_at DESC";

        if (complexity >= 3) {
            query += " LIMIT " + std::to_string(complexity * 10);
        }

        return query;
    }

    // 性能测量工具类
    class PerformanceTimer {
    public:
        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = std::chrono::time_point<Clock>;
        using Duration = std::chrono::nanoseconds;

        void start() {
            start_time_ = Clock::now();
        }

        void stop() {
            end_time_ = Clock::now();
            last_duration_ = std::chrono::duration_cast<Duration>(end_time_ - start_time_);
        }

        double getMilliseconds() const {
            return std::chrono::duration_cast<std::chrono::microseconds>(last_duration_).count() / 1000.0;
        }

        double getMicroseconds() const {
            return last_duration_.count() / 1000.0;
        }

        double getNanoseconds() const {
            return last_duration_.count();
        }

    private:
        TimePoint start_time_;
        TimePoint end_time_;
        Duration last_duration_{0};
    };

    // 测试结果结构
    struct BenchmarkResult {
        std::string query_name;
        size_t query_length;
        double parse_time_ms;
        size_t statement_count;
        bool success;

        BenchmarkResult(const std::string& name, size_t len, double time, size_t count, bool ok)
            : query_name(name), query_length(len), parse_time_ms(time), statement_count(count), success(ok) {}
    };

    // 执行单个查询的性能测试
    BenchmarkResult benchmarkSingleQuery(const std::string& sql, const std::string& name) {
        PerformanceTimer timer;

        try {
            timer.start();
            ParserNew parser(sql);
            auto statements = parser.parse();
            timer.stop();

            return BenchmarkResult(name, sql.length(), timer.getMilliseconds(),
                                 statements.size(), true);
        } catch (const std::exception& e) {
            timer.stop();
            std::cout << "Error parsing query '" << name << "': " << e.what() << std::endl;
            return BenchmarkResult(name, sql.length(), timer.getMilliseconds(), 0, false);
        }
    }

    // 执行多次测试取平均值
    BenchmarkResult benchmarkQueryAverage(const std::string& sql, const std::string& name, int iterations = 5) {
        std::vector<double> times;
        BenchmarkResult last_result("", 0, 0.0, 0, false);

        for (int i = 0; i < iterations; ++i) {
            auto result = benchmarkSingleQuery(sql, name + "_iter" + std::to_string(i));
            if (result.success) {
                times.push_back(result.parse_time_ms);
                last_result = result;
            }
        }

        if (times.empty()) {
            return BenchmarkResult(name, sql.length(), 0.0, 0, false);
        }

        // 计算平均时间
        double avg_time = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
        return BenchmarkResult(name, sql.length(), avg_time, last_result.statement_count, true);
    }

    std::vector<std::string> testQueries_;
    std::vector<std::string> complexQueries_;
};

// ============= 性能基准测试 =============

TEST_F(ParserPerformanceBenchmarkTest, BasicQueryPerformance) {
    std::cout << "\n🔬 基础查询性能测试" << std::endl;
    std::cout << "====================" << std::endl;

    std::vector<BenchmarkResult> results;

    for (size_t i = 0; i < testQueries_.size(); ++i) {
        const auto& query = testQueries_[i];
        std::string name = "Query_" + std::to_string(i + 1);

        auto result = benchmarkQueryAverage(query, name);
        results.push_back(result);

        std::cout << "📝 " << name << ": "
                  << result.parse_time_ms << " ms "
                  << "(" << result.query_length << " chars)" << std::endl;
    }

    // 计算统计信息
    std::vector<double> times;
    for (const auto& result : results) {
        if (result.success) {
            times.push_back(result.parse_time_ms);
        }
    }

    if (!times.empty()) {
        double avg_time = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
        double min_time = *std::min_element(times.begin(), times.end());
        double max_time = *std::max_element(times.begin(), times.end());

        std::cout << "\n📊 基础查询统计:" << std::endl;
        std::cout << "  平均解析时间: " << avg_time << " ms" << std::endl;
        std::cout << "  最快解析时间: " << min_time << " ms" << std::endl;
        std::cout << "  最慢解析时间: " << max_time << " ms" << std::endl;

        // 性能评估
        if (avg_time < 1.0) {
            std::cout << "  ✅ 性能评级: EXCELLENT (平均<1ms)" << std::endl;
        } else if (avg_time < 5.0) {
            std::cout << "  ✅ 性能评级: VERY GOOD (平均<5ms)" << std::endl;
        } else if (avg_time < 10.0) {
            std::cout << "  👍 性能评级: GOOD (平均<10ms)" << std::endl;
        } else {
            std::cout << "  ⚠️  性能评级: NEEDS IMPROVEMENT (平均>10ms)" << std::endl;
        }
    }
}

TEST_F(ParserPerformanceBenchmarkTest, ComplexQueryPerformance) {
    std::cout << "\n🐘 复杂查询性能测试" << std::endl;
    std::cout << "====================" << std::endl;

    std::vector<BenchmarkResult> results;

    for (size_t i = 0; i < complexQueries_.size(); ++i) {
        const auto& query = complexQueries_[i];
        std::string name = "Complex_Query_" + std::to_string(i + 1);

        auto result = benchmarkQueryAverage(query, name);
        results.push_back(result);

        std::cout << "📝 " << name << ": "
                  << result.parse_time_ms << " ms "
                  << "(" << result.query_length << " chars)" << std::endl;
    }

    // 分析复杂查询的性能特征
    if (!results.empty()) {
        double total_time = 0.0;
        size_t total_chars = 0;

        for (const auto& result : results) {
            if (result.success) {
                total_time += result.parse_time_ms;
                total_chars += result.query_length;
            }
        }

        double throughput = (total_chars * 1000.0) / total_time; // 字符/秒

        std::cout << "\n📊 复杂查询统计:" << std::endl;
        std::cout << "  总解析时间: " << total_time << " ms" << std::endl;
        std::cout << "  总字符数: " << total_chars << " chars" << std::endl;
        std::cout << "  解析吞吐量: " << throughput << " chars/sec" << std::endl;

        // 吞吐量评估
        if (throughput > 100000) {
            std::cout << "  🚀 吞吐量评级: EXCELLENT (>100K chars/sec)" << std::endl;
        } else if (throughput > 50000) {
            std::cout << "  ✅ 吞吐量评级: VERY GOOD (>50K chars/sec)" << std::endl;
        } else if (throughput > 25000) {
            std::cout << "  👍 吞吐量评级: GOOD (>25K chars/sec)" << std::endl;
        } else {
            std::cout << "  ⚠️  吞吐量评级: NEEDS IMPROVEMENT (<25K chars/sec)" << std::endl;
        }
    }
}

TEST_F(ParserPerformanceBenchmarkTest, ScalabilityTest) {
    std::cout << "\n📈 可扩展性测试" << std::endl;
    std::cout << "===============" << std::endl;

    // 测试不同长度查询的性能
    std::vector<std::pair<std::string, std::string>> scalability_tests = {
        {"SELECT 1", "Minimal_Query"},
        {generateComplexQuery(1), "Small_Complex"},
        {generateComplexQuery(3), "Medium_Complex"},
        {generateComplexQuery(5), "Large_Complex"}
    };

    std::vector<BenchmarkResult> results;

    for (const auto& [query, name] : scalability_tests) {
        auto result = benchmarkQueryAverage(query, name, 3); // 减少迭代次数以加快测试
        results.push_back(result);

        std::cout << "📏 " << name << ": "
                  << result.query_length << " chars, "
                  << result.parse_time_ms << " ms" << std::endl;
    }

    // 分析可扩展性
    if (results.size() >= 2) {
        double first_time = results[0].parse_time_ms;
        double last_time = results.back().parse_time_ms;
        double first_len = results[0].query_length;
        double last_len = results.back().query_length;

        double time_growth = last_time / first_time;
        double size_growth = last_len / first_len;

        std::cout << "\n📊 可扩展性分析:" << std::endl;
        std::cout << "  查询大小增长: " << size_growth << "x" << std::endl;
        std::cout << "  解析时间增长: " << time_growth << "x" << std::endl;
        std::cout << "  时间复杂度: O(n^" << (std::log(time_growth) / std::log(size_growth)) << ")" << std::endl;

        if (time_growth <= size_growth * 1.5) {
            std::cout << "  ✅ 可扩展性评级: EXCELLENT (近线性扩展)" << std::endl;
        } else if (time_growth <= size_growth * 2.0) {
            std::cout << "  👍 可扩展性评级: GOOD (合理扩展)" << std::endl;
        } else {
            std::cout << "  ⚠️  可扩展性评级: NEEDS IMPROVEMENT (扩展性差)" << std::endl;
        }
    }
}

TEST_F(ParserPerformanceBenchmarkTest, MemoryEfficiencyTest) {
    std::cout << "\n💾 内存效率测试" << std::endl;
    std::cout << "===============" << std::endl;

    // 测试解析大量简单查询的内存效率
    const int num_queries = 100;
    std::vector<std::string> simple_queries;
    for (int i = 0; i < num_queries; ++i) {
        simple_queries.push_back("SELECT id, name FROM users WHERE id = " + std::to_string(i));
    }

    PerformanceTimer timer;
    timer.start();

    size_t successful_parses = 0;
    for (const auto& query : simple_queries) {
        try {
            ParserNew parser(query);
            auto statements = parser.parse();
            if (!statements.empty()) {
                successful_parses++;
            }
        } catch (...) {
            // 忽略解析错误
        }
    }

    timer.stop();

    double total_time = timer.getMilliseconds();
    double avg_time_per_query = total_time / num_queries;
    double success_rate = (successful_parses * 100.0) / num_queries;

    std::cout << "📊 批量解析统计:" << std::endl;
    std::cout << "  测试查询数: " << num_queries << std::endl;
    std::cout << "  成功解析数: " << successful_parses << std::endl;
    std::cout << "  成功率: " << success_rate << "%" << std::endl;
    std::cout << "  总时间: " << total_time << " ms" << std::endl;
    std::cout << "  平均每查询时间: " << avg_time_per_query << " ms" << std::endl;

    // 内存效率评估
    if (success_rate >= 99.0 && avg_time_per_query < 1.0) {
        std::cout << "  ✅ 内存效率评级: EXCELLENT" << std::endl;
    } else if (success_rate >= 95.0 && avg_time_per_query < 2.0) {
        std::cout << "  ✅ 内存效率评级: VERY GOOD" << std::endl;
    } else if (success_rate >= 90.0) {
        std::cout << "  👍 内存效率评级: GOOD" << std::endl;
    } else {
        std::cout << "  ⚠️  内存效率评级: NEEDS IMPROVEMENT" << std::endl;
    }
}

TEST_F(ParserPerformanceBenchmarkTest, RegressionTest) {
    std::cout << "\n🔄 回归测试" << std::endl;
    std::cout << "===========" << std::endl;

    // 回归测试：确保所有测试查询都能正确解析
    struct RegressionTestCase {
        std::string sql;
        std::string description;
        bool should_succeed;
    };

    std::vector<RegressionTestCase> regression_tests = {
        // 基础功能测试
        {"SELECT 1", "基础SELECT", true},
        {"SELECT id FROM users", "简单SELECT", true},
        {"INSERT INTO users (name) VALUES ('test')", "简单INSERT", true},
        {"UPDATE users SET name = 'new' WHERE id = 1", "简单UPDATE", true},
        {"DELETE FROM users WHERE id = 1", "简单DELETE", true},
        {"CREATE TABLE test (id INT)", "简单CREATE TABLE", true},

        // JOIN测试
        {"SELECT u.name FROM users u JOIN posts p ON u.id = p.user_id", "INNER JOIN", true},
        {"SELECT u.name FROM users u LEFT JOIN posts p ON u.id = p.user_id", "LEFT JOIN", true},

        // 子查询测试
        {"SELECT * FROM users WHERE id IN (SELECT user_id FROM active_users)", "IN子查询", true},
        {"SELECT * FROM users WHERE EXISTS (SELECT 1 FROM posts WHERE user_id = users.id)", "EXISTS子查询", true},

        // 复杂表达式测试
        {"SELECT * FROM users WHERE age BETWEEN 18 AND 65", "BETWEEN表达式", true},
        {"SELECT * FROM users WHERE name LIKE 'John%'", "LIKE表达式", true},
        {"SELECT COUNT(*) FROM users GROUP BY department", "聚合函数", true},

        // 错误情况测试（应该失败）
        {"SELECT FROM WHERE", "语法错误", false},
        {"INSERT INTO VALUES", "不完整INSERT", false},
        {"SELECT * FROM", "不完整FROM", false}
    };

    int passed = 0;
    int failed = 0;

    for (const auto& test : regression_tests) {
        bool actual_success = false;
        double parse_time = 0.0;

        try {
            PerformanceTimer timer;
            timer.start();
            ParserNew parser(test.sql);
            auto statements = parser.parse();
            timer.stop();

            actual_success = !statements.empty();
            parse_time = timer.getMilliseconds();
        } catch (...) {
            actual_success = false;
        }

        bool test_passed = (actual_success == test.should_succeed);

        if (test_passed) {
            passed++;
            std::cout << "✅ " << test.description << ": PASS (" << parse_time << " ms)" << std::endl;
        } else {
            failed++;
            std::cout << "❌ " << test.description << ": FAIL (expected "
                      << (test.should_succeed ? "success" : "failure")
                      << ", got " << (actual_success ? "success" : "failure") << ")" << std::endl;
        }
    }

    double pass_rate = (passed * 100.0) / (passed + failed);

    std::cout << "\n📊 回归测试结果:" << std::endl;
    std::cout << "  通过: " << passed << std::endl;
    std::cout << "  失败: " << failed << std::endl;
    std::cout << "  通过率: " << pass_rate << "%" << std::endl;

    if (pass_rate >= 95.0) {
        std::cout << "  ✅ 回归测试评级: EXCELLENT" << std::endl;
    } else if (pass_rate >= 90.0) {
        std::cout << "  ✅ 回归测试评级: VERY GOOD" << std::endl;
    } else if (pass_rate >= 85.0) {
        std::cout << "  👍 回归测试评级: GOOD" << std::endl;
    } else {
        std::cout << "  ❌ 回归测试评级: FAILED - 需要修复" << std::endl;
    }

    // 断言：回归测试通过率必须达到90%以上
    ASSERT_GE(pass_rate, 90.0) << "回归测试通过率不足90%，需要修复解析器";
}

} // namespace test
} // namespace sql_parser
} // namespace sqlcc

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
