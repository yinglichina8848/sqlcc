#pragma once

#include "performance_test_base.h"
#include "sql_executor.h"

namespace sqlcc {
namespace test {

/**
 * 大规模索引约束测试类
 */
class LargeScaleIndexConstraintTest : public PerformanceTestBase {
public:
    /**
     * 构造函数
     */
    LargeScaleIndexConstraintTest() = default;

    /**
     * 析构函数
     */
    ~LargeScaleIndexConstraintTest() override = default;

    /**
     * 运行所有测试
     */
    void RunAllTests() override;

private:
    // 无成员变量
};

} // namespace test
} // namespace sqlcc
