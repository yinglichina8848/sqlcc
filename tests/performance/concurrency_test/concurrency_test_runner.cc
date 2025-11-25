#include "concurrency_performance_test.h"
#include "sharded_buffer_pool_concurrent_test.h"
#include "gtest/gtest.h"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
