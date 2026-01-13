/**
 * @file dcl_permission_model_advanced_test.cpp
 *
 * DCL权限模型高级功能测试
 * 测试角色继承、级联权限撤销、权限冲突检测、权限审计等高级功能
 *
 * 测试覆盖范围：
 * 1. 角色继承关系管理
 * 2. 级联权限撤销功能
 * 3. 权限冲突检测机制
 * 4. 权限审计记录功能
 * 5. 有效权限计算逻辑
 */

#include <gtest/gtest.h>
#include <memory>
#include "include/core/user_manager.h"

// 测试夹具类
class DCLPermissionModelAdvancedTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建UserManager实例
        user_manager_ = std::make_unique<sqlcc::UserManager>("./test_data");
    }

    void TearDown() override {
        // 清理测试数据
        user_manager_.reset();
    }

    std::unique_ptr<sqlcc::UserManager> user_manager_;
};

/**
 * 测试用例1：角色继承关系管理
 * 测试GrantRoleToRole和RevokeRoleFromRole功能
 */
TEST_F(DCLPermissionModelAdvancedTest, RoleInheritanceManagement) {
    // 创建角色
    ASSERT_TRUE(user_manager_->CreateRole("parent_role"));
    ASSERT_TRUE(user_manager_->CreateRole("child_role"));
    ASSERT_TRUE(user_manager_->CreateRole("grandchild_role"));

    // 测试角色继承建立
    ASSERT_TRUE(user_manager_->GrantRoleToRole("parent_role", "child_role"));
    ASSERT_TRUE(user_manager_->GrantRoleToRole("child_role", "grandchild_role"));

    // 验证角色继承关系
    ASSERT_TRUE(user_manager_->CheckRoleInheritance("parent_role", "child_role"));
    ASSERT_TRUE(user_manager_->CheckRoleInheritance("parent_role", "grandchild_role"));
    ASSERT_TRUE(user_manager_->CheckRoleInheritance("child_role", "grandchild_role"));

    // 验证不存在的继承关系
    ASSERT_FALSE(user_manager_->CheckRoleInheritance("child_role", "parent_role"));

    // 获取角色层次结构
    auto hierarchy = user_manager_->GetRoleHierarchy("parent_role");
    ASSERT_EQ(hierarchy.size(), 2);
    ASSERT_EQ(hierarchy[0], "child_role");
    ASSERT_EQ(hierarchy[1], "grandchild_role");

    // 测试角色继承撤销
    ASSERT_TRUE(user_manager_->RevokeRoleFromRole("child_role", "grandchild_role"));
    ASSERT_FALSE(user_manager_->CheckRoleInheritance("parent_role", "grandchild_role"));
    ASSERT_TRUE(user_manager_->CheckRoleInheritance("parent_role", "child_role"));
}

/**
 * 测试用例2：级联权限撤销
 * 测试RevokePrivilegeCascade功能
 */
TEST_F(DCLPermissionModelAdvancedTest, CascadePrivilegeRevocation) {
    // 创建用户和角色
    ASSERT_TRUE(user_manager_->CreateUser("test_user", "password", "user_role"));
    ASSERT_TRUE(user_manager_->CreateRole("parent_role"));
    ASSERT_TRUE(user_manager_->CreateRole("child_role"));

    // 建立角色继承
    ASSERT_TRUE(user_manager_->GrantRoleToRole("parent_role", "child_role"));
    ASSERT_TRUE(user_manager_->AlterUserRole("test_user", "child_role"));

    // 授予权限
    ASSERT_TRUE(user_manager_->GrantPrivilege("parent_role", "testdb", "testtable", "SELECT"));
    ASSERT_TRUE(user_manager_->GrantPrivilege("child_role", "testdb", "testtable", "INSERT"));

    // 验证权限存在
    ASSERT_TRUE(user_manager_->CheckPermission("test_user", "testdb", "testtable", "SELECT"));
    ASSERT_TRUE(user_manager_->CheckPermission("test_user", "testdb", "testtable", "INSERT"));

    // 级联撤销父角色权限
    ASSERT_TRUE(user_manager_->RevokePrivilegeCascade("parent_role", "testdb", "testtable", "SELECT"));

    // 验证子角色权限仍然存在
    ASSERT_FALSE(user_manager_->CheckPermission("test_user", "testdb", "testtable", "SELECT"));
    ASSERT_TRUE(user_manager_->CheckPermission("test_user", "testdb", "testtable", "INSERT"));
}

/**
 * 测试用例3：权限冲突检测
 * 测试CheckPermissionConflict功能
 */
TEST_F(DCLPermissionModelAdvancedTest, PermissionConflictDetection) {
    // 创建用户
    ASSERT_TRUE(user_manager_->CreateUser("test_user", "password", "user_role"));

    // 首次授予权限应该成功
    ASSERT_TRUE(user_manager_->GrantPrivilege("test_user", "testdb", "testtable", "SELECT"));
    ASSERT_FALSE(user_manager_->CheckPermissionConflict("test_user", "testdb", "testtable", "SELECT"));

    // 再次授予相同权限应该检测到冲突
    ASSERT_TRUE(user_manager_->CheckPermissionConflict("test_user", "testdb", "testtable", "SELECT"));

    // 不同权限不冲突
    ASSERT_FALSE(user_manager_->CheckPermissionConflict("test_user", "testdb", "testtable", "INSERT"));
}

/**
 * 测试用例4：权限审计功能
 * 测试AuditPermissionChanges功能
 */
TEST_F(DCLPermissionModelAdvancedTest, PermissionAudit) {
    // 创建用户
    ASSERT_TRUE(user_manager_->CreateUser("test_user", "password", "user_role"));

    // 测试权限授予审计
    ASSERT_TRUE(user_manager_->AuditPermissionChanges("GRANT", "test_user", "SELECT on testdb.testtable"));

    // 测试权限撤销审计
    ASSERT_TRUE(user_manager_->AuditPermissionChanges("REVOKE", "test_user", "INSERT on testdb.testtable"));

    // 测试角色变更审计
    ASSERT_TRUE(user_manager_->AuditPermissionChanges("ALTER_ROLE", "test_user", "changed from user_role to admin_role"));
}

/**
 * 测试用例5：有效权限计算
 * 测试GetEffectivePermissions功能
 */
TEST_F(DCLPermissionModelAdvancedTest, EffectivePermissionsCalculation) {
    // 创建用户和角色
    ASSERT_TRUE(user_manager_->CreateUser("test_user", "password", "user_role"));
    ASSERT_TRUE(user_manager_->CreateRole("parent_role"));
    ASSERT_TRUE(user_manager_->CreateRole("child_role"));

    // 建立角色继承
    ASSERT_TRUE(user_manager_->GrantRoleToRole("parent_role", "child_role"));
    ASSERT_TRUE(user_manager_->AlterUserRole("test_user", "child_role"));

    // 授予不同层次的权限
    ASSERT_TRUE(user_manager_->GrantPrivilege("test_user", "testdb", "testtable", "SELECT"));
    ASSERT_TRUE(user_manager_->GrantPrivilege("child_role", "testdb", "testtable", "INSERT"));
    ASSERT_TRUE(user_manager_->GrantPrivilege("parent_role", "testdb", "testtable", "UPDATE"));

    // 获取有效权限
    auto effective_permissions = user_manager_->GetEffectivePermissions("test_user", "testdb", "testtable");

    // 验证包含所有权限
    ASSERT_EQ(effective_permissions.size(), 3);
    ASSERT_NE(std::find(effective_permissions.begin(), effective_permissions.end(), "SELECT"), effective_permissions.end());
    ASSERT_NE(std::find(effective_permissions.begin(), effective_permissions.end(), "INSERT"), effective_permissions.end());
    ASSERT_NE(std::find(effective_permissions.begin(), effective_permissions.end(), "UPDATE"), effective_permissions.end());
}

/**
 * 测试用例6：复杂角色继承场景
 * 测试多层继承和权限传递
 */
TEST_F(DCLPermissionModelAdvancedTest, ComplexRoleInheritance) {
    // 创建多层角色层次结构
    ASSERT_TRUE(user_manager_->CreateRole("ceo_role"));
    ASSERT_TRUE(user_manager_->CreateRole("manager_role"));
    ASSERT_TRUE(user_manager_->CreateRole("employee_role"));
    ASSERT_TRUE(user_manager_->CreateRole("intern_role"));

    // 建立多层继承关系
    ASSERT_TRUE(user_manager_->GrantRoleToRole("ceo_role", "manager_role"));
    ASSERT_TRUE(user_manager_->GrantRoleToRole("manager_role", "employee_role"));
    ASSERT_TRUE(user_manager_->GrantRoleToRole("employee_role", "intern_role"));

    // 创建用户并分配角色
    ASSERT_TRUE(user_manager_->CreateUser("ceo", "password", "ceo_role"));
    ASSERT_TRUE(user_manager_->CreateUser("manager", "password", "manager_role"));
    ASSERT_TRUE(user_manager_->CreateUser("employee", "password", "employee_role"));
    ASSERT_TRUE(user_manager_->CreateUser("intern", "password", "intern_role"));

    // CEO授予高级权限
    ASSERT_TRUE(user_manager_->GrantPrivilege("ceo_role", "companydb", "salary", "SELECT"));
    ASSERT_TRUE(user_manager_->GrantPrivilege("ceo_role", "companydb", "salary", "UPDATE"));

    // Manager授予中级权限
    ASSERT_TRUE(user_manager_->GrantPrivilege("manager_role", "companydb", "projects", "SELECT"));
    ASSERT_TRUE(user_manager_->GrantPrivilege("manager_role", "companydb", "projects", "INSERT"));

    // Employee授予基本权限
    ASSERT_TRUE(user_manager_->GrantPrivilege("employee_role", "companydb", "tasks", "SELECT"));

    // 验证权限继承
    // CEO应该拥有所有权限
    auto ceo_permissions = user_manager_->GetEffectivePermissions("ceo", "companydb", "salary");
    ASSERT_EQ(ceo_permissions.size(), 2); // SELECT, UPDATE

    // Manager应该拥有上级权限
    auto manager_permissions = user_manager_->GetEffectivePermissions("manager", "companydb", "salary");
    ASSERT_EQ(manager_permissions.size(), 2); // 继承CEO的权限

    // Employee应该拥有上级权限
    auto employee_permissions = user_manager_->GetEffectivePermissions("employee", "companydb", "salary");
    ASSERT_EQ(employee_permissions.size(), 2); // 继承CEO的权限

    // Intern应该拥有所有上级权限
    auto intern_permissions = user_manager_->GetEffectivePermissions("intern", "companydb", "salary");
    ASSERT_EQ(intern_permissions.size(), 2); // 继承CEO的权限
}

/**
 * 测试用例7：权限边界情况测试
 * 测试各种边界条件和异常情况
 */
TEST_F(DCLPermissionModelAdvancedTest, PermissionBoundaryConditions) {
    // 测试不存在的用户
    auto permissions = user_manager_->GetEffectivePermissions("nonexistent_user", "testdb", "testtable");
    ASSERT_TRUE(permissions.empty());

    // 测试不存在的角色
    auto hierarchy = user_manager_->GetRoleHierarchy("nonexistent_role");
    ASSERT_TRUE(hierarchy.empty());

    // 测试权限冲突检测边界情况
    ASSERT_FALSE(user_manager_->CheckPermissionConflict("nonexistent_user", "testdb", "testtable", "SELECT"));

    // 测试角色继承检查边界情况
    ASSERT_FALSE(user_manager_->CheckRoleInheritance("nonexistent_role", "another_role"));
}

/**
 * 测试用例8：简单功能验证
 * 验证基本的权限管理功能正常工作
 */
TEST_F(DCLPermissionModelAdvancedTest, BasicFunctionalityTest) {
    // 创建基本角色
    ASSERT_TRUE(user_manager_->CreateRole("basic_role"));
    ASSERT_TRUE(user_manager_->CreateUser("basic_user", "password", "basic_role"));

    // 授予权限
    ASSERT_TRUE(user_manager_->GrantPrivilege("basic_user", "testdb", "testtable", "SELECT"));
    ASSERT_TRUE(user_manager_->GrantPrivilege("basic_role", "testdb", "testtable", "INSERT"));

    // 验证权限检查
    ASSERT_TRUE(user_manager_->CheckPermission("basic_user", "testdb", "testtable", "SELECT"));
    ASSERT_TRUE(user_manager_->CheckPermission("basic_user", "testdb", "testtable", "INSERT"));

    // 验证权限列表
    auto user_permissions = user_manager_->ListUserPermissions("basic_user");
    ASSERT_EQ(user_permissions.size(), 1);
    ASSERT_EQ(user_permissions[0].privilege, "SELECT");

    auto role_permissions = user_manager_->ListRolePermissions("basic_role");
    ASSERT_EQ(role_permissions.size(), 1);
    ASSERT_EQ(role_permissions[0].privilege, "INSERT");

    // 验证有效权限计算
    auto effective_permissions = user_manager_->GetEffectivePermissions("basic_user", "testdb", "testtable");
    ASSERT_EQ(effective_permissions.size(), 2);
    ASSERT_NE(std::find(effective_permissions.begin(), effective_permissions.end(), "SELECT"), effective_permissions.end());
    ASSERT_NE(std::find(effective_permissions.begin(), effective_permissions.end(), "INSERT"), effective_permissions.end());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
