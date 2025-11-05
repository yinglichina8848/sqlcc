#pragma once

#include <cstdint>
#include <cstring>

namespace sqlcc {

// 定义页面大小为8KB
static constexpr size_t PAGE_SIZE = 8192;

/**
 * @brief 数据库页面类，表示一个8KB的定长页面
 */
class Page {
public:
    /**
     * @brief 构造函数
     */
    Page();

    /**
     * @brief 构造函数，使用指定的页面ID初始化
     * @param page_id 页面ID
     */
    explicit Page(int32_t page_id);

    /**
     * @brief 析构函数
     */
    ~Page();

    /**
     * @brief 获取页面ID
     * @return 页面ID
     */
    inline int32_t GetPageId() const { return page_id_; }

    /**
     * @brief 设置页面ID
     * @param page_id 新的页面ID
     */
    inline void SetPageId(int32_t page_id) { page_id_ = page_id; }

    /**
     * @brief 获取页面数据指针
     * @return 指向页面数据的指针
     */
    inline char* GetData() { return data_; }

    /**
     * @brief 获取页面数据指针(const版本)
     * @return 指向页面数据的const指针
     */
    inline const char* GetData() const { return data_; }

    /**
     * @brief 将数据写入页面
     * @param offset 写入偏移量
     * @param data 要写入的数据
     * @param size 数据大小
     */
    void WriteData(size_t offset, const char* data, size_t size);

    /**
     * @brief 从页面读取数据
     * @param offset 读取偏移量
     * @param data 读取数据的缓冲区
     * @param size 要读取的数据大小
     */
    void ReadData(size_t offset, char* data, size_t size) const;

private:
    // 页面ID
    int32_t page_id_ = -1;
    
    // 页面数据缓冲区，大小为PAGE_SIZE(8KB)
    char data_[PAGE_SIZE] = {0};
};

}  // namespace sqlcc