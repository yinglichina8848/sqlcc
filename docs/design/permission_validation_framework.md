# Permission Validation Framework Design Document

**Document Version**: 1.0  
**Last Updated**: 2026-01-31  
**Author**: Gemini AI Agent  
**Related Files**: `src/core/permission_validator.h`, `src/core/permission_validator.cpp`, `src/core/user_manager.h`

---

## 1. WHY: 为什么要设计权限验证框架？

在多用户数据库管理系统（DBMS）中，**权限管理**（Authorization）是核心安全机制之一。它决定了哪些用户可以对哪些数据库对象执行哪些操作。一个健壮的权限框架必须解决以下问题：

1.  **安全隔离 (Security Isolation)**：防止未经授权的用户访问或修改数据。这是数据库安全的基石。
2.  **职责分离 (Separation of Concerns)**：权限检查逻辑不应与业务逻辑（例如，查询执行、表创建）混淆在一起。如果权限逻辑散布在代码库的各个角落，将导致：
    *   **难以维护**: 任何权限规则的变更都需要修改大量文件。
    *   **易出错**: 容易在某个地方遗漏权限检查，造成安全漏洞。
    *   **难以测试**: 权限逻辑与业务逻辑耦合，测试变得复杂。
3.  **可扩展性 (Extensibility)**：数据库权限模型可能会随着需求变化而演进（例如，从简单的基于角色的访问控制 RBAC 到更复杂的基于属性的访问控制 ABAC）。框架应支持方便地扩展新的权限规则和检查机制。
4.  **一致性 (Consistency)**：所有权限检查必须遵循统一的接口和规范，以确保整个系统权限行为的一致性。
5.  **可配置性 (Configurability)**：某些权限规则可能需要外部配置（例如，是否启用某个特性需要管理员权限）。

为了高效且安全地管理这些方面，我们需要一个解耦、集中且可扩展的权限验证框架。

---

## 2. WHAT: 权限验证框架的核心功能和组件？

本权限验证框架通过采用 **回调机制 (Callback Mechanism)** 实现解耦，并提供统一的验证入口。

### 2.1. 核心组件

1.  **`PermissionOperation` (权限操作枚举)**：
    *   **职责**: 定义所有需要进行权限检查的抽象操作类型。这些操作是**语义化**的，而非直接映射到 SQL 语句关键词（例如，`SELECT` 是操作，而不是 `SELECT * FROM ...` 这条具体的 SQL 语句）。
    *   **优点**: 提供了一致的、系统级的操作视图，避免了字符串匹配或特定 SQL 语句解析的复杂性。

2.  **`PermissionContext` (权限上下文结构体)**：
    *   **职责**: 一个数据传输对象（DTO），封装了进行权限检查所需的所有必要信息。
    *   **包含信息**: 当前用户、当前数据库、目标资源（例如，表名、用户名）以及正在执行的 `PermissionOperation`。
    *   **优点**: 将所有相关上下文打包传递，使权限检查函数签名简洁，且易于扩展。

3.  **`PermissionResult` (权限验证结果结构体)**：
    *   **职责**: 封装权限检查的结果，提供清晰的 "允许/拒绝" 决策。
    *   **包含信息**: 一个布尔值 `allowed` (是否允许)、一个 `message` (解释结果的人类可读信息)，以及一个 `ErrorInfo` 对象 (在拒绝时提供详细错误)。
    *   **优点**: 标准化的结果格式，方便调用方处理决策并向用户反馈。提供工厂方法 `createAllowed()`, `createDenied()` 等，简化结果创建。

4.  **`PermissionCheckCallback` (权限检查回调类型)**：
    *   **职责**: 定义了一个函数签名，用于实现详细、业务逻辑相关的权限检查。
    *   **类型**: `std::function<PermissionResult(const PermissionContext&)>`。
    *   **优点**: 这是实现解耦的关键。核心 `PermissionValidator` 不知道具体的权限规则，而是调用这个回调函数来获取最终决策。

5.  **`PermissionValidator` (权限验证器)**：
    *   **职责**: 整个权限框架的中央入口和协调者。
    *   **依赖**: 持有一个 `UserManager` 的共享指针，用于查询用户和角色的基本信息（例如，用户是否存在，是否为管理员）。
    *   **核心方法 `validate()`**: 接收操作类型和上下文信息。它首先执行一些基础的、通用权限检查（例如，管理员具有所有权限），然后**委托**给通过 `setPermissionCheckCallback` 注册的业务逻辑回调函数进行详细检查。
    *   **默认/简化规则**: 在没有注册详细回调函数时，出于安全考虑，默认拒绝所有复杂权限操作。
    *   **辅助方法**: 提供 `operationToPrivilege` 和 `operationToResourceType` 等静态方法，将抽象操作映射到实际的权限字符串和资源类型，以便与持久化存储的权限元数据进行交互。

### 2.2. 权限流概念

-   **管理员优先 (Admin Override)**：管理员用户通常被赋予最高权限，可以绕过详细的权限检查。
-   **安全失败 (Fail-Safe)**：如果没有明确的规则允许某个操作，或者权限检查机制未完全配置，则默认拒绝操作。这是一种安全的设计策略。

---

## 3. HOW: 权限验证框架的工作流程和实现细节？

### 3.1. 典型工作流程 (Typical Workflow)

1.  **系统启动和初始化**:
    *   `UserManager` 实例被创建，用于管理用户、角色和权限元数据（假设这些数据存储在系统表中）。
    *   `PermissionValidator` 实例被创建，并传入 `UserManager` 的共享指针。
    *   **关键步骤**: 业务逻辑层（例如，`SQLExecutor` 或其他负责执行 SQL 语句的模块）会实现一个具体的权限检查函数，并将其通过 `PermissionValidator::setPermissionCheckCallback()` 注册到 `PermissionValidator` 中。这个回调函数会包含查询系统权限表（如 `sys_privileges`）的逻辑。

2.  **权限检查请求**:
    *   当数据库的某个组件（例如，SQL 解析器完成语句解析后，或执行引擎准备访问表之前）需要验证用户是否有权限执行某个操作时，它会调用 `PermissionValidator::validate()` 方法：
        ```cpp
        // 示例：在SQLExecutor中调用
        PermissionResult result = permission_validator_->validate(
            PermissionOperation::SELECT, // 操作类型
            "my_table",                 // 资源 (例如表名)
            current_user,               // 当前用户
            current_database            // 当前数据库
        );

        if (!result.allowed) {
            // 权限不足，抛出异常或返回错误信息
            throw PermissionException(result.message);
        }
        // 继续执行操作
        ```

3.  **`PermissionValidator::validate()` 内部流程**:
    *   **获取上下文**: `validate` 方法首先会根据传入的 `current_user` 和 `current_database` 以及 `default_user_`/`default_database_` 来确定真实的 `user` 和 `database`。
    *   **基础权限检查 (`validateBasicPermissions`)**:
        *   **管理员检查**: 首先判断当前用户是否为管理员 (`isAdmin(user)` )。如果是管理员，则立即返回 `createAllowed()`。这是最高优先级的权限。
        *   **数据库上下文检查**: 对于需要特定数据库上下文的操作（例如 `CREATE TABLE`），检查是否已选择数据库。如果未选择，则返回 `createDenied()`。
        *   **默认拒绝**: 如果基础检查没有明确允许，则返回一个临时的 `createDenied()` 结果，指示需要进行更详细的检查。
    *   **委托给回调 (`validateWithCallback`)**:
        *   `validate` 方法会构建一个 `PermissionContext` 对象，其中包含 `user`, `database`, `resource`, `operation` 等信息。
        *   如果 `permission_callback_` 已注册，则调用该回调函数，并将 `PermissionContext` 传递给它。回调函数会执行业务逻辑层面的权限检查（例如，查询 `sys_privileges` 表，检查用户/角色是否有对 `resource` 的 `privilege`）。
        *   如果 `permission_callback_` 未注册，或者回调函数返回 `false`，则 `validateWithCallback` 返回一个拒绝结果。
    *   **返回最终结果**: `validate` 方法将 `validateWithCallback` 的结果返回给最初的调用方。

### 3.2. 实现要点

1.  **`std::shared_ptr<UserManager>`**: `PermissionValidator` 依赖 `UserManager` 来获取用户和角色的信息。通过 `shared_ptr` 进行管理，确保 `UserManager` 在 `PermissionValidator` 生命周期内有效。
2.  **`std::function<PermissionResult(const PermissionContext&)>`**: 这是一个强大的 C++11 特性，允许我们将任何可调用对象（函数指针、lambda、函数对象）作为回调函数存储和调用。这使得 `PermissionValidator` 能够与任何实现 `PermissionCheckCallback` 签名的具体权限检查逻辑解耦。
3.  **职责分层**:
    *   `PermissionValidator` 负责通用的流程控制和安全策略（如管理员绕过，默认拒绝）。
    *   `PermissionCheckCallback` 负责具体的、随业务逻辑变化的权限规则实现。
4.  **安全默认值**: 框架设计上倾向于“安全失败”。这意味着如果权限系统配置不完整或无法做出明确允许的决策，则默认采取拒绝的态度，以避免安全漏洞。
5.  **辅助映射函数**: `operationToPrivilege` 和 `operationToResourceType` 静态方法提供了一种方便的方式，将内部的 `PermissionOperation` 枚举转换为外部可识别的字符串，这对于与权限元数据表（通常以字符串形式存储权限名称和资源类型）交互至关重要。

### 3.3. 简化的类图

```mermaid
classDiagram
    class UserManager {
        +userExists(username): bool
        +isUserInRole(username, role): bool
        --
        // ... manages user/role metadata
    }

    class PermissionValidator {
        -std::shared_ptr<UserManager> user_manager_
        -PermissionCheckCallback permission_callback_
        -std::string default_user_
        -std::string default_database_
        +PermissionValidator(UserManager*)
        +setPermissionCheckCallback(callback): void
        +validate(op, res, user, db): PermissionResult
        +userExists(username): bool
        +isAdmin(username): bool
        +static operationToPrivilege(op): string
        +static operationToResourceType(op): string
        -validateBasicPermissions(...): PermissionResult
        -validateWithCallback(context): PermissionResult
    }

    class PermissionContext {
        +user: string
        +database: string
        +resource: string
        +operation: PermissionOperation
    }

    class PermissionResult {
        +allowed: bool
        +message: string
        +error_info: ErrorInfo
        +static createAllowed(): PermissionResult
        +static createDenied(reason): PermissionResult
    }

    enum PermissionOperation {
        CREATE_DATABASE
        DROP_DATABASE
        SELECT
        INSERT
        // ... many more ...
    }
    
    PermissionValidator "1" -- "1" UserManager : uses
    PermissionValidator "1" --> "1" PermissionCheckCallback : holds
    PermissionCheckCallback ..> PermissionContext : uses
    PermissionCheckCallback ..> PermissionResult : returns
    PermissionValidator ..> PermissionContext : creates
    PermissionValidator ..> PermissionResult : returns
    PermissionContext ..> PermissionOperation : contains
```

---

## 4. 总结

本权限验证框架的核心在于其**解耦性**和**可扩展性**。通过将通用的验证流程与具体业务权限规则分离，并利用回调函数进行桥接，系统能够灵活适应不断变化的权限需求，同时保持核心代码的简洁和稳定。这种设计模式对于维护大型、多功能数据库系统的安全性至关重要，并使得学生更容易理解权限管理的复杂性，以及如何在实际系统中实现职责分离。
