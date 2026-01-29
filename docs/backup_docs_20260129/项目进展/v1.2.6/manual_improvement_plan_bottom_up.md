# SQLCC Bazel构建系统手工改进计划 (Bottom-Up)

## 🎯 项目概述

**日期**: 2025-12-22
**剩余问题**: 138个
**策略**: 从底向上 (基础组件 → 高层组件)
**目标**: 系统性解决剩余构建配置问题

## 📊 当前问题分布分析

### 高优先级文件 (问题数量 > 5)
- `tests/sql_parser/BUILD.bazel`: 22个问题 (LABEL_ERROR)
- `src/storage_engine/BUILD.bazel`: 17个问题 (CRITICAL: MISSING_FILE + CONFIGURATION_ERROR + LABEL_ERROR)
- `tests/BUILD.bazel`: 10个问题 (CRITICAL: MISSING_FILE)
- `src/BUILD.bazel`: 9个问题 (CRITICAL: MISSING_FILE)
- `tests/storage_engine/BUILD.bazel`: 7个问题 (LABEL_ERROR)
- `tests/sql_executor/BUILD.bazel`: 7个问题 (LABEL_ERROR)
- `src/transaction/BUILD.bazel`: 7个问题 (CRITICAL: CONFIGURATION_ERROR + MISSING_FILE + LABEL_ERROR)
- `src/sql_parser/BUILD.bazel`: 7个问题 (CRITICAL: MISSING_FILE + LABEL_ERROR)
- `tests/components/core/BUILD.bazel`: 6个问题 (CRITICAL: MISSING_FILE + LABEL_ERROR)
- `src/network/BUILD.bazel`: 6个问题 (CRITICAL: MISSING_FILE)

### 按包分组问题统计
- `storage_engine`: 32个问题 (MOST COMPLEX)
- `sql_parser`: 29个问题
- `BUILD.bazel`: 20个问题
- `sql_executor`: 8个问题
- `components`: 7个问题
- `transaction`: 7个问题
- `network`: 6个问题
- `types`: 6个问题
- `procedure`: 5个问题
- `config_manager`: 4个问题
- `transaction_manager`: 4个问题

## 🏗️ Bottom-Up 改进计划

### Phase 1: 基础组件修复 (1-2天)

#### 1.1 核心工具库 (types, exception, utils)
**目标文件**:
- `include/exception/BUILD.bazel`
- `include/utils/BUILD.bazel`
- `src/types/BUILD.bazel`

**主要问题**: MISSING_FILE, CONFIGURATION_ERROR
**修复策略**:
1. 检查缺失的头文件，确认是否需要创建
2. 修正Bazel规则语法
3. 验证依赖关系

#### 1.2 存储引擎核心 (storage_engine)
**目标文件**:
- `src/storage_engine/BUILD.bazel` (17个问题)
- `include/storage_engine/BUILD.bazel`

**主要问题**: MISSING_TARGET, CONFIGURATION_ERROR, MISSING_FILE, LABEL_ERROR
**修复策略**:
1. 创建缺失的Bazel目标
2. 修复配置规则错误
3. 解决include路径问题
4. 修正标签格式

### Phase 2: 解析器组件修复 (2-3天)

#### 2.1 SQL解析器 (sql_parser)
**目标文件**:
- `src/sql_parser/BUILD.bazel` (7个问题)
- `include/sql_parser/BUILD.bazel`

**主要问题**: MISSING_FILE, LABEL_ERROR
**修复策略**:
1. 解决缺失文件引用
2. 修复复杂的标签格式问题

#### 2.2 网络组件 (network)
**目标文件**:
- `src/network/BUILD.bazel` (6个问题)

**主要问题**: MISSING_FILE
**修复策略**:
1. 检查网络相关头文件
2. 修正include路径

### Phase 3: 执行引擎修复 (2-3天)

#### 3.1 执行器组件 (execution, procedure, trigger)
**目标文件**:
- `src/execution/BUILD.bazel`
- `src/procedure/BUILD.bazel`
- `src/trigger/BUILD.bazel`

**主要问题**: MISSING_FILE, LABEL_ERROR
**修复策略**:
1. 解决执行器相关依赖
2. 修复标签格式问题

#### 3.2 事务管理器 (transaction, config_manager)
**目标文件**:
- `src/transaction/BUILD.bazel` (7个问题)
- `src/config_manager/BUILD.bazel`
- `src/transaction_manager/BUILD.bazel`

**主要问题**: CONFIGURATION_ERROR, MISSING_FILE, LABEL_ERROR
**修复策略**:
1. 修正事务管理配置
2. 解决缺失文件问题
3. 修复复杂标签

### Phase 4: 核心服务修复 (1-2天)

#### 4.1 核心服务 (core, sql_executor)
**目标文件**:
- `src/core/BUILD.bazel`
- `src/sql_executor/BUILD.bazel`

**主要问题**: MISSING_FILE
**修复策略**:
1. 检查核心服务依赖
2. 解决include路径问题

#### 4.2 日志和服务器组件
**目标文件**:
- `src/logger/BUILD.bazel`
- `src/sqlcc_server/BUILD.bazel`
- `src/isql_network/BUILD.bazel`

**主要问题**: LABEL_ERROR
**修复策略**:
1. 修复标签格式问题

## 🛠️ 修复工作流程

### 标准修复步骤

#### Step 1: 问题分析
```bash
# 查看具体问题详情
python3 -c "
import json
with open('final_issues_check.json', 'r') as f:
    data = json.load(f)
# 过滤特定文件的问题
[file for file in data['issues'] if 'storage_engine' in file['file_path']]
"
```

#### Step 2: 文件检查
```bash
# 检查目标文件是否存在
ls -la path/to/file.h

# 检查Bazel规则语法
bazel query //path/to:target
```

#### Step 3: 修复实施
1. **MISSING_FILE**: 创建缺失文件或修正路径
2. **LABEL_ERROR**: 简化重复的include路径
3. **CONFIGURATION_ERROR**: 修正Bazel规则语法
4. **MISSING_TARGET**: 创建缺失的目标定义

#### Step 4: 验证修复
```bash
# 运行检测验证
python3 tools/bazel_config_detector.py . --force --output validation_check.json

# 构建测试
bazel build //path/to:target
```

## 📋 质量保证

### 修复标准
- [ ] 所有MISSING_FILE问题已解决
- [ ] 所有CONFIGURATION_ERROR已修复
- [ ] 所有LABEL_ERROR已简化
- [ ] 所有MISSING_TARGET已创建
- [ ] 构建测试通过

### 验证流程
1. **单文件验证**: 每个修复的文件单独测试
2. **依赖验证**: 检查上下游依赖关系
3. **构建验证**: 确保可以成功编译
4. **集成验证**: 整体项目构建测试

## 📈 进度跟踪

### Phase 1 进度 (基础组件)
- [x] types/BUILD.bazel ✅ (修复重复include路径和测试文件引用)
- [ ] exception/BUILD.bazel
- [ ] utils/BUILD.bazel
- [ ] storage_engine/BUILD.bazel (核心)

### Phase 2 进度 (解析器)
- [ ] sql_parser/BUILD.bazel
- [ ] network/BUILD.bazel

### Phase 3 进度 (执行器)
- [ ] execution/BUILD.bazel
- [ ] procedure/BUILD.bazel
- [ ] trigger/BUILD.bazel
- [ ] transaction/BUILD.bazel
- [ ] config_manager/BUILD.bazel

### Phase 4 进度 (服务)
- [ ] core/BUILD.bazel
- [ ] sql_executor/BUILD.bazel
- [ ] logger/BUILD.bazel
- [ ] sqlcc_server/BUILD.bazel

## 🎯 成功标准

### 技术指标
- [ ] 问题数量从138个减少到0个
- [ ] 所有Bazel目标可以成功构建
- [ ] 依赖关系图完整正确

### 质量指标
- [ ] 配置规则符合Bazel最佳实践
- [ ] 标签格式规范统一
- [ ] 文档更新完整

### 效率指标
- [ ] 手工修复时间控制在5个工作日内
- [ ] 修复过程有完整记录
- [ ] 经验教训总结完善

---

*计划制定时间: 2025-12-22 03:52:33*
*预计完成时间: 2025-12-27 (5个工作日)*
*负责人: SQLCC AI Agent*
