#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <chrono>
#include <thread>

#include "src/utils/thread_pool.h"

namespace sqlcc {
namespace test {

class ThreadPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        pool = nullptr;
    }

    void TearDown() override {
        if (pool) {
            pool->shutdown();
            pool.reset();
        }
    }

    std::unique_ptr<utils::ThreadPool> pool;
};

TEST_F(ThreadPoolTest, ConstructorAndDestructor) {
    pool = std::make_unique<utils::ThreadPool>(2);
    EXPECT_TRUE(pool != nullptr);
    EXPECT_EQ(pool->queued_tasks(), 0);
}

TEST_F(ThreadPoolTest, SubmitTask) {
    pool = std::make_unique<utils::ThreadPool>(2);
    std::atomic<int> counter{0};

    for (int i = 0; i < 5; ++i) {
        pool->submit([&counter]() {
            counter++;
        });
    }

    pool->wait();
    EXPECT_EQ(counter.load(), 5);
}

TEST_F(ThreadPoolTest, SubmitTaskWithReturnValue) {
    pool = std::make_unique<utils::ThreadPool>(2);
    auto future = pool->submit([]() {
        return 42;
    });

    EXPECT_EQ(future.get(), 42);
}

TEST_F(ThreadPoolTest, SubmitMultipleTasks) {
    pool = std::make_unique<utils::ThreadPool>(4);
    std::atomic<int> sum{0};

    std::vector<std::future<int>> futures;
    for (int i = 0; i < 10; ++i) {
        futures.push_back(pool->submit([i]() {
            return i * i;
        }));
    }

    int total = 0;
    for (auto& f : futures) {
        total += f.get();
    }

    EXPECT_EQ(total, 285);
}

TEST_F(ThreadPoolTest, QueuedTasksCount) {
    pool = std::make_unique<utils::ThreadPool>(1);

    pool->submit([]() { std::this_thread::sleep_for(std::chrono::milliseconds(10)); });
    pool->submit([]() {});
    pool->submit([]() {});

    EXPECT_GE(pool->queued_tasks(), 2);
}

TEST_F(ThreadPoolTest, Shutdown) {
    pool = std::make_unique<utils::ThreadPool>(2);
    std::atomic<bool> task_executed{false};

    pool->submit([&task_executed]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        task_executed = true;
    });

    pool->shutdown();
    EXPECT_TRUE(task_executed.load());
}

} // namespace test
} // namespace sqlcc
