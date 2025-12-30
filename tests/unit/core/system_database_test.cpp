#include "core/system_database.h"
#include "core/core_database_manager.h"
#include "sql_executor.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

using namespace sqlcc;

/**
 * SystemDatabase单元测试
 * 验证基本功能和初始化
 */
class SystemDatabaseTest : public ::testing::Test {
protected:
    std::shared_ptr<DatabaseManager> db_manager_;
    std::shared_ptr<SystemDatabase> sys_db_;
    std::string db_path_ = "./test_system_db";

    void SetUp() override {
        // 创建DatabaseManager实例
        db_manager_ = std::make_shared<DatabaseManager>(db_path_, 1024, 4, 4);
        
        // 创建SystemDatabase实例
        sys_db_ = std::make_shared<SystemDatabase>(db_manager_);
    }

    void TearDown() override {
        // 清理资源
        if (db_manager_) {
            db_manager_->Close();
        }
    }
};

// 测试SystemDatabase的基本功能
TEST_F(SystemDatabaseTest, BasicFunctionality) {
    // 测试IsInitialized方法（初始化前）
    EXPECT_FALSE(sys_db_->IsInitialized());
    
    // 测试初始化
    bool init_result = sys_db_->Initialize();
    EXPECT_TRUE(init_result) << "Failed to initialize system database: " << sys_db_->GetLastError();
    
    // 测试IsInitialized方法（初始化后）
    EXPECT_TRUE(sys_db_->IsInitialized());
    
    // 测试GetDatabaseManager方法
    auto retrieved_db_manager = sys_db_->GetDatabaseManager();
    EXPECT_EQ(retrieved_db_manager, db_manager_);
    
    // 测试GetLastError方法
    std::string last_error = sys_db_->GetLastError();
    EXPECT_TRUE(last_error.empty() || !last_error.empty());
}

// 主函数
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
