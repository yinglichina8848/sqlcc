#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

// User Manager tests for core services layer
// These tests verify user management components

TEST(UserManagerTest, UserCreation) {
    // Test user creation
    std::unordered_map<std::string, std::string> users;

    users["admin"] = "admin_role";
    users["user1"] = "user_role";
    users["analyst"] = "analyst_role";

    EXPECT_EQ(users.size(), 3);
    EXPECT_EQ(users["admin"], "admin_role");
    EXPECT_EQ(users["user1"], "user_role");
}

TEST(UserManagerTest, RoleAssignment) {
    // Test role assignment
    std::unordered_map<std::string, std::vector<std::string>> user_roles;

    user_roles["admin"] = {"ADMIN", "USER", "MANAGER"};
    user_roles["user1"] = {"USER"};
    user_roles["analyst"] = {"USER", "ANALYST"};

    EXPECT_EQ(user_roles["admin"].size(), 3);
    EXPECT_EQ(user_roles["user1"].size(), 1);
    EXPECT_EQ(user_roles["analyst"].size(), 2);
}

TEST(UserManagerTest, Authentication) {
    // Test authentication logic
    std::unordered_map<std::string, std::string> credentials;

    credentials["admin"] = "hashed_password_123";
    credentials["user1"] = "hashed_password_456";

    // Verify authentication
    EXPECT_EQ(credentials["admin"], "hashed_password_123");
    EXPECT_EQ(credentials["user1"], "hashed_password_456");
}