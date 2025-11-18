#include <gtest/gtest.h>
#include <cstdlib>
#include "storage_engine.h"
#include "config_manager.h"

namespace sqlcc {

class StorageEngineNewPageTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_db = "test_newpage.db";
        config_manager = &ConfigManager::GetInstance();
        // Set the database file path in config manager
        config_manager->SetValue("database.db_file_path", test_db);
        engine = new StorageEngine(*config_manager);
    }

    void TearDown() override {
        delete engine;
        std::remove(test_db.c_str());
    }

    std::string test_db;
    ConfigManager* config_manager;
    StorageEngine* engine;
};

TEST_F(StorageEngineNewPageTest, NewPageBasic) {
    int32_t page_id;
    
    // Test basic NewPage functionality
    Page* page = engine->NewPage(&page_id);
    EXPECT_NE(page, nullptr);
    EXPECT_GE(page_id, 0);
    
    // Clean up
    engine->UnpinPage(page_id, false);
    EXPECT_TRUE(engine->DeletePage(page_id));
}

TEST_F(StorageEngineNewPageTest, NewPageSequential) {
    // Test sequential page ID allocation
    ConfigManager& config_manager = ConfigManager::GetInstance();
    StorageEngine test_engine(config_manager);
    config_manager.SetValue("database.db_file_path", "test_sequential.db");
    
    int32_t page_id1, page_id2, page_id3;
    
    // Create multiple pages sequentially
    Page* page1 = test_engine.NewPage(&page_id1);
    EXPECT_NE(page1, nullptr);
    EXPECT_GE(page_id1, 0);
    
    Page* page2 = test_engine.NewPage(&page_id2);
    EXPECT_NE(page2, nullptr);
    EXPECT_GE(page_id2, 0);
    
    Page* page3 = test_engine.NewPage(&page_id3);
    EXPECT_NE(page3, nullptr);
    EXPECT_GE(page_id3, 0);
    
    // Verify that page IDs are sequential (may not be exactly consecutive due to page reuse)
    EXPECT_LE(page_id1, page_id2);
    EXPECT_LE(page_id2, page_id3);
    
    // Clean up
    test_engine.UnpinPage(page_id1, false);
    test_engine.UnpinPage(page_id2, false);
    test_engine.UnpinPage(page_id3, false);
    EXPECT_TRUE(test_engine.DeletePage(page_id1));
    EXPECT_TRUE(test_engine.DeletePage(page_id2));
    EXPECT_TRUE(test_engine.DeletePage(page_id3));
    
    // Clean up test file
    std::remove("test_sequential.db");
}

} // namespace sqlcc
