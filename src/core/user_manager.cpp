/**
 * @file user_manager.cpp
 * @brief 用户管理器核心实现。
 *
 * @WHY
 * 在多用户数据库系统中，安全性是至关重要的。用户管理和权限控制是实现数据安全的核心。
 * 没有用户管理器，数据库就无法区分不同用户的权限，任何人都能访问所有数据。
 *
 * 主要问题解决：
 * 1.  **身份认证 (Authentication)**: 验证用户的合法身份（例如，用户名和密码），防止未经授权的访问。
 * 2.  **授权 (Authorization)**: 基于用户的身份和角色，决定其对数据库对象（如表、数据库）的访问权限。
 * 3.  **基于角色的访问控制 (RBAC)**: 通过为用户分配角色，简化权限管理。权限赋给角色，用户继承角色权限。
 * 4.  **可管理性**: 能够方便地创建、删除、修改用户和角色，以及授予和撤销权限。
 * 5.  **安全性**: 密码需要安全存储（哈希），权限检查必须严谨，防止越权操作。
 * 6.  **可扩展性**: 随着系统功能的增加，权限模型可能需要扩展。
 * 7.  **审计追踪**: 记录用户操作，便于安全审计和问题排查。
 *
 * 本UserManager旨在提供一个安全、高效且可扩展的用户和权限管理框架，支持RBAC。
 *
 * @WHAT
 * 本文件实现了 `UserManager` 类，它是实现用户身份认证、角色管理和权限检查的核心服务。
 * 它维护着用户、角色及其权限的内存状态，并提供持久化到文件（或SystemDatabase）的机制。
 *
 * 核心功能包括：
 * -   用户生命周期管理 (创建、删除、修改密码、修改角色)。
 * -   角色生命周期管理 (创建、删除、修改角色名)，并支持角色继承。
 * -   权限授予和撤销 (针对用户或角色，对数据库或表进行操作)。
 * -   细粒度权限检查 (CheckPermission)。
 * -   用户认证 (AuthenticateUser)。
 * -   权限审计和冲突检测 (AuditPermissionChanges, CheckPermissionConflict)。
 * -   持久化机制，能够将用户、角色和权限信息保存到文件或 SystemDatabase。
 *
 * 核心组件：
 * -   **用户存储**: 管理用户信息、密码哈希、安全状态。
 * -   **角色体系**: 定义角色、继承关系、权限聚合。
 * -   **权限模型**: 支持数据库、表级别的细粒度权限控制。
 * -   **认证引擎**: 安全的身份验证和密码管理。
 * -   **授权引擎**: 基于角色的访问控制决策。
 * -   **持久化层**: 用户和权限信息的持久化存储。
 * -   **权限矩阵**: 内存中的哈希表，用于高效权限查找和缓存。
 *
 * @HOW
 * `UserManager` 的实现遵循以下原则和技术：
 * 1.  **并发安全**: 使用 `std::mutex` 保护所有对内部数据结构的操作，确保在并发环境下的正确性。
 * 2.  **密码安全**: 用户的密码不直接存储明文，而是存储其安全的哈希值（当前为简化实现）。
 * 3.  **基于角色的访问控制 (RBAC)**: 用户被分配一个角色，权限则直接或间接（通过角色继承）赋给角色。`CheckPermission` 会检查用户的当前角色及其继承的所有角色是否拥有所需权限。
 * 4.  **权限矩阵 (Permission Matrix)**: 通过 `std::unordered_map` 将授权信息构建为内存中的哈希表（`permission_matrix_`），实现 O(1) 的权限检查。
 * 5.  **角色继承**: 支持角色间的层级继承关系，子角色自动拥有父角色的权限。
 * 6.  **持久化机制**: 提供 `SaveToFile()` 和 `LoadFromFile()` 方法，将内存中的用户、角色和权限数据序列化到文件。
 * 7.  **SystemDatabase 集成**: `UserManager` 可以与 `SystemDatabase` 交互，实现用户、角色和权限信息的持久化存储和同步。
 * 8.  **错误处理**: 通过 `last_error_` 记录最近的错误信息。
 * 9.  **现代 C++ 实践**: 采用智能指针、现代 C++ 数据结构和算法。
 *
 * @note 本实现专为SQLCC数据库系统优化，支持RBAC权限模型。
 * @see include/core/user_manager.h
 */

#include "user_manager.h"
#include <algorithm>
#include <iostream>
#include <queue>
#include <unordered_set>
#include <chrono> // For std::chrono::system_clock::now()
#include <ctime>  // For std::ctime

namespace sqlcc {

/**
 * @brief 构造UserManager实例。
 * @details 初始化UserManager，设置数据存储路径，并加载用户、角色和权限数据。
 * 如果文件不存在，会尝试创建默认的超级用户。
 * @param data_path 用户数据文件的存储目录。
 */
UserManager::UserManager(const std::string &data_path)
    : data_path_(data_path) {
    // TODO(#UM-002): 在实际实现中，这里应该尝试从data_path加载已存在的用户数据。
    // 如果数据加载失败或数据不存在，则调用CreateDefaultSuperuser()。
    CreateDefaultSuperuser();
    InitializePermissionMatrix(); // 初始化内存中的权限矩阵
}
/**
 * @brief 销毁UserManager实例。
 * @details 在销毁UserManager之前，通常会进行资源的清理。
 * 在此简化实现中，使用默认析构函数。
 */
UserManager::~UserManager() = default;

/**
 * @brief 设置SystemDatabase的共享指针引用。
 * @details UserManager需要SystemDatabase的引用来实现用户、角色和权限的持久化同步。
 * 此方法会锁定互斥量以保证线程安全。
 * @param sys_db SystemDatabase的共享指针。
 */
void UserManager::SetSystemDatabase(std::shared_ptr<SystemDatabase> sys_db) {
    std::lock_guard<std::mutex> lock(mutex_); // 保护对sys_db_成员变量的访问
    sys_db_ = sys_db;
}
/**
 * @brief 创建一个新用户。
 * @details 验证用户名和角色，对密码进行哈希处理，然后创建用户并存储。
 * 如果创建的是超级用户，会自动授予所有权限。操作成功后，会尝试持久化数据。
 * @param username 新用户的用户名。
 * @param password 新用户的密码（明文，内部会进行哈希）。
 * @param role 新用户所属的角色，默认为"USER"。
 * @return 创建成功返回true，否则返回false，错误信息存储在last_error_中。
 */
bool UserManager::CreateUser(const std::string &username, const std::string &password,
                             const std::string &role) {
    // WHY: 在多用户数据库中，创建新用户是建立安全环境的第一步。
    // 该过程必须确保用户名的唯一性，并对敏感信息（密码）进行保护。
    // WHAT: 验证用户名冲突，校验角色合法性，哈希存储密码，并初始化用户元数据。
    // HOW:
    // 1. 使用互斥锁保护 users_ 映射。
    // 2. 查找 username，确保其不重复。
    // 3. 调用 IsValidRole 检查角色是否存在。
    // 4. 调用 HashPassword 对明文密码进行不可逆处理。
    // 5. 将新构造的 User 对象存入 users_ 并更新当前活跃角色映射。
    // 6. 如果是超级用户，授予全局权限。
    // 7. 同步到持久化存储。
    std::lock_guard<std::mutex> lock(mutex_); // 保护对users_和roles_的访问

    // 1. 检查用户是否已存在。
    if (users_.find(username) != users_.end()) {
        last_error_ = "User already exists: " + username;
        return false;
    }

    // 2. 检查角色是否合法。
    if (!IsValidRole(role)) {
        last_error_ = "Invalid role: " + role;
        return false;
    }

    // 3. 构建User对象。
    User user;
    user.username = username;
    user.password_hash = HashPassword(password); // 密码哈希
    user.role = role;
    user.current_role = role; // 初始当前角色与主角色相同
    user.is_active = true;
    user.created_at = GetCurrentTimeString();

    // 4. 存储用户。
    users_[username] = user;
    user_current_roles_[username] = role; // 更新用户当前角色映射

    // 5. 如果是超级用户，授予所有权限。
    if (role == ROLE_SUPERUSER) {
        GrantAllPrivilegesToSuperuser(username); // TODO(#UM-003): 需要完整实现此方法
    }

    last_error_.clear(); // 清除之前的错误信息
    // 6. 尝试持久化数据。
    return SaveToFileInternal(); // TODO(#UM-004): 需要完整实现此方法
}

/**
 * @brief 删除一个用户。
 * @details 检查用户是否存在且不是系统超级用户。如果用户存在，将移除其所有权限，并从系统中删除用户记录。
 * 操作成功后，会尝试持久化数据。
 * @param username 待删除用户的用户名。
 * @return 删除成功返回true，否则返回false，错误信息存储在last_error_中。
 */
bool UserManager::DropUser(const std::string &username) {
    std::lock_guard<std::mutex> lock(mutex_); // 保护对users_和相关权限数据的访问

    // 1. 检查用户是否存在。
    if (users_.find(username) == users_.end()) {
        last_error_ = "User does not exist: " + username;
        return false;
    }

    // 2. 禁止删除预设的超级用户。
    // WHY: 系统的默认超级用户是保障数据库核心管理权限的最后一道防线，不应被随意删除。
    // WHAT: 检查用户名是否为配置中定义的超级用户。
    // HOW: 超级用户的名称（"superuser"）应该从配置文件中读取，而不是硬编码。
    if (username == "superuser") { // TODO(#UM-005-IMPL): 超级用户的名称应从配置中读取，而非硬编码。
        last_error_ = "Cannot drop superuser";
        return false;
    }

    // 3. 移除该用户的所有权限。
    RemoveUserPrivileges(username); // TODO(#UM-006): 需要完整实现此方法

    // 4. 从内存中删除用户记录和其当前角色映射。
    users_.erase(username);
    user_current_roles_.erase(username);

    last_error_.clear(); // 清除之前的错误信息
    // 5. 尝试持久化数据。
    return SaveToFileInternal(); // TODO(#UM-004): 需要完整实现此方法
}
/**
 * @brief 修改用户密码。
 * @details 查找用户，如果存在，则对其新密码进行哈希处理并更新用户记录中的密码哈希。
 * 操作成功后，会尝试持久化数据。
 * @param username 待修改密码的用户名。
 * @param new_password 新密码（明文）。
 * @return 修改成功返回true，否则返回false，错误信息存储在last_error_中。
 */
bool UserManager::AlterUserPassword(const std::string &username,
                                    const std::string &new_password) {
    std::lock_guard<std::mutex> lock(mutex_); // 保护对users_的访问

    // 1. 查找用户。
    auto it = users_.find(username);
    if (it == users_.end()) {
        last_error_ = "User does not exist: " + username;
        return false;
    }

    // 2. 更新用户密码哈希。
    // WHY: 在修改用户密码时，新密码同样不能明文存储。必须通过安全的哈希算法进行处理。
    // WHAT: 调用`HashPassword`函数对新密码进行哈希，并更新用户的`password_hash`字段。
    // HOW: 确保`HashPassword`方法已实现了加盐的、计算密集型的哈希算法。
    it->second.password_hash = HashPassword(new_password);

    last_error_.clear(); // 清除之前的错误信息
    // 3. 尝试持久化数据。
    return SaveToFileInternal(); // TODO(#UM-004): 需要完整实现此方法
}
/**
 * @brief 修改用户的主角色。
 * @details 查找用户，验证新角色合法性，然后更新用户记录中的主角色和当前激活角色。
 * 操作成功后，会尝试持久化数据。
 * @param username 待修改角色的用户名。
 * @param new_role 新的角色名称。
 * @return 修改成功返回true，否则返回false，错误信息存储在last_error_中。
 */
bool UserManager::AlterUserRole(const std::string &username, const std::string &new_role) {
    std::lock_guard<std::mutex> lock(mutex_); // 保护对users_和roles_的访问

    // 1. 查找用户。
    auto it = users_.find(username);
    if (it == users_.end()) {
        last_error_ = "User does not exist: " + username;
        return false;
    }

    // 2. 验证新角色合法性。
    // WHY: 在修改用户角色时，新角色必须是系统中合法且有效的角色。
    // WHAT: 调用`IsValidRole`方法检查`new_role`的合法性。
    // HOW: `IsValidRole`方法（已在`user_manager.cpp`中详细注释）会执行角色存在性、
    // 系统预留角色检查以及潜在的循环继承检查。
    if (!IsValidRole(new_role)) {
        last_error_ = "Invalid role: " + new_role;
        return false;
    }

    // 3. 更新用户的主角色和当前激活角色。
    it->second.role = new_role;
    it->second.current_role = new_role; // 更改主角色后，当前角色也应该更新
    user_current_roles_[username] = new_role; // 更新内存中的当前角色映射

    last_error_.clear(); // 清除之前的错误信息
    // 4. 尝试持久化数据。
    return SaveToFileInternal(); // TODO(#UM-004): 需要完整实现此方法
}
/**
 * @brief 验证用户身份。
 * @details 根据提供的用户名和密码，检查用户是否存在、是否活跃，并验证密码是否正确。
 * 密码验证通过将提供的明文密码进行哈希后与存储的哈希值进行比较。
 * @param username 待验证用户的用户名。
 * @param password 待验证用户的密码（明文）。
 * @return 认证成功返回true，否则返回false，错误信息存储在last_error_中。
 */
bool UserManager::AuthenticateUser(const std::string &username,
                                   const std::string &password) {
    std::lock_guard<std::mutex> lock(mutex_); // 保护对users_的访问

    // 1. 检查用户是否存在。
    auto it = users_.find(username);
    if (it == users_.end()) {
        last_error_ = "User does not exist: " + username;
        return false;
    }

    // 2. 检查用户是否活跃。
    if (!it->second.is_active) {
        last_error_ = "User is not active: " + username;
        return false;
    }

    // 3. 验证密码：将提供的密码哈希后与存储的哈希值进行比较。
    // WHY: 认证的核心是验证用户提供的密码与系统中存储的哈希值是否匹配。
    // 重要的是，用于比较的哈希值必须使用与存储时相同的哈希算法（包括盐值）生成。
    // WHAT: 对用户输入的明文密码进行哈希，然后与`User`对象中存储的`password_hash`进行比较。
    // HOW: 确保`HashPassword`方法返回的哈希值是与存储的哈希值一致格式，并且包含正确的盐值处理。
    if (it->second.password_hash != HashPassword(password)) {
        last_error_ = "Invalid password";
        return false;
    }

    last_error_.clear(); // 清除之前的错误信息
    return true; // 认证成功
}
/**
 * @brief 创建一个新角色。
 * @details 检查角色名称是否已存在。如果不存在，则创建一个新角色并存储。
 * 操作成功后，会尝试持久化数据。
 * @param role_name 新角色的名称。
 * @return 创建成功返回true，否则返回false，错误信息存储在last_error_中。
 */
bool UserManager::CreateRole(const std::string &role_name) {
    std::lock_guard<std::mutex> lock(mutex_); // 保护对roles_的访问

    // 1. 检查角色是否已存在。
    if (roles_.find(role_name) != roles_.end()) {
        last_error_ = "Role already exists: " + role_name;
        return false;
    }

    // 2. 构建Role对象。
    Role role;
    role.role_name = role_name;
    role.created_at = GetCurrentTimeString();

    // 3. 存储角色。
    roles_[role_name] = role;

    last_error_.clear(); // 清除之前的错误信息
    // 4. 尝试持久化数据。
    return SaveToFileInternal(); // TODO(#UM-004): 需要完整实现此方法
}
/**
 * @brief 删除一个角色。
 * @details 检查角色是否存在且不是系统预设角色。如果角色存在，将移除其所有权限，并从系统中删除角色记录。
 * 操作成功后，会尝试持久化数据。
 * @param role_name 待删除角色的名称。
 * @return 删除成功返回true，否则返回false，错误信息存储在last_error_中。
 */
bool UserManager::DropRole(const std::string &role_name) {
    std::lock_guard<std::mutex> lock(mutex_); // 保护对roles_和相关权限数据的访问

    // 1. 检查角色是否存在。
    if (roles_.find(role_name) == roles_.end()) {
        last_error_ = "Role does not exist: " + role_name;
        return false;
    }

    // 2. 禁止删除系统预设角色。
    // WHY: 预设的系统角色（如SUPERUSER, ADMIN, USER）是数据库安全模型的基础组成部分，
    // 不应被用户删除，以防止系统权限体系的破坏。
    // WHAT: 检查`role_name`是否与配置中定义的系统角色名称匹配。
    // HOW: 系统角色名称应该从配置文件中读取，而不是硬编码在代码中，这增加了系统的灵活性和可维护性。
    if (role_name == ROLE_SUPERUSER || role_name == ROLE_ADMIN || role_name == ROLE_USER) { // TODO(#UM-009-IMPL): 系统角色名称应从配置中读取，而非硬编码。
        last_error_ = "Cannot drop system role: " + role_name;
        return false;
    }
    // 3. 移除该角色的所有权限。
    RemoveRolePrivileges(role_name); // TODO(#UM-010): 需要完整实现此方法，包括级联撤销所有用户的该角色权限。

    // 4. 从内存中删除角色记录。
    roles_.erase(role_name);

    last_error_.clear(); // 清除之前的错误信息
    // 5. 尝试持久化数据。
    return SaveToFileInternal(); // TODO(#UM-004): 需要完整实现此方法
}
/**
 * @brief 修改角色名称。
 * @details 检查源角色是否存在，目标新名称是否已被占用。如果合法，则更新角色名称。
 * 操作成功后，会尝试持久化数据。
 * @param role_name 待修改角色的当前名称。
 * @param new_role_name 角色的新名称。
 * @return 修改成功返回true，否则返回false，错误信息存储在last_error_中。
 */
bool UserManager::AlterRole(const std::string &role_name,
                            const std::string &new_role_name) {
    std::lock_guard<std::mutex> lock(mutex_); // 保护对roles_的访问

    // 1. 检查源角色是否存在。
    if (roles_.find(role_name) == roles_.end()) {
        last_error_ = "Role does not exist: " + role_name;
        return false;
    }

    // 2. 检查新角色名称是否已被占用。
    if (roles_.find(new_role_name) != roles_.end()) {
        last_error_ = "Role already exists: " + new_role_name;
        return false;
    }

    // 3. 更新角色名称。
    Role role = roles_[role_name];
    role.role_name = new_role_name;
    roles_.erase(role_name); // 移除旧名称的角色
    roles_[new_role_name] = role; // 添加新名称的角色

    // TODO(#UM-011-IMPL): 需要更新所有用户对该角色的引用，以及权限矩阵中对该角色的所有引用。
    // WHY: 角色名称的变更是一个影响深远的操作。如果不对所有相关引用进行更新，
    // 将导致数据不一致、权限检查失败或安全漏洞。
    // WHAT: 遍历所有用户、所有角色以及权限数据结构，将旧的角色名称替换为新的角色名称。
    // HOW:
    // 1. **更新用户角色**: 遍历`users_`映射，如果用户的`role`或`current_role`是旧的`role_name`，
    //    则将其更新为`new_role_name`。同时更新`user_current_roles_`映射。
    // 2. **更新角色继承**: 遍历`roles_`映射中的所有角色，如果其`parent_roles`或`child_roles`列表中
    //    包含旧的`role_name`，则将其更新为`new_role_name`。
    // 3. **更新权限**: 遍历`permissions_`向量，如果`Permission::grantee`是旧的`role_name`且`is_role`为true，
    //    则更新为`new_role_name`。然后，需要重新构建`permission_matrix_`，
    //    因为它直接使用了`PermissionKey`中的角色名称作为键的一部分。
    last_error_.clear(); // 清除之前的错误信息
    // 4. 尝试持久化数据。
    return SaveToFileInternal(); // WHY: 任何对用户或权限状态的修改都必须持久化到磁盘，以确保数据的安全性和一致性。
                                 // WHAT: 将内存中的当前用户、角色和权限数据写入到文件。
                                 // HOW: `SaveToFileInternal`负责实际的序列化和文件写入。}
/**
 * @brief 为指定用户设置当前激活的角色。
 * @details 允许用户在会话期间切换其活动角色，影响其权限集合。
 * 检查用户和角色是否存在。此操作仅在内存中更新用户的`current_role`。
 * @param username 用户的用户名。
 * @param role_name 待激活的角色名称。
 * @return 设置成功返回true，否则返回false，错误信息存储在last_error_中。
 */
bool UserManager::SetCurrentRole(const std::string &username,
                                 const std::string &role_name) {
    std::lock_guard<std::mutex> lock(mutex_); // 保护对users_和roles_的访问

    // 1. 检查用户是否存在。
    auto user_it = users_.find(username);
    if (user_it == users_.end()) {
        last_error_ = "User does not exist: " + username;
        return false;
    }

    // 2. 检查角色是否存在。
    auto role_it = roles_.find(role_name);
    if (role_it == roles_.end()) {
        last_error_ = "Role does not exist: " + role_name;
        return false;
    }

    // 3. 更新用户当前激活的角色。
    user_it->second.current_role = role_name;
    user_current_roles_[username] = role_name; // 更新内存中的当前角色映射

    last_error_.clear(); // 清除之前的错误信息
    return true;
}
/**
 * @brief 获取指定用户当前激活的角色。
 * @details 首先尝试从`user_current_roles_`映射中查找，如果不存在，则返回用户的主角色。
 * @param username 用户的用户名。
 * @return 用户当前激活的角色名称。如果用户不存在或未设置当前角色，返回空字符串。
 */
std::string UserManager::GetUserCurrentRole(const std::string &username) const {
    std::lock_guard<std::mutex> lock(mutex_); // 保护对user_current_roles_和users_的访问

    // 1. 首先尝试从用户当前激活角色映射中查找。
    auto it = user_current_roles_.find(username);
    if (it != user_current_roles_.end()) {
        return it->second;
    }

    // 2. 如果用户没有设置当前激活角色，则返回其默认（主）角色。
    auto user_it = users_.find(username);
    if (user_it != users_.end()) {
        return user_it->second.role;
    }

    return ""; // 用户不存在或无有效角色
}
/**
 * @brief 授予用户或角色在特定数据库或表上的权限。
 * @details 创建新的权限记录，并将其添加到内存权限列表中和权限矩阵中。
 * 操作成功后，会尝试持久化数据。
 * @param grantee 被授权者名称（用户名或角色名）。
 * @param database 权限所属的数据库（空字符串表示全局）。
 * @param table 权限所属的表（空字符串表示数据库级权限）。
 * @param privilege 具体权限类型（例如 "SELECT", "INSERT", "ALL"）。
 * @return 授予成功返回true，否则返回false，错误信息存储在last_error_中。
 */
bool UserManager::GrantPrivilege(const std::string &grantee, const std::string &database,
                                 const std::string &table, const std::string &privilege) {
    std::lock_guard<std::mutex> lock(mutex_); // 保护对permissions_和permission_matrix_的访问

    // 1. 检查被授权者是否存在且类型正确（用户或角色）。
    bool is_grantee_role = (roles_.find(grantee) != roles_.end());
    bool is_grantee_user = (users_.find(grantee) != users_.end());
    if (!is_grantee_role && !is_grantee_user) {
        last_error_ = "Grantee does not exist or is not a user/role: " + grantee;
        return false;
    }
    
    // 2. 检查权限是否已存在，避免重复授予。
    for (const auto& existing_perm : permissions_) {
        if (existing_perm.grantee == grantee && existing_perm.database == database &&
            existing_perm.table == table && existing_perm.privilege == privilege) {
            last_error_ = "Privilege already granted: " + grantee + " on " + database + "." + table + " with " + privilege;
            return false;
        }
    }


    // 3. 构建Permission对象。
    Permission permission;
    permission.grantee = grantee;
    permission.database = database;
    permission.table = table;
    permission.privilege = privilege;
    permission.is_role = is_grantee_role;

    // 4. 添加到内存权限列表和权限矩阵。
    permissions_.push_back(permission);
    AddPermissionToMatrix(permission);

    last_error_.clear(); // 清除之前的错误信息
    // 5. 尝试持久化数据。
    return SaveToFileInternal(); // TODO(#UM-004): 需要完整实现此方法
}
/**
 * @brief 撤销用户或角色在特定数据库或表上的权限。
 * @details 从内存权限列表中和权限矩阵中移除匹配的权限记录。
 * 操作成功后，会尝试持久化数据。
 * @param grantee 被撤销者名称（用户名或角色名）。
 * @param database 权限所属的数据库。
 * @param table 权限所属的表。
 * @param privilege 权限类型。
 * @return 撤销成功返回true，否则返回false，错误信息存储在last_error_中。
 */
bool UserManager::RevokePrivilege(const std::string &grantee, const std::string &database,
                                  const std::string &table, const std::string &privilege) {
    std::lock_guard<std::mutex> lock(mutex_); // 保护对permissions_和permission_matrix_的访问

    // 1. 从权限列表中查找并移除匹配的权限记录。
    // std::remove_if 将匹配的元素移到范围末尾，并返回一个指向新逻辑末尾的迭代器。
    auto it = std::remove_if(permissions_.begin(), permissions_.end(),
                             [&](const Permission &p) {
                               return p.grantee == grantee && p.database == database &&
                                      p.table == table && p.privilege == privilege;
                             });
    
    // 2. 如果找到匹配的权限记录。
    if (it != permissions_.end()) {
        // 记录被移除的权限，以便从权限矩阵中移除。
        Permission permission_to_remove = *it; 
        permissions_.erase(it, permissions_.end()); // 实际从vector中移除

        // 3. 从权限矩阵中移除对应的权限。
        RemovePermissionFromMatrix(permission_to_remove);

        last_error_.clear(); // 清除之前的错误信息
        // 4. 尝试持久化数据。
        return SaveToFileInternal(); // TODO(#UM-004): 需要完整实现此方法
    }

    last_error_ = "Privilege not found"; // 权限不存在
    return false;
}
/**
 * @brief 检查用户是否拥有对特定数据库或表的所需权限。
 * @details 该方法是高级权限检查的入口，会考虑用户的活跃状态、超级用户特权以及通过权限矩阵进行的细粒度检查。
 * @param username 用户的用户名。
 * @param database 数据库名称。
 * @param table 表名称。
 * @param required_privilege 所需的权限类型。
 * @return 如果用户拥有所需权限则返回true，否则返回false。
 */
bool UserManager::CheckPermission(const std::string &username, const std::string &database,
                                  const std::string &table,
                                  const std::string &required_privilege) {
    std::lock_guard<std::mutex> lock(mutex_); // 保护对users_和权限数据的访问

    // 1. 检查用户是否存在且处于激活状态。
    auto user_it = users_.find(username);
    if (user_it == users_.end() || !user_it->second.is_active) {
        last_error_ = "User does not exist or is not active: " + username;
        return false;
    }

    // 2. 超级用户拥有所有权限，直接返回true。
    if (user_it->second.role == ROLE_SUPERUSER) { // TODO(#UM-005): 超级用户名称应从配置中读取
        return true;
    }

    // 3. 使用权限矩阵进行细粒度权限检查。
    // 这会考虑用户的当前激活角色以及通过角色继承获得的权限。
    return CheckPermissionInMatrix(username, database, table, required_privilege);
}
/**
 * @brief 检查用户是否存在。
 * @details 在用户映射中查找指定用户名。
 * @param username 待检查的用户名。
 * @return 如果用户存在返回true，否则返回false。
 */
bool UserManager::userExists(const std::string &username) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return users_.find(username) != users_.end();
}

/**
 * @brief 检查用户是否拥有指定角色。
 * @details 检查用户的直接角色（不考虑角色继承链）。
 * @param username 用户的用户名。
 * @param role_name 角色名称。
 * @return 如果用户拥有该角色返回true，否则返回false。
 */
bool UserManager::isUserInRole(const std::string &username, const std::string &role_name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto user_iter = users_.find(username);
    if (user_iter == users_.end()) {
        return false;
    }
    return user_iter->second.role == role_name;
}

/**
 * @brief 列出所有用户。
 * @details 返回当前系统中所有注册用户的列表。
 * @return 包含所有用户User结构体的向量。
 */
std::vector<User> UserManager::ListUsers() const {
    // WHY: 管理员和系统监控工具需要能够列出所有用户以进行审计、统计或管理操作。
    // WHAT: 返回内存中 users_ 映射中存储的所有用户对象的副本。
    // HOW: 锁定 mutex_ 后，遍历 users_ 映射并将其中的 User 对象推入结果向量中。
    std::lock_guard<std::mutex> lock(mutex_); // 保护对users_的访问
    std::vector<User> result;
    for (const auto &pair : users_) {
        result.push_back(pair.second);
    }
    return result;
}
/**
 * @brief 列出所有角色。
 * @details 返回当前系统中所有定义角色的列表。
 * @return 包含所有角色Role结构体的向量。
 */
std::vector<Role> UserManager::ListRoles() const {
    // WHY: 管理员需要了解系统中定义的角色，以便正确分配给用户或进行角色继承管理。
    // WHAT: 返回内存中 roles_ 映射中存储的所有角色对象的副本。
    // HOW: 锁定 mutex_ 后，遍历 roles_ 映射并将其中的 Role 对象推入结果向量中。
    std::lock_guard<std::mutex> lock(mutex_); // 保护对roles_的访问
    std::vector<Role> result;
    for (const auto &pair : roles_) {
        result.push_back(pair.second);
    }
    return result;
}
/**
 * @brief 列出指定用户直接拥有的权限（不包括通过角色继承的权限）。
 * @param username 用户的用户名。
 * @return 包含该用户直接拥有的Permission结构体的向量。
 */
std::vector<Permission>
UserManager::ListUserPermissions(const std::string &username) const {
    // WHY: 精确了解直接赋予用户的权限有助于排查权限泄露或进行细粒度的访问审计。
    // WHAT: 过滤出 permissions_ 列表中被授权者匹配且标识为非角色的条目。
    // HOW: 锁定后，遍历 permissions_ 向量，检查 grantee 和 is_role 字段。
    std::lock_guard<std::mutex> lock(mutex_); // 保护对permissions_的访问
    std::vector<Permission> result;
    for (const auto &permission : permissions_) {
        // 过滤出直接授予该用户且非角色权限的记录。
        if (permission.grantee == username && !permission.is_role) {
            result.push_back(permission);
        }
    }
    return result;
}
/**
 * @brief 列出指定角色拥有的权限。
 * @param role_name 角色的名称。
 * @return 包含该角色拥有的Permission结构体的向量。
 */
std::vector<Permission>
UserManager::ListRolePermissions(const std::string &role_name) const {
    // WHY: 角色是权限聚合的单位。了解角色直接拥有的权限是维护 RBAC 模型一致性的关键。
    // WHAT: 过滤出 permissions_ 列表中被授权者匹配且标识为角色的条目。
    // HOW: 锁定后，遍历 permissions_ 向量，检查 grantee 和 is_role 字段。
    std::lock_guard<std::mutex> lock(mutex_); // 保护对permissions_的访问
    std::vector<Permission> result;
    for (const auto &permission : permissions_) {
        // 过滤出直接授予该角色的权限记录。
        if (permission.grantee == role_name && permission.is_role) {
            result.push_back(permission);
        }
    }
    return result;
}
/**
 * @brief 将当前UserManager的状态（用户、角色、权限等）持久化到文件。
 * @details 该方法是公共接口，会锁定互斥量以保证线程安全，然后调用内部方法进行实际的保存操作。
 * @return 保存成功返回true，否则返回false。
 */
bool UserManager::SaveToFile() const {
    std::lock_guard<std::mutex> lock(mutex_); // 保护对内部状态的并发访问
    return SaveToFileInternal(); // TODO(#UM-004): 需要完整实现此方法
}
/**
 * @brief 从文件加载用户、角色和权限数据。
 * @details 该方法是公共接口，会锁定互斥量以保证线程安全，然后从磁盘文件读取并恢复UserManager的内部状态。
 * @return 加载成功返回true，否则返回false。
 */
bool UserManager::LoadFromFile() {
    std::lock_guard<std::mutex> lock(mutex_); // 保护对内部状态的并发访问

    // WHY: 在数据库启动时，需要从持久化存储中加载用户、角色和权限信息，
    // 以恢复系统的安全上下文。这是确保数据库安全模型连续性和数据一致性的关键一步。
    // WHAT: 从`data_path_`指定的文件中读取并反序列化用户 (`users_`), 角色 (`roles_`),
    // 和权限 (`permissions_`) 数据，然后使用这些数据重建内存中的状态。
    // HOW:
    // 1. **文件检查**: 检查数据文件是否存在。如果不存在，说明是首次启动或文件丢失，则返回`false`。
    // 2. **文件读取**: 打开数据文件并读取其内容。这可能涉及特定的序列化格式（如JSON、CSV、二进制）。
    // 3. **反序列化**: 将读取到的字节流或文本解析成`User`, `Role`, `Permission`对象。
    // 4. **数据填充**: 将反序列化后的对象填充到`users_`, `roles_`, `permissions_`等内部`unordered_map`和`vector`中。
    // 5. **权限矩阵重建**: 在加载所有原始权限后，调用`InitializePermissionMatrix()`重新构建内存中的快速查询权限矩阵。

    // 简化的实现：假设文件不存在则返回false
    if (!std::filesystem::exists(data_path_ + "/users.json")) { // 假设用户数据存储在users.json
        last_error_ = "User data file not found: " + data_path_ + "/users.json";
        return false;
    }

    // TODO(#UM-002-IMPL): 需要完整实现从文件加载用户、角色和权限数据的逻辑。
    // 这将涉及反序列化操作，例如使用JSON解析库读取和解析数据。
    // 示例伪代码:
    // try {
    //     std::ifstream ifs(data_path_ + "/users.json");
    //     nlohmann::json j;
    //     ifs >> j;
    //     // 反序列化用户
    //     for (auto& elem : j["users"]) {
    //         User user;
    //         user.username = elem["username"];
    //         user.password_hash = elem["password_hash"];
    //         // ...
    //         users_[user.username] = user;
    //     }
    //     // 类似地反序列化roles和permissions
    //     // ...
    //     InitializePermissionMatrix(); // 重新构建权限矩阵
    //     last_error_.clear();
    //     return true;
    // } catch (const std::exception& e) {
    //     last_error_ = "Failed to load user data from file: " + std::string(e.what());
    //     return false;
    // }

    last_error_.clear();
    return true;
}
/**
 * @brief 获取UserManager最近一次操作的错误信息。
 * @return 包含错误信息的字符串引用。
 */
const std::string &UserManager::GetLastError() const {
    return last_error_;
}
// --- 私有辅助方法实现 ---
/**
 * @brief 在UserManager首次初始化时，创建默认的超级用户和系统角色。
 * @details 该方法会检查系统中是否存在"superuser"用户以及"SUPERUSER", "ADMIN", "USER"等预设角色，
 * 如果不存在，则创建它们。
 */
void UserManager::CreateDefaultSuperuser() {
    // WHY: 在系统首次启动或没有现有用户数据时，创建一个默认的超级用户是必要的，
    // 以便管理员能够登录并配置系统。但是，超级用户的用户名和密码不应硬编码在代码中。
    // WHAT: 检查是否存在默认超级用户，如果不存在则创建。同时，强调用户名和密码的配置化。
    // HOW: 超级用户的名称和密码应该从一个安全的配置文件（例如，数据库启动参数、加密的环境变量或独立的配置文件）中读取。
    // 这样可以在不修改代码的情况下修改认证信息，并防止敏感信息泄露。
    // TODO(#UM-005-IMPL): 超级用户的名称和密码应从配置中读取，而非硬编码。
    if (users_.find("superuser") == users_.end()) {
        // 直接调用内部方法创建，避免外部接口的权限检查。
        User user;
        user.username = "superuser";
        // WHY: 即使是默认超级用户的密码，也必须进行安全的哈希处理。
        // 这防止了硬编码密码直接泄露带来的风险，并强制遵循安全存储的最佳实践。
        // WHAT: 调用`HashPassword`函数对默认密码进行哈希。
        // HOW: 确保`HashPassword`方法已实现了加盐的、计算密集型的哈希算法，
        // 并且该默认密码本身也应从安全配置中读取。
        user.password_hash = HashPassword("superuser_password");
        user.role = ROLE_SUPERUSER;
        user.current_role = ROLE_SUPERUSER;
        user.is_active = true;
        user.created_at = GetCurrentTimeString();
        users_["superuser"] = user;
        user_current_roles_["superuser"] = ROLE_SUPERUSER;
    }

    // 2. 检查并创建默认的"SUPERUSER"角色。
    if (roles_.find(ROLE_SUPERUSER) == roles_.end()) {
        Role superuser_role;
        superuser_role.role_name = ROLE_SUPERUSER;
        superuser_role.created_at = GetCurrentTimeString();
        roles_[ROLE_SUPERUSER] = superuser_role;
    }

    // 3. 检查并创建默认的"ADMIN"角色。
    if (roles_.find(ROLE_ADMIN) == roles_.end()) {
        Role admin_role;
        admin_role.role_name = ROLE_ADMIN;
        admin_role.created_at = GetCurrentTimeString();
        roles_[ROLE_ADMIN] = admin_role;
    }

    // 4. 检查并创建默认的"USER"角色。
    if (roles_.find(ROLE_USER) == roles_.end()) {
        Role user_role;
        user_role.role_name = ROLE_USER;
        user_role.created_at = GetCurrentTimeString();
        roles_[ROLE_USER] = user_role;
    }
    // WHY: 确保默认超级用户在创建后即拥有所有必要权限，能够完全管理数据库系统。
    // WHAT: 在创建默认超级用户和角色之后，为其授予对所有对象的最高权限。
    // HOW: 调用`GrantAllPrivilegesToSuperuser`辅助方法。
    GrantAllPrivilegesToSuperuser("superuser"); // TODO(#UM-003-IMPL): 需要完整实现此方法
}
/**
 * @brief 获取当前时间的格式化字符串。
 * @return 包含当前日期和时间的字符串。
 */
std::string UserManager::GetCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::string time_str = std::ctime(&time_t);
    // WHY: `std::ctime` 返回的字符串末尾会包含一个换行符，这在日志或存储时通常是不希望的。
    // WHAT: 移除 `std::ctime` 返回字符串末尾的换行符。
    // HOW: 使用 `pop_back()` 移除最后一个字符，前提是字符串非空且最后一个字符是换行符。
    if (!time_str.empty() && time_str.back() == '\n') {
        time_str.pop_back();
    }
    return time_str;
}
/**
 * @brief 为指定的超级用户授予所有权限。
 * @details 这是一个简化的实现，在生产环境中，需要根据权限常量为超级用户授予对所有数据库和表的全部权限。
 * @param username 超级用户的用户名。
 */
void UserManager::GrantAllPrivilegesToSuperuser(const std::string &username) {
    // WHY: 超级用户是系统的最高权限实体，必须拥有对所有数据库对象（包括未来创建的对象）的完全控制权。
    // 在系统初始化或创建超级用户时，必须确保其具备这些权限，以便进行系统管理和配置。
    // WHAT: 遍历所有已知的或可发现的数据库和表，并为指定的用户授予`ALL`权限。
    // HOW:
    // 1. **获取数据库列表**: 如果`sys_db_`（SystemDatabase）可用，它将提供当前数据库实例中的所有数据库名称。
    //    如果`sys_db_`不可用，可能需要一个默认的或硬编码的数据库列表（例如，"system", "default"）。
    // 2. **获取表列表**: 对于每个数据库，获取其下所有表的名称。
    // 3. **授予权限**: 对每个数据库和每个表，调用`GrantPrivilege(username, database_name, table_name, PRIVILEGE_ALL)`。
    //    此外，还需要授予对特定数据库的`ALL`权限（`GrantPrivilege(username, database_name, "", PRIVILEGE_ALL)`），
    //    以及全局`ALL`权限（`GrantPrivilege(username, "", "", PRIVILEGE_ALL)`）。

    // TODO(#UM-003-IMPL): 需要完整实现此方法，为超级用户授予对所有现有和未来数据库、表的所有权限。
    // 这将涉及遍历所有数据库和表，并为每个数据库和表调用GrantPrivilege。
    // 简化的实现
    (void)username; // 避免未使用参数警告
}
/**
 * @brief 移除指定用户的所有直接授予权限。
 * @details 遍历`permissions_`列表，移除所有直接授予给该用户的权限记录。
 * @param username 待移除权限的用户名。
 */
void UserManager::RemoveUserPrivileges(const std::string &username) {
    // 1. 使用std::remove_if和erase idiom移除所有直接授予该用户的权限。
    // 这包括更新内存中的`permissions_`列表。
    auto it = std::remove_if(permissions_.begin(), permissions_.end(),
                             [&](const Permission &p) {
                               return p.grantee == username && !p.is_role;
                             });
    permissions_.erase(it, permissions_.end());

    // TODO(#UM-006-IMPL): 在移除用户权限后，需要更新权限矩阵`permission_matrix_`。
    // WHY: `permission_matrix_`是内存中用于快速权限检查的核心数据结构。
    // 当用户的权限发生变化时（例如，用户被删除），必须同步更新这个矩阵，
    // 以确保后续的权限检查能够反映最新的安全策略。
    // WHAT: 移除用户所有直接权限后，从`permission_matrix_`中删除所有与该用户相关的条目。
    // HOW: 最直接的方式是遍历`permission_matrix_`，删除所有`key.grantee == username`且`!key.is_role`的条目。
    // 或者，在所有权限操作（Grant/Revoke）后，可以调用`InitializePermissionMatrix()`重新构建整个矩阵，
    // 确保其与`permissions_`列表的一致性。
}
/**
 * @brief 移除指定角色的所有直接授予权限。
 * @details 遍历`permissions_`列表，移除所有直接授予给该角色的权限记录。
 * @param role_name 待移除权限的角色名。
 */
void UserManager::RemoveRolePrivileges(const std::string &role_name) {
    // 1. 使用std::remove_if和erase idiom移除所有直接授予该角色的权限。
    // 这包括更新内存中的`permissions_`列表。
    auto it = std::remove_if(permissions_.begin(), permissions_.end(),
                             [&](const Permission &p) {
                               return p.grantee == role_name && p.is_role;
                             });
    permissions_.erase(it, permissions_.end());

    // TODO(#UM-010-IMPL): 在移除角色权限后，需要更新权限矩阵`permission_matrix_`。
    // WHY: 类似于移除用户权限，`permission_matrix_`必须反映角色的最新权限状态。
    // 此外，由于存在角色继承，移除父角色的权限可能会影响到所有继承该权限的子角色和用户。
    // WHAT: 移除角色所有直接权限后，从`permission_matrix_`中删除所有与该角色相关的条目。
    // 并且，需要触发级联撤销机制，以确保所有依赖于此角色的继承权限也被正确更新。
    // HOW: 遍历`permission_matrix_`，删除所有`key.grantee == role_name`且`key.is_role`的条目。
    // 对于级联撤销，可以利用`GetRoleHierarchy`或类似的机制找到所有受影响的子角色，
    // 然后对这些子角色进行权限重新计算或显式撤销。
}
/**
 * @brief 检查角色名称是否合法且存在。
 * @details 该方法目前仅检查角色是否存在于内存的`roles_`映射中。
 * @param role_name 待检查的角色名称。
 * @return 如果角色存在返回true，否则返回false。
 */
bool UserManager::IsValidRole(const std::string &role_name) const {
    // WHY: 验证角色不仅仅是检查其是否存在。一个健壮的权限系统还需要确保所使用的角色是
    // 有效的、可用的，并且不会引入逻辑上的问题（如循环继承）。
    // WHAT: 扩展`IsValidRole`以执行更复杂的合法性检查，包括：
    // 1.  **角色存在性**: 确保`role_name`在`roles_`映射中存在。
    // 2.  **系统预留角色**: 检查`role_name`是否是系统预留角色（如SUPERUSER, ADMIN, USER）。
    //     如果尝试创建或修改为这些角色，可能需要特殊权限或限制。
    // 3.  **角色继承合法性**: 在支持角色继承的场景中，需要检查`role_name`是否会导致
    //     循环继承（例如，角色A继承角色B，角色B又继承角色A），这会导致权限评估的无限循环。
    // HOW: 在当前实现中，仅检查角色是否存在。在未来，可以引入一个`isSystemRole(role_name)`函数
    // 和一个`checkCircularInheritance(role_name)`函数来增强此方法。
    // TODO(#UM-008-IMPL): IsValidRole的实现需要考虑更复杂的合法性检查，
    // 例如，是否是系统预留角色，以及在角色继承场景中，是否需要递归检查。
    return roles_.find(role_name) != roles_.end();
}
/**
 * @brief 对密码进行哈希处理。
 * @details 这是一个简化的实现，仅为密码追加"_hashed"后缀。
 * 在生产环境中，应使用安全的密码哈希算法（如 bcrypt, Argon2 或 PBKDF2）来存储密码，
 * 并包含盐值 (salt) 以增强安全性。
 * @param password 原始密码。
 * @return 哈希后的密码字符串。
 */
std::string UserManager::HashPassword(const std::string &password) const {
    // WHY: 明文存储密码是严重的安全漏洞。攻击者一旦获取到数据库，即可直接获取所有用户密码。
    // 使用哈希函数可以将密码转换为不可逆的固定长度字符串，即使数据库泄露，攻击者也无法直接获得原密码。
    // WHAT: 实现一个安全的、加盐的、计算密集型的密码哈希算法。
    // HOW:
    // 1. **加盐 (Salting)**: 为每个用户生成一个唯一的随机盐值。将盐值与用户密码拼接，然后进行哈希。
    //    盐值通常与哈希后的密码一起存储。盐值能够有效防御彩虹表攻击和预计算哈希链攻击。
    // 2. **慢哈希函数 (Slow Hashing Functions)**: 选择计算开销大的哈希算法，如 bcrypt, Argon2, PBKDF2。
    //    这些算法通过增加计算时间来抵御暴力破解和字典攻击，即使攻击者拥有高性能计算资源，
    //    也需要耗费极大的时间才能破解少量密码。
    // 3. **迭代次数 (Iterations)**: 许多慢哈希函数允许配置迭代次数或计算因子，
    //    以适应硬件性能的提升，确保随着时间推移，哈希的安全性依然保持。

    // TODO(#UM-007-IMPL): 需要完整实现安全的密码哈希算法 (如 bcrypt, Argon2, PBKDF2)。
    // 简化的哈希实现（实际应该使用更安全的哈希算法）
    return password + "_hashed";
}
/**
 * @brief 内部方法：将当前UserManager的所有用户、角色和权限数据持久化到文件。
 * @details 这是一个简化的实现。在生产环境中，需要将`users_`, `roles_`, `permissions_`等所有数据
 * 序列化并写入`data_path_`指定的文件，确保数据的一致性和完整性。
 * 此方法不加锁，假定调用者（例如`SaveToFile()`）已持有锁。
 * @return 保存成功返回true，否则返回false。
 */
bool UserManager::SaveToFileInternal() const {
    // WHY: 任何对用户、角色或权限的修改都必须持久化到磁盘，以确保数据库安全配置的连续性和数据完整性。
    // 如果没有持久化，系统重启后所有变更都将丢失，导致严重的安全漏洞。
    // WHAT: 将`users_`, `roles_`, `permissions_`等内部数据结构中的所有数据序列化，
    // 并安全地写入到`data_path_`指定的文件中。
    // HOW:
    // 1. **序列化**: 将内存中的`User`, `Role`, `Permission`对象转换为可存储的格式，例如JSON字符串。
    //    这通常涉及遍历`users_`, `roles_`, `permissions_`容器，并为每个对象生成其JSON表示。
    // 2. **文件写入**: 将序列化后的数据写入到文件。为了保证数据完整性，理想情况下应采用
    //    原子写入（Atomic Write）策略：先写入一个临时文件，成功后再原子性地替换原文件。
    //    这可以防止在写入过程中发生系统崩溃导致文件损坏。
    // 3. **格式选择**: 可以选择JSON、Protocol Buffers、CSV或自定义二进制格式进行存储。
    //    JSON易读性好，便于调试；Protobuf效率更高，更适合大量数据。

    // 简化的保存实现：假设成功
    // TODO(#UM-004-IMPL): 需要完整实现此方法，将所有用户、角色和权限数据序列化并写入文件。
    // 示例伪代码:
    // try {
    //     nlohmann::json j;
    //     // 序列化用户
    //     for (const auto& pair : users_) {
    //         j["users"].push_back({
    //             {"username", pair.second.username},
    //             {"password_hash", pair.second.password_hash},
    //             // ...
    //         });
    //     }
    //     // 类似地序列化roles和permissions
    //     // ...
    //     std::ofstream ofs(data_path_ + "/users.json.tmp"); // 写入临时文件
    //     ofs << std::setw(4) << j << std::endl;
    //     ofs.close();
    //     std::filesystem::rename(data_path_ + "/users.json.tmp", data_path_ + "/users.json"); // 原子替换
    //     return true;
    // } catch (const std::exception& e) {
    //     last_error_ = "Failed to save user data to file: " + std::string(e.what());
    //     return false;
    // }
    
    return true;
}
/**
 * @brief 初始化内存中的权限矩阵`permission_matrix_`。
 * @details 遍历`permissions_`列表，将所有权限记录加载到哈希表中，以便进行快速查找。
 * 在`LoadFromFile()`之后或权限列表变更后调用。
 */
void UserManager::InitializePermissionMatrix() {
    // WHY: `permission_matrix_`是用于高效权限检查的内存缓存。
    // 在系统启动时或当原始权限列表(`permissions_`)发生重大变更后，
    // 必须重建这个矩阵以确保权限查询的速度和准确性。
    // WHAT: 遍历`permissions_`向量，将其中存储的原始权限记录转换为适合哈希表查找的`PermissionKey`和`PermissionValue`对，
    // 并填充到`permission_matrix_`中。
    // HOW: 清空旧的`permission_matrix_`，然后对`permissions_`中的每个`Permission`对象，
    // 构造对应的`PermissionKey`和`PermissionValue`，并使用`AddPermissionToMatrix`将其添加到矩阵中。
    // TODO(#UM-016-IMPL): 遍历`permissions_`列表，将所有权限记录加载到哈希表中，以便进行快速查找。
    permission_matrix_.clear(); // 清空旧矩阵
    for (const auto& perm : permissions_) {
        AddPermissionToMatrix(perm);
    }
}
/**
 * @brief 将一个权限添加到内存权限矩阵`permission_matrix_`中。
 * @details 将权限结构转换为PermissionKey和PermissionValue，并存入哈希表。
 * @param permission 待添加的权限实例。
 */
void UserManager::AddPermissionToMatrix(const Permission &permission) {
    PermissionKey key;
    key.grantee = permission.grantee;
    key.database = permission.database;
    key.table = permission.table;
    key.privilege = permission.privilege;

    PermissionValue value;
    value.has_permission = true; // 默认拥有权限
    value.is_role = permission.is_role;

    permission_matrix_[key] = value;
}
/**
 * @brief 从内存权限矩阵`permission_matrix_`中移除一个权限。
 * @details 根据PermissionKey移除哈希表中的对应条目。
 * @param permission 待移除的权限实例。
 */
void UserManager::RemovePermissionFromMatrix(const Permission &permission) {
    PermissionKey key;
    key.grantee = permission.grantee;
    key.database = permission.database;
    key.table = permission.table;
    key.privilege = permission.privilege;

    permission_matrix_.erase(key);
}
/**
 * @brief 在内存权限矩阵`permission_matrix_`中检查用户是否拥有特定权限。
 * @details 这是底层、高效的权限查找方法，不涉及角色继承或当前激活角色的复杂逻辑。
 * 该方法会检查直接授予用户的权限，以及通过其当前激活角色授予的权限，并支持通配符匹配。
 * @param username 用户的用户名。
 * @param database 数据库名称。
 * @param table 表名称。
 * @param required_privilege 所需的权限类型。
 * @return 如果权限存在于矩阵中则返回true，否则返回false。
 */
bool UserManager::CheckPermissionInMatrix(
    const std::string &username, const std::string &database,
    const std::string &table,
    const std::string &required_privilege) const {

    // 1. 检查用户直接权限：首先查找直接授予给该用户的特定权限。
    PermissionKey user_key{username, database, table, required_privilege};
    auto it = permission_matrix_.find(user_key);
    if (it != permission_matrix_.end() && it->second.has_permission) {
        return true;
    }

    // 2. 检查用户角色的权限：获取用户当前激活的角色，并查找该角色拥有的特定权限。
    std::string user_role = GetUserCurrentRole(username); // 获取用户当前激活的角色
    if (!user_role.empty()) {
        PermissionKey role_key{user_role, database, table, required_privilege};
        auto role_it = permission_matrix_.find(role_key);
        if (role_it != permission_matrix_.end() && role_it->second.has_permission) {
            return true;
        }
    }
    }
    // WHY: 在基于角色的访问控制（RBAC）模型中，权限不仅可以直接授予用户或其当前角色，
    // 还可以通过角色继承机制获得。如果一个角色没有直接的权限，它可能通过其父角色继承了该权限。
    // WHAT: 扩展`CheckPermissionInMatrix`的逻辑，使其在用户当前激活角色没有直接权限时，
    // 向上遍历该角色的父角色链，检查这些父角色是否拥有所需权限。
    // HOW: 可以使用广度优先搜索（BFS）或深度优先搜索（DFS）算法来遍历角色继承图。
    // 从用户的当前激活角色开始，将其所有父角色添加到待检查队列/堆栈中，并确保不重复访问已检查过的角色，
    // 直到找到权限或遍历完所有可继承的父角色。
    // TODO(#UM-017-IMPL): CheckPermissionInMatrix需要扩展以支持角色继承链的检查。
    // 即，如果用户角色没有权限，需要向上检查父角色是否有权限。

    // 3. 检查通配符权限 (*): 支持数据库级别和全局级别的通配符权限。
    // 3a. 用户直接的数据库级别通配符权限 (例如: GRANT SELECT ON database.* TO user)。
    PermissionKey db_wildcard_key{username, database, "*", required_privilege};
    auto db_it = permission_matrix_.find(db_wildcard_key);
    if (db_it != permission_matrix_.end() && db_it->second.has_permission) {
        return true;
    }

    // 3b. 角色数据库级别通配符权限。
    if (!user_role.empty()) {
        PermissionKey role_db_wildcard_key{user_role, database, "*", required_privilege};
        auto role_db_it = permission_matrix_.find(role_db_wildcard_key);
        if (role_db_it != permission_matrix_.end() && role_db_it->second.has_permission) {
            return true;
        }
    }

    // 3c. 用户直接的全局通配符权限 (例如: GRANT SELECT ON *.* TO user)。
    PermissionKey global_wildcard_key{username, "*", "*", required_privilege};
    auto global_it = permission_matrix_.find(global_wildcard_key);
    if (global_it != permission_matrix_.end() && global_it->second.has_permission) {
        return true;
    }

    // 3d. 角色全局通配符权限。
    if (!user_role.empty()) {
        PermissionKey role_global_wildcard_key{user_role, "*", "*", required_privilege};
        auto role_global_it = permission_matrix_.find(role_global_wildcard_key);
        if (role_global_it != permission_matrix_.end() && role_global_it->second.has_permission) {
            return true;
        }
    }

    return false; // 未找到任何匹配的权限。
}
/**
 * @brief 更新用户当前激活的角色。
 * @details 该方法仅更新内存中的`user_current_roles_`映射，通常由`SetCurrentRole`方法调用。
 * @param username 用户的用户名。
 * @param role_name 用户当前激活的角色名称。
 */
void UserManager::UpdateUserCurrentRole(const std::string &username,
                                        const std::string &role_name) {
    // 该方法假设调用者（如SetCurrentRole）已经持有锁，所以在此处不再加锁。
    user_current_roles_[username] = role_name;
}
// 高级权限管理方法实现
/**
 * @brief 级联撤销权限。
 * @details 当一个权限从用户或角色中撤销时，此方法会尝试级联地撤销所有依赖此权限的授权。
 * 这对于维护基于角色的访问控制（RBAC）模型的权限一致性至关重要。
 * @param grantee 被授权者名称。
 * @param database 数据库名称。
 * @param table 表名称。
 * @param privilege 权限类型。
 * @return 撤销成功返回true，否则返回false。
 */
bool UserManager::RevokePrivilegeCascade(const std::string &grantee, const std::string &database,
                                         const std::string &table, const std::string &privilege) {
    // WHY: 在RBAC模型中，权限可以通过角色继承进行传递。
    // 当一个权限被从一个父角色或用户撤销时，如果不对其子角色或用户进行级联撤销，
    // 可能会导致这些子实体仍然通过继承保留该权限，从而违反了管理者的意图，造成权限混乱或安全漏洞。
    // WHAT: 此方法旨在确保当某个权限被撤销时，所有通过直接授予或角色继承方式获得该权限的实体，
    // 都能够被相应地移除。
    // HOW:
    // 1. **撤销直接权限**: 首先尝试调用`RevokePrivilege`撤销对`grantee`的直接权限。
    // 2. **角色级联**: 如果`grantee`是一个角色：
    //    a. 找出所有直接或间接继承自`grantee`的子角色（通过`GetRoleHierarchy`）。
    //    b. 遍历这些子角色，并尝试从它们那里撤销相同的权限。这里`RevokePrivilege`会再次被调用，
    //       它会处理权限是否确实存在，因此即使子角色没有该权限，调用也不会失败。
    // 3. **用户角色级联**: 如果`grantee`是一个用户：
    //    a. 找出该用户当前激活的角色。
    //    b. 尝试从该用户的角色中撤销相同的权限。
    // 注意：`RevokePrivilegeCascade`中的错误处理需要细致，因为子级联撤销可能失败，
    // 但不应影响主撤销的状态，除非是核心错误。

    std::lock_guard<std::mutex> lock(mutex_);

    // 撤销直接权限
    bool direct_revoked = RevokePrivilege(grantee, database, table, privilege);
    if (!direct_revoked && GetLastError() != "Privilege not found") {
        return false;
    }

    // 如果是角色，级联撤销所有子角色的权限
    auto role_it = roles_.find(grantee);
    if (role_it != roles_.end()) {
        std::vector<std::string> child_roles = GetRoleHierarchy(grantee);
        for (const auto &child_role : child_roles) {
            // 撤销子角色的权限（不改变错误状态，因为可能某些子角色没有该权限）
            RevokePrivilege(child_role, database, table, privilege);
        }
    }

    // 如果是用户，撤销其所有角色的权限
    auto user_it = users_.find(grantee);
    if (user_it != users_.end()) {
        std::string user_role = user_it->second.role;
        if (!user_role.empty()) {
            RevokePrivilege(user_role, database, table, privilege);
        }
    }

    last_error_.clear();
    return true;
}

bool UserManager::CheckPermissionConflict(const std::string &grantee, const std::string &database,
                                          const std::string &table, const std::string &privilege) const {
    // WHY: 在授予新权限之前，检查是否存在冲突或重复授予是权限管理中的一个最佳实践。
    // 这可以防止系统状态的不一致，避免冗余数据，并确保权限模型的清晰性。
    // WHAT: 此函数检查给定的授权请求（`grantee`在`database`和`table`上拥有`privilege`）
    // 是否与当前已存在的权限记录发生冲突。当前的简化实现主要检测完全相同的重复授权。
    // HOW:
    // 1.  **遍历现有权限**: 遍历`permissions_`向量中的所有权限记录。
    // 2.  **精确匹配**: 对于每一条记录，检查其`grantee`, `database`, `table`, `privilege`是否与传入参数完全匹配。
    // 3.  **返回冲突**: 如果找到完全匹配的权限，则认为存在冲突并返回`true`。

    std::lock_guard<std::mutex> lock(mutex_);

    // 检查是否已经存在相同的权限
    for (const auto &perm : permissions_) {
        if (perm.grantee == grantee && perm.database == database &&
            perm.table == table && perm.privilege == privilege) {
            // TODO(#UM-018): 权限冲突检查应更加精细。
            // 例如，如果已经授予了`ALL`权限，则再授予`SELECT`就不算冲突。
            // 或者，需要考虑不同粒度（全局、数据库级、表级）权限之间的覆盖关系。
            return true; // 权限冲突
        }
    }

    return false; // 无冲突
}

bool UserManager::AuditPermissionChanges(const std::string &operation, const std::string &grantee,
                                         const std::string &details) {
    // WHY: 审计权限变更在任何数据库系统中都是一项关键的安全实践。
    // 它提供了对谁、何时、如何修改了安全策略的历史记录。这对于满足合规性要求、
    // 进行安全事件的法医分析以及调试意外的访问问题至关重要。
    // WHAT: 记录与权限相关的操作（例如，授予或撤销权限），捕获操作类型、受影响的实体和详细信息。
    // HOW:
    // 1.  **当前简化实现**: 目前，它仅将审计信息打印到标准输出（控制台）。
    // 2.  **生产环境实现**: 在生产数据库系统中，审计信息不会直接打印到控制台。
    //     相反，它们会被写入到一个专门的、不可篡改的审计日志文件或日志服务中。
    //     这通常会通过`SystemDatabase`或独立的审计模块来完成，以确保日志的完整性和安全性。
    //     审计日志应包含时间戳、执行操作的用户、操作类型、受影响的对象等关键信息。
    std::lock_guard<std::mutex> lock(mutex_);

    // 记录权限变更审计信息
    // 简化实现，实际应该写入审计日志
    std::cout << "[AUDIT] " << GetCurrentTimeString()
              << " Operation: " << operation
              << " Grantee: " << grantee
              << " Details: " << details << std::endl;

    // 如果有SystemDatabase，可以同步审计信息
    if (sys_db_) {
        // TODO(#UM-019): 调用SystemDatabase的审计记录方法，将审计信息持久化。
        // sys_db_->RecordAuditLog(operation, grantee, details);
    }

    return true;
}

std::vector<std::string> UserManager::GetEffectivePermissions(const std::string &username,
                                                              const std::string &database,
                                                              const std::string &table) const {
    // WHY: 在复杂的RBAC（基于角色的访问控制）系统中，用户的权限并非仅仅是直接授予的权限。
    // 它是一个聚合的结果，包括用户直接获得的权限、通过其当前激活的角色获得的权限，
    // 以及通过角色继承链从父角色继承的权限。理解用户的“有效权限”对于安全策略的分析、
    // 权限审计以及诊断访问拒绝问题至关重要。
    // WHAT: 计算并返回指定用户在特定数据库和表上的所有实际可用的权限集合。
    // HOW:
    // 1.  **用户存在性与活跃状态检查**: 首先验证用户是否存在且处于活跃状态。
    // 2.  **超级用户特权**: 如果用户是超级用户，则直接授予所有最高权限，因为超级用户拥有完全控制权。
    // 3.  **收集唯一权限**: 使用`std::unordered_set`来收集权限字符串，自动处理重复权限，确保最终结果是唯一的。
    // 4.  **直接用户权限**: 遍历`permissions_`列表，将直接授予给该用户的权限添加到集合中。
    // 5.  **当前角色权限**: 获取用户的当前激活角色。遍历`permissions_`列表，将直接授予给该角色的权限添加到集合中。
    // 6.  **角色继承权限**: 如果启用了角色继承，则获取当前角色的所有父角色（通过`parent_roles`或`GetRoleHierarchy`），
    //     并遍历这些父角色直接拥有的权限，将其也添加到集合中。
    // 7.  **通配符处理**: 权限检查还需考虑通配符权限（例如，`ALL`，数据库或表级别的`*`），这些权限可以涵盖更广的范围。
    // 8.  **返回结果**: 将`std::unordered_set`中的权限转换为`std::vector`返回。

    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> effective_permissions;

    auto user_it = users_.find(username);
    if (user_it == users_.end() || !user_it->second.is_active) {
        return effective_permissions;
    }

    // 超级用户拥有所有权限
    if (user_it->second.role == ROLE_SUPERUSER) {
        effective_permissions = {PRIVILEGE_CREATE, PRIVILEGE_SELECT, PRIVILEGE_INSERT,
                                PRIVILEGE_UPDATE, PRIVILEGE_DELETE, PRIVILEGE_DROP,
                                PRIVILEGE_ALTER, PRIVILEGE_ALL}; // 超级用户拥有所有权限
        return effective_permissions;
    }

    // 检查用户直接权限
    std::string user_current_role = GetUserCurrentRole(username);
    std::unordered_set<std::string> unique_permissions;

    // 用户直接权限
    for (const auto &perm : permissions_) {
        if (perm.grantee == username && !perm.is_role &&
            (perm.database == database || perm.database == UserManager::PRIVILEGE_ALL) && // 考虑数据库通配符
            (perm.table == table || perm.table == UserManager::PRIVILEGE_ALL)) { // 考虑表通配符
            unique_permissions.insert(perm.privilege);
        }
    }

    // 用户角色权限
    if (!user_current_role.empty()) {
        // 获取当前激活角色及其所有祖先角色的权限
        std::unordered_set<std::string> roles_to_check;
        roles_to_check.insert(user_current_role); // 包括当前角色自身

        // TODO: (#UM-020): 应该遍历`GetRoleHierarchy`（祖先角色）而不是`parent_roles`。
        // `parent_roles`只包含直接父级，而`GetRoleHierarchy`会提供完整的继承链。
        // 这是`CheckPermissionInMatrix`中`TODO(#UM-017)`的类似问题，需要全面支持角色继承。
        
        // 简化实现：仅检查当前角色
        for (const auto &role_to_check : roles_to_check) {
            for (const auto &perm : permissions_) {
                if (perm.grantee == role_to_check && perm.is_role &&
                    (perm.database == database || perm.database == UserManager::PRIVILEGE_ALL) &&
                    (perm.table == table || perm.table == UserManager::PRIVILEGE_ALL)) {
                    unique_permissions.insert(perm.privilege);
                }
            }
        }
    }

    effective_permissions.assign(unique_permissions.begin(), unique_permissions.end());
    return effective_permissions;
}} // namespace sqlcc
