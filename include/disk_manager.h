#pragma once

#include "page.h"
#include <string>
#include <fstream>

namespace sqlcc {

/**
 * @brief 磁盘管理器类，负责页面的磁盘I/O操作
 */
class DiskManager {
public:
    /**
     * @brief 构造函数
     * @param db_file 数据库文件路径
     */
    explicit DiskManager(const std::string& db_file);

    /**
     * @brief 析构函数
     */
    ~DiskManager();

    /**
     * @brief 禁止拷贝构造
     */
    DiskManager(const DiskManager&) = delete;

    /**
     * @brief 禁止赋值操作
     */
    DiskManager& operator=(const DiskManager&) = delete;

    /**
     * @brief 将页面写入磁盘
     * @param page 要写入的页面
     * @return 写入成功返回true，否则返回false
     */
    bool WritePage(const Page& page);

    /**
     * @brief 从磁盘读取页面
     * @param page_id 页面ID
     * @param page 用于存储读取数据的页面对象
     * @return 读取成功返回true，否则返回false
     */
    bool ReadPage(int32_t page_id, Page* page);

    /**
     * @brief 分配新页面，返回页面ID
     * @return 新分配的页面ID
     */
    int32_t AllocatePage();

    /**
     * @brief 获取数据库文件大小
     * @return 数据库文件大小（以字节为单位）
     */
    size_t GetFileSize() const;

private:
    // 数据库文件路径
    std::string db_file_;

    // 数据库文件流
    std::fstream db_io_;

    // 数据库文件大小
    size_t file_size_;

    // 页面计数器
    int32_t next_page_id_;
};

}  // namespace sqlcc