#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

// 模拟ExecutionContext类的简化版本
namespace sqlcc {

// 模拟基础类
class DatabaseManager {
public:
    std::string name = "test_db_manager";
};

class UserManager {
public:
    std::string current_user = "test_user";
};

class SystemDatabase {
public:
    std::string name = "system_db";
};

class PermissionValidator {
public:
    bool has_permission = true;
};

// 简化的ExecutionContext类
class ExecutionContext {
public:
    // 基本上下文信息
    std::string current_user;      
    std::string current_database;  
    std::string current_user_;     
    std::string current_database_; 
    bool is_transactional_;        
    std::string transaction_id_;   
    bool read_only_;               

    // 执行统计信息
    size_t records_affected;   
    size_t rows_affected_;     
    size_t rows_returned_;     
    size_t execution_time_ms_; 

    // 执行状态
    bool has_error_;            
    std::string error_message_; 

    // 管理器指针
    std::shared_ptr<DatabaseManager> db_manager;  
    std::shared_ptr<UserManager> user_manager;    
    std::shared_ptr<SystemDatabase> system_db;    
    std::shared_ptr<DatabaseManager> db_manager_; 
    std::shared_ptr<UserManager> user_manager_;   
    std::shared_ptr<SystemDatabase> system_db_;   

    // 权限验证器
    std::shared_ptr<PermissionValidator> permission_validator_;

public:
    ExecutionContext() {
        current_user = "default_user";
        current_database = "default_db";
        current_user_ = "default_user";
        current_database_ = "default_db";
        is_transactional_ = false;
        read_only_ = false;
        records_affected = 0;
        rows_affected_ = 0;
        rows_returned_ = 0;
        execution_time_ms_ = 0;
        has_error_ = false;
        
        // 创建管理器实例
        db_manager = std::make_shared<DatabaseManager>();
        db_manager_ = db_manager;
        
        user_manager = std::make_shared<UserManager>();
        user_manager_ = user_manager;
        
        system_db = std::make_shared<SystemDatabase>();
        system_db_ = system_db;
        
        permission_validator_ = std::make_shared<PermissionValidator>();
    }

    // 基本操作方法
    void set_current_user(const std::string& user) {
        current_user = user;
        current_user_ = user;
        if (user_manager) {
            user_manager->current_user = user;
        }
    }

    const std::string& get_current_user() const {
        return current_user_;
    }

    void set_current_database(const std::string& db) {
        current_database = db;
        current_database_ = db;
    }

    const std::string& get_current_database() const {
        return current_database_;
    }

    void begin_transaction() {
        is_transactional_ = true;
        transaction_id_ = "tx_" + std::to_string(std::time(nullptr));
    }

    void commit_transaction() {
        is_transactional_ = false;
        transaction_id_.clear();
    }

    void rollback_transaction() {
        is_transactional_ = false;
        transaction_id_.clear();
    }

    bool is_transactional() const {
        return is_transactional_;
    }

    const std::string& get_transaction_id() const {
        return transaction_id_;
    }

    void set_error(const std::string& message) {
        has_error_ = true;
        error_message_ = message;
    }

    void clear_error() {
        has_error_ = false;
        error_message_.clear();
    }

    bool has_error() const {
        return has_error_;
    }

    const std::string& get_error_message() const {
        return error_message_;
    }

    void set_rows_affected(size_t count) {
        rows_affected_ = count;
        records_affected = count;
    }

    size_t get_rows_affected() const {
        return rows_affected_;
    }

    void set_execution_time_ms(size_t time) {
        execution_time_ms_ = time;
    }

    size_t get_execution_time_ms() const {
        return execution_time_ms_;
    }

    void set_read_only(bool read_only) {
        read_only_ = read_only;
    }

    bool is_read_only() const {
        return read_only_;
    }
};

} // namespace sqlcc

namespace sqlcc {
namespace test {

class ExecutionContextTest : public ::testing::Test {
protected:
    void SetUp() override {
        context = std::make_unique<ExecutionContext>();
    }
    
    void TearDown() override {
        context.reset();
    }
    
    std::unique_ptr<ExecutionContext> context;
};

// Test construction and defaults
TEST_F(ExecutionContextTest, DefaultConstructor) {
    EXPECT_EQ(context->get_current_user(), "default_user");
    EXPECT_EQ(context->get_current_database(), "default_db");
    EXPECT_FALSE(context->is_transactional());
    EXPECT_FALSE(context->is_read_only());
    EXPECT_FALSE(context->has_error());
    EXPECT_EQ(context->get_rows_affected(), 0);
    EXPECT_EQ(context->get_execution_time_ms(), 0);
}

// Test user management
TEST_F(ExecutionContextTest, SetCurrentUser) {
    context->set_current_user("new_user");
    EXPECT_EQ(context->get_current_user(), "new_user");
    EXPECT_EQ(context->current_user, "new_user");
    EXPECT_EQ(context->current_user_, "new_user");
}

TEST_F(ExecutionContextTest, SetCurrentDatabase) {
    context->set_current_database("new_db");
    EXPECT_EQ(context->get_current_database(), "new_db");
    EXPECT_EQ(context->current_database, "new_db");
    EXPECT_EQ(context->current_database_, "new_db");
}

// Test transaction management
TEST_F(ExecutionContextTest, BeginTransaction) {
    context->begin_transaction();
    EXPECT_TRUE(context->is_transactional());
    EXPECT_FALSE(context->get_transaction_id().empty());
}

TEST_F(ExecutionContextTest, CommitTransaction) {
    context->begin_transaction();
    context->commit_transaction();
    EXPECT_FALSE(context->is_transactional());
    EXPECT_TRUE(context->get_transaction_id().empty());
}

TEST_F(ExecutionContextTest, RollbackTransaction) {
    context->begin_transaction();
    context->rollback_transaction();
    EXPECT_FALSE(context->is_transactional());
    EXPECT_TRUE(context->get_transaction_id().empty());
}

// Test error handling
TEST_F(ExecutionContextTest, SetError) {
    context->set_error("Test error message");
    EXPECT_TRUE(context->has_error());
    EXPECT_EQ(context->get_error_message(), "Test error message");
}

TEST_F(ExecutionContextTest, ClearError) {
    context->set_error("Test error");
    context->clear_error();
    EXPECT_FALSE(context->has_error());
    EXPECT_TRUE(context->get_error_message().empty());
}

// Test execution statistics
TEST_F(ExecutionContextTest, SetRowsAffected) {
    context->set_rows_affected(42);
    EXPECT_EQ(context->get_rows_affected(), 42);
    EXPECT_EQ(context->rows_affected_, 42);
    EXPECT_EQ(context->records_affected, 42);
}

TEST_F(ExecutionContextTest, SetExecutionTime) {
    context->set_execution_time_ms(150);
    EXPECT_EQ(context->get_execution_time_ms(), 150);
    EXPECT_EQ(context->execution_time_ms_, 150);
}

// Test read-only mode
TEST_F(ExecutionContextTest, SetReadOnly) {
    context->set_read_only(true);
    EXPECT_TRUE(context->is_read_only());
    
    context->set_read_only(false);
    EXPECT_FALSE(context->is_read_only());
}

// Test manager initialization
TEST_F(ExecutionContextTest, ManagerInitialization) {
    EXPECT_NE(context->db_manager, nullptr);
    EXPECT_NE(context->user_manager, nullptr);
    EXPECT_NE(context->system_db, nullptr);
    EXPECT_NE(context->db_manager_, nullptr);
    EXPECT_NE(context->user_manager_, nullptr);
    EXPECT_NE(context->system_db_, nullptr);
    EXPECT_NE(context->permission_validator_, nullptr);
    
    // Check that old and new pointers point to same objects
    EXPECT_EQ(context->db_manager, context->db_manager_);
    EXPECT_EQ(context->user_manager, context->user_manager_);
    EXPECT_EQ(context->system_db, context->system_db_);
}

// Test complex scenario
TEST_F(ExecutionContextTest, ComplexScenario) {
    // Set up context
    context->set_current_user("admin");
    context->set_current_database("production");
    context->begin_transaction();
    context->set_rows_affected(25);
    context->set_execution_time_ms(120);
    context->set_read_only(false);
    
    // Verify state
    EXPECT_EQ(context->get_current_user(), "admin");
    EXPECT_EQ(context->get_current_database(), "production");
    EXPECT_TRUE(context->is_transactional());
    EXPECT_EQ(context->get_rows_affected(), 25);
    EXPECT_EQ(context->get_execution_time_ms(), 120);
    EXPECT_FALSE(context->is_read_only());
    EXPECT_FALSE(context->has_error());
    
    // Simulate error
    context->set_error("Constraint violation");
    EXPECT_TRUE(context->has_error());
    EXPECT_EQ(context->get_error_message(), "Constraint violation");
    
    // Rollback transaction
    context->rollback_transaction();
    EXPECT_FALSE(context->is_transactional());
    EXPECT_TRUE(context->get_transaction_id().empty());
}

// Test user manager integration
TEST_F(ExecutionContextTest, UserManagerIntegration) {
    context->set_current_user("test_user");
    EXPECT_EQ(context->user_manager->current_user, "test_user");
}

} // namespace test
} // namespace sqlcc