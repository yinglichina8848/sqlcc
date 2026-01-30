# SQLCC v1.3.8 Level3-6 头文件迁移重构 TODO

**创建时间**: 2026年1月30日
**版本**: v1.3.8
**目标**: 将 include/ 目录下的 level3-6 组件头文件迁移到 src/ 对应目录

## 📋 迁移范围

基于 v1.3.7 头文件迁移策略分析报告，本次迁移涵盖以下组件：

- **Level3**: transaction_manager, transaction
- **Level4**: sql_executor, execution_ast, execution_engine
- **Level5**: network
- **Level6**: monitoring, security, procedure, trigger, types

## 🎯 迁移任务清单

### Phase 1: Level3 - Transaction Manager (优先级: 高)

#### 任务 1.1: 迁移 transaction_manager.h
- [ ] 检查备份文件: `/home/liying/sqlcc/backups/include_deprecated_20260130_053625/transaction_manager.h`
- [ ] 检查目标目录: `/home/liying/sqlcc/src/transaction_manager/`
- [ ] 复制头文件到目标目录
- [ ] 验证头文件内容正确性

#### 任务 1.2: 迁移 transaction_context.h
- [ ] 检查备份文件: `/home/liying/sqlcc/backups/include_deprecated_20260130_053625/transaction_context.h`
- [ ] 检查目标目录: `/home/liying/sqlcc/src/transaction/`
- [ ] 复制头文件到目标目录
- [ ] 验证头文件内容正确性

#### 任务 1.3: 迁移 transaction_context_impl.h
- [ ] 检查备份文件: `/home/liying/sqlcc/backups/include_deprecated_20260130_053625/transaction_context_impl.h`
- [ ] 检查目标目录: `/home/liying/sqlcc/src/transaction/`
- [ ] 复制头文件到目标目录
- [ ] 验证头文件内容正确性

#### 任务 1.4: 更新 Level3 组件的 #include 路径
- [ ] 搜索所有引用 `transaction_manager.h` 的文件
- [ ] 更新为 `#include "src/transaction_manager/transaction_manager.h"`
- [ ] 搜索所有引用 `transaction_context.h` 的文件
- [ ] 更新为 `#include "src/transaction/transaction_context.h"`
- [ ] 搜索所有引用 `transaction_context_impl.h` 的文件
- [ ] 更新为 `#include "src/transaction/transaction_context_impl.h"`

#### 任务 1.5: 更新 Level3 组件的 BUILD.bazel
- [ ] 检查 `/home/liying/sqlcc/src/transaction_manager/BUILD.bazel`
- [ ] 更新头文件导出路径
- [ ] 检查 `/home/liying/sqlcc/src/transaction/BUILD.bazel`
- [ ] 更新头文件导出路径

#### 任务 1.6: 验证 Level3 组件编译
- [ ] 运行 `bazel build //src/transaction_manager:...`
- [ ] 运行 `bazel build //src/transaction:...`
- [ ] 运行 `bazel test //tests/level3_transaction_manager:...`
- [ ] 修复编译错误（如果有）

---

### Phase 2: Level4 - SQL Processing (优先级: 高)

#### 任务 2.1: 迁移 sql_executor.h
- [ ] 检查备份文件: `/home/liying/sqlcc/backups/include_deprecated_20260130_053625/sql_executor.h`
- [ ] 检查目标目录: `/home/liying/sqlcc/src/sql_executor/`
- [ ] 复制头文件到目标目录
- [ ] 验证头文件内容正确性

#### 任务 2.2: 对比 sql_executor/ 目录下的头文件
- [ ] 对比备份中的 sql_executor/*.h 与 src/sql_executor/*.h
- [ ] 确定需要更新的文件
- [ ] 处理文件差异（合并/替换/保留）

#### 任务 2.3: 验证 execution_ast/ast_interface.h
- [ ] 检查备份文件: `/home/liying/sqlcc/backups/include_deprecated_20260130_053625/execution_ast/ast_interface.h`
- [ ] 检查目标文件: `/home/liying/sqlcc/src/execution_ast/ast_interface.h`
- [ ] 对比文件内容，确定是否需要更新

#### 任务 2.4: 迁移 execution_engine.h
- [ ] 检查备份文件: `/home/liying/sqlcc/backups/include_deprecated_20260130_053625/execution_engine.h`
- [ ] 检查目标目录: `/home/liying/sqlcc/src/execution/`
- [ ] 复制头文件到目标目录
- [ ] 验证头文件内容正确性

#### 任务 2.5: 更新 Level4 组件的 #include 路径
- [ ] 搜索所有引用 `sql_executor.h` 的文件
- [ ] 更新为 `#include "src/sql_executor/sql_executor.h"`
- [ ] 搜索所有引用 `execution_engine.h` 的文件
- [ ] 更新为 `#include "src/execution/execution_engine.h"`
- [ ] 搜索所有引用 `execution_ast/ast_interface.h` 的文件
- [ ] 更新为 `#include "src/execution_ast/ast_interface.h"`

#### 任务 2.6: 更新 Level4 组件的 BUILD.bazel
- [ ] 检查 `/home/liying/sqlcc/src/sql_executor/BUILD.bazel`
- [ ] 更新头文件导出路径
- [ ] 检查 `/home/liying/sqlcc/src/execution_ast/BUILD.bazel`
- [ ] 更新头文件导出路径
- [ ] 检查 `/home/liying/sqlcc/src/execution/BUILD.bazel`
- [ ] 更新头文件导出路径

#### 任务 2.7: 验证 Level4 组件编译
- [ ] 运行 `bazel build //src/sql_executor:...`
- [ ] 运行 `bazel build //src/execution_ast:...`
- [ ] 运行 `bazel build //src/execution:...`
- [ ] 运行 `bazel test //tests/level4_sql_processing:...`
- [ ] 修复编译错误（如果有）

---

### Phase 3: Level5 - Network (优先级: 中)

#### 任务 3.1: 对比 network/ 目录下的头文件
- [ ] 列出备份中的所有 network/*.h 文件
- [ ] 列出 src/network/ 中的所有 .h 文件
- [ ] 对比差异，确定需要迁移的文件

#### 任务 3.2: 迁移缺失的 network 头文件
- [ ] 复制所有备份中存在但 src/network/ 中不存在的头文件
- [ ] 验证头文件内容正确性

#### 任务 3.3: 更新 Level5 组件的 #include 路径
- [ ] 搜索所有引用 network 头文件的文件
- [ ] 更新为 `#include "src/network/xxx.h"`
- [ ] 确保所有路径正确

#### 任务 3.4: 更新 Level5 组件的 BUILD.bazel
- [ ] 检查 `/home/liying/sqlcc/src/network/BUILD.bazel`
- [ ] 更新头文件导出路径
- [ ] 更新依赖关系

#### 任务 3.5: 验证 Level5 组件编译
- [ ] 运行 `bazel build //src/network:...`
- [ ] 运行 `bazel test //tests/level5_network:...`
- [ ] 修复编译错误（如果有）

---

### Phase 4: Level6 - Enterprise & Integration (优先级: 中)

#### 任务 4.1: 迁移 monitoring/ 目录
- [ ] 检查备份文件: `/home/liying/sqlcc/backups/include_deprecated_20260130_053625/monitoring/*`
- [ ] 检查目标目录: `/home/liying/sqlcc/src/monitoring/`
- [ ] 复制所有头文件到目标目录
- [ ] 验证头文件内容正确性

#### 任务 4.2: 迁移 security/ 目录
- [ ] 检查备份文件: `/home/liying/sqlcc/backups/include_deprecated_20260130_053625/security/*`
- [ ] 检查目标目录: `/home/liying/sqlcc/src/security/`
- [ ] 对比现有文件，确定需要更新的文件
- [ ] 复制缺失的头文件
- [ ] 验证头文件内容正确性

#### 任务 4.3: 迁移 procedure/ 目录
- [ ] 检查备份文件: `/home/liying/sqlcc/backups/include_deprecated_20260130_053625/procedure/*`
- [ ] 检查目标目录: `/home/liying/sqlcc/src/procedure/`
- [ ] 复制所有头文件到目标目录
- [ ] 验证头文件内容正确性

#### 任务 4.4: 验证 trigger/ 目录
- [ ] 检查备份文件: `/home/liying/sqlcc/backups/include_deprecated_20260130_053625/trigger/*`
- [ ] 检查目标文件: `/home/liying/sqlcc/src/trigger/*`
- [ ] 对比文件内容，确定是否需要更新

#### 任务 4.5: 验证 types/ 目录
- [ ] 检查备份文件: `/home/liying/sqlcc/backups/include_deprecated_20260130_053625/types/*`
- [ ] 检查目标文件: `/home/liying/sqlcc/src/types/*`
- [ ] 对比文件内容，确定是否需要更新

#### 任务 4.6: 更新 Level6 组件的 #include 路径
- [ ] 搜索所有引用 monitoring 头文件的文件
- [ ] 更新为 `#include "src/monitoring/xxx.h"`
- [ ] 搜索所有引用 security 头文件的文件
- [ ] 更新为 `#include "src/security/xxx.h"`
- [ ] 搜索所有引用 procedure 头文件的文件
- [ ] 更新为 `#include "src/procedure/xxx.h"`
- [ ] 搜索所有引用 trigger 头文件的文件
- [ ] 更新为 `#include "src/trigger/xxx.h"`
- [ ] 搜索所有引用 types 头文件的文件
- [ ] 更新为 `#include "src/types/xxx.h"`

#### 任务 4.7: 更新 Level6 组件的 BUILD.bazel
- [ ] 检查 `/home/liying/sqlcc/src/monitoring/BUILD.bazel`
- [ ] 更新头文件导出路径
- [ ] 检查 `/home/liying/sqlcc/src/security/BUILD.bazel`
- [ ] 更新头文件导出路径
- [ ] 检查 `/home/liying/sqlcc/src/procedure/BUILD.bazel`
- [ ] 更新头文件导出路径
- [ ] 检查 `/home/liying/sqlcc/src/trigger/BUILD.bazel`
- [ ] 更新头文件导出路径
- [ ] 检查 `/home/liying/sqlcc/src/types/BUILD.bazel`
- [ ] 更新头文件导出路径

#### 任务 4.8: 验证 Level6 组件编译
- [ ] 运行 `bazel build //src/monitoring:...`
- [ ] 运行 `bazel build //src/security:...`
- [ ] 运行 `bazel build //src/procedure:...`
- [ ] 运行 `bazel build //src/trigger:...`
- [ ] 运行 `bazel build //src/types:...`
- [ ] 运行 `bazel test //tests/level6_enterprise:...`
- [ ] 运行 `bazel test //tests/level6_integration:...`
- [ ] 修复编译错误（如果有）

---

### Phase 5: 全局验证与清理 (优先级: 高)

#### 任务 5.0: 确认 include 目录状态
- [x] 检查 include/ 目录是否已移动
- [x] 验证备份目录完整性
- [x] 更新文档记录

#### 任务 5.1: 全局编译验证
- [ ] 运行 `bazel build //src/...`
- [ ] 修复所有编译错误
- [ ] 确保零编译错误
- [ ] 确保零编译警告

#### 任务 5.2: 全局测试验证
- [ ] 运行 `bazel test //tests/level3_transaction_manager/...`
- [ ] 运行 `bazel test //tests/level4_sql_processing/...`
- [ ] 运行 `bazel test //tests/level5_network/...`
- [ ] 运行 `bazel test //tests/level6_enterprise/...`
- [ ] 运行 `bazel test //tests/level6_integration/...`
- [ ] 修复所有测试失败

#### 任务 5.3: 清理备份目录
- [ ] 验证所有头文件已正确迁移
- [ ] 检查是否有遗留的 include/ 引用
- [ ] 清理临时文件

#### 任务 5.4: 文档更新
- [x] 更新迁移完成报告
- [x] 记录所有更改
- [ ] 创建 Git commit

---

## 📊 风险评估

### 高风险项
1. **循环依赖**: 可能出现循环依赖问题
   - 缓解措施: 使用 `bazel query` 分析依赖关系

2. **编译错误**: 大量的 #include 路径更新可能导致编译错误
   - 缓解措施: 逐个 Phase 验证，及时修复

### 中风险项
1. **文件冲突**: 备份中的文件可能与现有文件冲突
   - 缓解措施: 对比文件内容，选择最新版本

2. **BUILD.bazel 错误**: BUILD 文件更新可能引入错误
   - 缓解措施: 仔细检查依赖关系

---

## ✅ 验收标准

### 编译验证
- ✅ 零编译错误
- ✅ 零编译警告
- ✅ 构建时间不增加 >20%

### 功能验证
- ✅ Level3 单元测试通过
- ✅ Level4 单元测试通过
- ✅ Level5 单元测试通过
- ✅ Level6 单元测试通过

### 结构验证
- ✅ 所有 level3-6 头文件已迁移到 src/ 目录
- ✅ 所有 #include 路径已更新
- ✅ 所有 BUILD.bazel 文件已更新

---

## 📝 备注

- 本次迁移基于 v1.3.7 头文件迁移策略分析报告
- 遵循 4-Phase 迁移策略
- 每个 Phase 完成后必须验证编译和测试
- 如遇重大问题，及时回滚到上一个稳定状态

---

*TODO 创建时间: 2026年1月30日*
*预计完成时间: 2-3个工作日*