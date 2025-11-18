#include "disk_manager.h"
#include "config_manager.h"
#include "logger.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cassert>
#include <variant>
#include <functional>

using namespace sqlcc;

void TestSyncFunctionality() {
    std::cout << "Testing Sync functionality..." << std::endl;
    
    // 创建临时测试文件
    std::string test_file = "test_sync.db";
    
    // 清理之前的测试文件
    std::remove(test_file.c_str());
    
    try {
        // 获取配置管理器单例实例
        ConfigManager& config_manager = ConfigManager::GetInstance();
        
        // 创建磁盘管理器
        DiskManager disk_manager(test_file, config_manager);
        
        // 分配一个页面
        int32_t page_id = disk_manager.AllocatePage();
        std::cout << "Allocated page ID: " << page_id << std::endl;
        
        // 创建测试数据
        std::vector<char> test_data(8192, 'A'); // 8KB 页面大小
        
        // 写入页面数据
        bool write_result = disk_manager.WritePage(page_id, test_data.data());
        assert(write_result == true);
        std::cout << "Successfully wrote page data" << std::endl;
        
        // 调用Sync方法确保数据持久化
        bool sync_result = disk_manager.Sync();
        assert(sync_result == true);
        std::cout << "Successfully synced data to disk" << std::endl;
        
        // 验证文件存在且包含数据
        std::ifstream file(test_file, std::ios::binary | std::ios::ate);
        assert(file.is_open());
        
        std::streamsize file_size = file.tellg();
        std::cout << "File size after sync: " << file_size << " bytes" << std::endl;
        assert(file_size > 0);
        file.close();
        
        // 读取页面数据验证
        std::vector<char> read_data(8192);
        bool read_result = disk_manager.ReadPage(page_id, read_data.data());
        assert(read_result == true);
        
        // 验证数据一致性
        bool data_match = (read_data == test_data);
        assert(data_match == true);
        std::cout << "Data consistency verified after sync" << std::endl;
        
        std::cout << "Sync functionality test PASSED!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        assert(false);
    }
    
    // 清理测试文件
    std::remove(test_file.c_str());
}

int main() {
    try {
        TestSyncFunctionality();
        std::cout << "All tests completed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test suite failed: " << e.what() << std::endl;
        return 1;
    }
}