# SqlExecutor重构设计文档

## 概述

本文档描述了SqlExecutor类从传统执行器架构向统一查询计划架构的重构设计。重构旨在解决现有架构中执行器分离过度、缺少统一查询计划、错误处理不一致等问题。

## 重构背景

### 当前架构问题

1. **执行器分离过度**：DDL、DML、DCL执行器各自独立，缺乏统一的执行流程
2. **缺少统一查询计划**：每个执行器重复实现验证、权限检查等公共逻辑
3. **错误处理不一致**：不同执行器的错误处理机制不统一
4. **代码重复**：验证逻辑、权限检查等公共功能在各个执行器中重复实现

### 重构目标

1. **统一执行流程**：为所有SQL语句类型提供统一的执行流程
2. **公共逻辑抽象**：将验证、权限检查、预处理等公共逻辑抽象到统一查询计划中
3. **错误处理标准化**：提供统一的错误处理机制
4. **执行统计**：统一的执行统计和性能监控

## 新架构设计

### 统一查询计划架构

新的SqlExecutor将基于统一查询计划架构，主要包含以下组件：

#### 1. UnifiedQueryPlan基类

```cpp
class UnifiedQueryPlan {
public:
    // 公共接口
    bool buildPlan(std::unique_ptr<sql_parser::Statement> stmt);
    ExecutionResult executePlan();
    
protected:
    // 公共验证方法
    bool validateStatement();
    bool validateDatabaseContext();
    bool checkPermission(const std::string& operation, const std::string& resource);
    
    // 公共预处理方法
    bool preProcessStatement();
    bool resolveObjectReferences();
    
    // 公共后处理方法
    bool postProcessStatement();
    bool updateSystemMetadata();
    
    // 子类需要实现的特定方法
    virtual bool buildSpecificPlan() = 0;
    virtual ExecutionResult executeSpecificPlan() = 0;
};
```

#### 2. 特定查询计划子类

- **DDLQueryPlan**：处理CREATE、DROP、ALTER等DDL语句
- **DMLQueryPlan**：处理SELECT、INSERT、UPDATE、DELETE等DML语句
- **DCLQueryPlan**：处理GRANT、REVOKE等DCL语句
- **UtilityQueryPlan**：处理USE、SHOW等工具语句

#### 3. 查询计划工厂

```cpp
class QueryPlanFactory {
public:
    static std::unique_ptr<UnifiedQueryPlan> createPlan(
        std::unique_ptr<sql_parser::Statement> stmt,
        std::shared_ptr<DatabaseManager> db_manager,
        std::shared_ptr<UserManager> user_manager,
        std::shared_ptr<SystemDatabase> system_db);
};
```

### SqlExecutor重构设计

#### 重构后的SqlExecutor类定义

```cpp
class SqlExecutor {
public:
    SqlExecutor();
    SqlExecutor(std::shared_ptr<DatabaseManager> db_manager);
    ~SqlExecutor();
    
    std::string Execute(const std::string& sql);
    std::string ExecuteFile(const std::string& file_path);
    std::string GetLastError() const;
    std::string GetExecutionStats() const;

private:
    // 核心依赖组件
    std::shared_ptr<DatabaseManager> db_manager_;
    std::shared_ptr<UserManager> user_manager_;
    std::shared_ptr<SystemDatabase> system_db_;
    std::unique_ptr<PermissionValidator> permission_validator_;
    
    // 状态管理
    std::string last_error_;
    std::string execution_stats_;
    std::string current_user_;
    std::string current_database_;
    
    // 私有方法
    void SetError(const std::string& error);
    void ClearError();
    bool InitializeSystemDatabase();
    bool InitializePermissionValidator();
    std::unique_ptr<sql_parser::Statement> ParseSQL(const std::string& sql);
    std::unique_ptr<UnifiedQueryPlan> CreateQueryPlan(std::unique_ptr<sql_parser::Statement> stmt);
    void UpdateCurrentDatabase(const std::string& sql);
};
```

#### 执行流程重构

**重构前执行流程：**
```
Execute(sql)
    ↓
解析SQL → 识别语句类型 → 分发到对应执行器 → 执行器各自处理 → 返回结果
```

**重构后执行流程：**
```
Execute(sql)
    ↓
ParseSQL(sql) → CreateQueryPlan(stmt) → query_plan->executePlan() → 返回结果
    ↓
统一查询计划执行流程：
验证 → 权限检查 → 预处理 → 执行 → 后处理 → 清理
```

## 详细设计

### 1. 构造函数重构

**重构前：**
```cpp
SqlExecutor::SqlExecutor() {
    // 初始化各个执行器
    ddl_executor_ = std::make_unique<DDLExecutor>();
    dml_executor_ = std::make_unique<DMLExecutor>();
    dcl_executor_ = std::make_unique<DCLExecutor>();
    // ... 其他初始化
}
```

**重构后：**
```cpp
SqlExecutor::SqlExecutor(std::shared_ptr<DatabaseManager> db_manager) 
    : db_manager_(db_manager) {
    
    // 初始化用户管理器
    user_manager_ = std::make_shared<UserManager>();
    
    // 初始化系统数据库
    system_db_ = std::make_shared<SystemDatabase>();
    
    // 初始化权限验证器
    permission_validator_ = std::make_unique<PermissionValidator>(
        user_manager_, system_db_);
    
    // 初始化执行统计
    execution_stats_ = "";
    current_user_ = "default";
    current_database_ = "default";
    
    // 初始化系统数据库
    InitializeSystemDatabase();
}
```

### 2. Execute方法重构

**重构前：**
```cpp
std::string SqlExecutor::Execute(const std::string& sql) {
    // 解析SQL
    auto parser = sql_parser::Parser(sql);
    auto stmt = parser.Parse();
    
    // 根据语句类型分发到不同执行器
    switch (stmt->type()) {
        case sql_parser::StatementType::CREATE:
            return ddl_executor_->ExecuteCreate(
                static_cast<sql_parser::CreateStatement&>(*stmt));
        case sql_parser::StatementType::SELECT:
            return dml_executor_->ExecuteSelect(
                static_cast<sql_parser::SelectStatement&>(*stmt));
        // ... 其他语句类型
    }
}
```

**重构后：**
```cpp
std::string SqlExecutor::Execute(const std::string& sql) {
    ClearError();
    
    try {
        // 1. 解析SQL
        auto stmt = ParseSQL(sql);
        if (!stmt) {
            SetError("SQL解析失败");
            return "ERROR: " + last_error_;
        }
        
        // 2. 创建统一查询计划
        auto query_plan = CreateQueryPlan(std::move(stmt));
        if (!query_plan) {
            SetError("查询计划创建失败");
            return "ERROR: " + last_error_;
        }
        
        // 3. 执行查询计划
        auto result = query_plan->executePlan();
        
        // 4. 更新执行统计
        UpdateExecutionStats(query_plan->getExecutionStats());
        
        // 5. 处理执行结果
        if (result.success) {
            return result.message;
        } else {
            SetError(result.error_message);
            return "ERROR: " + last_error_;
        }
        
    } catch (const std::exception& e) {
        SetError("执行异常: " + std::string(e.what()));
        return "ERROR: " + last_error_;
    }
}
```

### 3. 查询计划创建方法

```cpp
std::unique_ptr<UnifiedQueryPlan> SqlExecutor::CreateQueryPlan(
    std::unique_ptr<sql_parser::Statement> stmt) {
    
    return QueryPlanFactory::createPlan(
        std::move(stmt), 
        db_manager_, 
        user_manager_, 
        system_db_);
}
```

### 4. 错误处理机制

**统一错误处理：**
```cpp
void SqlExecutor::SetError(const std::string& error) {
    last_error_ = error;
    // 记录错误日志
    if (db_manager_) {
        db_manager_->GetLogger()->Error("SqlExecutor: " + error);
    }
}

void SqlExecutor::ClearError() {
    last_error_.clear();
}
```

### 5. 执行统计机制

```cpp
void SqlExecutor::UpdateExecutionStats(const std::string& stats) {
    execution_stats_ = stats;
}

std::string SqlExecutor::GetExecutionStats() const {
    return execution_stats_;
}
```

## 迁移策略

### 阶段1：基础架构准备
1. 实现UnifiedQueryPlan基类和子类
2. 实现QueryPlanFactory
3. 更新SqlExecutor头文件定义

### 阶段2：核心方法重构
1. 重构SqlExecutor构造函数
2. 重构Execute方法
3. 实现CreateQueryPlan方法

### 阶段3：功能完善
1. 实现错误处理机制
2. 实现执行统计功能
3. 完善权限验证集成

### 阶段4：测试验证
1. 单元测试
2. 集成测试
3. 性能测试

## 预期收益

### 代码质量提升
- **减少代码重复**：公共逻辑统一实现，减少约40%的重复代码
- **提高可维护性**：统一的执行流程，便于调试和维护
- **增强可扩展性**：新增SQL语句类型只需实现特定查询计划

### 功能增强
- **统一错误处理**：所有SQL语句使用相同的错误处理机制
- **执行统计**：统一的执行性能监控
- **权限验证**：统一的权限检查机制

### 性能优化
- **减少对象创建**：避免每次执行创建多个执行器实例
- **优化执行流程**：统一的预处理和后处理逻辑

## 风险与缓解措施

### 风险1：向后兼容性
- **风险**：重构可能影响现有功能的兼容性
- **缓解**：保持公共接口不变，逐步迁移，充分测试

### 风险2：性能影响
- **风险**：统一查询计划可能增加执行开销
- **缓解**：优化查询计划构建过程，减少不必要的步骤

### 风险3：测试覆盖
- **风险**：重构可能引入新的bug
- **缓解**：完善的单元测试和集成测试，逐步验证

## 总结

SqlExecutor重构采用统一查询计划架构，解决了现有架构中的多个问题，提供了更加统一、可维护、可扩展的执行引擎。重构将显著提升代码质量，增强功能特性，为未来的功能扩展奠定坚实基础。

重构采用渐进式迁移策略，确保平滑过渡，最小化对现有功能的影响。