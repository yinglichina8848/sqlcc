# Page类详细设计

## 概述

Page类是SQLCC数据库存储系统的基本单位，封装了一个8KB的固定大小数据块，包含页面ID和实际数据。它提供了安全的读写操作接口，是数据库在磁盘和内存之间高效传输和管理数据的核心组件。

## 核心功能

- **固定大小数据块**：8KB的固定大小设计，平衡了I/O效率和内存使用
- **唯一标识**：通过页面ID唯一标识每个页面
- **安全数据访问**：提供类型安全和边界检查的读写操作
- **跨版本兼容**：支持C++17和C++20的span特性
- **异常处理**：边界检查和错误处理机制
- **调试支持**：详细的日志记录用于调试和性能分析

## 类定义

```cpp
static constexpr size_t PAGE_SIZE = 8192;

class Page {
public:
    Page();
    explicit Page(int32_t page_id);
    ~Page();
    
    inline int32_t GetPageId() const;
    inline void SetPageId(int32_t page_id);
    
    [[deprecated("Use GetDataSpan() for safe access")]] inline char* GetData();
    [[deprecated("Use GetDataSpan() for safe access")]] inline const char* GetData() const;
    
    #ifdef __cpp_lib_span
    inline std::span<char> GetDataSpan();
    inline std::span<const char> GetDataSpan() const;
    #else
    struct PageDataView;
    inline PageDataView GetDataSpan();
    inline const PageDataView GetDataSpan() const;
    #endif
    
    [[deprecated("Use WriteDataSpan() for safe write operations")]] void WriteData(size_t offset, const char* data, size_t size);
    [[deprecated("Use ReadDataToSpan() for safe read operations")]] void ReadData(size_t offset, char* data, size_t size) const;
    
    void WriteDataSpan(size_t offset, const char* data, size_t size);
    void ReadDataToSpan(size_t offset, void* output_data, size_t size) const;

private:
    int32_t page_id_ = -1;
    char data_[PAGE_SIZE] = {0};
};
```
```

## 构造函数

### Page()

默认构造函数创建一个新页面：

1. 初始化页面ID为-1(表示无效页面)
2. 清零数据缓冲区

### Page(int32_t page_id)

带参数的构造函数创建一个新页面：

1. 使用指定的页面ID初始化
2. 清零数据缓冲区

## 析构函数

### ~Page()

析构函数负责清理页面对象的资源：

1. 当前实现使用默认析构函数，因为Page类没有动态分配的资源

## 公共方法

### int32_t GetPageId() const

获取页面ID：

1. 直接返回page_id_成员变量的值
2. 使用inline关键字提高性能

### void SetPageId(int32_t page_id)

设置页面ID：

1. 直接将传入的page_id参数赋值给page_id_成员变量
2. 使用inline关键字提高性能

### char* GetData()

获取页面数据指针（已弃用）：

1. 直接返回data_数组的指针
2. 使用inline关键字提高性能
3. 已标记为弃用，建议使用GetDataSpan()

### const char* GetData() const

获取页面数据指针(const版本)（已弃用）：

1. 直接返回data_数组的const指针
2. 使用inline关键字提高性能
3. 已标记为弃用，建议使用GetDataSpan()

### std::span<char> GetDataSpan() / PageDataView GetDataSpan()

获取页面数据的安全span视图：

1. 在C++20中返回std::span<char>，在C++17中返回自定义的PageDataView
2. 提供类型安全和边界检查的数据访问方式
3. 避免裸指针操作，提高代码安全性

### void WriteData(size_t offset, const char* data, size_t size)

将数据写入页面（已弃用）：

1. 将指定大小的数据从源缓冲区复制到页面的指定偏移量处
2. 使用memcpy函数进行内存复制
3. 检查边界条件确保不会越界写入
4. 已标记为弃用，建议使用WriteDataSpan()

### void ReadData(size_t offset, char* data, size_t size) const

从页面读取数据（已弃用）：

1. 从页面的指定偏移量处读取指定大小的数据到目标缓冲区
2. 使用memcpy函数进行内存复制
3. 检查边界条件确保不会越界读取
4. 已标记为弃用，建议使用ReadDataToSpan()

### void WriteDataSpan(size_t offset, const char* data, size_t size)

使用span安全地写入数据到页面：

1. 检查边界条件，防止越界写入
2. 使用memcpy进行安全的数据复制
3. 提供类型安全和边界检查的数据写入方式

### void ReadDataToSpan(size_t offset, void* output_data, size_t size) const

使用span安全地从页面读取数据：

1. 检查边界条件，防止越界读取
2. 使用memcpy进行安全的数据复制
3. 提供类型安全和边界检查的数据读取方式

## 成员变量

### int32_t page_id_

页面ID：

1. 存储当前页面的唯一标识符
2. -1表示无效页面
3. 使用int32_t类型存储页面ID，支持数十亿个页面

### char data_[PAGE_SIZE]

页面数据缓冲区：

1. 大小为PAGE_SIZE(8KB)
2. 存储页面的实际数据
3. 使用字符数组作为数据缓冲区
4. 初始化为全零