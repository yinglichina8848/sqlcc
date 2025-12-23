# SQLCC 编码规范

## 概述

本文档定义了SQLCC项目的编码规范和最佳实践，确保代码质量、维护性和一致性。

## 1. 通用编码原则

### 1.1 代码质量
- **可读性优先**：代码应清晰易懂，避免复杂的逻辑嵌套
- **简单性原则**：保持函数和类的职责单一，避免过度抽象
- **一致性**：遵循项目既有的代码风格和模式
- **可维护性**：编写易于理解和修改的代码

### 1.2 命名规范

#### 1.2.1 文件命名
- 源文件：`snake_case.cpp` 或 `snake_case.h`
- 测试文件：`test_target_function.cpp`
- 构建文件：`BUILD.bazel`
- 配置文件：`snake_case.conf`

#### 1.2.2 类型和变量命名
- **类和结构体**：`PascalCase`
- **函数和方法**：`snake_case`
- **成员变量**：`snake_case_`（带下划线结尾）
- **常量**：`SCREAMING_SNAKE_CASE`
- **命名空间**：`snake_case`

```cpp
// 正确示例
class BufferPoolManager {
private:
    std::unique_ptr<PageTable> page_table_;
    static constexpr size_t MAX_POOL_SIZE = 1024;

public:
    void flush_all_pages();
    Page* get_page(page_id_t page_id);
};

namespace storage_engine {
    // 命名空间内容
}
```

#### 1.2.3 智能指针命名
- 使用完整的类型名，避免使用`auto`
- 智能指针变量名应体现所有权语义

```cpp
// 推荐
std::unique_ptr<BufferPool> buffer_pool_;
std::shared_ptr<Page> shared_page_;
std::weak_ptr<Connection> connection_weak_;

// 不推荐
auto pool = std::make_unique<BufferPool>();
auto page = std::shared_ptr<Page>(new Page());
```

## 2. 内存管理规范

### 2.1 智能指针使用
- **首选智能指针**：95%+的场景使用`std::unique_ptr`、`std::shared_ptr`
- **避免裸指针**：除非必要，不要使用裸指针
- **明确所有权**：使用`unique_ptr`表示独占所有权，`shared_ptr`表示共享所有权

```cpp
// 正确：使用智能指针
class Table {
private:
    std::unique_ptr<IndexManager> index_manager_;
    std::vector<std::shared_ptr<Page>> pages_;

public:
    void add_page(std::shared_ptr<Page> page) {
        pages_.push_back(std::move(page));
    }
};

// 错误：使用裸指针
class BadExample {
private:
    IndexManager* index_manager_;  // 裸指针，内存管理不明确
};
```

### 2.2 RAII模式
- **资源管理**：所有资源使用RAII模式管理
- **构造函数获取资源**：在构造函数中获取资源
- **析构函数释放资源**：在析构函数中释放资源

```cpp
class FileHandler {
private:
    std::unique_ptr<FILE, decltype(&fclose)> file_;

public:
    explicit FileHandler(const std::string& filename)
        : file_(fopen(filename.c_str(), "r"), fclose) {
        if (!file_) {
            throw std::runtime_error("Failed to open file");
        }
    }

    // 自动释放文件句柄，无需手动close
};
```

### 2.3 异常安全
- **强异常保证**：确保异常发生时系统状态一致
- **资源清理**：使用智能指针和RAII确保资源正确释放

```cpp
void process_transaction(std::unique_ptr<Transaction> txn) {
    // 开始事务
    txn->begin();

    try {
        // 执行操作
        perform_operation(txn.get());

        // 提交事务
        txn->commit();
    } catch (const std::exception& e) {
        // 回滚事务，确保数据一致性
        txn->rollback();
        throw;
    }
}
```

## 3. 错误处理规范

### 3.1 异常使用
- **标准异常**：使用C++标准库异常
- **自定义异常**：继承自`std::exception`
- **异常信息**：提供清晰的错误信息

```cpp
class DatabaseException : public std::exception {
private:
    std::string message_;

public:
    explicit DatabaseException(const std::string& message)
        : message_(message) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }
};

// 使用示例
if (!table_exists(table_name)) {
    throw DatabaseException("Table '" + table_name + "' does not exist");
}
```

### 3.2 错误码使用
- **特定场景**：在性能敏感或C接口中使用错误码
- **枚举定义**：使用枚举定义错误码

```cpp
enum class ErrorCode {
    SUCCESS = 0,
    TABLE_NOT_FOUND = 1,
    PERMISSION_DENIED = 2,
    CONNECTION_LOST = 3
};

struct Result {
    ErrorCode code;
    std::string message;
};
```

## 4. 并发编程规范

### 4.1 线程安全
- **最小化共享**：减少共享状态，使用消息传递
- **原子操作**：使用`std::atomic`进行原子操作
- **锁管理**：使用RAII锁管理器

```cpp
class ThreadSafeCounter {
private:
    std::atomic<size_t> count_{0};
    mutable std::mutex mutex_;
    std::condition_variable cv_;

public:
    void increment() {
        count_.fetch_add(1, std::memory_order_relaxed);
    }

    size_t get_count() const {
        return count_.load(std::memory_order_acquire);
    }

    void wait_for(size_t target) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this, target]() {
            return count_.load() >= target;
        });
    }
};
```

### 4.2 锁的使用
- **细粒度锁**：使用细粒度锁提高并发性
- **锁顺序**：定义全局锁顺序避免死锁
- **读写锁**：读多写少场景使用`std::shared_mutex`

```cpp
class ConcurrentHashMap {
private:
    std::unordered_map<Key, Value> map_;
    mutable std::shared_mutex mutex_;

public:
    std::optional<Value> get(const Key& key) const {
        std::shared_lock lock(mutex_);
        auto it = map_.find(key);
        return it != map_.end() ? std::optional(it->second) : std::nullopt;
    }

    void put(const Key& key, Value value) {
        std::unique_lock lock(mutex_);
        map_[key] = std::move(value);
    }
};
```

## 5. 设计模式规范

### 5.1 工厂模式
- **智能指针工厂**：工厂函数返回智能指针
- **多态工厂**：使用模板实现类型安全的工厂

```cpp
// 智能指针工厂
std::unique_ptr<StorageEngine> create_storage_engine(EngineType type) {
    switch (type) {
        case EngineType::INNODB:
            return std::make_unique<InnoDBEngine>();
        case EngineType::MYISAM:
            return std::make_unique<MyISAMEngine>();
        default:
            throw std::invalid_argument("Unknown engine type");
    }
}

// 模板工厂
template<typename T>
std::unique_ptr<T> create_component() {
    return std::make_unique<T>();
}
```

### 5.2 观察者模式
- **智能指针管理**：使用`weak_ptr`避免循环引用
- **线程安全**：确保观察者模式的线程安全性

```cpp
class Observer {
public:
    virtual ~Observer() = default;
    virtual void on_event(const Event& event) = 0;
};

class Subject {
private:
    std::vector<std::weak_ptr<Observer>> observers_;
    mutable std::mutex mutex_;

public:
    void add_observer(std::weak_ptr<Observer> observer) {
        std::lock_guard lock(mutex_);
        observers_.push_back(std::move(observer));
    }

    void notify(const Event& event) {
        std::lock_guard lock(mutex_);
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [](const std::weak_ptr<Observer>& weak_obs) {
                    auto obs = weak_obs.lock();
                    if (obs) {
                        obs->on_event(event);
                        return false;  // 保持观察者
                    }
                    return true;      // 移除已销毁的观察者
                }),
            observers_.end());
    }
};
```

## 6. 注释规范

### 6.1 注释要求
- **函数注释**：每个public函数必须有注释
- **类注释**：每个类必须有用途说明
- **复杂逻辑**：复杂算法和业务逻辑必须注释
- **TODO注释**：标记待完成的工作

```cpp
/**
 * @brief 缓冲池管理器
 *
 * 负责管理数据库页面的缓存，提供页面获取、释放、刷新等功能。
 * 使用LRU策略进行页面替换，支持并发访问。
 */
class BufferPoolManager {
public:
    /**
     * @brief 获取页面
     * @param page_id 页面ID
     * @return 页面指针，如果页面不存在返回nullptr
     *
     * 该函数首先检查页面是否在缓存中，如果是则返回页面指针。
     * 如果不在，则从磁盘加载页面并放入缓存。
     */
    Page* get_page(page_id_t page_id);

    // TODO: 实现页面预取功能
    void prefetch_pages(const std::vector<page_id_t>& page_ids);
};
```

### 6.2 注释质量
- **准确性**：注释必须准确反映代码功能
- **完整性**：覆盖所有重要行为和约束
- **简洁性**：注释应简洁明了，避免冗余

## 7. 测试规范

### 7.1 单元测试
- **测试覆盖**：核心功能必须有单元测试
- **边界测试**：测试边界条件和异常情况
- **测试命名**：`test_function_name_scenario`

```cpp
TEST(BufferPoolTest, GetPage_ExistingPage_ReturnsValidPage) {
    auto bpm = std::make_unique<BufferPoolManager>();
    auto page = bpm->get_page(1);
    ASSERT_NE(page, nullptr);
    ASSERT_EQ(page->get_page_id(), 1);
}

TEST(BufferPoolTest, GetPage_NonExistingPage_ReturnsNullptr) {
    auto bpm = std::make_unique<BufferPoolManager>();
    auto page = bpm->get_page(999);
    ASSERT_EQ(page, nullptr);
}
```

### 7.2 性能测试
- **基准测试**：使用Google Benchmark进行性能测试
- **性能断言**：设定性能阈值，确保性能不退化

## 8. 构建和依赖规范

### 8.1 Bazel构建
- **BUILD文件**：清晰定义依赖关系
- **目标命名**：使用snake_case命名目标
- **依赖管理**：最小化依赖，避免循环依赖

```python
# BUILD.bazel 示例
cc_library(
    name = "buffer_pool",
    srcs = ["buffer_pool.cpp"],
    hdrs = ["buffer_pool.h"],
    deps = [
        ":page",
        ":disk_manager",
        "//src/utils:types",
    ],
)
```

### 8.2 头文件管理
- **前向声明**：在头文件中使用前向声明减少包含
- **包含守卫**：使用`#pragma once`防止重复包含
- **接口分离**：将接口和实现分离

```cpp
// buffer_pool.h
#pragma once

#include <memory>

class Page;
class DiskManager;

class BufferPoolManager {
public:
    explicit BufferPoolManager(std::unique_ptr<DiskManager> disk_manager);
    Page* get_page(page_id_t page_id);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
```

## 9. 性能优化规范

### 9.1 零拷贝设计
- **避免不必要拷贝**：使用引用和移动语义
- **缓冲区复用**：复用缓冲区减少内存分配

```cpp
// 零拷贝字符串处理
std::string_view process_string(std::string_view input) {
    // 不创建新的字符串，直接返回视图
    if (input.starts_with("prefix")) {
        return input.substr(7);
    }
    return input;
}

// 移动语义
std::vector<Data> process_data(std::vector<Data> data) {
    // 修改原数据，避免拷贝
    for (auto& item : data) {
        item.process();
    }
    return data;  // 移动返回
}
```

### 9.2 缓存友好
- **数据局部性**：优化数据结构提高缓存命中率
- **预取优化**：使用`__builtin_prefetch`进行数据预取

## 10. 代码审查清单

### 10.1 功能检查
- [ ] 代码实现功能需求
- [ ] 边界条件处理正确
- [ ] 异常情况处理完整
- [ ] 性能满足要求

### 10.2 质量检查
- [ ] 遵循编码规范
- [ ] 注释完整准确
- [ ] 单元测试覆盖充分
- [ ] 内存管理正确

### 10.3 设计检查
- [ ] 设计模式使用恰当
- [ ] 依赖关系合理
- [ ] 接口设计清晰
- [ ] 扩展性良好

---

*最后更新: 2025-12-23*

遵循这些规范，将确保SQLCC项目的代码质量、维护性和可扩展性，为构建企业级数据库系统奠定坚实基础。
