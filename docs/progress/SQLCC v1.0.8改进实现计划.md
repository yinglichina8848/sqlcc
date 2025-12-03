# SQLCC v1.0.8改进实现计划

## 1. 计划概述

本计划旨在解决SQLCC v1.0.7评估报告中提到的高优先级问题，包括实际权限验证集成和复杂查询功能实现。计划将分阶段实施，确保每个改进都经过充分测试和验证。

## 2. 改进内容和优先级

### 2.1 高优先级（P0）

#### 2.1.1 实际权限验证集成
- **问题**：当前权限检查默认允许，需要集成UserManager的实际权限验证逻辑
- **范围**：
  - 完善PermissionValidator的checkUserPermission方法
  - 确保ExecutionContext正确传递用户信息
  - 在执行引擎中集成权限验证
  - 实现用户存在性检查

#### 2.1.2 复杂查询功能基础架构
- **问题**：缺失窗口函数、CTE、递归查询等复杂查询功能
- **范围**：
  - 完善集合操作执行器
  - 实现基本的JOIN操作执行
  - 扩展AST支持复杂查询

### 2.2 中优先级（P1）

#### 2.2.1 高级JOIN支持
- **问题**：缺失FULL OUTER JOIN、CROSS JOIN、NATURAL JOIN和复杂ON条件支持
- **范围**：
  - 扩展Parser支持高级JOIN语法
  - 实现JOIN执行逻辑
  - 支持复杂ON条件

#### 2.2.2 子查询增强
- **问题**：缺失相关子查询、EXISTS/NOT EXISTS、IN/ANY/ALL和标量子查询支持
- **范围**：
  - 扩展Parser支持子查询语法
  - 实现子查询执行逻辑
  - 支持相关子查询

#### 2.2.3 聚合和分组增强
- **问题**：缺失HAVING子句、GROUPING SETS、ROLLUP/CUBE和窗口聚合支持
- **范围**：
  - 实现HAVING子句执行
  - 扩展聚合函数支持
  - 实现窗口函数基础架构

## 3. 技术实现方案

### 3.1 实际权限验证集成

#### 3.1.1 完善PermissionValidator
- **修改文件**：`src/permission_validator.cpp`
- **实现内容**：
  - 完善`checkUserPermission`方法，确保正确调用`UserManager::CheckPermission`
  - 实现用户存在性检查
  - 添加权限继承和角色权限处理

#### 3.1.2 集成到执行引擎
- **修改文件**：`src/execution_engine.cpp`, `src/unified_executor.cpp`
- **实现内容**：
  - 在执行前添加权限验证步骤
  - 确保ExecutionContext包含正确的用户信息
  - 添加权限验证失败的错误处理

### 3.2 集合操作执行器实现

#### 3.2.1 完成集合操作执行器
- **修改文件**：`src/execution/set_operation_executor.cpp`
- **实现内容**：
  - 完成UNION操作执行逻辑
  - 实现INTERSECT操作执行逻辑
  - 实现EXCEPT操作执行逻辑
  - 支持ALL修饰符

#### 3.2.2 集成到执行引擎
- **修改文件**：`src/unified_executor.cpp`
- **实现内容**：
  - 在执行引擎中添加集合操作执行分支
  - 支持CompositeSelectStatement执行

### 3.3 高级JOIN支持

#### 3.3.1 扩展AST支持JOIN
- **修改文件**：`include/sql_parser/advanced_ast.h`
- **实现内容**：
  - 添加JoinNode类，支持多种JOIN类型
  - 扩展SelectStatement支持JOIN子句
  - 支持复杂ON条件表达式

#### 3.3.2 扩展Parser支持JOIN语法
- **修改文件**：`src/sql_parser/parser.cpp`
- **实现内容**：
  - 扩展parseSelectStatement支持JOIN子句
  - 实现不同JOIN类型的解析
  - 支持复杂ON条件解析

#### 3.3.3 实现JOIN执行逻辑
- **修改文件**：`src/execution/join_executor.cpp`
- **实现内容**：
  - 实现Nested Loop JOIN算法
  - 支持INNER JOIN、LEFT JOIN、RIGHT JOIN、FULL JOIN、CROSS JOIN和NATURAL JOIN
  - 支持复杂ON条件求值

### 3.4 子查询增强

#### 3.4.1 扩展AST支持子查询
- **修改文件**：`include/sql_parser/advanced_ast.h`
- **实现内容**：
  - 添加SubqueryNode类
  - 支持相关子查询引用
  - 支持EXISTS/NOT EXISTS表达式

#### 3.4.2 扩展Parser支持子查询语法
- **修改文件**：`src/sql_parser/parser.cpp`
- **实现内容**：
  - 扩展parseExpression支持子查询
  - 实现IN/ANY/ALL子查询解析
  - 支持EXISTS/NOT EXISTS子查询解析

#### 3.4.3 实现子查询执行逻辑
- **修改文件**：`src/execution/subquery_executor.cpp`
- **实现内容**：
  - 实现子查询执行器
  - 支持相关子查询执行
  - 实现IN/ANY/ALL子查询求值
  - 实现EXISTS/NOT EXISTS子查询求值

### 3.5 窗口函数和高级聚合支持

#### 3.5.1 扩展AST支持窗口函数
- **修改文件**：`include/sql_parser/advanced_ast.h`
- **实现内容**：
  - 添加WindowFunctionNode类
  - 支持窗口规范定义
  - 支持窗口聚合函数

#### 3.5.2 实现窗口函数执行逻辑
- **修改文件**：`src/execution/window_function_executor.cpp`
- **实现内容**：
  - 实现窗口函数执行器
  - 支持ROW_NUMBER(), RANK(), DENSE_RANK()等排名函数
  - 支持窗口聚合函数
  - 实现窗口帧处理

#### 3.5.3 实现HAVING子句执行
- **修改文件**：`src/unified_executor.cpp`
- **实现内容**：
  - 在SELECT执行中添加HAVING子句处理
  - 支持GROUPING SETS、ROLLUP/CUBE

## 4. 测试计划

### 4.1 单元测试
- **权限验证测试**：测试不同用户权限场景
- **JOIN操作测试**：测试各种JOIN类型和复杂ON条件
- **子查询测试**：测试不同类型的子查询
- **集合操作测试**：测试UNION、INTERSECT、EXCEPT操作
- **窗口函数测试**：测试各种窗口函数和窗口规范

### 4.2 集成测试
- **复杂查询集成测试**：测试包含多种复杂功能的查询
- **权限验证集成测试**：测试权限验证在实际执行中的表现
- **性能测试**：测试复杂查询的性能表现

### 4.3 回归测试
- 确保现有功能不受影响
- 运行所有现有测试用例

## 5. 实现步骤

### 5.1 阶段一：实际权限验证集成（1-2周）
1. 完善PermissionValidator
2. 集成权限验证到执行引擎
3. 编写权限验证测试用例

### 5.2 阶段二：集合操作执行器实现（2-3周）
1. 完成集合操作执行器
2. 集成到执行引擎
3. 编写集合操作测试用例

### 5.3 阶段三：高级JOIN支持（2-3周）
1. 扩展AST支持JOIN
2. 扩展Parser支持JOIN语法
3. 实现JOIN执行逻辑
4. 编写JOIN测试用例

### 5.4 阶段四：子查询增强（3-4周）
1. 扩展AST支持子查询
2. 扩展Parser支持子查询语法
3. 实现子查询执行逻辑
4. 编写子查询测试用例

### 5.5 阶段五：窗口函数和高级聚合（3-4周）
1. 扩展AST支持窗口函数
2. 实现窗口函数执行逻辑
3. 实现HAVING子句和高级聚合
4. 编写窗口函数和聚合测试用例

## 6. 预期成果

### 6.1 功能成果
- ✅ 实际权限验证集成，支持UserManager的实际权限验证
- ✅ 完整的集合操作支持（UNION, INTERSECT, EXCEPT）
- ✅ 高级JOIN支持（FULL OUTER JOIN, CROSS JOIN, NATURAL JOIN, 复杂ON条件）
- ✅ 子查询增强（相关子查询, EXISTS/NOT EXISTS, IN/ANY/ALL, 标量子查询）
- ✅ 窗口函数支持（ROW_NUMBER(), RANK(), DENSE_RANK()等）
- ✅ 高级聚合支持（HAVING子句, GROUPING SETS, ROLLUP/CUBE）

### 6.2 技术成果
- 扩展的AST结构，支持复杂查询
- 完善的执行引擎，支持各种复杂查询执行
- 集成的权限验证机制
- 全面的测试套件

## 7. 风险评估和缓解措施

### 7.1 技术风险
- **复杂查询执行性能**：通过优化执行计划和添加索引支持缓解
- **权限验证性能影响**：通过缓存权限信息缓解
- **AST结构复杂度**：通过模块化设计和清晰的类层次结构缓解

### 7.2 项目管理风险
- **实现周期延长**：通过分阶段实施和优先处理核心功能缓解
- **测试覆盖不足**：通过自动化测试和持续集成缓解
- **代码质量下降**：通过代码审查和静态分析缓解

## 8. 后续版本规划

在完成v1.0.8版本的改进后，后续版本将重点关注：
- 事务隔离级别扩展
- 性能优化（锁粒度优化、MVCC支持）
- 查询优化器增强
- 数据库恢复机制完善

## 9. 结论

本计划提供了一个全面的框架来解决SQLCC v1.0.7评估报告中提到的高优先级问题。通过分阶段实施，确保每个改进都经过充分测试和验证，最终实现一个功能更完善、性能更优的SQL执行引擎。