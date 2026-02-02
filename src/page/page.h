/**
 * @file page.h
 * @brief SQLCC存储引擎核心 - 8KB定长页面管理架构
 *
 * 页面（Page）是数据库存储系统的最小物理单位。本项目采用8KB定长页面设计，
 * 实现了磁盘块与内存缓冲区之间的无缝映射。它是所有高级存储结构（如B+树、数据表）的物理载体。
 *
 * 📚 配套教材参考：
 * - [第6章：物理存储管理](../../textbook/《数据库系统原理与开发实践》.md#第六章物理存储管理)
 * - [6.1 页面物理布局](../../textbook/《数据库系统原理与开发实践》.md#61-页面物理布局)
 * - [6.2 记录在页面中的组织](../../textbook/《数据库系统原理与开发实践》.md#62-记录在页面中的组织)
 * - [6.3 页面空间管理与分配](../../textbook/《数据库系统原理与开发实践》.md#63-页面空间管理与分配)
 *
 * WHY层 - 设计意图：
 *   数据库系统不直接操作字节流，而是操作页面。通过固定页面大小（8KB），可以：
 *   1. 匹配现代磁盘扇区（Sector）和操作系统页面（OS Page）的大小，优化I/O对齐性能。
 *   2. 简化内存缓冲区（Buffer Pool）的管理，实现固定大小的块置换算法。
 *   3. 为B+树提供固定的节点容量，便于计算树的高度和扇出。
 *
 * WHAT层 - 功能说明：
 *   - 物理存储容器：提供8192字节的原始存储空间。
 *   - 页面标识：通过PageID实现全局唯一寻址。
 *   - 安全访问：通过std::span提供边界检查的数据访问，防止缓冲区溢出。
 *   - 元数据同步：维护页面状态（虽然在Page类中精简，但通过封装支持复杂元数据）。
 *
 * HOW层 - 实现机制：
 *   - 内存对齐：使用静态字符数组 data_[8192] 确保连续物理存储。
 *   - RAII管理：Page对象的生命周期由智能指针和BufferPool共同维护。
 *   - 视图分离：GetDataSpan()将物理存储与逻辑访问解耦，支持C++20现代语法。
 *   - 无状态设计：Page类本身保持极简（无锁、无状态），将并发控制上移至存储管理层。
 *
 * 页面内部布局建议（在具体实现类中应用）：
 *   | Header (24-64 bytes) | Slots/Records (Data) | Free Space |
 *   | LSN, PageID, Flags   | Actual tuples/nodes  | Unallocated|
 *
 * 性能优化考虑：
 *   - **缓存行对齐**：8KB大小是Cache Line（通常64B）的倍数，减少伪共享。
 *   - **零拷贝视图**：std::span 避免了数据的二次拷贝。
 *   - **预分配策略**：配合DiskManager实现物理空间的连续预分配。
 *
 * @author SQLCC技术委员会
 * @version 1.2.6
 * @date 2026-02-02
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <span> // C++20 span支持

namespace sqlcc {

/**
 * @brief 定义页面大小为8KB
 *
 * 为什么选择8KB？
 * 1. **平衡点**：较小的页面（4KB）增加管理开销，较大的页面（16KB+）增加单次I/O延迟。
 * 2. **硬件兼容**：8KB能很好地契合现代SSD的闪存页大小。
 * 3. **B+树优化**：8KB通常能容纳50-200个索引项，保证树的高度保持在3-4层。
 */
static constexpr size_t PAGE_SIZE = 8192;

/**
 * @brief 数据库页面类 - 存储引擎的物理载体
 *
 * Page类是数据库系统中最基础的内存对象，它在磁盘上对应一个物理块。
 * 所有的B+树节点、数据行、元数据表最终都存储在Page实例中。
 */
class Page {
public:
    /**
     * @brief 默认构造函数
     * 初始化无效页面（ID = -1），常用于预分配缓冲区占位。
     */
    Page();

    /**
     * @brief 指定ID构造函数
     * @param page_id 全局唯一页面标识符
     */
    explicit Page(int32_t page_id);

    /**
     * @brief 析构函数
     */
    ~Page();

    /**
     * @brief 获取页面ID
     * @return int32_t 页面标识符
     * 
     * WHY: 页面ID是逻辑寻址的唯一凭证，磁盘上的偏移量通常通过 ID * PAGE_SIZE 计算。
     */
    inline int32_t GetPageId() const { return page_id_; }

    /**
     * @brief 设置页面ID
     * @param page_id 新的页面标识符
     */
    inline void SetPageId(int32_t page_id) { page_id_ = page_id; }

    /**
     * @brief 获取原始数据指针 (不安全)
     * @deprecated 建议使用 GetDataSpan() 进行带边界检查的访问
     */
    [[deprecated("Use GetDataSpan() for safe access")]] inline char* GetData() { return data_; }

    /**
     * @brief 获取只读原始数据指针 (不安全)
     * @deprecated 建议使用 GetDataSpan() 进行带边界检查的访问
     */
    [[deprecated("Use GetDataSpan() for safe access")]] inline const char* GetData() const { return data_; }

    /**
     * @brief 获取页面数据的安全视图
     * 
     * WHY: std::span 提供了类似于指针的性能，但带有大小信息，是C++20推荐的内存操作方式。
     * WHAT: 返回指向 data_ 数组的 span，覆盖整个 PAGE_SIZE。
     * HOW: 调用方式：auto view = page.GetDataSpan(); view[0] = ...;
     */
    #ifdef __cpp_lib_span
    inline std::span<char> GetDataSpan() { return std::span<char>(data_, PAGE_SIZE); }
    inline std::span<const char> GetDataSpan() const { return std::span<const char>(data_, PAGE_SIZE); }
    #else
    // C++17兼容实现，模拟span行为
    struct PageDataView {
        char* data;
        size_t size;
        char* begin() { return data; }
        char* end() { return data + size; }
        const char* begin() const { return data; }
        const char* end() const { return data + size; }
        char& operator[](size_t idx) { return data[idx]; }
    };
    inline PageDataView GetDataSpan() { return PageDataView{data_, PAGE_SIZE}; }
    inline const PageDataView GetDataSpan() const { return PageDataView{const_cast<char*>(data_), PAGE_SIZE}; }
    #endif

    /**
     * @brief 写入数据到页面偏移位置
     * @param offset 页面内偏移量 [0, PAGE_SIZE)
     * @param data 源代码缓冲区
     * @param size 写入字节数
     * 
     * WHY: 提供统一的、带边界检查的数据写入接口，防止内存越界。
     */
    void WriteDataSpan(size_t offset, const char* data, size_t size);

    /**
     * @brief 从页面偏移位置读取数据
     * @param offset 页面内偏移量
     * @param output_data 目标缓冲区
     * @param size 读取字节数
     */
    void ReadDataToSpan(size_t offset, void* output_data, size_t size) const;

    // 遗留接口，保持兼容性但标记弃用
    [[deprecated("Use WriteDataSpan() for safe write operations")]] void WriteData(size_t offset, const char* data, size_t size);
    [[deprecated("Use ReadDataToSpan() for safe read operations")]] void ReadData(size_t offset, char* data, size_t size) const;

private:
    /**
     * @brief 页面唯一ID
     * -1 表示这是一个无效或未初始化的页面。
     */
    int32_t page_id_ = -1;
    
    /**
     * @brief 页面物理存储区
     * 严格限制为 PAGE_SIZE 大小。
     */
    char data_[PAGE_SIZE] = {0};
};

}  // namespace sqlcc
