#include "permission_validator.h"
#include <algorithm>
#include <sstream>

namespace sqlcc {

PermissionValidator::PermissionValidator(std::shared_ptr<UserManager> user_manager)
    : user_manager_(user_manager) {
    if (!user_manager_) {
        throw std::invalid_argument("UserManager cannot be null.");
    }
    // Set a default 'root' user, but in a real system, this might be configured elsewhere.
    default_user_ = "root";
    default_database_ = ""; // Default to no database selected.
}

void PermissionValidator::setPermissionCheckCallback(PermissionCheckCallback callback) {
    permission_callback_ = std::move(callback);
}

PermissionResult PermissionValidator::validate(PermissionOperation operation,
                                              const std::string& resource,
                                              const std::string& current_user,
                                              const std::string& current_database) {
    // First, perform basic, universal permission checks.
    PermissionResult basic_result = validateBasicPermissions(operation, resource, current_user, current_database);
    
    // If the basic check resulted in a definitive decision (i.e., allowed), return it immediately.
    // This is typically used for admin overrides.
    if (basic_result.allowed) {
        return basic_result;
    }

    // If no definitive basic permission was found, create the context for the detailed check.
    PermissionContext context(
        getCurrentUser(current_user),
        getCurrentDatabase(current_database),
        resource,
        operation
    );

    // Delegate the final decision to the registered, domain-specific callback.
    return validateWithCallback(context);
}

void PermissionValidator::setDefaultUser(const std::string& user) {
    default_user_ = user;
}

void PermissionValidator::setDefaultDatabase(const std::string& database) {
    default_database_ = database;
}

bool PermissionValidator::userExists(const std::string& username) const {
    return user_manager_->userExists(username);
}

bool PermissionValidator::isAdmin(const std::string& username) const {
    if (!user_manager_) return false;
    // In this design, being part of the 'admin' role confers admin privileges.
    return user_manager_->isUserInRole(username, "admin");
}

PermissionResult PermissionValidator::validateBasicPermissions(PermissionOperation operation,
                                                             const std::string& resource,
                                                             const std::string& current_user,
                                                             const std::string& current_database) {
    // Step 1: Resolve the user, falling back to the default if none is provided.
    std::string user = getCurrentUser(current_user);

    // Step 2: Check for admin override. If the user has the 'admin' role, they can do anything.
    // This is a common pattern that simplifies administration.
    if (isAdmin(user)) {
        return PermissionResult::createAllowed();
    }
    
    // Step 3: Check if the operation requires a database context.
    if (validateDatabaseContext(operation) && getCurrentDatabase(current_database).empty()) {
        return PermissionResult::createDenied("No database selected for a database-dependent operation.");
    }

    // Return a denied result by default, forcing the detailed callback to make the final "allow" decision.
    // This is a "fail-safe" approach.
    return PermissionResult::createDenied("No basic permission grant found. Awaiting detailed check.");
}

PermissionResult PermissionValidator::validateWithCallback(const PermissionContext& context) {
    // If a detailed permission-checking callback is registered, invoke it.
    if (permission_callback_) {
        return permission_callback_(context);
    }

    // If there's no callback registered, it means no detailed permission rules are loaded.
    // For security, we deny the operation. This prevents failing open.
    return PermissionResult::createDenied("Permission check handler not configured.");
}

bool PermissionValidator::validateDatabaseContext(PermissionOperation operation) const {
    // List all operations that absolutely require a database to be selected.
    switch (operation) {
        case PermissionOperation::CREATE_TABLE:
        case PermissionOperation::DROP_TABLE:
        case PermissionOperation::ALTER_TABLE:
        case PermissionOperation::SELECT:
        case PermissionOperation::INSERT:
        case PermissionOperation::UPDATE:
        case PermissionOperation::DELETE:
        case PermissionOperation::SHOW_TABLES:
            return true;
        default:
            return false;
    }
}

std::string PermissionValidator::getCurrentUser(const std::string& user) const {
    return user.empty() ? default_user_ : user;
}

std::string PermissionValidator::getCurrentDatabase(const std::string& database) const {
    return database.empty() ? default_database_ : database;
}

std::string PermissionValidator::operationToPrivilege(PermissionOperation operation) {
    // This static map translates the internal operation enum to a privilege string
    // that can be stored in metadata tables (e.g., in the `sys_privileges` table).
    static const std::unordered_map<PermissionOperation, std::string> privilege_map = {
        {PermissionOperation::CREATE_DATABASE, "CREATE_DATABASE"},
        {PermissionOperation::DROP_DATABASE, "DROP_DATABASE"},
        {PermissionOperation::CREATE_TABLE, "CREATE_TABLE"},
        {PermissionOperation::DROP_TABLE, "DROP_TABLE"},
        {PermissionOperation::ALTER_TABLE, "ALTER_TABLE"},
        {PermissionOperation::SELECT, "SELECT"},
        {PermissionOperation::INSERT, "INSERT"},
        {PermissionOperation::UPDATE, "UPDATE"},
        {PermissionOperation::DELETE, "DELETE"},
        {PermissionOperation::CREATE_USER, "CREATE_USER"},
        {PermissionOperation::DROP_USER, "DROP_USER"},
        {PermissionOperation::GRANT, "GRANT"},
        {PermissionOperation::REVOKE, "REVOKE"},
        {PermissionOperation::USE_DATABASE, "USE"},
        {PermissionOperation::SHOW_DATABASES, "SHOW_DATABASES"},
        {PermissionOperation::SHOW_TABLES, "SHOW_TABLES"}
    };

    auto it = privilege_map.find(operation);
    return it != privilege_map.end() ? it->second : "UNKNOWN";
}

std::string PermissionValidator::operationToResourceType(PermissionOperation operation) {
    // Maps an operation to the type of resource it typically acts upon.
    // This helps in querying privilege tables (e.g., `WHERE resource_type = 'DATABASE'`).
    switch (operation) {
        case PermissionOperation::CREATE_DATABASE:
        case PermissionOperation::DROP_DATABASE:
        case PermissionOperation::USE_DATABASE:
        case PermissionOperation::SHOW_DATABASES:
            return "DATABASE";

        case PermissionOperation::CREATE_TABLE:
        case PermissionOperation::DROP_TABLE:
        case PermissionOperation::ALTER_TABLE:
        case PermissionOperation::SELECT:
        case PermissionOperation::INSERT:
        case PermissionOperation::UPDATE:
        case PermissionOperation::DELETE:
        case PermissionOperation::SHOW_TABLES:
            return "TABLE";

        case PermissionOperation::CREATE_USER:
        case PermissionOperation::DROP_USER:
        case PermissionOperation::GRANT:
        case PermissionOperation::REVOKE:
            return "USER";

        default:
            // Operations that don't fit into a specific resource category.
            return "SYSTEM";
    }
}

} // namespace sqlcc
