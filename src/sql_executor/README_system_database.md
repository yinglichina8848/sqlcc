# 系统数据库组件重构说明

## 概述

本重构将原本单一的 `system_database.cpp` 文件拆分为多个职责明确的组件，提高了代码的可维护性和可扩展性。

## 组件结构

### 1. SystemSchemaManager (系统表管理器)
- **文件**: `system_schema_manager.h`, `system_schema_manager.cpp`
- **职责**: 负责创建和管理所有系统表结构
- **主要功能**:
  - 创建系统数据库表（sys_databases, sys_users, sys_roles等）
  - 检查系统表是否存在
  - 管理系统表结构定义

### 2. SystemDataInitializer (系统数据初始化器)
- **文件**: `system_data_initializer.h`, `system_data_initializer.cpp`
- **职责**: 负责初始化系统数据库的默认数据
- **主要功能**:
  - 创建默认角色（admin, dba, user等）
  - 创建默认超级用户（root）
  - 初始化默认权限设置

### 3. SystemPermissionManager (系统权限管理器)
- **文件**: `system_permission_manager.h`, `system_permission_manager.cpp`
- **职责**: 负责用户、角色和权限的管理
- **主要功能**:
  - 用户管理（创建、删除、更新、查询）
  - 角色管理（创建、删除、查询）
  - 权限管理（授权、撤销、查询）

### 4. SystemMetadataManager (系统元数据管理器)
- **文件**: `system_metadata_manager.h`, `system_metadata_manager.cpp`
- **职责**: 负责数据库对象元数据的管理
- **主要功能**:
  - 数据库元数据管理
  - 表、列、索引、约束元数据管理
  - 视图元数据管理

### 5. SystemDataStructures (系统数据结构定义)
- **文件**: `system_data_structures.h`
- **职责**: 定义系统数据库中使用的所有数据结构
- **主要内容**:
  - SysDatabase, SysUser, SysRole等结构体定义
  - 系统表字段定义

### 6. SystemDatabase (重构后的系统数据库类)
- **文件**: `system_database_new.h`, `system_database_new.cpp`
- **职责**: 整合所有组件，提供统一的系统数据库接口
- **主要功能**:
  - 初始化系统数据库
  - 提供数据库、用户、角色、权限等操作的高级接口
  - 管理各个组件的生命周期

## 使用方法

### 初始化系统数据库

```cpp
#include "system_database_new.h"

// 创建数据库管理器
auto db_manager = std::make_shared<DatabaseManager>();

// 创建系统数据库实例
SystemDatabase system_db(db_manager);

// 初始化系统数据库
if (!system_db.Initialize()) {
    std::cerr << "Failed to initialize system database: " << system_db.GetLastError() << std::endl;
    return false;
}
```

### 创建用户和数据库

```cpp
// 创建数据库
if (!system_db.CreateDatabase("mydb", "admin", "My application database")) {
    std::cerr << "Failed to create database: " << system_db.GetLastError() << std::endl;
}

// 创建用户
if (!system_db.CreateUser("myuser", "mypassword", "user@example.com", "user")) {
    std::cerr << "Failed to create user: " << system_db.GetLastError() << std::endl;
}
```

### 权限管理

```cpp
// 授予权限
if (!system_db.GrantPrivilege("myuser", "DATABASE", "mydb", "SELECT")) {
    std::cerr << "Failed to grant privilege: " << system_db.GetLastError() << std::endl;
}

// 检查权限
if (system_db.HasPrivilege("myuser", "DATABASE", "mydb", "SELECT")) {
    std::cout << "User has SELECT privilege on mydb" << std::endl;
}
```

### 查询系统信息

```cpp
// 列出所有数据库
auto databases = system_db.ListDatabases();
for (const auto& db : databases) {
    std::cout << "Database: " << db.db_name << ", Owner: " << db.owner << std::endl;
}

// 列出所有用户
auto users = system_db.ListUsers();
for (const auto& user : users) {
    std::cout << "User: " << user.username << ", Role: " << user.role_name << std::endl;
}
```

## 编译说明

使用提供的 `CMakeLists_system_database.txt` 文件编译系统数据库组件：

```bash
# 在项目根目录下
cmake -DCMAKE_BUILD_TYPE=Release .
make system_database
```

## 设计优势

1. **单一职责原则**: 每个组件只负责一个特定的功能领域
2. **松耦合**: 组件之间通过接口交互，降低耦合度
3. **高内聚**: 相关功能集中在同一组件内
4. **易于扩展**: 新功能可以通过添加新组件或扩展现有组件实现
5. **易于测试**: 每个组件可以独立测试

## 迁移指南

从旧的 `system_database.cpp` 迁移到新的组件化结构：

1. 将原有的 `#include "system_database.h"` 替换为 `#include "system_database_new.h"`
2. 更新类名从 `SystemDatabase` 保持不变（接口兼容）
3. 如果直接使用了内部方法，可能需要通过对应的管理器访问

## 后续改进

1. 添加事务支持
2. 实现更细粒度的权限控制
3. 添加审计日志功能
4. 优化性能，特别是大量用户和权限的场景
5. 添加缓存机制，减少系统数据库访问频率