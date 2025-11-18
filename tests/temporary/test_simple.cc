#include <iostream>
#include <filesystem>
#include "buffer_pool.h"
#include "config_manager.h"
#include "disk_manager.h"
#include <chrono>

using namespace sqlcc;

int main() {
    std::cout << "Starting simple BufferPool test..." << std::endl;
    
    try {
        // 创建临时测试文件
        std::string test_db = "/tmp/simple_test.db";
        auto& config_manager = ConfigManager::GetInstance();  // 使用单例实例
        auto disk_manager = std::make_unique<DiskManager>(test_db, config_manager);
        
        std::cout << "Creating BufferPool with enable_config_callback=false..." << std::endl;
        auto buffer_pool = std::make_unique<BufferPool>(disk_manager.get(), 2, config_manager, false);
        
        std::cout << "Testing FetchPage..." << std::endl;
        Page* page0 = buffer_pool->FetchPage(0);
        if (page0) {
            std::cout << "Successfully fetched page 0" << std::endl;
            buffer_pool->UnpinPage(0, false);
        } else {
            std::cout << "Failed to fetch page 0" << std::endl;
        }
        
        std::cout << "Fetching page 1..." << std::endl;
        Page* page1 = buffer_pool->FetchPage(1);
        if (page1) {
            std::cout << "Successfully fetched page 1" << std::endl;
            buffer_pool->UnpinPage(1, false);
        } else {
            std::cout << "Failed to fetch page 1" << std::endl;
        }
        
        std::cout << "Fetching page 2 (should trigger replacement)..." << std::endl;
        Page* page2 = buffer_pool->FetchPage(2);
        if (page2) {
            std::cout << "Successfully fetched page 2" << std::endl;
            buffer_pool->UnpinPage(2, false);
        } else {
            std::cout << "Failed to fetch page 2" << std::endl;
        }
        
        std::cout << "Test completed successfully!" << std::endl;
        
        // 清理
        std::filesystem::remove(test_db);
        
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}