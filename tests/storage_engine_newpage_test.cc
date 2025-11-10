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

TEST_F(StorageEngineNewPageTest, NewPageFailure) {
    int32_t page_id;
    
    // Set the environment variable to force NewPage to fail
    setenv("SQLCC_TEST_NEWPAGE_FAIL", "1", 1);
    
    // NewPage should return nullptr
    Page* page = engine->NewPage(&page_id);
    EXPECT_EQ(page, nullptr);
    
    // Unset the environment variable
    unsetenv("SQLCC_TEST_NEWPAGE_FAIL");
    
    // NewPage should succeed now
    page = engine->NewPage(&page_id);
    EXPECT_NE(page, nullptr);
    
    // Clean up - only delete the page if it was successfully created
    if (page != nullptr) {
        // Unpin the page first to decrease ref count
        engine->UnpinPage(page_id, false);
        // Now delete the page
        EXPECT_TRUE(engine->DeletePage(page_id));
    }
}

TEST_F(StorageEngineNewPageTest, NewPageFailureFromBufferPool) {
    int32_t page_id;
    
    // Set the environment variable to simulate BufferPool returning null
    setenv("SQLCC_TEST_BUFFERPOOL_NULL", "1", 1);
    
    // NewPage should return nullptr
    Page* page = engine->NewPage(&page_id);
    EXPECT_EQ(page, nullptr);
    
    // Unset the environment variable
    unsetenv("SQLCC_TEST_BUFFERPOOL_NULL");
    
    // NewPage should succeed now
    page = engine->NewPage(&page_id);
    EXPECT_NE(page, nullptr);
    
    // Clean up
    engine->UnpinPage(page_id, false);
    EXPECT_TRUE(engine->DeletePage(page_id));
}

} // namespace sqlcc
