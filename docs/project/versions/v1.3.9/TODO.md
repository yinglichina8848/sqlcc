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

## 📋 任务清单

### P0 - 立即执行（1-2周）

#### 修复核心模块依赖问题
- [ ] 在 `src/core/BUILD.bazel` 中添加 `//src/sql_executor:sql_executor` 依赖
- [ ] 在 `src/core/BUILD.bazel` 中添加 `//src/permission_validator:permission_validator` 依赖
- [ ] 在 `src/core/BUILD.bazel` 中添加 `//src/error_handler:error_handler` 依赖
- [ ] 修复 `src/core/permission_validator.cpp` 的头文件包含路径
- [ ] 修复 `src/core/sql_executor.cpp` 的头文件包含路径
- [ ] 验证 Level 1-3 测试能够编译通过

#### 统一 TableMetadata 类型定义
- [ ] 在 `src/core/core_database_manager.h:21` 将 `class TableMetadata;` 改为 `struct TableMetadata;`
- [ ] 在 `src/core_backup_20260121_001034/core_database_manager.h:22` 将 `class TableMetadata;` 改为 `struct TableMetadata;`
- [ ] 检查并更新所有其他模块中的 TableMetadata 声明
- [ ] 验证编译警告消除

#### 清理遗留构建配置
- [ ] 删除 `tests/level6_integration/CMakeLists.txt`
- [ ] 删除 `tests/level2_storage_engine/CMakeLists.txt`
- [ ] 删除 `tests/level3_transaction_manager/CMakeLists.txt`
- [ ] 删除 `tests/level5_network/CMakeLists.txt`
- [ ] 删除 `tests/level6_enterprise/CMakeLists.txt`
- [ ] 删除 `tests/level4_sql_processing/CMakeLists.txt`
- [ ] 删除 `tests/level4_sql_processing/execution_context/CMakeLists.txt`
- [ ] 删除 `tests/level2_core_services/CMakeLists.txt`
- [ ] 更新 AGENTS.md，明确使用 Bazel 构建系统
- [ ] 验证 Bazel 构建正常

#### 验证基础测试
- [ ] 运行 `bazel test //tests/level1_foundation/...`
- [ ] 运行 `bazel test //tests/level2_core/...`
- [ ] 运行 `bazel test //tests/level3_transaction_manager/config_tests`
- [ ] 收集 Level 1-3 基础覆盖率数据
- [ ] 生成初步覆盖率报告

---

### P1 - 短期改进（2-4周）

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