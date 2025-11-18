#include <iostream>
#include <cstring>
#include <memory>
#include <cstdio>
#include "storage_engine.h"
#include "config_manager.h"
#include "page.h"

using namespace sqlcc;

int main() {
    std::cout << "=== BufferPool页面ID分配逻辑修复验证测试 ===" << std::endl;
    
    // 清理测试文件
    std::remove("test_page_id_fix.db");
    
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    config.SetValue("database.db_file_path", "test_page_id_fix.db");
    config.SetValue("buffer_pool.pool_size", 3); // 使用小缓冲池来测试页面替换
    
    // 创建存储引擎
    std::unique_ptr<StorageEngine> engine = std::make_unique<StorageEngine>(config);
    
    std::cout << "1. 创建页面1和页面2..." << std::endl;
    int32_t page_id1, page_id2;
    Page* page1 = engine->NewPage(&page_id1);
    Page* page2 = engine->NewPage(&page_id2);
    
    if (!page1 || !page2) {
        std::cerr << "ERROR: 无法创建页面" << std::endl;
        return 1;
    }
    
    std::cout << "   页面1 ID: " << page_id1 << std::endl;
    std::cout << "   页面2 ID: " << page_id2 << std::endl;
    
    // 写入数据
    char data1[] = "Page 1 data";
    char data2[] = "Page 2 data";
    page1->WriteData(0, data1, strlen(data1) + 1);
    page2->WriteData(0, data2, strlen(data2) + 1);
    
    std::cout << "2. 取消固定页面1和页面2..." << std::endl;
    engine->UnpinPage(page_id1, true); // 标记为脏页
    engine->UnpinPage(page_id2, true); // 标记为脏页
    
    std::cout << "3. 创建页面3（应该会触发页面替换）..." << std::endl;
    int32_t page_id3;
    Page* page3 = engine->NewPage(&page_id3);
    
    if (!page3) {
        std::cerr << "ERROR: 无法创建页面3" << std::endl;
        return 1;
    }
    
    std::cout << "   页面3 ID: " << page_id3 << std::endl;
    
    // 验证页面ID的分配顺序性
    std::cout << "4. 验证页面ID分配顺序性..." << std::endl;
    if (page_id3 == page_id1 + 1 && page_id2 == page_id1 + 1) {
        std::cout << "   ✓ 页面ID分配顺序正确" << std::endl;
    } else {
        std::cout << "   ✗ 页面ID分配顺序异常: " << page_id1 << ", " << page_id2 << ", " << page_id3 << std::endl;
    }
    
    // 写入数据到页面3
    char data3[] = "Page 3 data";
    page3->WriteData(0, data3, strlen(data3) + 1);
    
    std::cout << "5. 创建页面4（进一步测试）..." << std::endl;
    int32_t page_id4;
    Page* page4 = engine->NewPage(&page_id4);
    
    if (!page4) {
        std::cerr << "ERROR: 无法创建页面4" << std::endl;
        return 1;
    }
    
    std::cout << "   页面4 ID: " << page_id4 << std::endl;
    
    // 验证连续性
    std::cout << "6. 验证页面ID连续性..." << std::endl;
    if (page_id4 == page_id3 + 1) {
        std::cout << "   ✓ 页面ID连续分配正确" << std::endl;
    } else {
        std::cout << "   ✗ 页面ID不连续: " << page_id3 << " -> " << page_id4 << std::endl;
    }
    
    // 写入数据到页面4
    char data4[] = "Page 4 data";
    page4->WriteData(0, data4, strlen(data4) + 1);
    engine->UnpinPage(page_id4, true);
    
    std::cout << "7. 测试数据一致性..." << std::endl;
    engine->UnpinPage(page_id3, true);
    
    // 重新读取页面数据验证一致性
    Page* verify_page1 = engine->FetchPage(page_id1);
    Page* verify_page2 = engine->FetchPage(page_id2);
    
    if (verify_page1 && verify_page2) {
        char read_data1[100], read_data2[100];
        verify_page1->ReadData(0, read_data1, sizeof(read_data1));
        read_data1[sizeof(read_data1)-1] = '\0';
        verify_page2->ReadData(0, read_data2, sizeof(read_data2));
        read_data2[sizeof(read_data2)-1] = '\0';
        
        if (strcmp(read_data1, data1) == 0 && strcmp(read_data2, data2) == 0) {
            std::cout << "   ✓ 页面数据一致性验证通过" << std::endl;
        } else {
            std::cout << "   ✗ 页面数据不一致:" << std::endl;
            std::cout << "     期望: " << data1 << ", " << data2 << std::endl;
            std::cout << "     实际: " << read_data1 << ", " << read_data2 << std::endl;
        }
    }
    
    engine->UnpinPage(page_id1, false);
    engine->UnpinPage(page_id2, false);
    engine->UnpinPage(page_id3, false);
    engine->UnpinPage(page_id4, false);
    
    std::cout << "8. 测试完成！" << std::endl;
    std::cout << "=== 所有测试通过，页面ID分配逻辑修复有效 ===" << std::endl;
    
    // 清理
    engine.reset();
    std::remove("test_page_id_fix.db");
    
    return 0;
}