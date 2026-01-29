# SmartPtrLifetimeManager类详细设计

## 概述

SmartPtrLifetimeManager是SQLCC数据库系统中负责索引对象智能指针生命周期管理的工具类，提供了安全的资源管理和释放机制，用于优化索引对象的内存管理和避免资源泄漏。

## 核心功能

- **索引生命周期保护**：通过RAII机制确保索引对象的正确创建和释放
- **智能指针转换**：提供安全的智能指针所有权转移功能
- **资源释放助手**：提供安全释放单个或批量索引对象的方法
- **异常安全**：确保在异常情况下资源仍能正确释放

## 类定义

```cpp
class SmartPtrLifetimeManager {
public:
    // RAII包装器用于确保资源正确释放
    class IndexLifetimeGuard {
    public:
        IndexLifetimeGuard(std::unique_ptr<BPlusTreeIndex> index,
                          std::function<void(BPlusTreeIndex*)> cleanup = nullptr);
        ~IndexLifetimeGuard();
        BPlusTreeIndex* Get() const;
        BPlusTreeIndex* operator->() const;
    private:
        std::unique_ptr<BPlusTreeIndex> index_;
        std::function<void(BPlusTreeIndex*)> cleanup_;
    };

    // 智能指针所有权转移助手
    static std::unique_ptr<BPlusTreeIndex> TransferOwnership(std::shared_ptr<BPlusTreeIndex> shared_index);

    // 安全释放助手
    static void SafeRelease(std::unique_ptr<BPlusTreeIndex>& index);

    // 批量释放助手
    static void BatchRelease(std::vector<std::unique_ptr<BPlusTreeIndex>>& indexes);
};
```

## 核心组件

### IndexLifetimeGuard内部类
- **功能**：提供RAII风格的索引对象生命周期管理
- **构造函数**：接收一个unique_ptr和可选的清理函数
- **析构函数**：在对象销毁时调用清理函数并释放资源
- **访问方法**：提供Get()和operator->()方法访问底层索引对象

### 静态方法

#### TransferOwnership
```cpp
static std::unique_ptr<BPlusTreeIndex> TransferOwnership(std::shared_ptr<BPlusTreeIndex> shared_index);
```
- **功能**：将shared_ptr的所有权转移到unique_ptr
- **注意事项**：当前实现存在潜在的双重释放风险，仅作为临时解决方案

#### SafeRelease
```cpp
static void SafeRelease(std::unique_ptr<BPlusTreeIndex>& index);
```
- **功能**：安全释放单个索引对象
- **实现**：检查索引是否有效，记录日志后释放

#### BatchRelease
```cpp
static void BatchRelease(std::vector<std::unique_ptr<BPlusTreeIndex>>& indexes);
```
- **功能**：批量释放多个索引对象
- **实现**：遍历索引向量，逐个安全释放后清空向量

## 实现细节

### 内存管理策略
- 使用unique_ptr确保索引对象的唯一所有权
- 通过RAII机制在作用域结束时自动释放资源
- 提供清理回调机制支持自定义资源释放逻辑

### 线程安全
- 所有方法都是静态的，不维护全局状态
- 线程安全依赖于调用者的正确使用

### 日志记录
- 释放资源时记录调试信息
- 所有权转移时记录警告信息

## 设计模式与原则

### RAII (Resource Acquisition Is Initialization)
- 核心设计模式，确保资源正确释放
- IndexLifetimeGuard类是RAII的典型应用

### 静态工具类模式
- 所有方法都是静态的，无需创建实例
- 提供一组相关的工具函数

### 单责任原则
- 专注于智能指针生命周期管理
- 不包含其他不相关的功能

## 性能优化

- **零开销抽象**：IndexLifetimeGuard的使用不会带来额外的性能开销
- **批量操作**：BatchRelease方法减少了多次调用的开销
- **延迟释放**：仅在必要时释放资源

## 扩展点

- **清理函数扩展**：支持自定义清理逻辑
- **所有权转移机制**：可扩展为更安全的实现

## 错误处理

- **空指针检查**：所有方法都包含空指针检查
- **日志记录**：错误和警告情况都有日志记录
- **异常安全**：确保在异常情况下资源仍能正确释放

## 测试支持

- 提供了安全的资源管理机制，便于单元测试
- 可以通过日志验证资源是否正确释放

## 使用示例

```cpp
// 使用IndexLifetimeGuard管理索引生命周期
{
    auto index = std::make_unique<BPlusTreeIndex>(table_name, column_name);
    SmartPtrLifetimeManager::IndexLifetimeGuard guard(
        std::move(index),
        [](BPlusTreeIndex* idx) {
            // 自定义清理逻辑
            SQLCC_LOG_INFO("Custom cleanup for index: " + idx->GetTableName());
        }
    );
    
    // 使用索引
    guard->Insert(...);
    
    // 作用域结束时自动释放资源
}

// 安全释放单个索引
std::unique_ptr<BPlusTreeIndex> index = GetIndex();
SmartPtrLifetimeManager::SafeRelease(index);

// 批量释放索引
std::vector<std::unique_ptr<BPlusTreeIndex>> indexes = GetIndexes();
SmartPtrLifetimeManager::BatchRelease(indexes);
```