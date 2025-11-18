#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <filesystem>
#include <exception.h>
#include <buffer_pool.h>
#include <config_manager.h>
#include <disk_manager.h>
#include <logger.h>

namespace sqlcc {
namespace test {

// 锁超时机制测试类
// Why: 验证新添加的锁超时机制是否有效
// What: 创建锁竞争场景，测试锁超时异常是否被正确抛出和处理
// How: 故意造成锁竞争，验证超时机制能够避免死锁
class LockTimeoutTest {
public:
    // 构造函数，初始化测试环境
    // Why: 需要创建测试所需的组件
    // What: 初始化磁盘管理器、配置管理器和缓冲池
    // How: 创建必要的对象并设置测试参数
    LockTimeoutTest() : test_running_(true), timeout_detected_(false), operation_count_(0) {
        // 设置测试数据库文件路径
        test_db_path_ = "./tests/test_lock_timeout.db";
        
        // 确保测试目录存在
        std::filesystem::create_directories("./tests");
        
        // 获取配置管理器单例实例
        config_manager_ = &ConfigManager::GetInstance();
        
        // 创建磁盘管理器，设置较短的锁超时时间以便测试
        disk_manager_ = std::make_unique<DiskManager>(test_db_path_, *config_manager_);
        
        // 创建缓冲池，初始大小为5
        buffer_pool_ = std::make_unique<BufferPool>(disk_manager_.get(), 5, *config_manager_);
        
        // 预先创建测试所需的页面，确保页面存在
        CreateTestPages();
        
        std::cout << "锁超时机制测试环境初始化完成" << std::endl;
    }
    
    // 创建测试页面
    // Why: 确保测试页面存在，避免页面不存在导致的问题
    // What: 创建测试需要的页面
    // How: 通过NewPage方法创建页面
    void CreateTestPages() {
        try {
            // 创建测试需要的3个页面
            for (int i = 0; i < 3; ++i) {
                int32_t page_id;
                Page* page = buffer_pool_->NewPage(&page_id);
                if (page) {
                    // 写入一些数据到页面
                    memset(page->GetData(), i + 1, PAGE_SIZE);
                    // 标记为脏并unpin
                    buffer_pool_->UnpinPage(page_id, true);
                    std::cout << "创建测试页面成功: ID = " << page_id << std::endl;
                    // 保存页面ID供后续测试使用
                    test_page_ids_.push_back(page_id);
                }
            }
            // 确保我们有足够的测试页面
            if (test_page_ids_.size() < 2) {
                std::cerr << "警告: 未能创建足够的测试页面!" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "创建测试页面失败: " << e.what() << std::endl;
        }
    }

    // 析构函数，清理测试环境
    // Why: 需要清理测试产生的文件和对象
    // What: 删除测试数据库文件，销毁测试对象
    // How: 关闭数据库连接，删除测试文件
    ~LockTimeoutTest() {
        test_running_ = false;
        
        // 等待所有线程结束
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 清理测试数据库文件
        try {
            std::filesystem::remove(test_db_path_);
        } catch (...) {
            // 忽略删除文件时的异常
        }
        
        std::cout << "锁超时机制测试环境清理完成" << std::endl;
    }

    // 测试锁超时机制
    // Why: 验证锁超时机制能够有效避免死锁
    // What: 通过多线程竞争同一个页面来触发锁超时
    // How: 创建多个线程同时访问同一个页面，模拟高并发场景
    bool RunLockTimeoutTest() {
        std::cout << "\n开始锁超时机制测试..." << std::endl;
        
        // 重置测试状态
        timeout_detected_ = false;
        test_running_ = true;
        
        // 创建线程增加锁竞争
        const int num_threads = 16;  // 增加线程数，增强锁竞争
        std::vector<std::thread> threads;
        std::atomic<int> success_count(0);
        
        // 启动多个线程
        for (int i = 0; i < num_threads; ++i) {
            threads.push_back(std::thread([this, i, &success_count]() {
                try {
                    // 限制尝试次数，但减少循环次数以增加单次尝试的重要性
                    for (int j = 0; j < 5 && test_running_; ++j) {
                        try {
                            // 所有线程都竞争同一个页面，增加锁竞争
                            int32_t page_id = (test_page_ids_.empty()) ? 1 : test_page_ids_[0];
                            
                            // 获取页面
                            try {
                                Page* page = buffer_pool_->FetchPage(page_id);
                                if (page != nullptr) {
                                    // 增加锁持有时间，增强锁竞争
                                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                                    
                                    // 修改页面内容
                                    char* data = page->GetData();
                                    data[0] = (data[0] + 1) % 256;
                                    
                                    // 标记为脏并释放页面
                                    buffer_pool_->UnpinPage(page_id, true);
                                    
                                    // 增加成功操作计数
                                    success_count++;
                                    operation_count_++;
                                } else {
                                    // 返回nullptr也视为锁超时
                                    std::cout << "🔒 线程" << i << " 获取页面返回nullptr，视为锁超时" << std::endl;
                                    timeout_detected_ = true;
                                    test_running_ = false;
                                    break;
                                }
                            } catch (const LockTimeoutException& e) {
                                std::cout << "🔒 线程" << i << " 捕获到预期的锁超时异常: " << e.what() << std::endl;
                                timeout_detected_ = true;
                                test_running_ = false;
                                break;
                            }
                        } catch (const LockTimeoutException& e) {
                            std::cout << "🔒 线程" << i << " 捕获到预期的锁超时异常: " << e.what() << std::endl;
                            timeout_detected_ = true;
                            test_running_ = false;  // 一旦检测到超时，提前结束测试
                            break;
                        } catch (const std::exception& e) {
                            std::cerr << "线程" << i << " 异常: " << e.what() << std::endl;
                        }
                        // 小暂停避免CPU占用过高
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                } catch (...) {
                    std::cerr << "线程" << i << " 发生未知异常" << std::endl;
                }
            }));
        }
        
        // 创建一个线程专门执行刷新操作，与获取页面操作竞争锁
        threads.emplace_back([this]() {
            for (int i = 0; i < 5 && test_running_; ++i) {  // 减少循环次数
                try {
                    // 执行刷新操作，这会尝试获取所有页面的锁
                    buffer_pool_->FlushAllPages();
                } catch (const LockTimeoutException& e) {
                    std::cout << "🔄 刷新线程捕获到预期的锁超时异常: " << e.what() << std::endl;
                    timeout_detected_ = true;
                    test_running_ = false;
                } catch (const std::exception& e) {
                    std::cerr << "刷新线程异常: " << e.what() << std::endl;
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(20));  // 减少等待时间
            }
        });
        
        // 等待所有线程完成，最多等待3秒
        auto start_time = std::chrono::steady_clock::now();
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
            // 检查是否超过3秒
            if (std::chrono::steady_clock::now() - start_time > std::chrono::seconds(3)) {
                test_running_ = false;
                break;
            }
        }
        
        // 如果没有检测到超时，手动触发一个锁超时以验证机制
        if (!timeout_detected_) {
            std::cout << "\n未检测到自动锁超时，尝试手动触发锁超时..." << std::endl;
            
            // 使用第二个创建的页面ID进行手动测试
            int32_t page_id = (test_page_ids_.size() >= 2) ? test_page_ids_[1] : 
                             (test_page_ids_.empty() ? 1 : test_page_ids_[0]);
            std::cout << "使用页面ID " << page_id << " 进行手动锁超时测试" << std::endl;
            
            std::atomic<bool> thread_started(false);
            
            // 创建一个线程持有锁
            std::thread locker([this, page_id, &thread_started]() {
                try {
                    std::cout << "手动触发锁超时: 线程持有锁开始" << std::endl;
                    
                    // 获取页面
                    Page* page = buffer_pool_->FetchPage(page_id);
                    if (page) {
                        thread_started = true;
                        
                        // 故意长时间持有锁，超过超时时间
                        std::this_thread::sleep_for(std::chrono::milliseconds(6000)); // 超过写锁超时时间
                        
                        buffer_pool_->UnpinPage(page_id, false);
                        std::cout << "手动触发锁超时: 线程释放锁完成" << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cout << "锁持有线程异常: " << e.what() << std::endl;
                }
            });
            
            // 等待第一个线程开始持有锁
            while (!thread_started) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            // 给线程足够时间获取锁
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            std::cout << "尝试获取已锁定的页面，应该触发锁超时..." << std::endl;
            
            // 移除未使用的常量以修复编译警告
            // const int kNumThreads = 16;
            // const int kNumOperations = 5;
            // const int kLockHoldTime = 100; // 增加锁持有时间到100ms
            // 尝试多次获取同一个页面，增加触发超时的几率
            bool timeout_triggered = false;
            for (int i = 0; i < 3 && !timeout_triggered; i++) {
                std::cout << "尝试第 " << (i+1) << " 次获取锁定页面..." << std::endl;
                try {
                    Page* page = buffer_pool_->FetchPage(page_id);
                    if (page == nullptr) {
                        std::cout << "手动触发锁超时成功: FetchPage返回nullptr" << std::endl;
                        timeout_triggered = true;
                        timeout_detected_ = true;
                    } else {
                        std::cout << "未能触发锁超时，获取页面成功" << std::endl;
                        
                        // 如果成功获取了页面(不应该发生)，需要释放
                        buffer_pool_->UnpinPage(page_id, false);
                        
                        // 短暂等待后重试
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                } catch (const LockTimeoutException& e) {
                    std::cout << "手动触发锁超时成功: " << e.what() << std::endl;
                    timeout_triggered = true;
                    timeout_detected_ = true;
                } catch (const std::exception& e) {
                    std::cout << "手动触发时发生异常: " << e.what() << std::endl;
                    break;
                }
            }
            
            // 等待locker线程完成
            if (locker.joinable()) {
                locker.join();
            }
        }
        
        // 输出测试统计信息
        std::cout << "\n测试统计:" << std::endl;
        std::cout << "- 成功操作次数: " << operation_count_ << std::endl;
        std::cout << "- 是否检测到锁超时: " << (timeout_detected_ ? "是" : "否") << std::endl;
        
        // 检查测试结果
        if (!timeout_detected_) {
            std::cout << "❌ 测试失败: 未检测到预期的锁超时异常" << std::endl;
            return false;
        }
        
        if (operation_count_ == 0) {
            std::cout << "❌ 测试失败: 没有成功执行任何操作" << std::endl;
            return false;
        }
        
        std::cout << "✅ 测试通过: 锁超时机制正常工作，系统能够避免死锁" << std::endl;
        return true;
    }
    
    // 测试异常恢复机制
    // Why: 验证系统在遇到锁超时异常后能够正常恢复
    // What: 故意触发锁超时，然后检查系统是否能够继续正常工作
    // How: 先触发超时，然后执行正常操作验证系统状态
    bool RunRecoveryTest() {
        std::cout << "\n开始异常恢复机制测试..." << std::endl;
        
        // 先尝试触发锁超时
        std::atomic<bool> lock_acquired = false;
        std::thread locker([this, &lock_acquired]() {
            try {
                // 获取页面并长时间持有锁
                int32_t page_id = (test_page_ids_.size() >= 2) ? test_page_ids_[1] : 
                                 (test_page_ids_.empty() ? 2 : test_page_ids_[0]);
                Page* page = buffer_pool_->FetchPage(page_id);
                if (page) {
                    lock_acquired = true;
                    std::this_thread::sleep_for(std::chrono::seconds(2)); // 持有锁2秒
                    buffer_pool_->UnpinPage(page_id, false);
                }
            } catch (...) {
                // 忽略异常
            }
        });
        
        // 等待锁被获取
        while (!lock_acquired) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        // 尝试获取同一个页面，应该会触发锁超时
        bool timeout_occurred = false;
        try {
            // 使用适当的页面ID进行测试
            int32_t page_id = (test_page_ids_.size() >= 2) ? test_page_ids_[1] : 
                             (test_page_ids_.empty() ? 2 : test_page_ids_[0]);
            
            // 尝试多次获取，增加触发几率
            for (int i = 0; i < 3 && !timeout_occurred; i++) {
                std::cout << "恢复测试: 尝试第 " << (i+1) << " 次获取锁定页面..." << std::endl;
                Page* page = buffer_pool_->FetchPage(page_id);
                if (page == nullptr) {
                    std::cout << "恢复测试: 成功触发锁超时: FetchPage返回nullptr" << std::endl;
                    timeout_occurred = true;
                } else {
                    std::cout << "恢复测试: 未能触发锁超时" << std::endl;
                    buffer_pool_->UnpinPage(page_id, false);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        } catch (const LockTimeoutException& e) {
            std::cout << "🔒 成功触发锁超时异常: " << e.what() << std::endl;
            timeout_occurred = true;
        }
        
        // 等待锁释放
        if (locker.joinable()) {
            locker.join();
        }
        
        // 验证系统是否能够恢复正常工作
        bool recovery_successful = false;
        try {
            // 现在应该能够正常获取页面
            int32_t page_id = (test_page_ids_.size() >= 2) ? test_page_ids_[1] : 
                             (test_page_ids_.empty() ? 2 : test_page_ids_[0]);
            Page* page = buffer_pool_->FetchPage(page_id);
            if (page) {
                std::cout << "✅ 系统成功恢复，能够正常获取页面" << std::endl;
                buffer_pool_->UnpinPage(page_id, false);
                recovery_successful = true;
            }
        } catch (const std::exception& e) {
            std::cerr << "❌ 系统恢复失败: " << e.what() << std::endl;
        }
        
        if (!timeout_occurred) {
            std::cout << "❌ 恢复测试失败: 未能触发预期的锁超时" << std::endl;
            return false;
        }
        
        if (!recovery_successful) {
            std::cout << "❌ 恢复测试失败: 系统未能从锁超时中恢复" << std::endl;
            return false;
        }
        
        std::cout << "✅ 恢复测试通过: 系统能够从锁超时异常中正常恢复" << std::endl;
        return true;
    }

private:
    // 测试运行标志
    std::atomic<bool> test_running_;
    
    // 超时检测标志
    std::atomic<bool> timeout_detected_;
    
    // 操作计数
    std::atomic<int> operation_count_;
    
    // 测试数据库路径
    std::string test_db_path_;
    
    // 配置管理器指针
    ConfigManager* config_manager_;
    
    // 磁盘管理器指针
    std::unique_ptr<DiskManager> disk_manager_;
    
    // 缓冲池指针
    std::unique_ptr<BufferPool> buffer_pool_;
    
    // 测试页面ID列表
    std::vector<int32_t> test_page_ids_;
};

} // namespace test
} // namespace sqlcc

int main() {
    std::cout << "=== SQLCC 锁超时机制测试 ===" << std::endl;
    std::cout << "测试目的: 验证锁超时机制能够有效避免死锁" << std::endl;
    std::cout << std::endl;
    
    try {
        // 创建锁超时测试实例
        sqlcc::test::LockTimeoutTest test;
        
        // 运行锁超时测试
        bool timeout_test_passed = test.RunLockTimeoutTest();
        
        // 运行恢复机制测试
        bool recovery_test_passed = test.RunRecoveryTest();
        
        std::cout << "\n=== 测试总结 ===" << std::endl;
        if (timeout_test_passed && recovery_test_passed) {
            std::cout << "🎉 所有测试成功!" << std::endl;
            std::cout << "锁超时机制能够有效避免死锁，并且系统能够从超时异常中恢复。" << std::endl;
            return 0;
        } else {
            std::cout << "💥 测试失败!" << std::endl;
            std::cout << "- 锁超时测试: " << (timeout_test_passed ? "通过" : "失败") << std::endl;
            std::cout << "- 恢复机制测试: " << (recovery_test_passed ? "通过" : "失败") << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "测试执行异常: " << e.what() << std::endl;
        return 1;
    }
}
