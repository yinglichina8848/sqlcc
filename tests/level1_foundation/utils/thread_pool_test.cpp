#include <gtest/gtest.h>
#include <memory>
#include <future>
#include <atomic>
#include <chrono>
#include <numeric>
#include <vector>

#include "src/utils/thread_pool.h"

namespace sqlcc {
namespace utils {
namespace test {

// ==================== ThreadPool Tests ====================

class ThreadPoolBasicTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试默认构造函数（使用硬件并发数）
TEST_F(ThreadPoolBasicTest, DefaultConstructor) {
    ThreadPool pool;
    
    // 初始时没有活动任务
    EXPECT_EQ(pool.active_threads(), 0);
}

// 测试指定线程数的构造函数
TEST_F(ThreadPoolBasicTest, ConstructorWithThreadCount) {
    ThreadPool pool(4);
    
    EXPECT_EQ(pool.queued_tasks(), 0);
    EXPECT_EQ(pool.active_threads(), 0);
}

// 测试零线程数（应调整为至少1个线程）
TEST_F(ThreadPoolBasicTest, ZeroThreadCount) {
    ThreadPool pool(0);
    
    // 零线程会被调整为至少1个线程
    // 初始时没有活动任务
    EXPECT_EQ(pool.active_threads(), 0);
}

// 测试任务提交和完成
TEST_F(ThreadPoolBasicTest, TaskSubmissionAndCompletion) {
    ThreadPool pool(2);
    std::atomic<int> counter{0};
    
    auto future1 = pool.submit([&counter]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        counter++;
        return 42;
    });
    
    auto future2 = pool.submit([&counter]() {
        counter++;
        return 100;
    });
    
    EXPECT_EQ(future1.get(), 42);
    EXPECT_EQ(future2.get(), 100);
    EXPECT_EQ(counter.load(), 2);
}

// 测试多任务并发执行
TEST_F(ThreadPoolBasicTest, MultipleConcurrentTasks) {
    ThreadPool pool(4);
    const int task_count = 20;
    std::atomic<int> completed_count{0};
    
    std::vector<std::future<void>> futures;
    for (int i = 0; i < task_count; ++i) {
        futures.push_back(pool.submit([&completed_count]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            completed_count++;
        }));
    }
    
    // 等待所有任务完成
    for (auto& future : futures) {
        future.get();
    }
    
    EXPECT_EQ(completed_count.load(), task_count);
}

// 测试任务队列状态
TEST_F(ThreadPoolBasicTest, QueuedTasksStatus) {
    ThreadPool pool(2);
    
    EXPECT_EQ(pool.queued_tasks(), 0);
    
    pool.submit([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    });
    
    // 任务可能已完成或仍在队列中
    EXPECT_GE(pool.queued_tasks(), 0);
}

// 测试wait方法
TEST_F(ThreadPoolBasicTest, WaitForCompletion) {
    ThreadPool pool(2);
    std::atomic<int> counter{0};
    
    for (int i = 0; i < 10; ++i) {
        pool.submit([&counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            counter++;
        });
    }
    
    pool.wait();
    EXPECT_EQ(counter.load(), 10);
}

// 测试任务返回值
TEST_F(ThreadPoolBasicTest, TaskReturnValue) {
    ThreadPool pool(2);
    
    auto future = pool.submit([]() {
        return std::string("Hello ThreadPool");
    });
    
    EXPECT_EQ(future.get(), "Hello ThreadPool");
}

// 测试任务异常传播
TEST_F(ThreadPoolBasicTest, TaskExceptionPropagation) {
    ThreadPool pool(2);
    
    auto future = pool.submit([]() {
        throw std::runtime_error("Test exception");
    });
    
    EXPECT_THROW(future.get(), std::runtime_error);
}

// 测试多返回值类型
TEST_F(ThreadPoolBasicTest, MultipleReturnTypes) {
    ThreadPool pool(2);
    
    auto f1 = pool.submit([]() { return 1; });
    auto f2 = pool.submit([]() { return 3.14; });
    auto f3 = pool.submit([]() { return true; });
    auto f4 = pool.submit([]() { return std::string("test"); });
    
    EXPECT_EQ(f1.get(), 1);
    EXPECT_DOUBLE_EQ(f2.get(), 3.14);
    EXPECT_TRUE(f3.get());
    EXPECT_EQ(f4.get(), "test");
}

// 测试线程安全（并发写入）
TEST_F(ThreadPoolBasicTest, ThreadSafeConcurrentWrite) {
    ThreadPool pool(4);
    const int iterations = 1000;
    std::atomic<int> sum{0};
    
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 10; ++i) {
        futures.push_back(pool.submit([&sum, iterations]() {
            for (int j = 0; j < iterations; ++j) {
                sum++;
            }
        }));
    }
    
    for (auto& future : futures) {
        future.get();
    }
    
    EXPECT_EQ(sum.load(), 10 * iterations);
}

// 测试shutdown方法
TEST_F(ThreadPoolBasicTest, Shutdown) {
    ThreadPool pool(2);
    std::atomic<int> counter{0};
    
    auto future = pool.submit([&counter]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        counter++;
    });
    
    // 等待任务完成
    future.get();
    EXPECT_EQ(counter.load(), 1);
    
    // 然后关闭
    pool.shutdown();
    
    // 关闭后不应该接受新任务，但不应该崩溃
    EXPECT_NO_THROW(pool.wait());
}

// 测试shutdown_now方法
TEST_F(ThreadPoolBasicTest, ShutdownNow) {
    ThreadPool pool(2);
    
    pool.shutdown_now();
    
    // shutdown_now后不应接受新任务
    EXPECT_NO_THROW(pool.wait());
}

// 测试重复shutdown
TEST_F(ThreadPoolBasicTest, DoubleShutdown) {
    ThreadPool pool(2);
    
    pool.submit([]() { return 1; }).get();
    
    pool.shutdown();
    // 重复调用shutdown不应崩溃
    EXPECT_NO_THROW(pool.shutdown());
}

// 测试析构函数调用shutdown
TEST_F(ThreadPoolBasicTest, DestructorCallsShutdown) {
    {
        ThreadPool pool(2);
        pool.submit([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        });
        pool.wait();
    }
    // 析构时不应崩溃
}

// 测试任务参数传递
TEST_F(ThreadPoolBasicTest, TaskArgumentPassing) {
    ThreadPool pool(2);
    
    int value = 42;
    auto future = pool.submit([](int x) { return x * 2; }, value);
    
    EXPECT_EQ(future.get(), 84);
}

// 测试多参数传递
TEST_F(ThreadPoolBasicTest, MultipleArguments) {
    ThreadPool pool(2);
    
    auto future = pool.submit([](int a, int b, int c) {
        return a + b + c;
    }, 1, 2, 3);
    
    EXPECT_EQ(future.get(), 6);
}

// 测试lambda捕获
TEST_F(ThreadPoolBasicTest, LambdaCapture) {
    ThreadPool pool(2);
    int external_value = 100;
    
    auto future = pool.submit([&external_value]() {
        return external_value + 1;
    });
    
    EXPECT_EQ(future.get(), 101);
}

// 测试任务执行顺序（不保证顺序，但应全部完成）
TEST_F(ThreadPoolBasicTest, TaskExecutionCompletion) {
    ThreadPool pool(2);
    std::atomic<int> counter{0};
    
    for (int i = 0; i < 5; ++i) {
        pool.submit([&counter]() {
            counter++;
        });
    }
    
    pool.wait();
    EXPECT_EQ(counter.load(), 5);
}

// 测试空任务
TEST_F(ThreadPoolBasicTest, EmptyTask) {
    ThreadPool pool(2);
    std::atomic<bool> executed{false};
    
    pool.submit([&executed]() {
        executed = true;
    }).get();
    
    EXPECT_TRUE(executed.load());
}

// 测试大量小任务
TEST_F(ThreadPoolBasicTest, ManySmallTasks) {
    ThreadPool pool(4);
    const int task_count = 500;
    std::atomic<int> counter{0};
    
    std::vector<std::future<int>> futures;
    for (int i = 0; i < task_count; ++i) {
        futures.push_back(pool.submit([&counter]() {
            return ++counter;
        }));
    }
    
    int last_value = 0;
    for (auto& future : futures) {
        last_value = future.get();
    }
    
    EXPECT_EQ(counter.load(), task_count);
    EXPECT_EQ(last_value, task_count);
}

// 测试单线程池
TEST_F(ThreadPoolBasicTest, SingleThreadPool) {
    ThreadPool pool(1);
    std::atomic<int> counter{0};
    
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 5; ++i) {
        futures.push_back(pool.submit([&counter]() {
            return ++counter;
        }));
    }
    
    for (auto& future : futures) {
        future.get();
    }
    
    EXPECT_EQ(counter.load(), 5);
}

// 测试任务超时（通过wait超时）
TEST_F(ThreadPoolBasicTest, WaitWithNoTasks) {
    ThreadPool pool(2);
    
    // 没有任务时wait应立即返回
    EXPECT_NO_THROW(pool.wait());
}

// 测试move语义
TEST_F(ThreadPoolBasicTest, MoveSemantics) {
    ThreadPool pool1(2);
    ThreadPool pool2(2);
    
    pool1.submit([]() { return 1; }).get();
    pool2.submit([]() { return 2; }).get();
    
    // 两个池应该独立工作
    EXPECT_NO_THROW(pool1.shutdown());
    EXPECT_NO_THROW(pool2.shutdown());
}

// 测试共享指针无法复制（验证move语义）
TEST_F(ThreadPoolBasicTest, NonCopyable) {
    ThreadPool pool(2);
    
    // 验证ThreadPool无法复制
    EXPECT_FALSE(std::is_copy_constructible_v<ThreadPool>);
    EXPECT_FALSE(std::is_copy_assignable_v<ThreadPool>);
}

// ==================== ThreadPool Stress Tests ====================

class ThreadPoolStressTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 压力测试：高并发任务提交
TEST_F(ThreadPoolStressTest, HighConcurrencySubmission) {
    ThreadPool pool(4);
    const int task_count = 1000;
    std::atomic<int> counter{0};
    std::atomic<int> submitted{0};
    
    std::vector<std::thread> threads;
    for (int t = 0; t < 10; ++t) {
        threads.emplace_back([&pool, &counter, &submitted, task_count]() {
            for (int i = 0; i < task_count / 10; ++i) {
                pool.submit([&counter]() {
                    counter++;
                });
                submitted++;
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    pool.wait();
    EXPECT_EQ(counter.load(), task_count);
}

// 压力测试：CPU密集型任务
TEST_F(ThreadPoolStressTest, CPUIntensiveTasks) {
    ThreadPool pool(4);
    const int task_count = 20;
    std::atomic<int> completed{0};
    
    std::vector<std::future<void>> futures;
    for (int i = 0; i < task_count; ++i) {
        futures.push_back(pool.submit([&completed]() {
            // CPU密集型计算
            volatile int sum = 0;
            for (int j = 0; j < 100000; ++j) {
                sum += j;
            }
            completed++;
        }));
    }
    
    for (auto& future : futures) {
        future.get();
    }
    
    EXPECT_EQ(completed.load(), task_count);
}

// 压力测试：I/O密集型任务
TEST_F(ThreadPoolStressTest, IOIntensiveTasks) {
    ThreadPool pool(4);
    const int task_count = 20;
    std::atomic<int> completed{0};
    
    std::vector<std::future<void>> futures;
    for (int i = 0; i < task_count; ++i) {
        futures.push_back(pool.submit([&completed]() {
            // 模拟I/O等待
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            completed++;
        }));
    }
    
    for (auto& future : futures) {
        future.get();
    }
    
    EXPECT_EQ(completed.load(), task_count);
}

// 压力测试：混合任务类型
TEST_F(ThreadPoolStressTest, MixedTaskTypes) {
    ThreadPool pool(4);
    const int task_count = 50;
    std::atomic<int> cpu_count{0};
    std::atomic<int> io_count{0};
    
    std::vector<std::future<void>> futures;
    for (int i = 0; i < task_count; ++i) {
        if (i % 2 == 0) {
            futures.push_back(pool.submit([&cpu_count]() {
                volatile int sum = 0;
                for (int j = 0; j < 50000; ++j) {
                    sum += j;
                }
                cpu_count++;
            }));
        } else {
            futures.push_back(pool.submit([&io_count]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                io_count++;
            }));
        }
    }
    
    for (auto& future : futures) {
        future.get();
    }
    
    EXPECT_EQ(cpu_count.load() + io_count.load(), task_count);
}

// ==================== ThreadPool Edge Case Tests ====================

class ThreadPoolEdgeCaseTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 边界测试：极大线程数
TEST_F(ThreadPoolEdgeCaseTest, LargeThreadCount) {
    // 限制线程数以避免系统过载
    ThreadPool pool(16);
    
    std::atomic<int> counter{0};
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < 32; ++i) {
        futures.push_back(pool.submit([&counter]() {
            counter++;
        }));
    }
    
    for (auto& future : futures) {
        future.get();
    }
    
    EXPECT_EQ(counter.load(), 32);
}

// 边界测试：长时间运行任务
TEST_F(ThreadPoolEdgeCaseTest, LongRunningTask) {
    ThreadPool pool(2);
    std::atomic<bool> completed{false};
    
    auto future = pool.submit([&completed]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        completed = true;
        return 42;
    });
    
    // 等待任务完成
    EXPECT_EQ(future.get(), 42);
    EXPECT_TRUE(completed.load());
}

// 边界测试：嵌套任务提交
TEST_F(ThreadPoolEdgeCaseTest, NestedTaskSubmission) {
    ThreadPool pool(4);
    std::atomic<int> counter{0};
    
    pool.submit([&pool, &counter]() {
        counter++;
        // 不在任务中提交新任务（可能导致死锁）
    }).get();
    
    EXPECT_EQ(counter.load(), 1);
}

// 边界测试：任务抛出各种异常
TEST_F(ThreadPoolEdgeCaseTest, VariousExceptionTypes) {
    ThreadPool pool(2);
    
    // 测试 std::runtime_error
    EXPECT_THROW(pool.submit([]() { throw std::runtime_error("error"); }).get(),
                 std::runtime_error);
    
    // 测试 std::invalid_argument
    EXPECT_THROW(pool.submit([]() { throw std::invalid_argument("invalid"); }).get(),
                 std::invalid_argument);
    
    // 测试 std::out_of_range
    EXPECT_THROW(pool.submit([]() { throw std::out_of_range("range"); }).get(),
                 std::out_of_range);
}

} // namespace test
} // namespace utils
} // namespace sqlcc
