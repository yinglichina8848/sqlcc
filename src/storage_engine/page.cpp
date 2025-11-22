#include "page.h"
#include "exception.h"
#include "logger.h"
#include <cstring>

namespace sqlcc {

// 默认构造函数实现
// Why: 需要创建一个页面对象，使用默认初始化值
// What: 使用编译器生成的默认构造函数，初始化page_id_为-1，data_数组为全零
// How: 使用= default让编译器生成默认实现，提高代码简洁性
Page::Page() = default;

// 带参数的构造函数实现
// Why: 需要创建一个具有指定ID的页面对象，并初始化其数据
// What: 构造函数接收一个页面ID，初始化page_id_成员，并清零数据缓冲区
// How: 使用成员初始化列表设置page_id_，使用memset清零data_数组
Page::Page(int32_t page_id) : page_id_(page_id) {
    // 记录页面创建，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在创建的页面ID
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Creating page with ID: " + std::to_string(page_id_));
    
    // 初始化页面数据为0
    // Why: 需要确保新页面的数据区域是干净的，不包含随机数据
    // What: 使用memset将data_数组的所有字节设置为0
    // How: 调用memset函数，传入数组指针、0值和数组大小
    memset(data_, 0, PAGE_SIZE);
}

// 析构函数实现
// Why: 需要在页面对象销毁时记录日志，便于调试和资源管理
// What: 析构函数记录页面销毁的日志信息
// How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
Page::~Page() {
    // 记录页面销毁，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在销毁的页面ID
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Destroying page with ID: " + std::to_string(page_id_));
}

// 写入数据到页面实现
// Why: 需要将外部数据写入到页面的特定位置，例如存储记录或元数据
// What: WriteData方法将指定大小的数据从源缓冲区复制到页面的指定偏移量处
// How: 检查边界条件，使用memcpy进行内存复制
void Page::WriteData(size_t offset, const char* src, size_t size) {
    // 记录写入操作，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在写入的页面ID、偏移量和数据大小
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Writing data to page ID " + std::to_string(page_id_) + 
                   " at offset " + std::to_string(offset) + " with size " + std::to_string(size));
    
    // 检查边界
    // Why: 防止写入超出页面边界，避免内存越界和缓冲区溢出
    // What: 检查偏移量加上数据大小是否超过页面大小
    // How: 使用if语句比较offset+size和PAGE_SIZE，如果越界则抛出异常
    if (offset + size > PAGE_SIZE) {
        // 创建错误消息
        // Why: 需要提供详细的错误信息，帮助调试和问题定位
        // What: 构建包含偏移量、数据大小和页面大小的错误消息
        // How: 使用字符串拼接和std::to_string函数
        std::string error_msg = "WriteData out of bounds: offset=" + std::to_string(offset) + 
                               ", size=" + std::to_string(size) + ", page_size=" + std::to_string(PAGE_SIZE);
        // 记录错误，便于调试
        // Why: 错误日志有助于问题排查和系统监控
        // What: 记录错误消息
        // How: 使用SQLCC_LOG_ERROR宏记录错误级别日志
        SQLCC_LOG_ERROR(error_msg);
        // 抛出异常，终止操作
        // Why: 边界检查失败是严重错误，需要终止操作以避免数据损坏
        // What: 抛出PageException异常
        // How: 使用throw关键字抛出异常
        throw PageException(error_msg);
    }
    
    // 复制数据
    // Why: 需要将源数据复制到页面的指定位置
    // What: 使用memcpy函数进行内存复制
    // How: 调用memcpy函数，传入目标地址、源地址和大小
    memcpy(data_ + offset, src, size);
    // 记录写入成功，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录写入成功的页面ID
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Successfully wrote data to page ID " + std::to_string(page_id_));
}

// 从页面读取数据实现
// Why: 需要从页面的特定位置读取数据，例如读取记录或元数据
// What: ReadData方法从页面的指定偏移量处读取指定大小的数据到目标缓冲区
// How: 检查边界条件，使用memcpy进行内存复制
void Page::ReadData(size_t offset, char* dest, size_t size) const {
    // 记录读取操作，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在读取的页面ID、偏移量和数据大小
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Reading data from page ID " + std::to_string(page_id_) + 
                   " at offset " + std::to_string(offset) + " with size " + std::to_string(size));
    
    // 检查边界
    // Why: 防止读取超出页面边界，避免内存越界和读取无效数据
    // What: 检查偏移量加上数据大小是否超过页面大小
    // How: 使用if语句比较offset+size和PAGE_SIZE，如果越界则抛出异常
    if (offset + size > PAGE_SIZE) {
        // 创建错误消息
        // Why: 需要提供详细的错误信息，帮助调试和问题定位
        // What: 构建包含偏移量、数据大小和页面大小的错误消息
        // How: 使用字符串拼接和std::to_string函数
        std::string error_msg = "ReadData out of bounds: offset=" + std::to_string(offset) + 
                               ", size=" + std::to_string(size) + ", page_size=" + std::to_string(PAGE_SIZE);
        // 记录错误，便于调试
        // Why: 错误日志有助于问题排查和系统监控
        // What: 记录错误消息
        // How: 使用SQLCC_LOG_ERROR宏记录错误级别日志
        SQLCC_LOG_ERROR(error_msg);
        // 抛出异常，终止操作
        // Why: 边界检查失败是严重错误，需要终止操作以避免数据损坏
        // What: 抛出PageException异常
        // How: 使用throw关键字抛出异常
        throw PageException(error_msg);
    }
    
    // 复制数据
    // Why: 需要将页面数据复制到目标缓冲区
    // What: 使用memcpy函数进行内存复制
    // How: 调用memcpy函数，传入目标地址、源地址和大小
    memcpy(dest, data_ + offset, size);
    // 记录读取成功，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录读取成功的页面ID
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Successfully read data from page ID " + std::to_string(page_id_));
}

}  // namespace sqlcc