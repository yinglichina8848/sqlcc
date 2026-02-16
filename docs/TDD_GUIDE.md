# SQLCC TDD 开发指南

## 核心理念：红-绿-重构

1. **红色阶段**：先写失败的测试
2. **绿色阶段**：写最少代码让测试通过
3. **重构阶段**：优化代码，保持测试通过

## TDD 工作流

### 步骤 1: 分析需求
- 理解功能需求
- 识别边界条件
- 确定输入输出

### 步骤 2: 编写测试（红色）
- 创建 `*_test.cpp` 文件
- 使用 Google Test 格式：
  ```cpp
  #include <gtest/gtest.h>
  
  TEST(TestSuiteName, TestDescription) {
      // 测试代码
      EXPECT_EQ(expected, actual);
  }
  ```
- 运行测试，确保失败

### 步骤 3: 实现功能（绿色）
- 编写最小实现代码
- 不追求完美，先让测试通过
- 运行测试验证通过

### 步骤 4: 重构
- 优化代码结构
- 提升性能
- 保持测试通过

### 步骤 5: 提交
- 更新文档
- 运行完整测试套件
- 代码审查

## 测试命令

```bash
# 运行所有测试
bazel test //tests/...

# 运行特定测试
bazel test //tests/level1_foundation/exception:exception_test

# 运行并查看输出
bazel test //tests/... --test_output=all

# 代码覆盖率
bazel coverage //tests/...
```

## 示例：新增功能

假设要新增 "字符串处理函数"：

1. **写测试**：`tests/level1_foundation/utils/string_util_test.cpp`
2. **运行测试**：应该失败（红色）
3. **实现**：在 `src/utils/string_util.cpp` 实现
4. **运行测试**：通过（绿色）
5. **重构**：优化代码
6. **提交**

## 代码规范

- 使用 Google C++ Style Guide
- 遵循项目现有结构
- 保持测试独立性和可重复性
- 每个测试只验证一个行为

## 更严格的TDD执行要求（v1.4.0）

1. **必须记录 Red/Green/Refactor**
   - Red: 首次失败命令与失败摘要
   - Green: 通过测试的命令输出摘要
   - Refactor: 重构后全量测试通过

2. **最小覆盖矩阵**
   - 正常路径（至少 1）
   - 边界条件（至少 1）
   - 异常路径（至少 1）

3. **命名规范**
   - `Module_Behavior_Expected` 或 `ModuleTest.Behavior_Expected`

4. **验收与追踪**
   - 对应的 SDD 任务编号必须写入测试文件注释或提交信息
