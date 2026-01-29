# Page类详细设计

## 概述

Page类是存储引擎中表示内存中数据页的基本单元。每个页面大小固定为8KB，用于存储数据库中的实际数据、索引信息或元数据。

## 类定义

```cpp
class Page {
public:
    Page();
    explicit Page(int32_t page_id);
    ~Page();
    
    inline int32_t GetPageId() const;
    inline void SetPageId(int32_t page_id);
    inline char* GetData();
    inline const char* GetData() const;
    void WriteData(size_t offset, const char* data, size_t size);
    void ReadData(size_t offset, char* data, size_t size) const;

private:
    int32_t page_id_ = -1;
    char data_[PAGE_SIZE] = {0};
};
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

获取页面数据指针：

1. 直接返回data_数组的指针
2. 使用inline关键字提高性能

### const char* GetData() const

获取页面数据指针(const版本)：

1. 直接返回data_数组的const指针
2. 使用inline关键字提高性能

### void WriteData(size_t offset, const char* data, size_t size)

将数据写入页面：

1. 将指定大小的数据从源缓冲区复制到页面的指定偏移量处
2. 使用memcpy函数进行内存复制
3. 检查边界条件确保不会越界写入

### void ReadData(size_t offset, char* data, size_t size) const

从页面读取数据：

1. 从页面的指定偏移量处读取指定大小的数据到目标缓冲区
2. 使用memcpy函数进行内存复制
3. 检查边界条件确保不会越界读取

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