#pragma once

#include "performance_test_base.h"

namespace sqlcc {
namespace test {

/**
 * 索引约束性能基准测试类
 */
class IndexConstraintBenchmark : public PerformanceTestBase {
public:
    /**
     * 构造函数
     */
    IndexConstraintBenchmark() = default;

    /**
     * 析构函数
     */
    ~IndexConstraintBenchmark() override = default;

    /**
     * 运行所有测试
     */
    void RunAllTests() override;
};

} // namespace test
} // namespace sqlcc
