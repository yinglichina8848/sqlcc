# SQLCC AI 辅助开发规范 v1.3.9

**版本**: 1.3.9  
**生效日期**: 2026-01-30  
**适用范围**: 所有 AI Agent 参与 SQLCC 项目开发

---

## 🎯 核心原则

### 1. FIRST 原则

- **F**ind First: 先查找、先阅读、先理解
- **I**nvestigate Before Implement: 调研先于实现
- **R**espect Existing Style: 尊重现有代码风格
- **S**ystematic Approach: 系统性方法
- **T**est Everything: 测试一切

### 2. 约束条件

AI Agent 必须遵守以下约束：

| 约束类型 | 要求 | 说明 |
|----------|------|------|
| **语言标准** | C++20 | 必须使用 C++20 标准 |
| **构建系统** | Bazel 8.5.0+ | 只修改 BUILD.bazel，不碰 CMake |
| **编译器** | Clang 20+ | 使用 libc++ 标准库 |
| **智能指针** | 强制使用 | 所有权管理必须使用智能指针 |
| **测试框架** | Google Test | 单元测试使用 GTest |

### 3. 禁止行为

以下行为严格禁止：

- ❌ 不读取文件直接修改代码
- ❌ 使用裸指针管理资源所有权
- ❌ 引入新的编码风格
- ❌ 修改非目标文件
- ❌ 不测试就提交代码
- ❌ 破坏向后兼容性
- ❌ 引入循环依赖
- ❌ 不更新文档

---

## 📁 项目结构理解

### 目录层次

```
sqlcc/
├── src/                    # 源代码
│   ├── core/              # 核心组件
│   ├── storage_engine/    # 存储引擎
│   ├── sql_parser/        # SQL解析器
│   ├── execution/         # 执行引擎
│   ├── transaction/       # 事务管理
│   ├── network/           # 网络通信
│   ├── exception/         # 异常处理
│   ├── logger/            # 日志系统
│   └── utils/             # 工具类
├── include/               # 头文件（与 src 对应）
├── tests/                 # 测试代码
│   ├── level1_foundation/ # 基础层测试
│   ├── level2_core/       # 核心层测试
│   ├── level2_storage_engine/ # 存储引擎测试
│   ├── level3_transaction_manager/ # 事务测试
│   ├── level4_sql_processing/ # SQL处理测试
│   ├── level5_network/    # 网络测试
│   └── level6_integration/ # 集成测试
├── docs/                  # 文档
├── tools/                 # 开发工具
└── scripts/               # 构建脚本
```

### 头文件组织

```
include/
├── core/                  # 核心组件头文件
├── sql_parser/           # SQL解析器头文件
├── storage_engine/       # 存储引擎头文件
├── execution/            # 执行引擎头文件
├── transaction/          # 事务管理头文件
├── network/              # 网络通信头文件
├── exception/            # 异常处理头文件
├── utils/                # 工具类头文件
└── types/                # 数据类型头文件
```

**关键规则**:
- 头文件路径: `include/<module>/<file>.h`
- 源文件路径: `src/<module>/<file>.cpp`
- 测试文件路径: `tests/<level>/<module>/<file>_test.cpp`

---

## 🛠️ 开发工作流程

### 阶段1: 问题理解

```
1. 读取相关文件
   - 使用 read_file 读取目标文件
   - 使用 glob 查找相关文件
   - 使用 grep 搜索代码模式

2. 分析依赖关系
   - 查看 BUILD.bazel 中的依赖
   - 分析头文件包含关系
   - 理解模块边界

3. 查阅规范文档
   - AGENTS.md - 项目规范
   - BUILD_FILE_SPECIFICATION.md - BUILD规范
   - header_index.md - 头文件索引
```

### 阶段2: 方案设计

```
1. 确定修改范围
   - 只修改必要文件
   - 明确接口变更
   - 评估影响范围

2. 设计实现方案
   - 遵循现有模式
   - 使用项目工具类
   - 考虑异常处理

3. 制定测试计划
   - 单元测试覆盖
   - 边界条件测试
   - 集成测试验证
```

### 阶段3: 代码实现

```
1. 创建/修改文件
   - 遵循命名规范
   - 保持代码风格一致
   - 添加必要注释

2. 更新 BUILD 配置
   - 添加新文件到 srcs
   - 声明新的依赖
   - 更新测试目标

3. 实现测试代码
   - 使用 TEST/TEST_F 宏
   - 覆盖正常和异常路径
   - 验证边界条件
```

### 阶段4: 验证测试

```bash
# 1. 编译验证
bazel build //src/<module>:<module>

# 2. 测试验证
bazel test //tests/<level>/<module>:all

# 3. 覆盖率验证
bazel coverage //tests/<level>/<module>:all

# 4. 全量验证（重要修改）
bazel build //...
bazel test //...
```

### 阶段5: 文档更新

```
1. 更新 AGENTS.md（如修改规范）
2. 更新 WORKLOG.md（记录工作）
3. 更新 TODO.md（标记完成）
4. 更新 CHANGELOG.md（记录变更）
```

---

## 📝 编码规范

### 命名规范

| 类型 | 规范 | 示例 |
|------|------|------|
| **文件名** | snake_case | `buffer_pool.cpp` |
| **类名** | PascalCase | `BufferPoolManager` |
| **函数** | PascalCase（公有） | `GetInstance()` |
| **变量** | snake_case | `buffer_pool_size` |
| **成员** | snake_case + _ | `buffer_pool_size_` |
| **常量** | kPascalCase | `kDefaultPoolSize` |
| **宏** | UPPER_SNAKE_CASE | `SQLCC_BUFFER_POOL_H` |
| **命名空间** | 全小写 | `namespace sqlcc` |

### 头文件规范

```cpp
// 1. 文件头注释
/**
 * @file buffer_pool.h
 * @brief 缓冲池管理器
 * @author SQLCC Team
 * @date 2026-01-30
 */

// 2. 头文件保护
#pragma once
// 或
#ifndef SQLCC_BUFFER_POOL_H
#define SQLCC_BUFFER_POOL_H

// 3. Include 顺序
// (1) 对应头文件（如果是 .cpp）
// (2) 项目头文件
#include "storage_engine/buffer_pool.h"
// (3) 第三方头文件
#include <gtest/gtest.h>
// (4) 系统头文件
#include <memory>
#include <vector>

// 4. 命名空间
namespace sqlcc {
namespace storage {

// 5. 类定义
class BufferPool {
    // ...
};

}  // namespace storage
}  // namespace sqlcc

#endif  // SQLCC_BUFFER_POOL_H
```

### 智能指针使用

```cpp
// ✅ 正确：使用智能指针管理所有权
class BufferPool {
public:
    std::unique_ptr<Page> FetchPage(int32_t page_id);
    std::shared_ptr<DiskManager> GetDiskManager() { return disk_manager_; }
    
private:
    std::shared_ptr<DiskManager> disk_manager_;
    std::unique_ptr<ReplaceStrategy> replace_strategy_;
    std::weak_ptr<BufferPoolStats> stats_;  // 打破循环引用
};

// ❌ 错误：使用裸指针管理所有权
class BufferPool {
private:
    DiskManager* disk_manager_;  // 不要这样！
};
```

### 异常处理

```cpp
// ✅ 正确：使用异常处理错误
void BufferPool::PinPage(int32_t page_id) {
    if (page_id < 0) {
        throw BufferPoolException("Invalid page id: " + std::to_string(page_id));
    }
    // ...
}

// ✅ 正确：RAII 模式
class PageGuard {
public:
    explicit PageGuard(Page* page) : page_(page) {}
    ~PageGuard() { if (page_) page_->Unpin(); }
    // 禁止拷贝，允许移动
    PageGuard(const PageGuard&) = delete;
    PageGuard(PageGuard&& other) noexcept : page_(other.page_) {
        other.page_ = nullptr;
    }
private:
    Page* page_;
};
```

### 注释规范

```cpp
/**
 * WHY: 为什么需要这个类？
 * SQLCC需要高效的页面缓存机制来减少磁盘I/O。
 * 
 * WHAT: 这是什么？
 * BufferPool是一个分片式缓冲池管理器，采用LRU替换策略。
 * 
 * HOW: 如何使用？
 * 1. 创建实例: auto bp = std::make_unique<BufferPool>(size);
 * 2. 获取页面: auto page = bp->FetchPage(page_id);
 * 3. 使用完毕后页面自动Unpin
 */
class BufferPool {
public:
    /**
     * @brief 获取页面
     * @param page_id 页面ID
     * @return 页面指针，如果页面不存在返回nullptr
     * @throws BufferPoolException 如果page_id无效
     */
    std::unique_ptr<Page> FetchPage(int32_t page_id);
    
    // 成员变量注释
    int32_t pool_size_;  // 缓冲池大小（页面数）
};
```

---

## 🧪 测试规范

### 测试文件结构

```cpp
// tests/level1_foundation/types/types_test.cpp

#include "types/value.h"  // 被测头文件
#include <gtest/gtest.h>   // 测试框架

namespace sqlcc {
namespace types {

// 使用测试夹具
class ValueTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前准备
    }
    
    void TearDown() override {
        // 测试后清理
    }
    
    Value int_value_{42};
    Value string_value_{"hello"};
};

// 基本功能测试
TEST_F(ValueTest, ConstructorInt) {
    EXPECT_EQ(int_value_.GetType(), DataType::INTEGER);
    EXPECT_EQ(int_value_.GetInt(), 42);
}

// 边界条件测试
TEST_F(ValueTest, MaxIntValue) {
    Value max_val{std::numeric_limits<int32_t>::max()};
    EXPECT_EQ(max_val.GetInt(), std::numeric_limits<int32_t>::max());
}

// 异常测试
TEST_F(ValueTest, InvalidTypeCast) {
    EXPECT_THROW(int_value_.GetString(), TypeMismatchException);
}

}  // namespace types
}  // namespace sqlcc
```

### 测试标签使用

```bazel
cc_test(
    name = "types_test",
    srcs = ["types_test.cpp"],
    deps = [
        "//src/types:types",
        "@com_google_googletest//:gtest_main",
    ],
    tags = [
        "foundation",  # 层次标签
        "unit",        # 类型标签
    ],
)
```

### 测试运行

```bash
# 运行特定测试
bazel test //tests/level1_foundation/types:types_test

# 运行所有基础层测试
bazel test //tests/level1_foundation/... --test_tag_filters=foundation

# 运行并查看输出
bazel test //tests/... --test_output=all
```

---

## 🔧 工具使用

### 常用 Shell 命令

```bash
# 构建
bazel build //src/<module>:<target>
bazel build //...

# 测试
bazel test //tests/<level>/<module>:<target>
bazel test //...

# 覆盖率
bazel coverage //tests/<level>/<module>:<target>

# 查询依赖
bazel query 'deps(//src/<module>:<target>)'

# 格式化（如有配置）
bazel run //:format
```

### Python 工具

```bash
# 检查 BUILD 文件
python3 tools/bazel_code_checker.py

# 修复依赖
python3 tools/bazel_dep_fixer_enhanced.py . --dry-run

# 内存安全检查
python3 scripts/memory_audit.py
```

---

## 📊 质量门禁

### 提交前检查清单

- [ ] 代码编译通过 (`bazel build`)
- [ ] 所有测试通过 (`bazel test`)
- [ ] 无内存泄漏 (`valgrind` 或 sanitizer)
- [ ] 代码风格一致
- [ ] 注释完整
- [ ] 文档已更新

### 覆盖率要求

| 层次 | 目标覆盖率 | 最低覆盖率 |
|------|-----------|-----------|
| Level 1 (Foundation) | 100% | 90% |
| Level 2 (Core) | 80% | 70% |
| Level 3-5 | 70% | 60% |
| Level 6-7 (Integration) | 60% | 50% |

---

## 🆘 故障排除

### 常见问题

**Q1: 编译错误 "file not found"**
```bash
# 检查 BUILD.bazel 中的 deps
# 确保包含正确的头文件依赖
"//include/module:headers"
```

**Q2: 链接错误 "undefined reference"**
```bash
# 检查 BUILD.bazel 中的 deps
# 确保链接了实现库
"//src/module:module"
```

**Q3: 测试失败**
```bash
# 查看详细输出
bazel test //tests/... --test_output=all

# 运行单个测试
bazel test //tests/module:test --test_filter=TestName
```

---

**维护者**: SQLCC AI 开发团队  
**最后更新**: 2026-01-30  
**版本**: v1.3.9
