#include "buffer_pool_performance_test.h"

namespace sqlcc {
namespace test {

// 实现缓冲池性能测试中的方法
void BufferPoolPerformanceTest::SetUp() {
    // 基类中没有SetUp方法，直接设置测试环境
    // 注意：sql_executor_应通过基类或类成员获取
}

void BufferPoolPerformanceTest::TearDown() {
    // 基类中没有TearDown方法，直接调用Cleanup
    Cleanup();
}

} // namespace test
} // namespace sqlcc
