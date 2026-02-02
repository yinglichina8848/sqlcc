#include "permission_validator.h"
#include <algorithm>
#include <sstream>

namespace sqlcc {

PermissionValidator::PermissionValidator(std::shared_ptr<UserManager> user_manager)
    : user_manager_(user_manager) {
    // WHY: `PermissionValidator`需要一个`UserManager`的实例来查询用户信息和角色信息，
    // 以便执行基于用户的权限检查（例如，判断用户是否是管理员）。没有`UserManager`，
    // `PermissionValidator`就无法履行其职责。
    // WHAT: 构造函数初始化`PermissionValidator`实例，并确保`UserManager`依赖是有效的。
    // 同时，它设置了默认的用户和数据库，这些默认值在权限检查时，如果未显式提供用户或数据库，
    // 将被用作回退机制。
    // HOW:
    // 1.  **依赖注入**: `user_manager_`通过构造函数参数进行初始化，这是一种依赖注入的实践，
    //     提高了模块的解耦性和可测试性。
    // 2.  **前置条件检查**: 检查`user_manager_`是否为空。如果为空，则抛出`std::invalid_argument`异常，
    //     这是为了确保`PermissionValidator`始终在一个有效且可操作的状态下运行。
    // 3.  **默认值设置**: `default_user_`被初始化为"root"，`default_database_`被初始化为空字符串。
    //     在实际生产系统中，这些默认值应该从配置文件中加载，以提高灵活性和安全性。
    if (!user_manager_) {
        throw std::invalid_argument("UserManager cannot be null.");
    }
    // Set a default 'root' user, but in a real system, this might be configured elsewhere.
    default_user_ = "root";
    default_database_ = ""; // Default to no database selected.
}

void PermissionValidator::setPermissionCheckCallback(PermissionCheckCallback callback) {
    // WHY: `PermissionValidator`的核心设计理念是解耦。它本身不应该知道所有具体的业务规则。
    // 相反，它提供一个框架，允许外部模块（例如，知道如何查询`sys_privileges`表的`SQLExecutor`）
    // 注入实现详细权限检查的逻辑。
    // WHAT: 此方法用于注册一个回调函数（`PermissionCheckCallback`），这个回调函数包含了
    // 应用程序中针对特定资源和操作的详细权限检查业务逻辑。
    // HOW: 外部模块通过调用此方法，传入一个`std::function`对象。当`PermissionValidator::validate`
    // 方法无法通过其基本检查做出决定时，它将调用这个已注册的回调函数来执行更复杂的权限判断。
    // `std::move(callback)`用于高效地转移`callback`的所有权。
    permission_callback_ = std::move(callback);
}

PermissionResult PermissionValidator::validate(PermissionOperation operation,
                                              const std::string& resource,
                                              const std::string& current_user,
                                              const std::string& current_database) {
    // WHY: `validate`方法是整个权限框架的统一入口点。它提供了一个单一、一致的接口供
    // 系统中的其他组件进行权限检查，从而确保所有权限请求都通过相同的逻辑路径处理，
    // 提高了权限管理的统一性和可维护性。其两阶段验证设计（基本检查 + 详细回调）兼顾了效率和灵活性。
    // WHAT: 此方法根据传入的操作类型、资源、用户和数据库上下文，执行权限验证。
    // 它首先执行一些通用的、基础的检查（例如，管理员特权），如果这些检查无法确定结果，
    // 则将权限检查委托给外部注册的业务逻辑回调函数。
    // HOW:
    // 1.  **基本权限检查**: 首先调用`validateBasicPermissions`执行一些快速、通用的检查。
    //     这些检查通常是与业务逻辑解耦的，例如检查用户是否是拥有全局特权的超级管理员。
    // 2.  **立即返回**: 如果`validateBasicPermissions`返回`allowed=true`（例如，是管理员），
    //     则立即返回结果，无需进一步检查，这提高了效率。
    // 3.  **构建上下文**: 如果基本检查未能做出最终决定，则构建一个`PermissionContext`对象。
    //     这个上下文包含了所有进行详细权限检查所需的参数，如解析后的用户和数据库。
    // 4.  **委托给回调**: 调用`validateWithCallback`方法，将构建好的`PermissionContext`传递给
    //     外部注册的、包含业务特定权限逻辑的回调函数进行最终判断。
    //     这种委托机制是框架解耦的关键。
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
    // WHY: 确定一个用户是否具有管理员权限是权限验证中的一个基础且关键的步骤。
    // 管理员通常被赋予绕过所有常规权限检查的特权。此函数旨在提供一个清晰的判断依据。
    // WHAT: 检查给定的用户是否属于`UserManager`中定义的`ROLE_SUPERUSER`或`ROLE_ADMIN`角色。
    // HOW: 通过`user_manager_->isUserInRole`方法检查用户是否属于这两个预设的管理角色之一。
    // 如果用户是任一角色，则认为其是管理员。
    if (!user_manager_) return false;
    // In this design, being part of the 'SUPERUSER' or 'ADMIN' role confers admin privileges.
    return user_manager_->isUserInRole(username, UserManager::ROLE_SUPERUSER) ||
           user_manager_->isUserInRole(username, UserManager::ROLE_ADMIN);
}

PermissionResult PermissionValidator::validateBasicPermissions(PermissionOperation operation,
                                                             const std::string& resource,
                                                             const std::string& current_user,
                                                             const std::string& current_database) {
    // WHY: 此方法旨在执行快速、通用的权限检查，这些检查独立于具体的业务逻辑或资源类型。
    // 这种“快速失败/快速成功”的策略可以提高验证效率，减少对更复杂、开销更大的回调函数的调用。
    // 它也实现了“默认拒绝”（fail-safe）原则，即如果没有明确的允许，则操作被拒绝。
    // WHAT: 检查用户是否存在，是否是超级管理员（拥有所有权限），以及操作是否需要在特定数据库上下文中执行。
    // HOW:
    // 1.  **解析用户**: 调用`getCurrentUser`来确定实际进行操作的用户。
    // 2.  **管理员特权**: 调用`isAdmin`检查用户是否是管理员。如果是，则直接返回允许结果，
    //     因为管理员被视为拥有所有权限（这是系统级的“万能钥匙”）。
    //     注意，`UserManager::isAdmin`现在应该检查用户是否属于`UserManager::ROLE_SUPERUSER`。
    // 3.  **数据库上下文验证**: 如果操作依赖于数据库上下文（例如，`SELECT`操作必须指定一个数据库），
    //     则检查`current_database`是否为空。如果为空且操作需要数据库，则返回拒绝结果。
    // 4.  **默认拒绝**: 如果上述任何检查都没有明确允许操作，则返回一个拒绝结果，
    //     并将进一步的详细检查委托给外部回调函数。
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
    // WHY: `validateWithCallback`是`PermissionValidator`实现核心解耦策略的关键点。
    // 它允许框架将复杂的、业务逻辑相关的权限判断，委托给一个外部注册的函数来处理。
    // 这使得`PermissionValidator`自身保持通用和轻量级，而不需要了解所有资源类型的具体权限规则。
    // WHAT: 此方法负责调用已注册的`permission_callback_`函数，传入`PermissionContext`，
    // 以执行详细的、业务特定的权限检查。如果未注册回调，则默认拒绝操作，防止“默认允许”（fail-open）的安全漏洞。
    // HOW:
    // 1.  **检查回调**: 首先检查`permission_callback_`是否已被注册（即是否为`nullptr`）。
    // 2.  **调用回调**: 如果回调已注册，则直接调用它，并将其返回的`PermissionResult`作为最终结果返回。
    // 3.  **默认拒绝**: 如果没有回调被注册，这表明系统没有配置详细的权限规则。出于安全性考虑，
    //     此时操作必须被拒绝（`fail-safe`），因为“未知的操作”或“未知的权限规则”应被视为不允许。
    // If a detailed permission-checking callback is registered, invoke it.
    if (permission_callback_) {
        return permission_callback_(context);
    }

    // If there's no callback registered, it means no detailed permission rules are loaded.
    // For security, we deny the operation. This prevents failing open.
    return PermissionResult::createDenied("Permission check handler not configured.");
}

bool PermissionValidator::validateDatabaseContext(PermissionOperation operation) const {
    // WHY: 数据库中的许多操作（例如，创建表、查询表）本质上是针对某个特定数据库进行的。
    // 如果没有选择数据库上下文就尝试执行这些操作，可能会导致SQL错误、行为不确定性，
    // 或者需要系统猜测用户的意图，这通常不是一个好策略。此函数在权限验证阶段就强制执行这个约束。
    // WHAT: 此方法用于判断给定的`PermissionOperation`是否是一个必须在已选择数据库上下文中执行的操作。
    // HOW: 通过一个`switch`语句，列出所有需要明确数据库上下文的操作（如所有表相关的DML和DDL操作）。
    // 如果操作在列表中，则返回`true`，表示它需要数据库上下文；否则返回`false`。
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
    // WHY: 在执行权限检查时，如果调用方没有明确提供用户身份，系统仍然需要一个有效的用户上下文来操作。
    // 这允许更灵活的API调用，同时确保权限检查始终基于一个已知的用户。
    // WHAT: 此辅助方法负责确定进行权限检查的最终用户名称。如果传入的`user`字符串为空，
    // 则回退到`PermissionValidator`实例中配置的`default_user_`。
    // HOW: 简单的条件判断：如果`user`为空，返回`default_user_`；否则，返回`user`。
    return user.empty() ? default_user_ : user;
}

std::string PermissionValidator::getCurrentDatabase(const std::string& database) const {
    // WHY: 类似地，对于需要数据库上下文的权限检查，如果调用方没有明确提供数据库名称，
    // 系统也需要一个有效的数据库上下文来操作。
    // WHAT: 此辅助方法负责确定进行权限检查的最终数据库名称。如果传入的`database`字符串为空，
    // 则回退到`PermissionValidator`实例中配置的`default_database_`。
    // HOW: 简单的条件判断：如果`database`为空，返回`default_database_`；否则，返回`database`。
    return database.empty() ? default_database_ : database;
}

std::string PermissionValidator::operationToPrivilege(PermissionOperation operation) {
    // WHY: `PermissionOperation`枚举提供了一种类型安全且内部一致的方式来表示数据库操作。
    // 然而，在数据库的权限元数据表（例如`sys_privileges`）中存储权限时，以及在SQL `GRANT` / `REVOKE`
    // 语句中，通常使用字符串形式的权限名称（如"SELECT", "CREATE_TABLE"）。
    // 此映射函数作为枚举和字符串表示之间的桥梁。
    // WHAT: 此方法负责将内部的`PermissionOperation`枚举值转换为对应的外部字符串权限名称。
    // HOW: 使用一个`static const std::unordered_map`来存储枚举到字符串的映射。
    // 这种方法在第一次调用时构建映射，之后提供O(1)的平均查找时间，效率高。
    // 如果找不到匹配项，则返回"UNKNOWN"字符串。
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
