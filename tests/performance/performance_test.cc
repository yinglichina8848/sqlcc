#include "gtest/gtest.h"
#include "performance_test_base.h"

// 包含所有性能测试头文件
#include "concurrency_test/concurrency_performance_test.h"
#include "cpu_test/cpu_intensive_performance_test.h"
#include "memory_stress_test/memory_stress_test.h"
#include "stability_test/long_term_stability_test.h"
#include "million_insert_test.h"
#include "batch_prefetch_performance_test.h"
#include "buffer_pool_performance_test.h"
#include "disk_io_performance_test.h"
#include "index_performance_test.h"
#include "mixed_workload_test.h"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
