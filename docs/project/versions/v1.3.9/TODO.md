# TODO v1.3.9 - 测试覆盖率改进

**版本**: 1.3.9  
**创建日期**: 2026-01-30  
**目标**: 提升测试覆盖率至 80%，修复编译错误，实现核心功能测试

---

## 🎯 版本目标

- **主要目标**: 将测试覆盖率从 45% 提升至 80%
- **次要目标**: 修复所有编译错误，使编译通过率达到 100%
- **长期目标**: 实现所有 Level 1-6 的完整测试覆盖

---

## ⚠️ 核心约束（必须严格遵守）

### 头文件包含规范（绝对禁止违反）
**规则**: 绝对不允许引用 src/ 目录以外的头文件！

**具体要求**:
- ✅ 头文件必须与源文件在 src/ 同级目录
- ✅ 使用相对路径引用同级目录的头文件（如 `../core/`）
- ✅ 同一目录内的头文件使用无路径包含（如 `"database_manager.h"`）
- ❌ 禁止引用 `backups/`、`include/` 等其他目录的头文件
- ❌ 禁止使用 `../../` 引用父目录以上的头文件
- ❌ 禁止引用项目外的头文件

**违反后果**:
- 编译错误（找不到头文件）
- 违反项目规范
- 影响构建稳定性

**Commit**: `a8cd25f6` - 严格遵守 include 规范，移除对 src/ 外部头文件的引用

---

## 📋 任务清单

### P0 - 立即执行（1-2周）已完成任务

#### 任务 1: 修复循环依赖、统一 TableMetadata 类型、删除 CMake 配置 ✅ 2026-01-30
- [x] 修复核心模块循环依赖问题
  - 从 `src/core/BUILD.bazel` 移除对 `src/sql_executor:sql_executor` 的依赖
  - 在 `src/execution/BUILD.bazel` 添加对 `src:error_handler` 的依赖
  - 修复头文件包含路径（3个文件）
- [x] 统一 TableMetadata 类型定义
  - 将所有 `class TableMetadata;` 改为 `struct TableMetadata;`（8个文件）
  - 消除编译警告
- [x] 清理遗留构建配置
  - 删除所有 CMakeLists.txt 文件（9个）
  - 项目统一使用 Bazel 构建系统

**Commit**: `8d512fd8` - P0 任务 - 修复循环依赖、统一 TableMetadata 类型、删除 CMake 配置
**Commit**: `f546fa77` - 修复 error_handler.cpp 头文件包含路径

#### 任务 2: 合并 Level 2 Core 和 Level 2 Core Services 测试目录 ✅ 2026-01-30

#### 任务 2: 合并 Level 2 Core 和 Level 2 Core Services 测试目录 ✅ 2026-01-30
- [x] 创建备份目录: `tests/level2_core_backup_20260130/`
- [x] 删除重复测试文件 (6个)
- [x] 删除 Mock 测试目录 (mocks/)
- [x] 移动保留测试到 core_services (4个文件)
- [x] 新建 permission_validator 测试
- [x] 更新 BUILD.bazel 配置 (5个文件)
- [x] 清空 level2_core 目录

**产出文档**:
- `LEVEL2_CORE_MERGE_REPORT.md` - 详细分析报告
- `LEVEL2_CORE_MERGE_EXECUTION_REPORT.md` - 执行报告

**统计**:
- 删除文件: 6个
- 移动文件: 4个
- 新建文件: 3个
- 净减少: -29% 文件数

---

### P0 - 立即执行（1-2周）待完成任务

#### 修复核心模块依赖问题 ✅ 已完成 2026-01-30
- [x] 从 `src/core/BUILD.bazel` 移除循环依赖（移除 `//src/sql_executor:sql_executor`）
- [x] 在 `src/execution/BUILD.bazel` 添加 `//src:error_handler` 依赖
- [x] 修复 `src/core/permission_validator.cpp` 的头文件包含路径
- [x] 修复 `src/core/sql_executor.cpp` 的头文件包含路径
- [x] 修复 `src/core/error_handler.cpp` 的头文件包含路径

**Commit**: `8d512fd8` 和 `f546fa77`

#### 统一 TableMetadata 类型定义 ✅ 已完成 2026-01-30
- [x] 将所有 `class TableMetadata;` 改为 `struct TableMetadata;`（8个文件）
- [x] 消除编译警告

**Commit**: `8d512fd8`

#### 清理遗留构建配置 ✅ 已完成 2026-01-30
- [x] 删除所有 CMakeLists.txt 文件（9个）
- [x] 项目统一使用 Bazel 构建系统

**Commit**: `8d512fd8`

#### 验证基础测试 🟡 部分完成
- [x] ~~运行 Level 1 测试~~（发现 sql_parser::Statement API 导出问题）
- [x] ~~合并 level2_core 和 level2_core_services 测试目录~~ ✅ 2026-01-30 完成
- [x] ~~删除重复的测试文件~~ ✅ 已删除5个重复文件
- [x] ~~删除Mock方式的测试~~ ✅ 已删除 mocks/ 目录
- [x] ~~保留真实实现的测试并移动到 core_services~~ ✅ 已移动4个测试
- [x] ~~清空 level2_core 目录~~ ✅ 已清空
- [x] ~~创建新的 permission_validator 测试~~ ✅ 2026-01-30 完成
- [x] ~~更新 BUILD.bazel 配置文件~~ ✅ 2026-01-30 完成
- [x] ~~修复 Level 1 types_test 6个失败用例~~ ✅ 2026-01-31 完成
- [ ] 收集 Level 1-3 基础覆盖率数据
- [ ] 生成初步覆盖率报告
- [ ] 修复 sql_parser::Statement API 导出问题

**发现的问题**:
- `sql_parser::Statement` 类的 API 导出问题
- 这是一个更深层的架构问题，需要在后续版本中系统性地解决

---

### P1 - 短期改进（2-4周）待执行任务

#### 优先级 P1-0: 解决 sql_parser API 导出问题 🔴 高优先级
- [ ] 修复 `sql_parser::Statement` 类的 API 导出
- [ ] 确保 `unified_query_plan` 能正确访问 Statement 类
- [ ] 验证 Level 1 测试能够编译通过
- [ ] 验证 Level 2 测试能够编译通过
- [ ] 验证 Level 3 测试能够编译通过

**预计时间**: 1-2天

#### 实现 Level 4 核心功能测试

**JSON Operations 测试（18个占位符）**
- [ ] 实现 JsonValue 构造函数测试
- [ ] 实现 JsonValue 类型转换测试
- [ ] 实现 JSON 解析器测试
- [ ] 实现 JSON 查询函数测试
- [ ] 实现 JSON 聚合函数测试
- [ ] 实现 JSON 路径表达式测试
- [ ] 实现异常处理测试

**Stored Procedure 测试（16个占位符）**
- [ ] 实现存储过程创建测试
- [ ] 实现存储过程执行测试
- [ ] 实现输入参数测试
- [ ] 实现输出参数测试
- [ ] 实现返回值测试
- [ ] 实现存储过程删除测试
- [ ] 实现递归调用测试

**Trigger 测试（16个占位符）**
- [ ] 实现 BEFORE/AFTER 触发器测试
- [ ] 实现 INSERT 触发器测试
- [ ] 实现 UPDATE 触发器测试
- [ ] 实现 DELETE 触发器测试
- [ ] 实现行级触发器测试
- [ ] 实现语句级触发器测试
- ] 实现触发器链测试
- [ ] 实现递归防护测试

#### 实现边界值和路径覆盖测试
- [ ] 实现 Level 3 Transaction Boundary 测试（已创建，需修复）
- [ ] 实现 Level 4 Execution Path 测试（已创建，需修复）
- [ ] 实现 Level 5 Network Boundary 测试（已创建，部分通过）
- [ ] 实现 Level 6 Exception Scenarios 测试（已创建，需修复）

#### 重构模块架构
- [ ] 重新设计分层架构文档
- [ ] 消除 core 模块的循环依赖
- [ ] 清理 src/core 目录下的非核心文件
- [ ] 更新所有 BUILD.bazel 的依赖声明
- [ ] 添加 visibility 控制

---

### P2 - 中期改进（1-2月）

#### 实现 Level 5 网络测试

**Connection Pool 测试**
- [ ] 实现连接池创建和销毁测试
- [ ] 实现连接获取和释放测试
- [ ] 实现并发访问测试
- [ ] 实现连接超时测试
- [ ] 实现连接池大小限制测试

**Network Manager 测试**
- [ ] 实现网络启动和停止测试
- [ ] 实现客户端连接测试
- [ ] 实现消息传递测试
- [ ] 实现网络异常处理测试

**Protocol Handler 测试**
- [ ] 实现协议解析测试
- [ ] 实现协议编码测试
- [ ] 实现错误处理测试
- [ ] 实现协议版本兼容性测试

---

### P3 - 长期改进（3-6月）

#### 实现 Level 6 企业级功能

**Audit Trail 功能**
- [ ] 实现审计日志记录功能
- [ ] 实现审计日志查询功能
- [ ] 实现审计日志归档功能
- [ ] 实现合规报告生成功能
- [ ] 实现审计日志测试

**Enterprise Security 功能**
- [ ] 实现访问控制功能
- [ ] 实现数据加密功能
- [ ] 实现身份认证和授权功能
- [ ] 实现安全策略管理功能
- [ ] 实现安全测试

**Compliance Manager 功能**
- [ ] 实现策略管理功能
- [ ] 实现合规检查功能
- [ ] 实现报告生成功能
- [ ] 实现合规测试

#### 建立集成测试环境
- [ ] 搭建端到端测试环境
- [ ] 配置测试数据库
- [ ] 配置网络环境
- [ ] 配置分布式节点
- [ ] 实现端到端测试用例
- [ ] 自动化测试流程
- [ ] 集成到 CI/CD

---

## 📊 里程碑

### 短期里程碑（2026-02-13）
- [ ] 所有编译错误解决
- [ ] Level 1-3 测试能够运行
- [ ] 基础覆盖率数据收集完成
- [ ] 编译通过率达到 80%

### 中期里程碑（2026-03-30）
- [ ] Level 4 测试完成
- [ ] Level 5 测试完成
- [ ] 模块架构重构完成
- [ ] 测试覆盖率达到 65%

### 长期里程碑（2026-07-30）
- [ ] Level 6 测试完成
- [ ] 集成测试环境搭建完成
- [ ] 所有测试覆盖率目标达成
- [ ] 系统达到生产就绪状态

---

## 🔧 工具和脚本

### 自动化修复脚本
- [ ] `fix_build_dependencies.py` - 自动修复 BUILD.bazel 依赖
- [ ] `fix_table_metadata.py` - 自动统一 TableMetadata 类型
- [ ] `remove_cmake_files.sh` - 自动删除 CMakeLists.txt 文件
- [ ] `generate_coverage_report.sh` - 生成覆盖率报告

### 测试运行脚本
- [ ] `run_level1_tests.sh` - 运行 Level 1 测试
- [ ] `run_level2_tests.sh` - 运行 Level 2 测试
- [ ] `run_all_tests.sh` - 运行所有测试
- [ ] `collect_coverage.sh` - 收集覆盖率数据

---

## 📈 质量指标

### 当前状态（2026-01-30）
- 测试覆盖率: 45%
- 编译通过率: 30%
- 综合评分: 53% (C-)

### 目标状态（2026-07-30）
- 测试覆盖率: 80% (+35%)
- 编译通过率: 100% (+70%)
- 综合评分: 91% (A-) (+38%)

---

## 📝 备注

- 所有 P0 任务必须在一周内完成
- P1 任务应在 4 周内完成
- P2 和 P3 任务可以根据实际情况调整优先级
- 每完成一个里程碑，需要更新质量指标
- 所有测试必须通过 `bazel test` 验证
- 覆盖率数据使用 LLVM cov 收集