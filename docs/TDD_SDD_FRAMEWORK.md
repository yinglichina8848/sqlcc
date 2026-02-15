# SQLCC TDD/SDD 开发框架规范

## 📋 概述

本框架为 SQLCC 项目定义 **Spec-Driven Development (SDD)** 和 **Test-Driven Development (TDD)** 的开发规范。

---

## 🎯 核心原则

### 1. Spec-Driven Development (规范驱动)

**顺序：** 规范 → 接口 → 实现

1. **定义规格 (Spec)**
   - 功能描述：用例、输入、输出
   - 性能要求：时间复杂度、空间复杂度
   - 约束条件：边界条件、异常情况
   - 依赖关系：前置条件、后置条件

2. **定义接口 (Interface)**
   - 函数签名（参数类型、返回值类型）
   - 错误码定义
   - 状态码定义
   - 事件回调定义

3. **实现 (Implementation)**
   - 遵循接口规范
   - 满足性能要求
   - 处理边界情况

### 2. Test-Driven Development (测试驱动)

**顺序：** 红 → 绿 → 重构

1. **红色阶段 (Red)**
   - 编写失败的测试
   - 定义预期行为
   - 确保测试可运行

2. **绿色阶段 (Green)**
   - 编写最小实现
   - 让测试通过
   - 不追求完美

3. **重构阶段 (Refactor)**
   - 优化代码结构
   - 提升性能
   - 保持测试通过

---

## 📁 目录结构

```
tests/
├── level1_foundation/       # 基础层测试
├── level2_core/             # 核心层测试
├── level2_storage_engine/   # 存储引擎测试
├── level3_transaction_manager/
├── level4_sql_processing/
├── level5_network/
├── level6_integration/
├── level7_integration/
├── unit/                    # 独立单元测试（少量）
└── sql_parser/              # 解析器专项测试
```

---

## 📝 规范文档模板

> 提示：规格文档统一放在 `docs/sdd/features/<feature>/` 下，并与 requirements/design/tasks/verification 文档保持一致。

### 规格定义 (Spec) - parser_spec.md

```markdown
# Parser 模块规格

## 1. 功能概述

解析 SQL 语句，生成抽象语法树 (AST)。

## 2. 接口定义

### 2.1 函数签名

```cpp
Result<ASTNode*> parseSQL(const std::string& sql);
```

### 2.2 参数说明

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| sql | string | 是 | SQL 语句字符串 |

### 2.3 返回值

| 返回值 | 类型 | 说明 |
|--------|------|------|
| ASTNode* | Result | 解析后的 AST 节点指针 |

### 2.4 错误码

| 错误码 | 说明 |
|--------|------|
| ERR_SYNTAX_ERROR | 语法错误 |
| ERR_INVALID_INPUT | 无效输入 |
| ERR_MEMORY_ERROR | 内存错误 |

## 3. 性能要求

- 时间复杂度：O(n)，n 为 SQL 长度
- 空间复杂度：O(m)，m 为 AST 节点数

## 4. 边界条件

- 空字符串
- 超长 SQL (> 1MB)
- 特殊字符
- 嵌套括号 (> 100 层)

## 5. 测试用例

| 用例 | 输入 | 预期输出 |
|------|------|----------|
| 基础 SELECT | "SELECT * FROM t" | 成功解析 |
| 空输入 | "" | ERR_INVALID_INPUT |
| 语法错误 | "SELECT FROM" | ERR_SYNTAX_ERROR |
```

---

## 🧪 测试用例模板

### 单元测试 - parser_impl_test.cpp

```cpp
#include <gtest/gtest.h>
#include <memory>
#include "parser_interface.h"

namespace sqlcc {
namespace test {

// 测试用例模板
class ParserTDDTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // 初始化测试环境
    parser_ = std::make_unique<Parser>();
  }

  std::unique_ptr<Parser> parser_;
};

// 红色阶段：编写失败的测试
TEST_F(ParserTDDTest, EmptyInput_ReturnsError) {
  // 预期：空字符串返回 ERR_INVALID_INPUT
  auto result = parser_->parseSQL("");
  ASSERT_TRUE(result.isErr());
  EXPECT_EQ(result.error(), ERR_INVALID_INPUT);
}

// 绿色阶段：通过的测试
TEST_F(ParserTDDTest, BasicSelect_Success) {
  // 预期：基础 SELECT 语句解析成功
  auto result = parser_->parseSQL("SELECT * FROM t");
  ASSERT_TRUE(result.isOk());
  EXPECT_NE(result.value(), nullptr);
}

}  // namespace test
}  // namespace sqlcc
```

---

## 🔄 开发流程

### 新功能开发流程

1. **需求分析**
   - 理解功能需求
   - 识别边界条件
   - 确定性能要求

2. **编写规格 (SDD Phase 1)**
   - 编写 spec.md
   - 定义接口 .h 文件
   - 定义错误码、状态码

3. **编写测试 (TDD Phase 1 - Red)**
   - 编写 *_test.cpp
   - 定义预期行为
   - 验证测试失败

4. **实现功能 (TDD Phase 2 - Green)**
   - 最小化实现
   - 让测试通过
   - 不优化代码

5. **重构优化 (TDD Phase 3 - Refactor)**
   - 优化代码结构
   - 提升性能
   - 保持测试通过

6. **集成测试**
   - 编写集成测试
   - 验证模块间协作
   - 性能测试

---

## ✅ 更严格的 SDD/TDD 规范要求

### SDD 规范要求（必须项）
- `requirements.md` 必须包含 EARS 需求 + 验收标准 + 边界条件
- `design.md` 必须包含 Mermaid 图 + 接口定义 + 依赖关系
- `tasks.md` 必须包含任务状态机、依赖、验收命令
- `verification.md` 必须包含编译/测试/覆盖率记录

### TDD 规范要求（必须项）
- **Red/Green/Refactor 证据**：每个测试需记录首次失败的命令与输出摘要
- **测试矩阵**：对正常/边界/异常/并发至少覆盖 1 条用例
- **命名规范**：`<Module>_<Behavior>_<Expected>` 或 `ModuleTest.Behavior_Expected`
- **覆盖率要求**：新增模块要求达到项目 Level 覆盖率标准

### TDD 记录模板（示例）
```
Test: BufferPool_FetchPage_NotFound
Red: bazel test //tests/level2_storage_engine/buffer_pool:buffer_pool_test --test_filter=FetchPage_NotFound
Green: commit after fix
Refactor: lints/tests re-run
```

## 🛠️ 工具链

### 测试框架

- **Google Test (gtest)** - C++ 单元测试
- **Google Benchmark** - 性能测试

### 代码覆盖率

- **llvm-profdata** - 覆盖率数据合并
- **llvm-cov** - 覆盖率报告生成

### CI/CD

- **Bazel** - 构建系统
- **GitHub Actions** - 自动化测试

---

## ✅ 检查清单

在提交代码前，确保：

- [ ] 规格文档完整 (spec.md)
- [ ] 接口定义清晰 (.h 文件)
- [ ] 测试用例覆盖边界条件
- [ ] 所有测试通过 (100% pass)
- [ ] 代码覆盖率 > 80%
- [ ] 性能测试通过
- [ ] 文档已更新
- [ ] 代码审查通过

---

## 📚 参考资源

- [Spec-Driven Development](https://github.com/github/spec-kit)
- [Test-Driven Development](https://github.com/garora/TDD-Katas)
- [Google Test 文档](https://google.github.io/googletest/)
- [Bazel 测试规则](https://bazel.build/reference/be/test)

---

*版本：v1.0.0*  
*创建日期：2026-02-02*  
*作者：高小原 🌱*
