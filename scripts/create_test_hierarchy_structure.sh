#!/bin/bash
# SQLCC测试层次重构脚本 - 创建新的8层目录结构

echo "=== SQLCC测试体系层次重构 ==="

# 定义新的8层目录结构
HIERARCHY_DIRS=(
    "tests/level1_foundation"           # 层次1: 基础工具测试
    "tests/level2_core"                 # 层次2: 核心组件测试
    "tests/level3_storage"              # 层次3: 存储引擎测试
    "tests/level4_parser"               # 层次4: SQL解析器测试
    "tests/level5_execution"            # 层次5: 执行引擎测试
    "tests/level6_network"              # 层次6: 网络通信测试
    "tests/level7_enterprise"           # 层次7: 企业级特性测试
    "tests/level8_integration"          # 层次8: 系统集成测试
)

echo "创建新的8层测试目录结构..."

# 创建目录
for dir in "${HIERARCHY_DIRS[@]}"; do
    if [ ! -d "$dir" ]; then
        mkdir -p "$dir"
        echo "创建目录: $dir"
    else
        echo "目录已存在: $dir"
    fi
done

# 创建各级别的子目录结构
echo "创建各级别子目录..."

# 层次1: 基础工具测试
mkdir -p tests/level1_foundation/{utils,logger,config,types,basic}

# 层次2: 核心组件测试
mkdir -p tests/level2_core/{database_manager,user_manager,system_db,config}

# 层次3: 存储引擎测试
mkdir -p tests/level3_storage/{buffer_pool,b_plus_tree,disk_manager,wal,index}

# 层次4: SQL解析器测试
mkdir -p tests/level4_parser/{lexer,parser,ast,token}

# 层次5: 执行引擎测试
mkdir -p tests/level5_execution/{query_executor,transaction,execution_context,unified_executor}

# 层次6: 网络通信测试
mkdir -p tests/level6_network/{connection,session,protocol,security,encryption}

# 层次7: 企业级特性测试
mkdir -p tests/level7_enterprise/{procedure,trigger,advanced_features}

# 层次8: 系统集成测试
mkdir -p tests/level8_integration/{e2e,performance,compatibility,deployment}

echo "创建CMakeLists.txt文件..."

# 创建各级别的CMakeLists.txt
for dir in "${HIERARCHY_DIRS[@]}"; do
    cat > "${dir}/CMakeLists.txt" << EOF
# ${dir} CMakeLists.txt
# 自动生成于 $(date)

# 包含上级目录的CMake设置
include_directories(
    \${CMAKE_SOURCE_DIR}/include
    \${CMAKE_SOURCE_DIR}/tests/framework
)

# 添加gtest
find_package(GTest REQUIRED)
include_directories(\${GTEST_INCLUDE_DIRS})

# 子目录测试
# 注意: 请根据实际测试文件添加executable和测试定义

EOF
    echo "创建: ${dir}/CMakeLists.txt"
done

# 创建迁移计划文档
cat > "test_hierarchy_migration_plan.md" << 'EOF'
# SQLCC测试体系层次重构迁移计划

## 概述
根据v1.3.2测试体系优化方案，重构测试层次结构为8层架构。

## 新层次架构
```
层次8: 系统集成测试 (level8_integration)
├── 层次7: 企业级特性测试 (level7_enterprise)
├── 层次6: 网络通信测试 (level6_network)
├── 层次5: 执行引擎测试 (level5_execution)
├── 层次4: SQL解析器测试 (level4_parser)
├── 层次3: 存储引擎测试 (level3_storage)
├── 层次2: 核心组件测试 (level2_core)
└── 层次1: 基础工具测试 (level1_foundation)
```

## 迁移规则

### 层次1: 基础工具测试 (level1_foundation)
- **职责**: 测试基础工具类、数据类型、配置管理等
- **包含组件**: utils/, logger/, config/, basic types
- **迁移来源**:
  - `tests/unit/basic/` (基础工具相关)
  - `tests/unit/core/config_manager_test.cpp`
  - `tests/unit/core/test_gtest.cpp`

### 层次2: 核心组件测试 (level2_core)
- **职责**: 测试核心数据库组件
- **包含组件**: database_manager, user_manager, system_database
- **迁移来源**:
  - `tests/unit/core/` (除config_manager外)
  - `tests/legacy/` (数据库相关)

### 层次3: 存储引擎测试 (level3_storage)
- **职责**: 测试存储引擎组件
- **包含组件**: buffer_pool, b_plus_tree, disk_manager, wal
- **迁移来源**:
  - `tests/unit/storage/`
  - `tests/storage_engine/`

### 层次4: SQL解析器测试 (level4_parser)
- **职责**: 测试SQL解析功能
- **包含组件**: lexer, parser, ast, token
- **迁移来源**:
  - `tests/unit/parser/`
  - `tests/sql/` (解析相关)

### 层次5: 执行引擎测试 (level5_execution)
- **职责**: 测试查询执行、事务管理
- **包含组件**: execution/, transaction/, sql_executor
- **迁移来源**:
  - `tests/unit/executor/`
  - `tests/execution/`
  - `tests/unit/basic/execution_context_test.cpp` ⭐ **重点迁移**

### 层次6: 网络通信测试 (level6_network)
- **职责**: 测试网络通信、安全功能
- **包含组件**: network/, security/, encryption/
- **迁移来源**:
  - `tests/network/`
  - `tests/integration/` (网络相关)

### 层次7: 企业级特性测试 (level7_enterprise)
- **职责**: 测试存储过程、触发器等高级特性
- **包含组件**: procedure/, trigger/
- **迁移来源**:
  - `tests/integration/procedure_trigger_*`
  - `tests/integration/trigger_*`

### 层次8: 系统集成测试 (level8_integration)
- **职责**: 端到端集成测试、性能测试
- **包含组件**: e2e, performance, deployment
- **迁移来源**:
  - `tests/integration/` (除网络、企业级特性外)
  - `tests/performance/`

## 迁移步骤

### Phase 1: 目录结构准备 (当前阶段)
- [x] 创建8层目录结构
- [x] 生成CMakeLists.txt模板
- [ ] 制定详细迁移清单

### Phase 2: 文件迁移实施
- [ ] 按层次迁移测试文件
- [ ] 更新include路径
- [ ] 修改CMakeLists.txt

### Phase 3: 依赖关系梳理
- [ ] 更新各层依赖关系
- [ ] 验证编译依赖
- [ ] 解决循环依赖问题

### Phase 4: 验证与测试
- [ ] 各层单独编译测试
- [ ] 层次间集成测试
- [ ] 整体回归测试

## 注意事项
1. 迁移过程中保持原有功能不变
2. 逐步迁移，避免大面积修改
3. 充分测试每个迁移步骤
4. 备份重要文件和配置

## 验收标准
- [ ] 8层目录结构完整
- [ ] 所有测试文件正确分类
- [ ] 编译通过率95%+
- [ ] 测试执行正常
EOF

echo "迁移计划文档已生成: test_hierarchy_migration_plan.md"

echo "=== 层次重构准备完成 ==="
echo "下一步: 根据迁移计划开始文件迁移"