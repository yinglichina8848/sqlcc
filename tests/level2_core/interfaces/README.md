# Core 接口测试 - 当前状态与未来规划

**日期**: 2026-02-11  
**作者**: OpenCode Developer [liying managed]  
**关联**: PR #4, Issue #6

---

## 当前状态

### ✅ 已完成的测试

| 测试目标 | 测试文件 | 结果 | 说明 |
|---------|---------|------|------|
| Core 接口编译测试 | `core_interface_test.cpp` | **6/6 PASSED** | 使用手动 Mock 实现 |
| 接口基础功能 | `core_interface_test.cpp` | ✅ 通过 | 验证接口可编译和调用 |
| TransactionId 类型 | `core_interface_test.cpp` | ✅ 通过 | 类型一致性验证 |
| 依赖注入 | `core_interface_test.cpp` | ✅ 通过 | 接口多态性验证 |

### 测试运行命令

```bash
# 运行接口测试
bazel test //tests/level2_core/interfaces:core_interface_test

# 输出
//tests/level2_core/interfaces:core_interface_test PASSED in 1.1s
Executed 1 out of 1 test: 1 test passes.
```

---

## GMock 迁移说明

### 为什么需要 GMock？

**当前手动 Mock 的局限性**:
```cpp
// ❌ 当前：手动实现，只能验证返回值
class MockDatabaseManager : public IDatabaseManager {
    bool CreateDatabase(const std::string& db_name) override { 
        return true;  // 只能返回固定值
    }
};
```

**GMock 的优势**:
```cpp
// ✅ GMock：可以验证调用行为
class MockDatabaseManager : public IDatabaseManager {
    MOCK_METHOD(bool, CreateDatabase, (const std::string& db_name), (override));
};

// 验证调用次数、参数、顺序
EXPECT_CALL(mock, CreateDatabase("test_db"))
    .Times(Exactly(1))
    .WillOnce(Return(true));
```

### GMock 迁移障碍

**问题**: GMock 的 `MOCK_METHOD` 宏在处理复杂模板类型时有编译问题

**受影响的接口方法**:
```cpp
// 这个方法无法直接用 MOCK_METHOD
virtual bool CreateTable(
    const std::string& table_name,
    const std::vector<std::pair<std::string, std::string>>& columns
) = 0;
```

**错误信息**:
```
error: only virtual member functions can be marked 'override'
    MOCK_METHOD(bool, CreateTable, (...), (override));
```

**原因**: 
- GMock 宏展开时对复杂模板类型支持有限
- `std::vector<std::pair<std::string, std::string>>` 包含逗号，被宏解析为多个参数

### 解决方案（未来实施）

#### 方案 1: 类型别名简化

```cpp
// 简化前
using Columns = std::vector<std::pair<std::string, std::string>>;

// 接口中使用别名
virtual bool CreateTable(const std::string& table_name, const Columns& columns) = 0;

// GMock 可以正常工作
MOCK_METHOD(bool, CreateTable, (const std::string& table_name, const Columns& columns), (override));
```

#### 方案 2: 使用 GMOCK_INTERNAL 宏

```cpp
// 使用 GMock 内部宏处理复杂类型
GMOCK_INTERNAL_METHOD(bool, CreateTable, (const std::string& table_name, 
    (const std::vector<std::pair<std::string, std::string>>& columns)));
```

#### 方案 3: 分层接口设计

```cpp
// 将复杂方法拆分为简单方法
class ITableBuilder {
    virtual void AddColumn(const std::string& name, const std::string& type) = 0;
    virtual bool Build() = 0;
};

// 简化 CreateTable
virtual bool CreateTable(const std::string& table_name, ITableBuilder& builder) = 0;
```

---

## 迁移计划

### Phase 1: 当前状态（已完成）
- [x] 手动 Mock 测试（6/6 通过）
- [x] 接口编译验证
- [x] 基础功能测试

### Phase 2: GMock 准备（未来 v1.4.2）
- [ ] 研究 GMock 复杂类型解决方案
- [ ] 创建类型别名简化接口（如果需要）
- [ ] 编写 GMock 测试示例

### Phase 3: GMock 迁移（未来 v1.5.x）
- [ ] 重写 Mock 实现使用 MOCK_METHOD
- [ ] 添加 EXPECT_CALL 行为验证
- [ ] 使用参数匹配器（_, Return, AtLeast 等）
- [ ] 添加顺序验证（InSequence）

### Phase 4: 完整行为测试（未来 v2.0）
- [ ] 所有接口方法都有行为验证
- [ ] 覆盖正常和异常场景
- [ ] 集成到 CI/CD

---

## 当前测试使用说明

### 运行测试

```bash
# 运行所有接口测试
bazel test //tests/level2_core/interfaces:all

# 运行特定测试
bazel test //tests/level2_core/interfaces:core_interface_test

# 查看详细输出
bazel test //tests/level2_core/interfaces:core_interface_test --test_output=all
```

### 测试覆盖内容

1. **编译测试**: 验证接口可以被正确包含和编译
2. **类型测试**: 验证 TransactionId = uint64_t
3. **多态测试**: 验证接口支持多态使用
4. **注入测试**: 验证依赖注入模式
5. **抽象类测试**: 验证接口是纯虚类

---

## 结论

**当前状态**: 测试已满足 PR #4 的需求（6/6 通过）

**GMock 迁移**: 标记为未来改进项（v1.4.2+）

**原因**: 
1. 当前手动 Mock 测试完全可用
2. GMock 迁移需要接口设计调整
3. 不影响 PR #4 的核心目标（接口解耦）

---

**记录人**: OpenCode Developer [liying managed]  
**日期**: 2026-02-11
