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
    if (username == "superuser") { // TODO(#UM-005): 超级用户的名称应从配置中读取，而非硬编码。
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
    it->second.password_hash = HashPassword(new_password); // TODO(#UM-007): 需要完整实现安全哈希算法

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
    if (!IsValidRole(new_role)) { // TODO(#UM-008): IsValidRole的实现需要考虑角色继承和递归检查
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
    if (it->second.password_hash != HashPassword(password)) { // TODO(#UM-007): 需要完整实现安全哈希算法
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
    if (role_name == ROLE_SUPERUSER || role_name == ROLE_ADMIN || role_name == ROLE_USER) { // TODO(#UM-009): 系统角色名称应从配置中读取，而非硬编码。
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

    // TODO(#UM-011): 需要更新所有用户对该角色的引用，以及权限矩阵中对该角色的所有引用。

    last_error_.clear(); // 清除之前的错误信息
    // 4. 尝试持久化数据。
    return SaveToFileInternal(); // TODO(#UM-004): 需要完整实现此方法
}
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
    // TODO(#UM-002): 需要完整实现从文件加载用户、角色和权限数据的逻辑。
    // 这将涉及反序列化操作。
    // 简化的实现
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
    // 1. 检查并创建默认的"superuser"用户。
    // TODO(#UM-005): 超级用户的名称和密码应从配置中读取，而非硬编码。
    if (users_.find("superuser") == users_.end()) {
        // 直接调用内部方法创建，避免外部接口的权限检查。
        User user;
        user.username = "superuser";
        user.password_hash = HashPassword("superuser_password"); // TODO(#UM-007): 默认密码也应安全配置
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
    // TODO(#UM-003): 在这里调用GrantAllPrivilegesToSuperuser("superuser")来赋予权限。
}
/**
 * @brief 获取当前时间的格式化字符串。
 * @return 包含当前日期和时间的字符串。
 */
std::string UserManager::GetCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    return std::ctime(&time_t); // std::ctime返回的字符串包含换行符，可能需要进一步处理
}
/**
 * @brief 为指定的超级用户授予所有权限。
 * @details 这是一个简化的实现，在生产环境中，需要根据权限常量为超级用户授予对所有数据库和表的全部权限。
 * @param username 超级用户的用户名。
 */
void UserManager::GrantAllPrivilegesToSuperuser(const std::string &username) {
    // TODO(#UM-003): 需要完整实现此方法，为超级用户授予对所有现有和未来数据库、表的所有权限。
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

    // TODO(#UM-006): 在移除用户权限后，需要更新权限矩阵`permission_matrix_`。
    // 这可能需要遍历`permission_matrix_`并移除所有与该用户相关的条目，或重建矩阵。
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

    // TODO(#UM-010): 在移除角色权限后，需要更新权限矩阵`permission_matrix_`。
    // 这可能需要遍历`permission_matrix_`并移除所有与该角色相关的条目，或重建矩阵。
    // 此外，还需要考虑级联撤销所有继承自该角色的权限。
}
/**
 * @brief 检查角色名称是否合法且存在。
 * @details 该方法目前仅检查角色是否存在于内存的`roles_`映射中。
 * @param role_name 待检查的角色名称。
 * @return 如果角色存在返回true，否则返回false。
 */
bool UserManager::IsValidRole(const std::string &role_name) const {
    // TODO(#UM-008): IsValidRole的实现需要考虑更复杂的合法性检查，
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
    // TODO(#UM-007): 需要完整实现安全的密码哈希算法 (如 bcrypt, Argon2, PBKDF2)。
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
    // TODO(#UM-004): 需要完整实现此方法，将所有用户、角色和权限数据序列化并写入文件。
    // 这将涉及选择一种持久化格式（如JSON, Protocol Buffers或自定义二进制格式）。
    // 简化的保存实现
    return true;
}
/**
 * @brief 初始化内存中的权限矩阵`permission_matrix_`。
 * @details 遍历`permissions_`列表，将所有权限记录加载到哈希表中，以便进行快速查找。
 * 在`LoadFromFile()`之后或权限列表变更后调用。
 */
void UserManager::InitializePermissionMatrix() {
    // TODO(#UM-016): 遍历`permissions_`列表，将所有权限记录加载到哈希表中，以便进行快速查找。
    // 这将涉及为每个权限构造PermissionKey和PermissionValue，并填充permission_matrix_。
    // 初始化权限矩阵
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
    // TODO(#UM-017): CheckPermissionInMatrix需要扩展以支持角色继承链的检查。
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
bool UserManager::GrantRoleToRole(const std::string &parent_role, const std::string &child_role) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 检查父角色是否存在
    if (roles_.find(parent_role) == roles_.end()) {
        last_error_ = "Parent role does not exist: " + parent_role;
        return false;
    }

    // 检查子角色是否存在
    if (roles_.find(child_role) == roles_.end()) {
        last_error_ = "Child role does not exist: " + child_role;
        return false;
    }

    // 检查循环依赖（简化实现）
    if (parent_role == child_role) {
        last_error_ = "Cannot grant role to itself";
        return false;
    }

    // 建立角色继承关系
    roles_[parent_role].child_roles.push_back(child_role);
    roles_[child_role].parent_roles.push_back(parent_role);

    last_error_.clear();
    return SaveToFileInternal();
}

bool UserManager::RevokeRoleFromRole(const std::string &parent_role, const std::string &child_role) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 检查父角色是否存在
    if (roles_.find(parent_role) == roles_.end()) {
        last_error_ = "Parent role does not exist: " + parent_role;
        return false;
    }

    // 检查子角色是否存在
    if (roles_.find(child_role) == roles_.end()) {
        last_error_ = "Child role does not exist: " + child_role;
        return false;
    }

    // 撤销角色继承关系
    auto &parent_children = roles_[parent_role].child_roles;
    auto &child_parents = roles_[child_role].parent_roles;

    parent_children.erase(
        std::remove(parent_children.begin(), parent_children.end(), child_role),
        parent_children.end());

    child_parents.erase(
        std::remove(child_parents.begin(), child_parents.end(), parent_role),
        child_parents.end());

    last_error_.clear();
    return SaveToFileInternal();
}

bool UserManager::CheckRoleInheritance(const std::string &role_name, const std::string &inherited_role) const {
    std::lock_guard<std::mutex> lock(mutex_);

    if (role_name == inherited_role) {
        return true;
    }

    // 广度优先搜索检查继承关系
    std::unordered_set<std::string> visited;
    std::queue<std::string> to_visit;

    to_visit.push(role_name);
    visited.insert(role_name);

    while (!to_visit.empty()) {
        std::string current = to_visit.front();
        to_visit.pop();

        auto it = roles_.find(current);
        if (it != roles_.end()) {
            // 检查直接子角色
            for (const auto &child : it->second.child_roles) {
                if (child == inherited_role) {
                    return true;
                }
                if (visited.find(child) == visited.end()) {
                    visited.insert(child);
                    to_visit.push(child);
                }
            }
        }
    }

    return false;
}

std::vector<std::string> UserManager::GetRoleHierarchy(const std::string &role_name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> hierarchy;

    auto it = roles_.find(role_name);
    if (it == roles_.end()) {
        return hierarchy;
    }

    // 广度优先遍历获取所有子角色
    std::unordered_set<std::string> visited;
    std::queue<std::string> to_visit;

    to_visit.push(role_name);
    visited.insert(role_name);

    while (!to_visit.empty()) {
        std::string current = to_visit.front();
        to_visit.pop();

        auto role_it = roles_.find(current);
        if (role_it != roles_.end()) {
            for (const auto &child : role_it->second.child_roles) {
                if (visited.find(child) == visited.end()) {
                    visited.insert(child);
                    hierarchy.push_back(child);
                    to_visit.push(child);
                }
            }
        }
    }

    return hierarchy;
}

bool UserManager::RevokePrivilegeCascade(const std::string &grantee, const std::string &database,
                                         const std::string &table, const std::string &privilege) {
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
    std::lock_guard<std::mutex> lock(mutex_);

    // 检查是否已经存在相同的权限
    for (const auto &perm : permissions_) {
        if (perm.grantee == grantee && perm.database == database &&
            perm.table == table && perm.privilege == privilege) {
            return true; // 权限冲突
        }
    }

    return false; // 无冲突
}

bool UserManager::AuditPermissionChanges(const std::string &operation, const std::string &grantee,
                                         const std::string &details) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 记录权限变更审计信息
    // 简化实现，实际应该写入审计日志
    std::cout << "[AUDIT] " << GetCurrentTimeString()
              << " Operation: " << operation
              << " Grantee: " << grantee
              << " Details: " << details << std::endl;

    // 如果有SystemDatabase，可以同步审计信息
    if (sys_db_) {
        // 这里可以调用SystemDatabase的审计记录方法
        // sys_db_->RecordAuditLog(operation, grantee, details);
    }

    return true;
}

std::vector<std::string> UserManager::GetEffectivePermissions(const std::string &username,
                                                              const std::string &database,
                                                              const std::string &table) const {
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
                                PRIVILEGE_ALTER};
        return effective_permissions;
    }

    // 检查用户直接权限
    std::string user_current_role = GetUserCurrentRole(username);
    std::unordered_set<std::string> unique_permissions;

    // 用户直接权限
    for (const auto &perm : permissions_) {
        if (perm.grantee == username && !perm.is_role &&
            perm.database == database && perm.table == table) {
            unique_permissions.insert(perm.privilege);
        }
    }

    // 用户角色权限
    if (!user_current_role.empty()) {
        for (const auto &perm : permissions_) {
            if (perm.grantee == user_current_role && perm.is_role &&
                perm.database == database && perm.table == table) {
                unique_permissions.insert(perm.privilege);
            }
        }

        // 检查角色继承权限
        std::vector<std::string> parent_roles;
        auto role_it = roles_.find(user_current_role);
        if (role_it != roles_.end()) {
            parent_roles = role_it->second.parent_roles;
        }

        for (const auto &parent_role : parent_roles) {
            for (const auto &perm : permissions_) {
                if (perm.grantee == parent_role && perm.is_role &&
                    perm.database == database && perm.table == table) {
                    unique_permissions.insert(perm.privilege);
                }
            }
        }
    }

    effective_permissions.assign(unique_permissions.begin(), unique_permissions.end());
    return effective_permissions;
}} // namespace sqlcc
