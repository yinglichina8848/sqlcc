#include "page.h"
#include "exception.h"
#include "logger.h"
#include <cstring>

namespace sqlcc {

// 默认构造函数实现
Page::Page() : page_id_(-1) {
    // 初始化页面数据为0
    memset(data_, 0, PAGE_SIZE);
    
    // 记录页面创建，便于调试
    SQLCC_LOG_DEBUG("Creating default page with ID: -1");
}

// 带参数的构造函数实现
Page::Page(int32_t page_id) : page_id_(page_id) {
    // 初始化页面数据为0
    memset(data_, 0, PAGE_SIZE);
    
    // 记录页面创建，便于调试
    SQLCC_LOG_DEBUG("Creating page with ID: " + std::to_string(page_id));
}

// 析构函数实现
Page::~Page() {
    // 记录页面销毁，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在销毁的页面ID
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Destroying page with ID: " + std::to_string(page_id_));
}

// 写入数据到页面实现
void Page::WriteData(size_t offset, const char* src, size_t size) {
    // 记录写入操作，便于调试
    SQLCC_LOG_DEBUG("Writing data to page ID " + std::to_string(page_id_) + 
                   " at offset " + std::to_string(offset) + " with size " + std::to_string(size));
    
    // 检查边界
    if (offset + size > PAGE_SIZE) {
        // 创建错误消息
        std::string error_msg = "WriteData out of bounds: offset=" + std::to_string(offset) + 
                               ", size=" + std::to_string(size) + ", page_size=" + std::to_string(PAGE_SIZE);
        // 记录错误，便于调试
        SQLCC_LOG_ERROR(error_msg);
        // 抛出异常，终止操作
        throw PageException(error_msg);
    }
    
    // 复制数据
    memcpy(data_ + offset, src, size);
    // 记录写入成功，便于调试
    SQLCC_LOG_DEBUG("Successfully wrote data to page ID " + std::to_string(page_id_));
}

// 从页面读取数据实现
void Page::ReadData(size_t offset, char* dest, size_t size) const {
    // 记录读取操作，便于调试
    SQLCC_LOG_DEBUG("Reading data from page ID " + std::to_string(page_id_) + 
                   " at offset " + std::to_string(offset) + " with size " + std::to_string(size));
    
    // 检查边界
    if (offset + size > PAGE_SIZE) {
        // 创建错误消息
        std::string error_msg = "ReadData out of bounds: offset=" + std::to_string(offset) + 
                               ", size=" + std::to_string(size) + ", page_size=" + std::to_string(PAGE_SIZE);
        // 记录错误，便于调试
        SQLCC_LOG_ERROR(error_msg);
        // 抛出异常，终止操作
        throw PageException(error_msg);
    }
    
    // 复制数据
    // Why: 需要将页面数据复制到目标缓冲区
    // What: 使用memcpy函数进行内存复制
    // How: 调用memcpy函数，传入目标地址、源地址和大小
    memcpy(dest, data_ + offset, size);
    // 记录读取成功，便于调试
    SQLCC_LOG_DEBUG("Successfully read data from page ID " + std::to_string(page_id_));
}

} // namespace sqlcc