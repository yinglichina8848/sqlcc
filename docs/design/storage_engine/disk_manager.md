# DiskManager类详细设计

## 概述

DiskManager是存储引擎的磁盘I/O管理组件，负责数据库文件的读写操作、页面分配和释放等底层磁盘操作。它提供了线程安全的接口，确保在并发环境下的数据一致性。

## 类定义

```cpp
class DiskManager {
public:
    explicit DiskManager(const std::string& db_file, ConfigManager& config_manager);
    ~DiskManager();
    
    DiskManager(const DiskManager&) = delete;
    DiskManager& operator=(const DiskManager&) = delete;
    
    bool WritePage(int32_t page_id, const char* page_data);
    bool ReadPage(int32_t page_id, char* page_data);
    bool BatchReadPages(const std::vector<int32_t>& page_ids, std::vector<char*>& page_data);
    bool PrefetchPage(int32_t page_id);
    bool BatchPrefetchPages(const std::vector<int32_t>& page_ids);
    int32_t AllocatePage();
    bool DeallocatePage(int32_t page_id);
    int32_t GetFileSize() const;
    bool Sync();
    std::unordered_map<std::string, uint64_t> GetIOStats() const;
    void ResetIOStats();

private:
    bool OpenFile();
    void CloseFile();
    bool InitializeFile();
    bool ReadFileHeader();
    bool WriteFileHeader();
    void OnConfigChange(const std::string& key, const ConfigValue& value);

private:
    std::string db_file_name_;
    ConfigManager& config_manager_;
    std::fstream db_io_;
    size_t file_size_;
    mutable std::recursive_timed_mutex io_mutex_;
    int32_t next_page_id_;
    std::vector<int32_t> free_pages_;
    size_t lock_timeout_ms_;
};
```

## 构造函数

### DiskManager(const std::string& db_file, ConfigManager& config_manager)

构造函数负责初始化磁盘管理器：

1. 存储数据库文件名和配置管理器引用
2. 打开数据库文件
3. 初始化文件流
4. 设置读写位置
5. 初始化互斥锁

## 析构函数

### ~DiskManager()

析构函数负责清理资源：

1. 调用文件流的close方法关闭文件

## 核心方法

### bool WritePage(int32_t page_id, const char* page_data)

写入页面到磁盘：

1. 定位到页面对应的文件位置
2. 写入页面数据
3. 刷新文件缓冲区

### bool ReadPage(int32_t page_id, char* page_data)

从磁盘读取页面：

1. 定位到页面对应的文件位置
2. 读取页面数据到内存缓冲区

### bool BatchReadPages(const std::vector<int32_t>& page_ids, std::vector<char*>& page_data)

批量读取页面：

1. 对页面ID进行排序以优化磁盘访问
2. 使用批量I/O操作读取页面

### bool PrefetchPage(int32_t page_id)

预取页面到缓冲区：

1. 使用异步I/O或内部缓冲区预取页面数据

### bool BatchPrefetchPages(const std::vector<int32_t>& page_ids)

批量预取页面到缓冲区：

1. 对页面ID进行排序以优化磁盘访问
2. 使用批量预取策略

### int32_t AllocatePage()

分配新页面：

1. 增加页面计数器
2. 返回新的页面ID

### bool DeallocatePage(int32_t page_id)

释放页面：

1. 将页面ID添加到空闲页面列表
2. 供后续分配使用

### int32_t GetFileSize() const

获取数据库文件大小（页面数）：

1. 通过文件大小除以页面大小计算页面数量

### bool Sync()

同步文件到磁盘：

1. 调用文件流的sync方法或操作系统提供的同步函数

### std::unordered_map<std::string, uint64_t> GetIOStats() const

获取磁盘I/O统计信息：

1. 收集并返回各种I/O统计指标

### void ResetIOStats()

重置磁盘I/O统计信息：

1. 将所有统计计数器重置为0

## 私有方法

### bool OpenFile()

打开数据库文件：

1. 使用std::fstream打开文件
2. 设置适当的打开模式

### void CloseFile()

关闭数据库文件：

1. 调用文件流的close方法

### bool InitializeFile()

初始化文件（如果不存在）：

1. 创建新文件
2. 写入文件头信息
3. 初始化页面分配信息

### bool ReadFileHeader()

读取文件头：

1. 定位到文件开头
2. 读取固定大小的文件头数据

### bool WriteFileHeader()

写入文件头：

1. 定位到文件开头
2. 写入固定大小的文件头数据

### void OnConfigChange(const std::string& key, const ConfigValue& value)

配置变更回调处理：

1. 根据变更的配置项调整相应的磁盘管理器参数

## 成员变量

### std::string db_file_name_

数据库文件名，用于打开和操作文件。

### ConfigManager& config_manager_

配置管理器引用，用于获取配置参数。

### std::fstream db_io_

数据库文件流，负责数据库文件的I/O操作。

### size_t file_size_

文件大小（字节），用于边界检查和页面分配。

### mutable std::recursive_timed_mutex io_mutex_

递归定时互斥锁，保护所有文件I/O操作，支持同一线程多次锁定和超时功能。

### int32_t next_page_id_

下一个要分配的页面ID，确保页面ID的唯一性。

### std::vector<int32_t> free_pages_

空闲页面列表，记录被释放的页面，以便重新使用。

### size_t lock_timeout_ms_

锁超时时间（毫秒），限制锁获取的等待时间，避免死锁导致的长时间阻塞。