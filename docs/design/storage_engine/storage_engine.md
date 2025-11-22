# StorageEngine类详细设计

## 概述

StorageEngine是存储引擎的核心类，负责协调磁盘管理器(DiskManager)和缓冲池(BufferPool)的工作，为上层应用提供统一的页面管理接口。

## 类定义

```cpp
class StorageEngine {
public:
    explicit StorageEngine(ConfigManager& config_manager);
    ~StorageEngine();
    
    StorageEngine(const StorageEngine&) = delete;
    StorageEngine& operator=(const StorageEngine&) = delete;
    
    Page* NewPage(int32_t* page_id);
    Page* FetchPage(int32_t page_id);
    bool UnpinPage(int32_t page_id, bool is_dirty);
    bool FlushPage(int32_t page_id);
    bool DeletePage(int32_t page_id);
    void FlushAllPages();
    std::string GetStats() const;
    
    IndexManager* GetIndexManager();

private:
    ConfigManager& config_manager_;
    std::unique_ptr<DiskManager> disk_manager_;
    std::unique_ptr<BufferPool> buffer_pool_;
    std::unique_ptr<IndexManager> index_manager_;
};
```

## 构造函数

### StorageEngine(ConfigManager& config_manager)

构造函数负责初始化存储引擎的各个组件：

1. 从配置管理器获取数据库文件路径和缓冲池大小
2. 创建DiskManager实例
3. 创建BufferPool实例
4. 创建IndexManager实例

## 析构函数

### ~StorageEngine()

析构函数负责释放存储引擎占用的资源：

1. 调用FlushAllPages方法确保所有数据持久化
2. 智能指针自动释放DiskManager、BufferPool和IndexManager资源

## 核心方法

### Page* NewPage(int32_t* page_id)

创建新页面：

1. 通过磁盘管理器分配新的页面ID
2. 通过缓冲池创建页面对象
3. 将页面标记为脏页
4. 通过输出参数返回页面ID

### Page* FetchPage(int32_t page_id)

获取页面：

1. 通过缓冲池获取页面
2. 如果页面不在内存中则通过磁盘管理器从磁盘加载

### bool UnpinPage(int32_t page_id, bool is_dirty)

取消固定页面：

1. 减少页面的固定计数
2. 如果页面被修改则标记为脏页

### bool FlushPage(int32_t page_id)

刷新页面到磁盘：

1. 调用缓冲池的FlushPage方法
2. 由缓冲池协调与磁盘管理器的交互

### bool DeletePage(int32_t page_id)

删除页面：

1. 调用缓冲池的DeletePage方法
2. 由缓冲池协调与磁盘管理器的交互

### void FlushAllPages()

刷新所有页面到磁盘：

1. 调用缓冲池的FlushAllPages方法
2. 由缓冲池协调与磁盘管理器的交互

### std::string GetStats() const

获取数据库统计信息：

1. 调用缓冲池的GetStats方法获取缓冲池统计信息
2. 结合磁盘管理器的信息生成完整统计报告

### IndexManager* GetIndexManager()

获取索引管理器：

1. 返回IndexManager对象指针
2. 用于索引相关操作

## 成员变量

### ConfigManager& config_manager_

配置管理器引用，用于获取配置参数。

### std::unique_ptr<DiskManager> disk_manager_

磁盘管理器智能指针，负责磁盘I/O操作。

### std::unique_ptr<BufferPool> buffer_pool_

缓冲池智能指针，负责内存中的页面管理。

### std::unique_ptr<IndexManager> index_manager_

索引管理器智能指针，负责索引相关操作。