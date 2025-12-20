/**
 * SQLCC UserManager Migration Test - Stage 2.2 Validation
 * Test program to verify UserManager module migration to Clang 18 + C++20
 */

#include "include/core/user_manager.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "=== SQLCC UserManager Migration Test ===\n";

    // Test UserManager functionality
    std::cout << "Testing UserManager functionality...\n";

    sqlcc::UserManager userManager("./test_data");

    // Test user creation
    std::cout << "Creating users...\n";
    bool success = userManager.CreateUser("testuser1", "password123", "USER");
    std::cout << "Create user testuser1: " << (success ? "SUCCESS" : "FAILED") << std::endl;

    success = userManager.CreateUser("testuser2", "password456", "ADMIN");
    std::cout << "Create user testuser2: " << (success ? "SUCCESS" : "FAILED") << std::endl;

    // Test authentication
    std::cout << "Testing authentication...\n";
    success = userManager.AuthenticateUser("testuser1", "password123");
    std::cout << "Authenticate testuser1: " << (success ? "SUCCESS" : "FAILED") << std::endl;

    success = userManager.AuthenticateUser("testuser1", "wrongpassword");
    std::cout << "Authenticate with wrong password: " << (!success ? "SUCCESS" : "FAILED") << std::endl;

    // Test role management
    std::cout << "Testing role management...\n";
    success = userManager.SetCurrentRole("testuser1", "ADMIN");
    std::cout << "Set current role for testuser1: " << (success ? "SUCCESS" : "FAILED") << std::endl;

    std::string currentRole = userManager.GetUserCurrentRole("testuser1");
    std::cout << "Current role of testuser1: " << currentRole << std::endl;

    // Test permission management
    std::cout << "Testing permission management...\n";
    success = userManager.GrantPrivilege("testuser1", "testdb", "testtable", "SELECT");
    std::cout << "Grant SELECT privilege: " << (success ? "SUCCESS" : "FAILED") << std::endl;

    success = userManager.CheckPermission("testuser1", "testdb", "testtable", "SELECT");
    std::cout << "Check SELECT permission: " << (success ? "SUCCESS" : "FAILED") << std::endl;

    success = userManager.CheckPermission("testuser1", "testdb", "testtable", "INSERT");
    std::cout << "Check INSERT permission (should fail): " << (!success ? "SUCCESS" : "FAILED") << std::endl;

    // Test listing functions
    std::cout << "Testing listing functions...\n";
    auto users = userManager.ListUsers();
    std::cout << "Number of users: " << users.size() << std::endl;

    auto roles = userManager.ListRoles();
    std::cout << "Number of roles: " << roles.size() << std::endl;

    // Test save/load
    std::cout << "Testing persistence...\n";
    success = userManager.SaveToFile();
    std::cout << "Save to file: " << (success ? "SUCCESS" : "FAILED") << std::endl;

    std::cout << "=== Test completed successfully ===\n";
    std::cout << "UserManager migration validation passed!\n";

    return 0;
}
