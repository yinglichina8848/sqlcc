/**
 * @file core_interface_test.cpp
 * @brief SQLCC Core 模块接口基础验证测试
 * @author SQLCC Team
 * @date 2026-02-11
 *
 * 测试目的：
 * 验证新创建的 Core 接口（IDatabaseManager, IExecutionContext, IUserManager）
 * 是否能被正确包含、编译和使用。
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

// 引入新创建的接口
#include "src/core/interfaces/i_database_manager.h"
#include "src/core/interfaces/i_execution_context.h"
#include "src/core/interfaces/i_user_manager.h"

using namespace sqlcc::core::interfaces;

/**
 * @brief Mock DatabaseManager 实现
 * 
 * 用于验证 IDatabaseManager 接口设计的合理性
 */
class MockDatabaseManager : public IDatabaseManager {
public:
    bool Initialize() override { return true; }
    bool Close() override { return true; }
    bool IsInitialized() const override { return true; }
    
    bool CreateDatabase(const std::string& db_name) override { 
        last_call_ = "CreateDatabase(" + db_name + ")";
        return true; 
    }
    bool DropDatabase(const std::string& db_name) override { return true; }
    bool UseDatabase(const std::string& db_name) override { return true; }
    std::string GetCurrentDatabase() const override { return "test_db"; }
    std::vector<std::string> ListDatabases() override { return {}; }
    bool DatabaseExists(const std::string& db_name) const override { return false; }
    
    bool CreateTable(const std::string& table_name,
                    const std::vector<std::pair<std::string, std::string>>& columns) override { 
        return true; 
    }
    bool DropTable(const std::string& table_name) override { return true; }
    bool TableExists(const std::string& table_name) const override { return false; }
    std::vector<std::string> ListTables() override { return {}; }
    std::shared_ptr<ITableMetadata> GetTableMetadata(const std::string& table_name) override { 
        return nullptr; 
    }
    
    bool CreateIndex(const std::string& index_name,
                    const std::string& table_name,
                    const std::vector<std::string>& columns,
                    bool unique = false,
                    const std::string& condition = "") override { return true; }
    bool DropIndex(const std::string& index_name) override { return true; }
    
    TransactionId BeginTransaction(IsolationLevel isolation_level = IsolationLevel::READ_COMMITTED) override { 
        return 1; 
    }
    bool CommitTransaction(TransactionId txn_id) override { return true; }
    bool RollbackTransaction(TransactionId txn_id) override { return true; }
    
    bool Execute(const std::string& sql) override { return true; }
    std::string ExecuteQuery(const std::string& sql) override { return "[]"; }
    
    std::shared_ptr<IStorageEngine> GetStorageEngine() override { return nullptr; }
    std::shared_ptr<IIndexManager> GetIndexManager() override { return nullptr; }
    std::shared_ptr<ITransactionManager> GetTransactionManager() override { return nullptr; }
    std::shared_ptr<IConfigManager> GetConfig() override { return nullptr; }
    
    std::string last_call_;
};

/**
 * @brief Mock ExecutionContext 实现
 */
class MockExecutionContext : public IExecutionContext {
public:
    mutable std::string current_user_ = "test_user";
    mutable std::string current_database_ = "test_db";
    TransactionId transaction_id_ = 123;
    bool is_transactional_ = false;
    bool read_only_ = false;
    size_t rows_affected_ = 0;
    size_t rows_returned_ = 0;
    size_t execution_time_ms_ = 0;
    bool used_index_ = false;
    std::string execution_plan_;
    bool has_error_ = false;
    std::string error_message_;
    std::shared_ptr<IDatabaseManager> db_manager_;
    std::shared_ptr<IUserManager> user_manager_;
    
    std::string GetCurrentUser() const override { return current_user_; }
    void SetCurrentUser(const std::string& user) override { current_user_ = user; }
    std::string GetCurrentDatabase() const override { return current_database_; }
    void SetCurrentDatabase(const std::string& database) override { current_database_ = database; }
    
    bool IsTransactional() const override { return is_transactional_; }
    void SetTransactional(bool is_transactional) override { is_transactional_ = is_transactional; }
    TransactionId GetTransactionId() const override { return transaction_id_; }
    void SetTransactionId(TransactionId transaction_id) override { transaction_id_ = transaction_id; }
    bool IsReadOnly() const override { return read_only_; }
    void SetReadOnly(bool read_only) override { read_only_ = read_only; }
    
    size_t GetRowsAffected() const override { return rows_affected_; }
    void SetRowsAffected(size_t rows) override { rows_affected_ = rows; }
    void IncrementRowsAffected(size_t rows = 1) override { rows_affected_ += rows; }
    size_t GetRowsReturned() const override { return rows_returned_; }
    void SetRowsReturned(size_t rows) override { rows_returned_ = rows; }
    size_t GetExecutionTimeMs() const override { return execution_time_ms_; }
    void SetExecutionTimeMs(size_t time_ms) override { execution_time_ms_ = time_ms; }
    
    bool IsUsedIndex() const override { return used_index_; }
    void SetUsedIndex(bool used_index) override { used_index_ = used_index; }
    std::string GetExecutionPlan() const override { return execution_plan_; }
    void SetExecutionPlan(const std::string& execution_plan) override { execution_plan_ = execution_plan; }
    
    bool HasError() const override { return has_error_; }
    void SetError(bool has_error, const std::string& error_message = "") override { 
        has_error_ = has_error; 
        error_message_ = error_message;
    }
    std::string GetErrorMessage() const override { return error_message_; }
    void ClearError() override { has_error_ = false; error_message_.clear(); }
    
    std::shared_ptr<IDatabaseManager> GetDbManager() const override { return db_manager_; }
    void SetDbManager(std::shared_ptr<IDatabaseManager> db_manager) override { db_manager_ = db_manager; }
    std::shared_ptr<IUserManager> GetUserManager() const override { return user_manager_; }
    void SetUserManager(std::shared_ptr<IUserManager> user_manager) override { user_manager_ = user_manager; }
    
    void Reset() override {
        current_user_.clear();
        current_database_.clear();
        transaction_id_ = 0;
        is_transactional_ = false;
        read_only_ = false;
        rows_affected_ = 0;
        rows_returned_ = 0;
        execution_time_ms_ = 0;
        used_index_ = false;
        execution_plan_.clear();
        has_error_ = false;
        error_message_.clear();
        db_manager_.reset();
        user_manager_.reset();
    }
    std::shared_ptr<IExecutionContext> Clone() const override { return nullptr; }
    std::string ToString() const override { return "MockExecutionContext"; }
};

/**
 * @brief Mock UserManager 实现
 */
class MockUserManager : public IUserManager {
public:
    bool CreateUser(const std::string& username, const std::string& password,
                   const std::string& role = roles::kUser) override { return true; }
    bool DropUser(const std::string& username) override { return true; }
    bool AlterUserPassword(const std::string& username, const std::string& new_password) override { return true; }
    bool AlterUserRole(const std::string& username, const std::string& new_role) override { return true; }
    bool AuthenticateUser(const std::string& username, const std::string& password) override { return true; }
    bool UserExists(const std::string& username) const override { return false; }
    std::vector<UserInfo> ListUsers() const override { return {}; }
    
    bool CreateRole(const std::string& role_name) override { return true; }
    bool DropRole(const std::string& role_name) override { return true; }
    bool AlterRole(const std::string& role_name, const std::string& new_role_name) override { return true; }
    bool GrantRoleToRole(const std::string& parent_role, const std::string& child_role) override { return true; }
    bool RevokeRoleFromRole(const std::string& parent_role, const std::string& child_role) override { return true; }
    std::vector<RoleInfo> ListRoles() const override { return {}; }
    bool CheckRoleInheritance(const std::string& role_name, const std::string& inherited_role) const override { return false; }
    
    bool GrantPrivilege(const std::string& grantee, const std::string& database,
                       const std::string& table, const std::string& privilege) override { return true; }
    bool RevokePrivilege(const std::string& grantee, const std::string& database,
                        const std::string& table, const std::string& privilege) override { return true; }
    bool CheckPermission(const std::string& username, const std::string& database,
                        const std::string& table, const std::string& privilege) const override { return true; }
    std::vector<std::string> GetEffectivePermissions(const std::string& username,
                                                    const std::string& database,
                                                    const std::string& table) const override { return {}; }
    std::vector<PermissionEntry> ListUserPermissions(const std::string& username) const override { return {}; }
    std::vector<PermissionEntry> ListRolePermissions(const std::string& role_name) const override { return {}; }
    
    bool SaveToFile() const override { return true; }
    bool LoadFromFile() override { return true; }
    std::string GetLastError() const override { return ""; }
};

// ==================== 测试用例 ====================

/**
 * @test IDatabaseManager_InterfaceCompilation
 * @brief 验证 IDatabaseManager 接口可以被正确编译和使用
 */
TEST(CoreInterfaceTest, IDatabaseManager_InterfaceCompilation) {
    // 验证接口类型可以被实例化（通过 Mock）
    std::unique_ptr<IDatabaseManager> db_mgr = std::make_unique<MockDatabaseManager>();
    EXPECT_NE(db_mgr, nullptr);
    
    // 验证接口方法可以被调用
    EXPECT_TRUE(db_mgr->Initialize());
    EXPECT_TRUE(db_mgr->IsInitialized());
    EXPECT_TRUE(db_mgr->CreateDatabase("test_db"));
    EXPECT_EQ(db_mgr->GetCurrentDatabase(), "test_db");
    
    // 验证 TransactionId 类型使用正确
    TransactionId txn_id = db_mgr->BeginTransaction(IsolationLevel::READ_COMMITTED);
    EXPECT_EQ(txn_id, 1);
    EXPECT_TRUE(db_mgr->CommitTransaction(txn_id));
}

/**
 * @test IExecutionContext_InterfaceCompilation
 * @brief 验证 IExecutionContext 接口可以被正确编译和使用
 */
TEST(CoreInterfaceTest, IExecutionContext_InterfaceCompilation) {
    std::unique_ptr<IExecutionContext> ctx = std::make_unique<MockExecutionContext>();
    EXPECT_NE(ctx, nullptr);
    
    // 验证基本上下文操作
    EXPECT_EQ(ctx->GetCurrentUser(), "test_user");
    EXPECT_EQ(ctx->GetCurrentDatabase(), "test_db");
    
    // 验证事务状态
    EXPECT_FALSE(ctx->IsTransactional());
    EXPECT_EQ(ctx->GetTransactionId(), 123);  // TransactionId 类型验证
    
    // 验证执行统计
    EXPECT_EQ(ctx->GetRowsAffected(), 0);
    EXPECT_EQ(ctx->GetRowsReturned(), 0);
    EXPECT_EQ(ctx->GetExecutionTimeMs(), 0);
    
    // 验证错误处理
    EXPECT_FALSE(ctx->HasError());
    EXPECT_EQ(ctx->GetErrorMessage(), "");
    
    // 验证克隆
    auto cloned = ctx->Clone();
    // Mock 返回 nullptr，这里不验证具体内容
}

/**
 * @test IUserManager_InterfaceCompilation
 * @brief 验证 IUserManager 接口可以被正确编译和使用
 */
TEST(CoreInterfaceTest, IUserManager_InterfaceCompilation) {
    std::unique_ptr<IUserManager> user_mgr = std::make_unique<MockUserManager>();
    EXPECT_NE(user_mgr, nullptr);
    
    // 验证用户管理
    EXPECT_TRUE(user_mgr->CreateUser("test_user", "password", roles::kUser));
    EXPECT_TRUE(user_mgr->AuthenticateUser("test_user", "password"));
    EXPECT_FALSE(user_mgr->UserExists("test_user"));
    
    // 验证角色管理
    EXPECT_TRUE(user_mgr->CreateRole("admin"));
    EXPECT_TRUE(user_mgr->GrantRoleToRole("admin", "user"));
    
    // 验证权限管理
    EXPECT_TRUE(user_mgr->GrantPrivilege("test_user", "db1", "table1", privileges::kSelect));
    EXPECT_TRUE(user_mgr->CheckPermission("test_user", "db1", "table1", privileges::kSelect));
}

/**
 * @test TransactionId_TypeConsistency
 * @brief 验证 TransactionId 类型在接口间的一致性
 */
TEST(CoreInterfaceTest, TransactionId_TypeConsistency) {
    // 验证类型定义一致性
    static_assert(std::is_same<TransactionId, uint64_t>::value, 
                  "TransactionId should be uint64_t");
    
    // 验证可以在接口间传递
    std::unique_ptr<IDatabaseManager> db_mgr = std::make_unique<MockDatabaseManager>();
    std::unique_ptr<IExecutionContext> ctx = std::make_unique<MockExecutionContext>();
    
    TransactionId txn_id = db_mgr->BeginTransaction();
    EXPECT_GT(txn_id, 0);
    
    // 验证 TransactionId 可以在 IExecutionContext 中设置
    ctx->SetTransactionId(txn_id);
    EXPECT_EQ(ctx->GetTransactionId(), txn_id);
}

/**
 * @test InterfaceDependencyInjection
 * @brief 验证接口支持依赖注入模式
 */
TEST(CoreInterfaceTest, InterfaceDependencyInjection) {
    // 创建 Mock 实现
    auto mock_db = std::make_shared<MockDatabaseManager>();
    auto mock_user = std::make_shared<MockUserManager>();
    
    // 验证可以通过接口指针访问
    std::shared_ptr<IDatabaseManager> db_interface = mock_db;
    std::shared_ptr<IUserManager> user_interface = mock_user;
    
    EXPECT_NE(db_interface, nullptr);
    EXPECT_NE(user_interface, nullptr);
    
    // 验证可以注入到 ExecutionContext
    std::unique_ptr<IExecutionContext> ctx = std::make_unique<MockExecutionContext>();
    ctx->SetDbManager(db_interface);
    ctx->SetUserManager(user_interface);
    
    EXPECT_EQ(ctx->GetDbManager(), db_interface);
    EXPECT_EQ(ctx->GetUserManager(), user_interface);
}

/**
 * @test ITableMetadata_Structure
 * @brief 验证 ITableMetadata 接口结构
 */
TEST(CoreInterfaceTest, ITableMetadata_Structure) {
    // ITableMetadata 是纯接口，无法直接实例化
    // 这里验证编译时结构正确
    EXPECT_TRUE((std::is_abstract<ITableMetadata>::value));
}

// ==================== 主函数 ====================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
