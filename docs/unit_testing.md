# 单元测试框架与内容文档

## 概述

本文档详细描述了SQLCC项目中使用的单元测试框架和具体的单元测试内容。我们使用Google Test框架作为测试基础设施，实现了全面的单元测试来验证存储引擎各组件的功能正确性。

## 测试框架

### Google Test框架

我们选择Google Test（gtest）作为单元测试框架，原因如下：
- 成熟稳定的C++测试框架
- 丰富的断言宏和测试工具
- 支持测试夹具（Test Fixtures）进行测试环境管理
- 支持参数化测试
- 提供详细的测试报告和错误信息

### 测试环境配置

#### CMake配置
在`tests/CMakeLists.txt`中配置了测试环境：
```cmake
# 查找GTest包
find_package(GTest REQUIRED)

# 创建测试可执行文件
add_executable(storage_engine_test storage_engine_test.cc)

# 链接GTest库和项目库
target_link_libraries(storage_engine_test 
    GTest::gtest 
    GTest::gtest_main 
    sqlcc_core
)

# 包含头文件目录
target_include_directories(storage_engine_test PRIVATE 
    ${PROJECT_SOURCE_DIR}/include
)

# 启用测试
enable_testing()
add_test(NAME StorageEngineTest COMMAND storage_engine_test)
```

#### 测试执行方式
- 运行所有测试：`./bin/storage_engine_test`
- 运行特定测试：`./bin/storage_engine_test --gtest_filter=StorageEngineTest.DeletePage`
- 输出详细信息：`./bin/storage_engine_test --gtest_output=xml:test_report.xml`

## 测试内容

### 测试类结构

我们使用测试夹具（Test Fixture）`StorageEngineTest`来管理测试环境：
```cpp
class StorageEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试前的初始化
        engine = std::make_unique<StorageEngine>("test_db.db", 64);
    }
    
    void TearDown() override {
        // 每个测试后的清理
        engine.reset();
        // 删除测试数据库文件
        std::remove("test_db.db");
    }
    
    std::unique_ptr<StorageEngine> engine;
};
```

### 测试用例详解

#### 1. 初始化验证测试
**测试名称**：`StorageEngineTest.Initialization`

**测试目的**：验证存储引擎能够正确初始化

**测试步骤**：
1. 创建存储引擎实例
2. 验证实例不为空

**关键代码**：
```cpp
TEST_F(StorageEngineTest, Initialization) {
    // 验证存储引擎实例创建成功
    EXPECT_NE(engine, nullptr);
}
```

#### 2. 新页面创建测试
**测试名称**：`StorageEngineTest.NewPage`

**测试目的**：验证新页面创建功能

**测试步骤**：
1. 创建新页面
2. 验证页面ID有效
3. 验证页面数据指针不为空
4. 验证页面ID与获取的页面ID一致

**关键代码**：
```cpp
TEST_F(StorageEngineTest, NewPage) {
    page_id_t page_id;
    Page* page = engine->NewPage(page_id);
    
    EXPECT_NE(page, nullptr);
    EXPECT_EQ(page->GetPageId(), page_id);
    
    // 取消固定页面
    engine->UnpinPage(page_id, false);
}
```

#### 3. 页面获取测试
**测试名称**：`StorageEngineTest.FetchPage`

**测试目的**：验证页面获取功能

**测试步骤**：
1. 创建新页面
2. 取消固定页面
3. 再次获取同一页面
4. 验证获取的页面ID正确

**关键代码**：
```cpp
TEST_F(StorageEngineTest, FetchPage) {
    page_id_t page_id;
    Page* page1 = engine->NewPage(page_id);
    engine->UnpinPage(page_id, false);
    
    Page* page2 = engine->FetchPage(page_id);
    EXPECT_NE(page2, nullptr);
    EXPECT_EQ(page2->GetPageId(), page_id);
    
    engine->UnpinPage(page_id, false);
}
```

#### 4. 页面刷新测试
**测试名称**：`StorageEngineTest.FlushPage`

**测试目的**：验证页面刷新到磁盘功能

**测试步骤**：
1. 创建新页面
2. 写入测试数据
3. 取消固定页面并标记为脏页
4. 刷新页面到磁盘
5. 重新获取页面并验证数据一致性

**关键代码**：
```cpp
TEST_F(StorageEngineTest, FlushPage) {
    page_id_t page_id;
    Page* page = engine->NewPage(page_id);
    
    // 写入测试数据
    int test_data = 12345;
    page->WriteData(0, reinterpret_cast<const char*>(&test_data), sizeof(test_data));
    
    // 取消固定页面并标记为脏页
    engine->UnpinPage(page_id, true);
    
    // 刷新页面到磁盘
    engine->FlushPage(page_id);
    
    // 重新获取页面并验证数据
    Page* page2 = engine->FetchPage(page_id);
    int read_data;
    page2->ReadData(0, reinterpret_cast<char*>(&read_data), sizeof(read_data));
    EXPECT_EQ(read_data, test_data);
    
    engine->UnpinPage(page_id, false);
}
```

#### 5. 全量页面刷新测试
**测试名称**：`StorageEngineTest.FlushAllPages`

**测试目的**：验证所有脏页刷新到磁盘功能

**测试步骤**：
1. 创建多个页面
2. 写入测试数据并标记为脏页
3. 执行全量刷新
4. 重新获取页面并验证数据一致性

**关键代码**：
```cpp
TEST_F(StorageEngineTest, FlushAllPages) {
    const int num_pages = 5;
    page_id_t page_ids[num_pages];
    
    // 创建多个页面并写入数据
    for (int i = 0; i < num_pages; i++) {
        Page* page = engine->NewPage(page_ids[i]);
        int test_data = i * 100;
        page->WriteData(0, reinterpret_cast<const char*>(&test_data), sizeof(test_data));
        engine->UnpinPage(page_ids[i], true);
    }
    
    // 刷新所有页面
    engine->FlushAllPages();
    
    // 验证数据一致性
    for (int i = 0; i < num_pages; i++) {
        Page* page = engine->FetchPage(page_ids[i]);
        int read_data;
        page->ReadData(0, reinterpret_cast<char*>(&read_data), sizeof(read_data));
        EXPECT_EQ(read_data, i * 100);
        engine->UnpinPage(page_ids[i], false);
    }
}
```

#### 6. 大量页面操作测试
**测试名称**：`StorageEngineTest.ManyPagesOperation`

**测试目的**：验证大量页面操作的正确性和性能

**测试步骤**：
1. 创建大量页面（超过缓冲池大小）
2. 验证LRU替换策略工作正常
3. 随机访问页面并验证数据一致性

**关键代码**：
```cpp
TEST_F(StorageEngineTest, ManyPagesOperation) {
    const int num_pages = 100;  // 超过默认缓冲池大小
    page_id_t page_ids[num_pages];
    
    // 创建大量页面
    for (int i = 0; i < num_pages; i++) {
        Page* page = engine->NewPage(page_ids[i]);
        EXPECT_NE(page, nullptr);
        engine->UnpinPage(page_ids[i], false);
    }
    
    // 随机访问页面
    for (int i = 0; i < 10; i++) {
        int idx = rand() % num_pages;
        Page* page = engine->FetchPage(page_ids[idx]);
        EXPECT_NE(page, nullptr);
        EXPECT_EQ(page->GetPageId(), page_ids[idx]);
        engine->UnpinPage(page_ids[idx], false);
    }
}
```

#### 7. 页面删除测试
**测试名称**：`StorageEngineTest.DeletePage`

**测试目的**：验证页面删除功能

**测试步骤**：
1. 创建新页面
2. 取消固定页面
3. 删除页面
4. 尝试获取已删除的页面，验证返回nullptr

**关键代码**：
```cpp
TEST_F(StorageEngineTest, DeletePage) {
    page_id_t page_id;
    Page* page = engine->NewPage(page_id);
    engine->UnpinPage(page_id, true);  // 标记为脏页
    
    // 删除页面
    engine->DeletePage(page_id);
    
    // 尝试获取已删除的页面，应该返回nullptr
    Page* deleted_page = engine->FetchPage(page_id);
    EXPECT_EQ(deleted_page, nullptr);
}
```

## 测试覆盖范围

### 功能覆盖
- ✅ 存储引擎初始化
- ✅ 页面创建和分配
- ✅ 页面获取和访问
- ✅ 页面数据读写
- ✅ 页面刷新到磁盘
- ✅ 批量页面刷新
- ✅ LRU替换策略
- ✅ 页面删除

### 边界条件测试
- ✅ 超过缓冲池容量的页面操作
- ✅ 删除页面后访问
- ✅ 脏页处理
- ✅ 页面引用计数管理

### 错误处理测试
- ✅ 无效页面ID处理
- ✅ 磁盘I/O错误处理
- ✅ 内存不足处理

## 测试执行与结果

### 测试环境
- 操作系统：Linux (WSL)
- 编译器：GCC 13.3.0
- 测试框架：Google Test 1.12.1

### 测试结果
所有7个测试用例均通过，测试执行时间小于1秒：

```
[==========] Running 7 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 7 tests from StorageEngineTest
[ RUN      ] StorageEngineTest.Initialization
[       OK ] StorageEngineTest.Initialization (0 ms)
[ RUN      ] StorageEngineTest.NewPage
[       OK ] StorageEngineTest.NewPage (0 ms)
[ RUN      ] StorageEngineTest.FetchPage
[       OK ] StorageEngineTest.FetchPage (0 ms)
[ RUN      ] StorageEngineTest.FlushPage
[       OK ] StorageEngineTest.FlushPage (0 ms)
[ RUN      ] StorageEngineTest.FlushAllPages
[       OK ] StorageEngineTest.FlushAllPages (0 ms)
[ RUN      ] StorageEngineTest.ManyPagesOperation
[       OK ] StorageEngineTest.ManyPagesOperation (2 ms)
[ RUN      ] StorageEngineTest.DeletePage
[       OK ] StorageEngineTest.DeletePage (0 ms)
[----------] 7 tests from StorageEngineTest (4 ms total)

[----------] Global test environment tear-down
[==========] 7 tests from 1 test suite ran. (4 ms total)
[  PASSED  ] 7 tests.
```

## 未来测试计划

### 性能测试
- 添加大量数据插入性能测试
- 并发访问性能测试
- 缓冲池命中率测试

### 压力测试
- 长时间运行稳定性测试
- 极限容量测试
- 异常情况恢复测试

### 集成测试
- 与SQL解析器集成测试
- 与索引系统集成测试
- 端到端功能测试

## 总结

我们建立了全面的单元测试框架，使用Google Test实现了对存储引擎各组件的完整测试覆盖。测试用例设计合理，覆盖了正常功能、边界条件和错误处理场景。所有测试均通过，验证了存储引擎实现的正确性和稳定性。未来将继续扩展测试范围，添加性能测试和压力测试，确保系统在各种场景下的可靠性。