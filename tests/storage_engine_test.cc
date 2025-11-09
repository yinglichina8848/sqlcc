#include <gtest/gtest.h>
#include "storage_engine.h"
#include "config_manager.h"
#include "exception.h"
#include <filesystem>
#include <cstdio>

// 使用sqlcc命名空间中的类
using sqlcc::StorageEngine;
using sqlcc::ConfigManager;
using sqlcc::Page;
using sqlcc::ConfigValue;

class StorageEngineTest : public ::testing::Test {
protected:
    const std::string TEST_DB_FILE = "test_db.db";
    ConfigManager* config_manager;
    
    void SetUp() override {
        // 删除测试文件（如果存在）
        std::filesystem::remove(TEST_DB_FILE);
        
        // 创建配置管理器
        config_manager = &ConfigManager::GetInstance();
        
        // 设置测试数据库文件路径
        config_manager->SetValue("database.db_file_path", ConfigValue(TEST_DB_FILE));
    }
    
    void TearDown() override {
        // 清理测试文件
        std::filesystem::remove(TEST_DB_FILE);
    }
};

// 测试存储引擎初始化
TEST_F(StorageEngineTest, InitializeStorageEngine) {
    EXPECT_NO_THROW({
        StorageEngine engine(*config_manager);
    });
    
    // 检查数据库文件是否创建
    EXPECT_TRUE(std::filesystem::exists(TEST_DB_FILE));
}

// 测试创建新页面
TEST_F(StorageEngineTest, CreateNewPage) {
    StorageEngine engine(*config_manager);
    int32_t page_id;
    
    Page* page = engine.NewPage(&page_id);
    
    EXPECT_NE(page, nullptr);
    EXPECT_GE(page_id, 0);
    EXPECT_EQ(page->GetPageId(), page_id);
}

// 测试获取页面
TEST_F(StorageEngineTest, FetchPage) {
    StorageEngine engine(*config_manager);
    int32_t page_id;
    
    // 创建新页面
    Page* page1 = engine.NewPage(&page_id);
    ASSERT_NE(page1, nullptr);
    
    // 取消固定页面
    EXPECT_TRUE(engine.UnpinPage(page_id, false));
    
    // 获取页面
    Page* page2 = engine.FetchPage(page_id);
    EXPECT_NE(page2, nullptr);
    EXPECT_EQ(page2->GetPageId(), page_id);
}

// 测试页面刷新
TEST_F(StorageEngineTest, FlushPage) {
    StorageEngine engine(*config_manager);
    int32_t page_id;
    
    // 创建新页面
    Page* page = engine.NewPage(&page_id);
    ASSERT_NE(page, nullptr);
    
    // 修改页面数据
    char test_data[] = "Test data for flushing";
    page->WriteData(0, test_data, sizeof(test_data));
    
    // 取消固定页面并标记为脏页
    EXPECT_TRUE(engine.UnpinPage(page_id, true));
    
    // 刷新页面
    EXPECT_TRUE(engine.FlushPage(page_id));
}

// 测试删除页面
TEST_F(StorageEngineTest, DeletePage) {
    StorageEngine engine(*config_manager);
    int32_t page_id;
    
    // 创建新页面
    Page* page = engine.NewPage(&page_id);
    ASSERT_NE(page, nullptr);
    
    // 取消固定页面并标记为脏页，确保页面被刷新到磁盘
    EXPECT_TRUE(engine.UnpinPage(page_id, true));
    
    // 删除页面
    EXPECT_TRUE(engine.DeletePage(page_id));
    
    // 尝试获取已删除的页面应该失败
    Page* fetched_page = engine.FetchPage(page_id);
    EXPECT_EQ(fetched_page, nullptr);
}

// 测试刷新所有页面
TEST_F(StorageEngineTest, FlushAllPages) {
    StorageEngine engine(*config_manager);
    const int NUM_PAGES = 5;
    int32_t page_ids[NUM_PAGES];
    
    // 创建多个页面
    for (int i = 0; i < NUM_PAGES; ++i) {
        Page* page = engine.NewPage(&page_ids[i]);
        ASSERT_NE(page, nullptr);
        
        // 修改页面数据
        char test_data[100];
        snprintf(test_data, sizeof(test_data), "Test data for page %d", i);
        page->WriteData(0, test_data, strlen(test_data) + 1);
        
        // 取消固定页面并标记为脏页
        EXPECT_TRUE(engine.UnpinPage(page_ids[i], true));
    }
    
    // 刷新所有页面
    EXPECT_NO_THROW({
        engine.FlushAllPages();
    });
}

// 测试大量页面操作
TEST_F(StorageEngineTest, ManyPagesOperation) {
    StorageEngine engine(*config_manager);
    const int NUM_PAGES = 100;
    std::vector<int32_t> page_ids;
    
    // 创建大量页面
    for (int i = 0; i < NUM_PAGES; ++i) {
        int32_t page_id;
        Page* page = engine.NewPage(&page_id);
        ASSERT_NE(page, nullptr);
        page_ids.push_back(page_id);
        
        // 写入一些数据
        page->WriteData(0, reinterpret_cast<const char*>(&i), sizeof(i));
        
        // 取消固定页面
        EXPECT_TRUE(engine.UnpinPage(page_id, i % 2 == 0)); // 偶数页标记为脏页
    }
    
    // 随机获取一些页面并验证数据
    for (int i = 0; i < 10; ++i) {
        int idx = rand() % NUM_PAGES;
        int32_t page_id = page_ids[idx];
        
        Page* page = engine.FetchPage(page_id);
        ASSERT_NE(page, nullptr);
        
        int stored_value;
        page->ReadData(0, reinterpret_cast<char*>(&stored_value), sizeof(stored_value));
        EXPECT_EQ(stored_value, idx);
        
        // 取消固定页面
        EXPECT_TRUE(engine.UnpinPage(page_id, false));
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}