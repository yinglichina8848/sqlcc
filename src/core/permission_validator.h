/**
 * @file permission_validator.h
 * @brief Defines the core components for a decoupled and extensible permission validation framework.
 *
 * @WHY
 * In a database, authorization is a critical cross-cutting concern. A monolithic permission system, tightly
 * coupled with business logic (e.g., checking 'SELECT' permission inside the 'SELECT' execution code),
 * is hard to maintain, test, and extend. This leads to scattered permission checks and inconsistent rules.
 *
 * This framework was designed to solve these problems by:
 * 1.  **Decoupling**: Separating the "what" (the permission check request) from the "how" (the implementation
 *     of the check). The core validator doesn't know the specifics of every resource, making it universally applicable.
 * 2.  **Centralization**: Providing a single point (`PermissionValidator::validate`) for all permission checks,
 *     ensuring consistency.
 * 3.  **Extensibility**: Using a callback mechanism (`PermissionCheckCallback`) that allows higher-level modules
 *     (like the `SQLExecutor`) to inject business-specific validation logic without modifying the core validator.
 *
 * @WHAT
 * This file defines:
 * 1.  **`PermissionOperation`**: An enum representing the abstract actions a user can perform (e.g., `SELECT`, `CREATE_TABLE`).
 * 2.  **`PermissionContext`**: A struct that carries all necessary information for a permission check (who, what, where).
 * 3.  **`PermissionResult`**: A struct to return a clear "allow/deny" result with an explanatory message.
 * 4.  **`PermissionValidator`**: The central class. It performs basic, universal checks (e.g., is the user an admin?)
 *     and then delegates to a registered callback for more complex, context-specific validation. It holds a
 *     reference to the `UserManager` to get information about users and roles.
 *
 * @HOW
 * 1.  A central `PermissionValidator` instance is created and initialized with a `UserManager`.
 * 2.  A higher-level component (e.g., the main server) registers a callback function using `setPermissionCheckCallback`. This callback contains the detailed business logic for checking permissions against metadata tables.
 * 3.  When any part of the system needs to check a permission, it calls `validator.validate(...)`, passing an operation and context.
 * 4.  The `validate` method first performs basic checks. If those pass, it invokes the registered callback with the `PermissionContext`.
 * 5.  The callback executes the detailed check and returns a `PermissionResult`.
 * 6.  The `validator` returns this result to the caller, which then enforces the decision.
 *
 * This design cleanly separates the generic validation framework from the domain-specific rules.
 */

#ifndef SQLCC_PERMISSION_VALIDATOR_H
#define SQLCC_PERMISSION_VALIDATOR_H

#include "user_manager.h"
#include "../error_handler.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

namespace sqlcc {

/**
 * @brief Represents a generic, abstract operation that requires a permission check.
 * These operations are decoupled from specific SQL statements.
 */
enum class PermissionOperation {
    CREATE_DATABASE,
    DROP_DATABASE,
    CREATE_TABLE,
    DROP_TABLE,
    ALTER_TABLE,
    SELECT,
    INSERT,
    UPDATE,
    DELETE,
    CREATE_USER,
    DROP_USER,
    GRANT,
    REVOKE,
    USE_DATABASE,
    SHOW_DATABASES,
    SHOW_TABLES
};

/**
 * @brief Encapsulates the result of a permission validation check.
 */
struct PermissionResult {
    bool allowed;               ///< True if the operation is permitted, false otherwise.
    std::string message;        ///< A human-readable message explaining the result.
    ErrorInfo error_info;       ///< Detailed error information if the operation is denied.

    PermissionResult(bool allowed, const std::string& msg = "", const ErrorInfo& error = ErrorInfo(ErrorCode::SUCCESS, ErrorLevel::INFO, "", "", "PERMISSION"))
        : allowed(allowed), message(msg), error_info(error) {}

    /**
     * @brief Factory method for a successful permission result.
     */
    static PermissionResult createAllowed() {
        return PermissionResult(true, "Permission granted");
    }

    /**
     * @brief Factory method for a denied permission result with a simple reason.
     */
    static PermissionResult createDenied(const std::string& reason) {
        return PermissionResult(false, reason);
    }

    /**
     * @brief Factory method for a denied permission result with detailed error info.
     */
    static PermissionResult createDeniedWithError(const ErrorInfo& error) {
        return PermissionResult(false, error.message, error);
    }
};

/**
 * @brief Provides the context for a permission check.
 * This struct is a data transfer object (DTO) that carries all necessary information
 * from the point of request to the point of validation.
 */
struct PermissionContext {
    std::string user;                   ///< The name of the user performing the operation.
    std::string database;               ///< The database context for the operation.
    std::string resource;               ///< The target resource (e.g., table name, username).
    PermissionOperation operation;      ///< The operation being performed.

    PermissionContext(const std::string& u = "", const std::string& db = "", const std::string& res = "", PermissionOperation op = PermissionOperation::SELECT)
        : user(u), database(db), resource(res), operation(op) {}
};

/**
 * @brief A function signature for a callback that performs detailed permission checks.
 * This allows higher-level modules to inject domain-specific validation logic.
 */
using PermissionCheckCallback = std::function<PermissionResult(const PermissionContext&)>;

/**
 * @brief A centralized, decoupled engine for handling all authorization checks.
 *
 * @details This class is designed to be independent of any specific business logic. It provides a
 * unified `validate` entry point and uses a callback mechanism to delegate complex,
 * context-dependent checks to the appropriate module, thus promoting separation of concerns.
 */
class PermissionValidator {
public:
    /**
     * @brief Constructs the validator.
     * @param user_manager A shared pointer to a UserManager to retrieve user and role information.
     */
    PermissionValidator(std::shared_ptr<UserManager> user_manager);

    ~PermissionValidator() = default;

    /**
     * @brief Registers the callback function for detailed, business-logic-specific permission checks.
     * @param callback The function to be called by `validate`.
     */
    void setPermissionCheckCallback(PermissionCheckCallback callback);

    /**
     * @brief The main entry point for all permission checks.
     * @param operation The operation being validated.
     * @param resource The target resource (e.g., table name).
     * @param current_user The user performing the operation.
     * @param current_database The active database.
     * @return A PermissionResult indicating if the operation is allowed or denied.
     */
    PermissionResult validate(PermissionOperation operation,
                             const std::string& resource = "",
                             const std::string& current_user = "",
                             const std::string& current_database = "");

    /**
     * @brief Sets a default user to be used if no user is specified in the `validate` call.
     * @param user The default username.
     */
    void setDefaultUser(const std::string& user);

    /**
     * @brief Sets a default database to be used if no database is specified in the `validate` call.
     * @param database The default database name.
     */
    void setDefaultDatabase(const std::string& database);

    /**
     * @brief Maps a PermissionOperation enum to the corresponding privilege string (e.g., "SELECT", "INSERT").
     * @param operation The enum operation.
     * @return The string representation of the privilege.
     */
    static std::string operationToPrivilege(PermissionOperation operation);

    /**
     * @brief Maps a PermissionOperation to the type of resource it acts upon (e.g., "DATABASE", "TABLE").
     * @param operation The enum operation.
     * @return The string representation of the resource type.
     */
    static std::string operationToResourceType(PermissionOperation operation);

    /**
     * @brief Checks if a user exists.
     * @param username The name of the user.
     * @return True if the user exists.
     */
    bool userExists(const std::string& username) const;

    /**
     * @brief Checks if a user has administrative privileges.
     * @param username The name of the user.
     * @return True if the user is an admin.
     */
    bool isAdmin(const std::string& username) const;

private:
    std::shared_ptr<UserManager> user_manager_;
    PermissionCheckCallback permission_callback_;
    std::string default_user_;
    std::string default_database_;

    // Performs fundamental checks that are universal (e.g., admin override).
    PermissionResult validateBasicPermissions(PermissionOperation operation,
                                             const std::string& resource,
                                             const std::string& current_user,
                                             const std::string& current_database);

    // Invokes the registered callback for domain-specific validation.
    PermissionResult validateWithCallback(const PermissionContext& context);

    // Checks if an operation requires a database to be selected.
    bool validateDatabaseContext(PermissionOperation operation) const;

    // Helper to resolve the current user, using the default if necessary.
    std::string getCurrentUser(const std::string& user) const;
    // Helper to resolve the current database, using the default if necessary.
    std::string getCurrentDatabase(const std::string& database) const;
};

/**
 * @brief A helper macro for concisely validating permissions.
 */
#define VALIDATE_PERMISSION(validator, operation, resource, user, database) \
    validator.validate(operation, resource, user, database)

/**
 * @brief A helper macro for validating an entire statement (intended for future use).
 */
#define VALIDATE_STATEMENT(validator, stmt, user, database) \
    validator.validateStatement(std::move(stmt), user, database)

} // namespace sqlcc

#endif // SQLCC_PERMISSION_VALIDATOR_H
