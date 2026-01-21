#pragma once

#include <cstdint>
#include <cstring>
#include <span> // C++20 span支持

namespace sqlcc {

// 定义页面大小为8KB
// Why: 数据库系统通常使用固定大小的页面作为磁盘和内存之间数据传输的基本单位
// What: PAGE_SIZE常量定义了每个页面的固定大小为8192字节(8KB)
// How: 使用static constexpr关键字定义编译时常量，8KB是常见的数据库页面大小，平衡了I/O效率和内存使用
static constexpr size_t PAGE_SIZE = 8192;

/**
 * @brief 数据库页面类，表示一个8KB的定长页面
 * 
 * Why: 数据库系统需要将数据组织成固定大小的页面，以便在磁盘和内存之间高效传输和管理
 * What: Page类封装了一个固定大小的数据块，包含页面ID和实际数据，是数据库存储系统的基本单位
 * How: 使用字符数组存储页面数据，提供读写操作接口，通过页面ID唯一标识每个页面
 */
class Page {
public:
    /**
     * @brief 构造函数
     * 
     * Why: 需要创建页面对象并初始化其内部状态
     * What: 默认构造函数创建一个新页面，初始化页面ID为-1(表示无效页面)，清零数据缓冲区
     * How: 使用成员初始化列表设置page_id_为-1，使用{0}初始化data_数组为全零
     */
    Page();

    /**
     * @brief 构造函数，使用指定的页面ID初始化
     * @param page_id 页面ID
     * 
     * Why: 有时需要在创建页面对象时直接指定其ID，例如从磁盘读取页面时
     * What: 带参数的构造函数创建一个新页面，使用指定的页面ID初始化，清零数据缓冲区
     * How: 使用成员初始化列表设置page_id_为传入的参数值，使用{0}初始化data_数组为全零
     */
    explicit Page(int32_t page_id);

    /**
     * @brief 析构函数
     * 
     * Why: 需要在页面对象销毁时释放资源(虽然当前实现没有动态资源)
     * What: 析构函数负责清理页面对象的资源
     * How: 当前实现使用默认析构函数，因为Page类没有动态分配的资源
     */
    ~Page();

    /**
     * @brief 获取页面ID
     * @return 页面ID
     * 
     * Why: 需要知道页面的唯一标识符，以便进行页面管理和定位
     * What: GetPageId方法返回当前页面的ID值
     * How: 直接返回page_id_成员变量的值，使用inline关键字提高性能
     */
    inline int32_t GetPageId() const { return page_id_; }

    /**
     * @brief 设置页面ID
     * @param page_id 新的页面ID
     * 
     * Why: 有时需要修改页面的ID，例如从磁盘读取页面后设置正确的ID
     * What: SetPageId方法将当前页面的ID设置为指定值
     * How: 直接将传入的page_id参数赋值给page_id_成员变量，使用inline关键字提高性能
     */
    inline void SetPageId(int32_t page_id) { page_id_ = page_id; }

    /**
     * @brief 获取页面数据指针
     * @return 指向页面数据的指针
     * 
     * Why: 需要访问页面中的实际数据，以便进行读写操作
     * What: GetData方法返回指向页面数据缓冲区的指针
     * How: 直接返回data_数组的指针，使用inline关键字提高性能
     * @deprecated 建议使用GetDataSpan()进行安全访问
     */
    [[deprecated("Use GetDataSpan() for safe access")]] inline char* GetData() { return data_; }

    /**
     * @brief 获取页面数据指针(const版本)
     * @return 指向页面数据的const指针
     * 
     * Why: 在const上下文中需要访问页面数据，但不允许修改
     * What: GetData方法(const版本)返回指向页面数据缓冲区的const指针
     * How: 直接返回data_数组的const指针，使用inline关键字提高性能
     * @deprecated 建议使用GetDataSpan()进行安全访问
     */
    [[deprecated("Use GetDataSpan() for safe access")]] inline const char* GetData() const { return data_; }

    /**
     * @brief 获取页面数据的安全span视图
     * @return std::span<char> 页面数据的安全视图
     * 
     * Why: 提供类型安全和边界检查的数据访问方式，避免裸指针操作
     * What: GetDataSpan方法返回一个std::span，包含整个页面的数据
     * How: 使用std::span包装data_数组，提供安全的内存访问
     */
    #ifdef __cpp_lib_span
    inline std::span<char> GetDataSpan() { return std::span<char>(data_, PAGE_SIZE); }
    inline std::span<const char> GetDataSpan() const { return std::span<const char>(data_, PAGE_SIZE); }
    #else
    // C++17兼容实现
    struct PageDataView {
        char* data;
        size_t size;
        char* begin() { return data; }
        char* end() { return data + size; }
        const char* begin() const { return data; }
        const char* end() const { return data + size; }
    };
    inline PageDataView GetDataSpan() { return PageDataView{data_, PAGE_SIZE}; }
    inline const PageDataView GetDataSpan() const { return PageDataView{const_cast<char*>(data_), PAGE_SIZE}; }
    #endif

    /**
     * @brief 将数据写入页面
     * @param offset 写入偏移量
     * @param data 要写入的数据
     * @param size 数据大小
     * 
     * Why: 需要将外部数据写入到页面的特定位置，例如存储记录或元数据
     * What: WriteData方法将指定大小的数据从源缓冲区复制到页面的指定偏移量处
     * How: 使用memcpy函数进行内存复制，检查边界条件确保不会越界写入
     * @deprecated 建议使用WriteDataSpan()进行安全写入
     */
    [[deprecated("Use WriteDataSpan() for safe write operations")]] void WriteData(size_t offset, const char* data, size_t size);

    /**
     * @brief 从页面读取数据
     * @param offset 读取偏移量
     * @param data 读取数据的缓冲区
     * @param size 要读取的数据大小
     * 
     * Why: 需要从页面的特定位置读取数据，例如读取记录或元数据
     * What: ReadData方法从页面的指定偏移量处读取指定大小的数据到目标缓冲区
     * How: 使用memcpy函数进行内存复制，检查边界条件确保不会越界读取
     * @deprecated 建议使用ReadDataToSpan()进行安全读取
     */
    [[deprecated("Use ReadDataToSpan() for safe read operations")]] void ReadData(size_t offset, char* data, size_t size) const;

    /**
     * @brief 使用span安全地写入数据到页面
     * @param offset 写入偏移量
     * @param data_span 要写入的数据span
     * 
     * Why: 提供类型安全和边界检查的数据写入方式
     * What: WriteDataSpan方法将span中的数据写入页面的指定偏移量处
     * How: 检查边界条件，使用memcpy进行安全的数据复制
     */
    void WriteDataSpan(size_t offset, const char* data, size_t size);

    /**
     * @brief 使用span安全地从页面读取数据
     * @param offset 读取偏移量
     * @param output_data 用于接收数据的缓冲区
     * @param size 要读取的数据大小
     *
     * Why: 提供类型安全和边界检查的数据读取方式
     * What: ReadDataToSpan方法从页面的指定偏移量处读取数据到目标缓冲区
     * How: 检查边界条件，使用memcpy进行安全的数据复制
     */
    void ReadDataToSpan(size_t offset, void* output_data, size_t size) const;

private:
    // 页面ID
    // Why: 每个页面需要唯一标识符，以便在数据库中定位和管理
    // What: page_id_成员变量存储当前页面的唯一标识符，-1表示无效页面
    // How: 使用int32_t类型存储页面ID，支持数十亿个页面
    int32_t page_id_ = -1;
    
    // 页面数据缓冲区，大小为PAGE_SIZE(8KB)
    // Why: 需要固定大小的缓冲区来存储页面数据，这是数据库存储的基本单位
    // What: data_数组存储页面的实际数据，大小为PAGE_SIZE(8KB)
    // How: 使用字符数组作为数据缓冲区，初始化为全零
    char data_[PAGE_SIZE] = {0};
};

}  // namespace sqlcc
