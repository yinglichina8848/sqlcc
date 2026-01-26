# LRUManager类详细设计

## 概述

LRUManager（最近最少使用管理器）是SQLCC数据库存储引擎缓冲池的核心组件，负责实现LRU（Least Recently Used）页面替换策略。它通过维护一个页面访问顺序链表和哈希表，高效地跟踪页面的使用情况，并在需要时选择最久未使用的页面进行淘汰，以优化内存使用和I/O性能。

## 核心功能

- **页面访问跟踪**：记录页面的访问顺序，最近访问的页面移至链表头部
- **页面淘汰决策**：选择最久未使用的页面进行淘汰
- **线程安全**：提供线程安全的操作接口，支持并发访问
- **高效查询**：O(1)时间复杂度的页面访问、添加、删除和淘汰操作
- **调试支持**：详细的日志记录用于调试和性能分析

## 类定义

```cpp
class LRUManager {
public:
    LRUManager();
    ~LRUManager();
    
    void Access(int32_t page_id);
    void Add(int32_t page_id);
    void Remove(int32_t page_id);
    int32_t GetLeastRecentlyUsed() const;
    bool Contains(int32_t page_id) const;
    void Clear();
    size_t Size() const;

private:
    mutable std::mutex mutex_;
    std::list<int32_t> lru_list_;
    std::unordered_map<int32_t, std::list<int32_t>::iterator> lru_map_;
};
```

## 设计思路

### 数据结构选择

- **双向链表（std::list）**：用于维护页面的访问顺序，最近访问的页面位于链表头部，最久未使用的页面位于链表尾部
- **哈希表（std::unordered_map）**：用于快速定位页面在链表中的位置，实现O(1)时间复杂度的访问
- **互斥锁（std::mutex）**：确保在并发环境下的线程安全

### 核心算法

1. **页面访问（Access）**：
   - 查找页面在哈希表中的位置
   - 如果页面存在，将其从当前位置移除并移至链表头部
   - 更新哈希表中的迭代器
   - 如果页面不存在，记录调试信息但不执行任何操作

2. **页面添加（Add）**：
   - 检查页面是否已经存在
   - 如果存在，先移除再添加（确保位置正确）
   - 将页面添加到链表头部
   - 更新哈希表

3. **页面删除（Remove）**：
   - 查找页面在哈希表中的位置
   - 如果存在，从链表和哈希表中同时移除

4. **获取最久未使用页面（GetLeastRecentlyUsed）**：
   - 返回链表尾部的页面ID（最久未使用的页面）
   - 如果链表为空，返回-1表示无效页面

## 实现细节

### 线程安全

所有公共方法都使用`std::lock_guard<std::mutex>`确保线程安全，防止并发访问导致的数据不一致。

### 异常处理

- 当访问不存在的页面时，不抛出异常，而是记录调试日志
- 所有操作都进行边界检查和错误处理

### 性能优化

- O(1)时间复杂度的核心操作
- 避免不必要的内存分配和复制
- 使用高效的标准库容器

## 接口说明

### LRUManager()

构造函数：

- 初始化内部数据结构
- 记录调试日志

### ~LRUManager()

析构函数：

- 清理内部数据结构
- 记录调试日志

### void Access(int32_t page_id)

访问页面：

- 将页面移至链表头部，表示最近使用
- 线程安全

### void Add(int32_t page_id)

添加页面：

- 将新页面添加到链表头部
- 如果页面已存在，先移除再添加
- 线程安全

### void Remove(int32_t page_id)

移除页面：

- 从链表和哈希表中同时移除页面
- 线程安全

### int32_t GetLeastRecentlyUsed() const

获取最久未使用的页面ID：

- 返回链表尾部的页面ID
- 如果链表为空，返回-1
- 线程安全

### bool Contains(int32_t page_id) const

检查页面是否存在：

- 返回页面是否在LRU管理器中
- 线程安全

### void Clear()

清空所有页面：

- 移除链表和哈希表中的所有元素
- 线程安全

### size_t Size() const

获取当前页面数量：

- 返回链表中页面的数量
- 线程安全

## 使用示例

```cpp
// 创建LRU管理器
LRUManager lru_manager;

// 添加页面
lru_manager.Add(10);
lru_manager.Add(20);
lru_manager.Add(30);

// 访问页面
lru_manager.Access(20); // 此时顺序为 [20, 30, 10]

// 获取最久未使用的页面
int32_t victim = lru_manager.GetLeastRecentlyUsed(); // 返回10

// 移除页面
lru_manager.Remove(10);

// 检查页面是否存在
bool exists = lru_manager.Contains(20); // 返回true

// 获取当前大小
size_t size = lru_manager.Size(); // 返回2

// 清空所有页面
lru_manager.Clear();
```

## 与缓冲池的关系

LRUManager是缓冲池的重要组成部分，与缓冲池的关系如下：

1. **缓冲池依赖LRUManager**：缓冲池使用LRUManager来管理页面的使用顺序
2. **页面淘汰决策**：当缓冲池需要淘汰页面时，调用LRUManager的GetLeastRecentlyUsed方法选择淘汰对象
3. **页面访问更新**：当缓冲池访问、添加或删除页面时，同步更新LRUManager的状态
4. **性能协同**：两者协同工作，优化内存使用和I/O性能

## 性能分析

- **时间复杂度**：所有核心操作（Access、Add、Remove、GetLeastRecentlyUsed、Contains）均为O(1)
- **空间复杂度**：O(N)，其中N为缓存的页面数量
- **并发性能**：使用互斥锁确保线程安全，在高并发环境下可能成为性能瓶颈

## 扩展点

- **替换策略扩展**：支持不同的页面替换策略（如LFU、ARC等）
- **并发优化**：使用更细粒度的锁或无锁数据结构提高并发性能
- **统计信息**：添加页面访问统计信息，用于性能分析和优化

## 总结

LRUManager是SQLCC数据库存储引擎中实现LRU页面替换策略的核心组件，通过高效的数据结构和算法，提供了O(1)时间复杂度的页面管理操作。它与缓冲池紧密协作，优化内存使用和I/O性能，是数据库系统高效运行的重要保障。

**注意**：当前实现中发现头文件和源文件之间存在一些不一致（头文件使用BufferFrame*，源文件使用int32_t page_id）。这可能是重构过程中的遗留问题，建议在后续版本中统一接口定义。