# UserManager Design Document

**Document Version**: 1.0  
**Last Updated**: 2026-01-31  
**Author**: Gemini AI Agent  
**Related Files**: `src/core/user_manager.h`, `src/core/user_manager.cpp`, `src/core/permission_validator.h`

---

## 1. WHY: 为什么要设计 UserManager？

在任何多用户数据库管理系统（DBMS）中，**安全性**是核心功能之一。`UserManager` 是实现数据安全和完整性的基石，它提供了一套机制来管理用户身份、定义其权限，并控制他们对数据库资源的访问。如果没有 `UserManager`，数据库将无法区分不同用户的身份和授权级别，导致以下严重问题：

1.  **数据安全漏洞**: 任何人都可以访问、修改甚至删除敏感数据，造成数据泄露或破坏。
2.  **职责混淆**: 无法根据用户的角色或职责来限制其操作范围，增加误操作和恶意行为的风险。
3.  **合规性问题**: 无法满足行业规范和法律法规对数据访问控制和审计的要求。
4.  **审计追踪缺失**: 无法追踪特定操作是由哪个用户执行的，给故障排查和安全分析带来困难。
5.  **系统不稳定**: 未经授权的用户可能执行资源密集型操作，导致系统性能下降甚至崩溃。

`UserManager` 旨在提供一个集中、安全、高效且可扩展的解决方案，以应对这些挑战，确保数据库的访问控制能够精细化管理。

---

## 2. WHAT: UserManager 的核心功能和组件？

`UserManager` 实现了一个**基于角色的访问控制 (RBAC)** 模型，并提供了用户认证和授权的核心服务。

### 2.1. 核心概念

*   **用户 (User)**: 数据库的最终使用者，通过用户名进行标识，并拥有一个密码哈希用于认证。用户被分配一个主角色，并可以在会话期间切换其“当前激活角色”。
*   **角色 (Role)**: 一组命名权限的集合。权限被授予给角色，而不是直接授予给用户。用户通过被分配角色来获得权限。角色可以继承其他角色的权限，形成一个层级结构。
*   **权限 (Privilege)**: 允许用户或角色在特定数据库对象（如数据库、表）上执行特定操作的授权（如 SELECT, INSERT, UPDATE, DELETE, CREATE 等）。
*   **权限矩阵 (Permission Matrix)**: 一个内存中的高效数据结构，用于快速查找用户或角色在特定资源上的权限。

### 2.2. 核心组件

1.  **`User` (用户数据结构)**：
    *   **字段**: `username`, `password_hash`, `role` (主角色), `current_role` (当前激活角色), `is_active`, `created_at`。
    *   **职责**: 封装单个用户的基本信息和状态。

2.  **`Role` (角色数据结构)**：
    *   **字段**: `role_name`, `created_at`, `parent_roles` (父角色列表), `child_roles` (子角色列表)。
    *   **职责**: 封装角色的名称和角色继承关系。

3.  **`Permission` (权限数据结构)**：
    *   **字段**: `grantee` (被授权者，可以是用户名或角色名), `database`, `table`, `privilege`, `is_role` (标识 `grantee` 是用户还是角色)。
    *   **职责**: 记录一个具体的授权条目。

4.  **`PermissionKey` / `PermissionKeyHash` (权限矩阵键及其哈希函数)**：
    *   **职责**: `PermissionKey` 是用于在 `std::unordered_map` 中唯一标识一个权限条目的复合键（`grantee`, `database`, `table`, `privilege`）。`PermissionKeyHash` 提供了 `PermissionKey` 的哈希函数。
    *   **优点**: 使得权限矩阵的查找操作具有 O(1) 的平均时间复杂度。

5.  **`UserManager` (用户管理器类)**：
    *   **成员变量**:
        *   `users_`: `std::unordered_map<string, User>`，存储所有用户信息。
        *   `roles_`: `std::unordered_map<string, Role>`，存储所有角色信息。
        *   `permissions_`: `std::vector<Permission>`，原始的权限列表。
        *   `permission_matrix_`: `std::unordered_map<PermissionKey, PermissionValue, PermissionKeyHash>`，内存中的权限矩阵，用于快速检查。
        *   `user_current_roles_`: `UserRoleMap`，存储用户当前激活的角色。
        *   `sys_db_`: `std::shared_ptr<SystemDatabase>`，与 SystemDatabase 交互以进行持久化。
        *   `mutex_`: `mutable std::mutex`，保护内部数据结构，确保线程安全。
    *   **主要功能**:
        *   **用户管理**: `CreateUser`, `DropUser`, `AlterUserPassword`, `AlterUserRole`, `AuthenticateUser`。
        *   **角色管理**: `CreateRole`, `DropRole`, `AlterRole`, `SetCurrentRole`, `GetUserCurrentRole`。
        *   **权限授予/撤销**: `GrantPrivilege`, `RevokePrivilege`。
        *   **权限检查**: `CheckPermission` (高级别检查), `CheckPermissionInMatrix` (底层矩阵检查)。
        *   **高级权限管理**: `GrantRoleToRole` (角色继承), `RevokePrivilegeCascade`, `GetEffectivePermissions` (获取有效权限)。
        *   **持久化**: `SaveToFile`, `LoadFromFile`。
        *   **审计**: `AuditPermissionChanges`。

### 2.3. 核心原则和特性

*   **RBAC 模型**: 权限赋给角色，用户赋给角色，简化权限管理。
*   **密码安全**: 存储密码哈希而非明文（`HashPassword`）。
*   **线程安全**: 所有修改共享状态的操作都通过 `std::mutex` 保护。
*   **内存缓存**: 权限矩阵 (`permission_matrix_`) 提供 O(1) 平均时间复杂度的权限查找。
*   **可持久化**: 用户、角色和权限数据可以持久化到文件或数据库。

---

## 3. HOW: UserManager 的工作流程和实现细节？

### 3.1. 初始化和默认配置

1.  **构造函数**: `UserManager(const std::string &data_path)`
    *   设置数据文件路径 `data_path_`。
    *   **`TODO(#UM-002)`**: 实际应尝试从 `data_path_` 加载现有数据。
    *   调用 `CreateDefaultSuperuser()` 创建默认的“superuser”用户和系统角色 (`SUPERUSER`, `ADMIN`, `USER`)。
    *   调用 `InitializePermissionMatrix()` 初始化内存中的权限矩阵。

### 3.2. 用户认证 (`AuthenticateUser`)

1.  **查找用户**: 检查用户名是否存在于 `users_` 映射中。
2.  **检查活跃状态**: 确认用户 `is_active`。
3.  **密码验证**: 将提供的明文密码通过 `HashPassword()` 转换为哈希值，与存储的 `password_hash` 进行比较。
4.  **`TODO(#UM-007)`**: `HashPassword()` 当前为简化实现，生产环境需使用安全的密码哈希算法（如 bcrypt）。

### 3.3. 权限检查 (`CheckPermission`, `CheckPermissionInMatrix`)

这是授权的核心流程，`UserManager` 提供了两层检查：

1.  **高级别检查 (`CheckPermission`)**:
    *   **用户验证**: 检查用户是否存在且活跃。
    *   **超级用户特权**: 如果用户角色为 `ROLE_SUPERUSER`，直接授予所有权限。
    *   **委托给矩阵**: 将详细检查委托给 `CheckPermissionInMatrix`。
2.  **底层矩阵检查 (`CheckPermissionInMatrix`)**:
    *   **直接权限**: 检查用户是否直接拥有所需的精确权限。
    *   **角色权限**: 获取用户当前激活的角色 (`GetUserCurrentRole`)，检查该角色是否拥有权限。
    *   **角色继承**: **`TODO(#UM-017)`**: 当前实现尚未包含完整的角色继承链检查。在更完善的系统中，如果用户直接角色没有权限，需要递归检查其父角色是否拥有权限。
    *   **通配符匹配**: 支持 `database.*` 和 `*.*` 等通配符权限匹配，允许授予数据库级或全局级权限。

### 3.4. 用户和角色管理 (`CreateUser`, `DropUser`, `CreateRole`, `DropRole`, `AlterUserPassword`, `AlterUserRole`, `AlterRole`, `SetCurrentRole`)

这些方法均遵循类似模式：
1.  **锁定**: 通过 `std::lock_guard<std::mutex>` 保护 `users_`, `roles_` 等共享数据结构。
2.  **校验**: 检查用户/角色是否存在、合法性，或新名称是否冲突。
3.  **操作**: 执行实际的增、删、改操作。
4.  **持久化**: **`TODO(#UM-004)`**: 操作成功后，调用 `SaveToFileInternal()` 尝试将变更持久化到磁盘。

### 3.5. 角色继承 (`GrantRoleToRole`, `RevokeRoleFromRole`, `CheckRoleInheritance`, `GetRoleHierarchy`)

*   `GrantRoleToRole` 和 `RevokeRoleFromRole` 允许建立和断开角色间的父子关系，构建角色继承层级。
*   `CheckRoleInheritance` 和 `GetRoleHierarchy` 则用于查询角色的继承关系链。

### 3.6. 持久化机制 (`SaveToFile`, `LoadFromFile`, `SaveToFileInternal`)

*   **`SaveToFile()`**: 公共接口，加锁后调用 `SaveToFileInternal()`。
*   **`LoadFromFile()`**: 公共接口，加锁后从文件加载数据。
*   **`SaveToFileInternal()`**: **`TODO(#UM-004)`**: 简化实现，需完整实现将内存中的 `users_`, `roles_`, `permissions_` 数据序列化并写入文件。
*   **`LoadFromFile()`**: **`TODO(#UM-002)`**: 简化实现，需完整实现从文件反序列化数据。

### 3.7. 简化的类图

```mermaid
classDiagram
    class User {
        +username: string
        +password_hash: string
        +role: string
        +current_role: string
        +is_active: bool
        +created_at: string
    }

    class Role {
        +role_name: string
        +created_at: string
        +parent_roles: vector<string>
        +child_roles: vector<string>
    }

    class Permission {
        +grantee: string
        +database: string
        +table: string
        +privilege: string
        +is_role: bool
    }

    class PermissionKey {
        +grantee: string
        +database: string
        +table: string
        +privilege: string
        +operator==(): bool
    }
    struct PermissionKeyHash {
        +operator()(): size_t
    }
    class PermissionValue {
        +has_permission: bool
        +is_role: bool
    }

    class UserManager {
        -users_: unordered_map<string, User>
        -roles_: unordered_map<string, Role>
        -permissions_: vector<Permission>
        -permission_matrix_: unordered_map<PermissionKey, PermissionValue, PermissionKeyHash>
        -user_current_roles_: UserRoleMap
        -last_error_: mutable string
        -data_path_: string
        -sys_db_: shared_ptr<SystemDatabase>
        -mutex_: mutable mutex
        
        +UserManager(data_path)
        +~UserManager()
        +CreateUser(...): bool
        +DropUser(...): bool
        +AlterUserPassword(...): bool
        +AlterUserRole(...): bool
        +AuthenticateUser(...): bool
        +CreateRole(...): bool
        +DropRole(...): bool
        +AlterRole(...): bool
        +SetCurrentRole(...): bool
        +GetUserCurrentRole(...): string
        +GrantPrivilege(...): bool
        +RevokePrivilege(...): bool
        +CheckPermission(...): bool
        +ListUsers(): vector<User>
        +ListRoles(): vector<Role>
        +SaveToFile(): bool
        +LoadFromFile(): bool
        -CreateDefaultSuperuser(): void
        -HashPassword(password): string
        -SaveToFileInternal(): bool
        -InitializePermissionMatrix(): void
        -AddPermissionToMatrix(perm): void
        -CheckPermissionInMatrix(...): bool
        // ... many other methods
    }

    UserManager "1" *-- "N" User
    UserManager "1" *-- "N" Role
    UserManager "1" *-- "N" Permission
    UserManager "1" *-- "1" SystemDatabase : uses_for_persistence
    UserManager --> PermissionKeyHash : uses
    UserManager --> PermissionKey : uses
    UserManager --> PermissionValue : uses
```

---

## 4. 总结与 TODO 列表

`UserManager` 是 SQLCC 数据库安全的核心。本设计文档及其对应的代码注释详细阐述了其 RBAC 模型、认证授权机制和实现细节。

**当前版本的关键 TODO 列表：**

*   **数据持久化 (`#UM-002`, `#UM-004`)**: `UserManager` 的 `LoadFromFile()` 和 `SaveToFileInternal()` 仅为简化实现，需要完整实现用户、角色和权限数据的序列化与反序列化，选择合适的持久化格式（如 JSON）。
*   **`CreateDefaultSuperuser` (`#UM-003`, `#UM-005`, `#UM-007`)**: `CreateDefaultSuperuser()` 的实现需要完善，包括：
    *   `#UM-005`: 超级用户名称和默认密码应从配置中读取，而非硬编码。
    *   `#UM-007`: 确保 `HashPassword()` 采用安全的密码哈希算法。
    *   `#UM-003`: 完整实现 `GrantAllPrivilegesToSuperuser()` 为超级用户授予所有权限。
*   **用户/角色删除后的权限清理 (`#UM-006`, `#UM-010`)**:
    *   `#UM-006`: `RemoveUserPrivileges()` 后，需要更新 `permission_matrix_`。
    *   `#UM-010`: `RemoveRolePrivileges()` 后，需要更新 `permission_matrix_`，并考虑级联撤销继承自该角色的权限。
*   **角色名称变更后的引用更新 (`#UM-011`)**: `AlterRole()` 变更角色名称后，需要更新所有用户对该角色的引用以及权限矩阵中对该角色的引用。
*   **`IsValidRole` 复杂检查 (`#UM-008`)**: `IsValidRole()` 需要扩展以包含更复杂的合法性检查（如系统预留角色、继承关系）。
*   **系统角色硬编码 (`#UM-009`)**: 系统角色名称（如 `SUPERUSER`）应从配置中读取，避免硬编码。
*   **权限矩阵初始化 (`#UM-016`)**: `InitializePermissionMatrix()` 需要完整实现加载`permissions_`到`permission_matrix_`的逻辑。
*   **`CheckPermissionInMatrix` 角色继承 (`#UM-017`)**: `CheckPermissionInMatrix()` 需要扩展以支持角色继承链的检查。

未来的工作将集中在完成上述 TODO 项，并将其与 `SystemDatabase` 深度集成，实现一个功能完善、安全可靠的用户和权限管理系统。
