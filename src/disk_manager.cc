#include "disk_manager.h"
#include "exception.h"
#include "logger.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

// 定义页面大小为8KB，与page.h中的定义保持一致
constexpr size_t PAGE_SIZE = 8192;

namespace sqlcc {

// 磁盘管理器构造函数实现
// Why: 需要初始化磁盘管理器，打开数据库文件并准备进行I/O操作
// What: 构造函数接收数据库文件路径和配置管理器引用，打开文件流，初始化文件大小和页面计数器
// How: 使用fstream打开文件，如果文件不存在则创建新文件，获取文件大小并计算页面数量
DiskManager::DiskManager(const std::string& db_file, ConfigManager& config_manager)
    : db_file_name_(db_file), config_manager_(config_manager), file_size_(0), next_page_id_(0) {
    // 记录初始化信息，便于调试和监控
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在初始化的数据库文件路径
    // How: 使用SQLCC_LOG_INFO宏记录信息级别日志
    SQLCC_LOG_INFO("Initializing DiskManager for database file: " + db_file_name_);
    
    // 注册配置变更回调
    // Why: 需要响应配置变更，动态调整磁盘管理器行为
    // What: 注册配置变更回调函数，当配置发生变更时调用OnConfigChange方法
    // How: 使用配置管理器的RegisterChangeCallback方法注册回调，并存储返回的回调ID
    direct_io_callback_id_ = config_manager_.RegisterChangeCallback("disk_manager.enable_direct_io", 
        [this](const std::string& key, const ConfigValue& value) {
            this->OnConfigChange(key, value);
        });
    
    io_queue_depth_callback_id_ = config_manager_.RegisterChangeCallback("disk_manager.io_queue_depth", 
        [this](const std::string& key, const ConfigValue& value) {
            this->OnConfigChange(key, value);
        });
    
    async_io_callback_id_ = config_manager_.RegisterChangeCallback("disk_manager.enable_async_io", 
        [this](const std::string& key, const ConfigValue& value) {
            this->OnConfigChange(key, value);
        });
    
    batch_io_size_callback_id_ = config_manager_.RegisterChangeCallback("disk_manager.batch_io_size", 
        [this](const std::string& key, const ConfigValue& value) {
            this->OnConfigChange(key, value);
        });
    
    sync_strategy_callback_id_ = config_manager_.RegisterChangeCallback("disk_manager.sync_strategy", 
        [this](const std::string& key, const ConfigValue& value) {
            this->OnConfigChange(key, value);
        });
    
    sync_interval_callback_id_ = config_manager_.RegisterChangeCallback("disk_manager.sync_interval", 
        [this](const std::string& key, const ConfigValue& value) {
            this->OnConfigChange(key, value);
        });

    // 以读写模式打开文件，如果文件不存在则创建
    // Why: 需要打开数据库文件进行读写操作，如果文件不存在则需要创建
    // What: 使用fstream的open方法打开文件，指定二进制模式和读写权限
    // How: 使用std::ios::binary指定二进制模式，std::ios::in|std::ios::out指定读写权限，std::ios::app允许追加
    db_io_.open(db_file_name_, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
    
    // 如果文件不存在，创建一个空文件
    // Why: 如果文件不存在，第一次打开会失败，需要创建新文件
    // What: 检查文件流状态，如果失败则清除错误状态并重新创建文件
    // How: 使用good()方法检查文件流状态，使用clear()清除错误状态，使用trunc模式创建空文件
    if (!db_io_.good()) {
        SQLCC_LOG_INFO("Database file does not exist, creating new file: " + db_file_name_);
        db_io_.clear();
        db_io_.open(db_file_name_, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
    }
    
    // 获取文件大小
    // Why: 需要知道文件大小以计算页面数量和进行边界检查
    // What: 将文件指针移动到文件末尾，获取当前位置即为文件大小
    // How: 使用seekg移动到文件末尾，使用tellg获取当前位置
    if (db_io_.good()) {
        db_io_.seekg(0, std::ios::end);
        file_size_ = db_io_.tellg();
        // 计算下一个可用的页面ID（文件大小除以页面大小）
        // Why: 需要知道下一个可用的页面ID，以便分配新页面
        // What: 将文件大小除以页面大小，得到当前页面数量，即为下一个可用的页面ID
        // How: 使用整数除法计算，将结果转换为int32_t类型
        next_page_id_ = static_cast<int32_t>(file_size_ / PAGE_SIZE);
        SQLCC_LOG_INFO("Opened database file: " + db_file_name_ + ", file size: " + 
                      std::to_string(file_size_) + ", next page ID: " + std::to_string(next_page_id_));
    } else {
        // 文件打开失败，抛出异常
        // Why: 如果无法打开数据库文件，磁盘管理器无法正常工作，需要抛出异常
        // What: 创建错误消息，记录错误日志，然后抛出DiskManagerException异常
        // How: 使用SQLCC_LOG_ERROR记录错误级别日志，然后抛出异常
        std::string error_msg = "Failed to open database file: " + db_file_name_;
        SQLCC_LOG_ERROR(error_msg);
        throw DiskManagerException(error_msg);
    }
}

// 磁盘管理器析构函数实现
// Why: 需要释放文件流资源，确保文件正确关闭，防止数据丢失
// What: 析构函数负责关闭数据库文件流，释放系统资源
// How: 检查文件流是否打开，如果打开则调用close方法关闭
DiskManager::~DiskManager() {
    // 取消注册所有配置变更回调，避免内存泄漏和悬空指针
    // Why: 防止回调函数指向已销毁的对象，避免在后续配置变更时访问无效内存
    // What: 取消注册所有在构造函数中注册的配置变更回调
    // How: 使用配置管理器的UnregisterChangeCallback方法取消注册
    config_manager_.UnregisterChangeCallback(direct_io_callback_id_);
    config_manager_.UnregisterChangeCallback(io_queue_depth_callback_id_);
    config_manager_.UnregisterChangeCallback(async_io_callback_id_);
    config_manager_.UnregisterChangeCallback(batch_io_size_callback_id_);
    config_manager_.UnregisterChangeCallback(sync_strategy_callback_id_);
    config_manager_.UnregisterChangeCallback(sync_interval_callback_id_);
    
    // 检查文件流是否打开
    // Why: 只有在文件流打开的情况下才需要关闭
    // What: 使用is_open方法检查文件流状态
    // How: 如果文件流打开，则调用close方法关闭
    if (db_io_.is_open()) {
        SQLCC_LOG_INFO("Closing database file: " + db_file_name_);
        db_io_.close();
    }
}

/**
 * @brief 写入指定页的数据
 * 
 * @why 提供可靠的页写入功能：
 *       - 数据库修改操作需要持久化到磁盘
 *       - 保证写入操作的原子性和一致性
 *       - 处理各种I/O错误和边界情况
 *       - 提供详细的错误信息便于调试
 * 
 * @what 执行完整的页写入流程：
 *        - 验证页ID的有效性
 *        - 计算页在文件中的偏移位置
 *        - 定位到指定位置
 *        - 写入完整的页数据
 *        - 强制刷新到磁盘确保持久化
 *        - 记录操作结果日志
 * 
 * @how 使用文件流定位和写入：
 *       1. 检查页ID不能为负数
 *       2. 计算偏移量：page_id * PAGE_SIZE
 *       3. 使用seekp()定位到文件偏移位置
 *       4. 使用write()写入PAGE_SIZE字节数据
 *       5. 使用flush()强制刷新到磁盘
 *       6. 使用异常处理捕获写入错误
 *       7. 使用日志记录操作结果
 * 
 * @param page 要写入的页对象，包含页ID和数据
 * @return bool 写入成功返回true，失败返回false
 * 
 * @note 数据完整性：
 *       - 写入失败时文件状态保持不变
 *       - 刷新操作确保数据写入磁盘
 *       - 异常情况下返回false而不是抛出异常
 *       - 详细的错误日志便于问题定位
 * 
 * @warning 使用限制：
 *         - 页ID必须有效（>= 0）
 *         - 页数据必须有效且大小为PAGE_SIZE
 *         - 文件必须已打开且可写
 *         - 频繁调用flush可能影响性能
 *         - 大偏移量写入可能耗时较长
 * 
 * @performance 性能特点：
 *            - seekp()操作在SSD上很快，在HDD上需要寻道时间
 *            - write()操作性能取决于存储设备
 *            - flush()操作会强制磁盘同步，影响性能
 *            - 建议批量写入后统一刷新
 */
// 写入页面到磁盘实现
// Why: 数据库需要将修改后的页面持久化到磁盘，以保证数据的持久性和一致性
// What: WritePage方法接收页面ID和页面数据指针，将其内容写入到磁盘文件的对应位置
// How: 计算页面在文件中的偏移量，使用seekp定位到该位置，然后调用write方法写入页面数据
bool DiskManager::WritePage(int32_t page_id, const char* page_data) {
    // 添加锁定保护，避免递归锁定
    std::lock_guard<std::recursive_mutex> lock(io_mutex_);
    
    // 验证页面ID的有效性
    // Why: 页面ID必须是非负数，负数是无效的页面ID
    // What: 检查页面ID是否小于0
    // How: 使用if语句检查页面ID，如果无效则记录错误并返回false
    if (page_id < 0) {
        std::string error_msg = "Invalid page ID: " + std::to_string(page_id);
        SQLCC_LOG_ERROR(error_msg);
        return false;
    }
    
    // 验证页面数据指针的有效性
    // Why: 页面数据指针不能为空，否则无法写入数据
    // What: 检查页面数据指针是否为空
    // How: 使用if语句检查指针，如果为空则记录错误并返回false
    if (page_data == nullptr) {
        std::string error_msg = "Null page data pointer for page ID: " + std::to_string(page_id);
        SQLCC_LOG_ERROR(error_msg);
        return false;
    }
    
    // 计算页面在文件中的偏移量
    // Why: 需要知道页面在文件中的位置才能写入数据
    // What: 页面ID乘以页面大小得到页面在文件中的偏移量
    // How: 使用乘法计算，将结果转换为size_t类型
    size_t offset = static_cast<size_t>(page_id) * PAGE_SIZE;
    
    // 记录写入页面操作，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在写入的页面ID和在文件中的偏移量
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Writing page ID " + std::to_string(page_id) + " at offset " + std::to_string(offset));
    
    // 模拟定位失败（仅用于测试）
    if (simulate_seek_failure_) {
        std::string error_msg = "Failed to seek to page " + std::to_string(page_id);
        SQLCC_LOG_ERROR(error_msg);
        return false;
    }
    
    // 定位到页面位置
    // Why: 需要将文件指针移动到页面的起始位置才能写入数据
    // What: 使用seekp方法将文件指针移动到指定偏移量处
    // How: 调用fstream的seekp方法，传入偏移量和起始位置
    db_io_.seekp(offset, std::ios::beg);
    // 检查定位是否成功
    // Why: 定位失败可能是由于磁盘错误或文件损坏
    // What: 检查文件流状态，如果失败则记录错误并返回false
    // How: 使用fail方法检查文件流状态，使用clear清除错误状态
    if (db_io_.fail()) {
        std::string error_msg = "Failed to seek to page " + std::to_string(page_id);
        SQLCC_LOG_ERROR(error_msg);
        db_io_.clear();
        return false;
    }
    
    // 写入页面数据
    // Why: 需要将页面的数据写入到磁盘文件中
    // What: 调用write方法将页面数据写入文件
    // How: 使用页面数据指针，写入PAGE_SIZE字节
    db_io_.write(page_data, PAGE_SIZE);
    // 检查写入是否成功
    // Why: 写入失败可能是由于磁盘空间不足或磁盘错误
    // What: 检查文件流状态，如果失败则记录错误并返回false
    // How: 使用fail方法检查文件流状态，使用clear清除错误状态
    if (db_io_.fail()) {
        std::string error_msg = "Failed to write page " + std::to_string(page_id);
        SQLCC_LOG_ERROR(error_msg);
        db_io_.clear();
        return false;
    }
    
    // 更新文件大小
    // Why: 如果写入的页面超出了当前文件大小，需要更新文件大小
    // What: 计算新的文件大小，如果大于当前文件大小则更新
    // How: 计算页面偏移量加上页面大小，与当前文件大小比较
    size_t new_size = offset + PAGE_SIZE;
    if (new_size > file_size_) {
        file_size_ = new_size;
        SQLCC_LOG_DEBUG("Updated file size to " + std::to_string(file_size_));
    }
    
    // 刷新到磁盘
    // Why: 需要确保数据真正写入磁盘，而不仅仅是停留在缓冲区
    // What: 调用flush方法将缓冲区数据写入磁盘
    // How: 直接调用fstream的flush方法
    db_io_.flush();
    // 检查刷新是否成功
    // Why: 刷新失败可能是由于磁盘错误或磁盘空间不足
    // What: 检查文件流状态，如果失败则记录错误并返回false
    // How: 使用fail方法检查文件流状态，使用clear清除错误状态
    if (db_io_.fail()) {
        std::string error_msg = "Failed to flush page " + std::to_string(page_id) + " to disk";
        SQLCC_LOG_ERROR(error_msg);
        db_io_.clear();
        return false;
    }
    
    // 记录写入成功，便于调试
    SQLCC_LOG_DEBUG("Successfully wrote page ID " + std::to_string(page_id));
    return true;
}

// 从磁盘读取页面实现
// Why: 当缓冲池需要加载不在内存中的页面时，需要从磁盘读取页面数据
// What: ReadPage方法接收页面ID和页面数据缓冲区指针，从磁盘文件中读取对应页面的数据
// How: 计算页面在文件中的偏移量，使用seekg定位到该位置，然后调用read方法读取页面数据
bool DiskManager::ReadPage(int32_t page_id, char* page_data) {
    // 添加锁定保护，避免递归锁定
    std::lock_guard<std::recursive_mutex> lock(io_mutex_);
    
    // 验证参数的有效性
    // Why: 页面ID必须是非负数，页面数据缓冲区指针不能为空
    // What: 检查页面ID是否小于0，页面数据缓冲区指针是否为空
    // How: 使用if语句检查参数，如果无效则记录错误并返回false
    if (page_id < 0 || page_data == nullptr) {
        std::string error_msg = "Invalid page ID: " + std::to_string(page_id) + " or null page data pointer";
        SQLCC_LOG_ERROR(error_msg);
        return false;
    }
    
    // 计算页面在文件中的偏移量
    // Why: 需要知道页面在文件中的位置才能读取数据
    // What: 页面ID乘以页面大小得到页面在文件中的偏移量
    // How: 使用乘法计算，将结果转换为size_t类型
    size_t offset = static_cast<size_t>(page_id) * PAGE_SIZE;
    
    // 记录读取页面操作，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在读取的页面ID和在文件中的偏移量
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Reading page ID " + std::to_string(page_id) + " at offset " + std::to_string(offset));
    
    // 检查页面是否在文件范围内
    // Why: 不能读取超出文件范围的页面，否则会读取到无效数据
    // What: 检查页面偏移量是否小于文件大小
    // How: 使用if语句比较偏移量和文件大小
    if (offset >= file_size_) {
        std::string warn_msg = "Page " + std::to_string(page_id) + " does not exist in file";
        SQLCC_LOG_WARN(warn_msg);
        return false;
    }
    
    // 定位到页面位置
    // Why: 需要将文件指针移动到页面的起始位置才能读取数据
    // What: 使用seekg方法将文件指针移动到指定偏移量处
    // How: 调用fstream的seekg方法，传入偏移量和起始位置
    db_io_.seekg(offset, std::ios::beg);
    // 检查定位是否成功
    // Why: 定位失败可能是由于磁盘错误或文件损坏
    // What: 检查文件流状态，如果失败则记录错误并返回false
    // How: 使用fail方法检查文件流状态，使用clear清除错误状态
    if (db_io_.fail()) {
        std::string error_msg = "Failed to seek to page " + std::to_string(page_id);
        SQLCC_LOG_ERROR(error_msg);
        db_io_.clear();
        return false;
    }
    
    // 模拟读取失败（仅用于测试）
    if (simulate_read_failure_) {
        std::string error_msg = "Failed to read page " + std::to_string(page_id);
        SQLCC_LOG_ERROR(error_msg);
        return false;
    }
    
    // 读取页面数据
    // Why: 需要将页面数据从磁盘文件读取到内存缓冲区中
    // What: 调用read方法从文件中读取页面数据
    // How: 使用页面数据缓冲区指针，读取PAGE_SIZE字节
    db_io_.read(page_data, PAGE_SIZE);
    // 检查读取是否成功
    // Why: 读取失败可能是由于磁盘错误或文件损坏
    // What: 检查文件流状态，如果失败则记录错误并返回false
    // How: 使用fail方法检查文件流状态，使用clear清除错误状态
    if (db_io_.fail()) {
        std::string error_msg = "Failed to read page " + std::to_string(page_id);
        SQLCC_LOG_ERROR(error_msg);
        db_io_.clear();
        return false;
    }
    
    // 记录读取成功，便于调试
    SQLCC_LOG_DEBUG("Successfully read page ID " + std::to_string(page_id));
    return true;
}

// 分配新页面实现
// Why: 数据库需要为新数据分配新的页面存储空间
// What: AllocatePage方法分配一个新的页面ID，并返回该页面ID
// How: 先检查空闲页面列表，如果有则重用；否则分配新的页面ID
int32_t DiskManager::AllocatePage() {
    // 添加锁定保护，避免递归锁定
    std::lock_guard<std::recursive_mutex> lock(io_mutex_);
    
    // 检查是否有可用的空闲页面
    if (!free_pages_.empty()) {
        // 从空闲页面列表中获取最后一个页面ID
        int32_t page_id = free_pages_.back();
        free_pages_.pop_back();
        
        // 记录页面分配操作，便于调试
        SQLCC_LOG_DEBUG("Reused freed page ID: " + std::to_string(page_id));
        return page_id;
    }
    
    // 如果没有空闲页面，分配一个新的页面ID
    int32_t page_id = next_page_id_;
    
    // 递增页面ID计数器
    next_page_id_++;
    
    // 记录页面分配操作，便于调试
    SQLCC_LOG_DEBUG("Allocated new page ID: " + std::to_string(page_id) + ", next available: " + std::to_string(next_page_id_));
    
    return page_id;
}

// 释放页面实现
// Why: 数据库在删除数据后需要释放不再使用的页面，以便后续重用
// What: DeallocatePage方法接收要释放的页面ID，将其添加到空闲页面列表中
// How: 将页面ID添加到free_pages_向量中，以便后续分配时可以重用
bool DiskManager::DeallocatePage(int32_t page_id) {
    // 添加锁定保护，避免递归锁定
    std::lock_guard<std::recursive_mutex> lock(io_mutex_);
    
    // 验证页面ID的有效性
    // Why: 只有有效的页面ID才能被释放
    // What: 检查页面ID是否大于等于0
    // How: 使用if语句检查页面ID，如果无效则记录警告并返回false
    if (page_id < 0) {
        SQLCC_LOG_WARN("Attempted to deallocate invalid page ID: " + std::to_string(page_id));
        return false;
    }
    
    // 将页面ID添加到空闲页面列表
    // Why: 将释放的页面ID保存起来，以便后续分配时可以重用
    // What: 将页面ID添加到free_pages_向量中
    // How: 使用向量的push_back方法添加元素
    free_pages_.push_back(page_id);
    
    // 记录页面释放操作，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录释放的页面ID
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Deallocated page ID: " + std::to_string(page_id));
    return true;
}



// 获取文件大小实现
// Why: 上层模块需要知道数据库文件的当前大小，用于空间管理和监控
// What: GetFileSize方法返回当前数据库文件的大小（以页面数为单位）
// How: 通过文件大小除以页面大小来计算页面数量
int32_t DiskManager::GetFileSize() const {
    // 添加锁定保护，避免递归锁定
    std::lock_guard<std::recursive_mutex> lock(io_mutex_);
    
    return static_cast<int32_t>(file_size_ / PAGE_SIZE);
}

// 批量读取页面实现
// Why: 批量读取多个页面可以提高I/O效率，减少磁盘寻道时间
// What: BatchReadPages方法接收页面ID数组和数据缓冲区数组，批量读取多个页面数据
// How: 使用文件描述符和lseek/read系统调用进行高效的批量读取
bool DiskManager::BatchReadPages(const std::vector<int32_t>& page_ids, 
                                 std::vector<char*>& data_buffers) {
    // 添加锁定保护，避免递归锁定
    std::lock_guard<std::recursive_mutex> lock(io_mutex_);
    
    // 验证参数的有效性
    // Why: 页面ID数组和数据缓冲区数组必须大小一致，且不能为空
    // What: 检查两个数组的大小是否相等，以及是否为空
    // How: 使用if语句检查参数，如果无效则记录错误并返回false
    if (page_ids.size() != data_buffers.size() || page_ids.empty()) {
        SQLCC_LOG_ERROR("Invalid batch read parameters: mismatched sizes or empty arrays");
        return false;
    }
    
    // 创建页面ID和缓冲区的配对，并按页面ID排序以优化磁盘访问
    std::vector<std::pair<int32_t, char*>> page_pairs;
    page_pairs.reserve(page_ids.size());
    
    for (size_t i = 0; i < page_ids.size(); ++i) {
        if (page_ids[i] >= 0 && data_buffers[i] != nullptr) {
            page_pairs.emplace_back(page_ids[i], data_buffers[i]);
        }
    }
    
    if (page_pairs.empty()) {
        return false;
    }
    
    // 按页面ID排序，优化磁盘访问模式
    std::sort(page_pairs.begin(), page_pairs.end());
    
    // 使用文件描述符进行更高效的读取
    int fd = open(db_file_name_.c_str(), O_RDONLY);
    if (fd == -1) {
        SQLCC_LOG_ERROR("Failed to open database file for batch read: " + db_file_name_);
        return false;
    }
    
    bool success = true;
    
    // 批量读取页面
    for (const auto& pair : page_pairs) {
        int32_t page_id = pair.first;
        char* data = pair.second;
        
        // 计算页面偏移量
        off_t offset = static_cast<off_t>(page_id) * PAGE_SIZE;
        
        // 定位到页面位置
        if (lseek(fd, offset, SEEK_SET) == -1) {
            SQLCC_LOG_ERROR("Failed to seek to page " + std::to_string(page_id) + " during batch read");
            success = false;
            continue;
        }
        
        // 读取页面数据
        ssize_t bytes_read = read(fd, data, PAGE_SIZE);
        if (bytes_read == -1) {
            SQLCC_LOG_ERROR("Failed to read page " + std::to_string(page_id) + " during batch read");
            success = false;
            continue;
        } else if (bytes_read < static_cast<ssize_t>(PAGE_SIZE)) {
            // 如果读取的字节数小于页面大小，可能是文件末尾，填充剩余部分为0
            memset(data + bytes_read, 0, PAGE_SIZE - bytes_read);
        }
    }
    
    close(fd);
    return success;
}

bool DiskManager::PrefetchPage(int32_t page_id) {
    // 添加锁定保护，避免递归锁定
    std::lock_guard<std::recursive_mutex> lock(io_mutex_);
    
    if (page_id < 0) {
        return false;
    }
    
    // 使用文件描述符进行预读
    int fd = open(db_file_name_.c_str(), O_RDONLY);
    if (fd == -1) {
        SQLCC_LOG_ERROR("Failed to open database file for prefetch: " + db_file_name_);
        return false;
    }
    
    // 计算页面偏移量
    off_t offset = static_cast<off_t>(page_id) * PAGE_SIZE;
    
    // 使用posix_fadvise建议操作系统预读页面
    int result = posix_fadvise(fd, offset, PAGE_SIZE, POSIX_FADV_WILLNEED);
    close(fd);
    
    if (result != 0) {
        SQLCC_LOG_ERROR("Failed to prefetch page " + std::to_string(page_id));
        return false;
    }
    
    return true;
}

bool DiskManager::BatchPrefetchPages(const std::vector<int32_t>& page_ids) {
    // 添加锁定保护，避免递归锁定
    std::lock_guard<std::recursive_mutex> lock(io_mutex_);
    
    if (page_ids.empty()) {
        return false;
    }
    
    // 过滤有效的页面ID并排序
    std::vector<int32_t> valid_pages;
    for (int32_t page_id : page_ids) {
        if (page_id >= 0) {
            valid_pages.push_back(page_id);
        }
    }
    
    if (valid_pages.empty()) {
        return false;
    }
    
    // 按页面ID排序，优化磁盘访问模式
    std::sort(valid_pages.begin(), valid_pages.end());
    
    // 使用文件描述符进行批量预读
    int fd = open(db_file_name_.c_str(), O_RDONLY);
    if (fd == -1) {
        SQLCC_LOG_ERROR("Failed to open database file for batch prefetch: " + db_file_name_);
        return false;
    }
    
    bool success = true;
    
    // 合并连续的页面范围，提高预读效率
    for (size_t i = 0; i < valid_pages.size(); ) {
        int32_t start_page = valid_pages[i];
        int32_t end_page = start_page;
        
        // 找到连续的页面范围
        while (i + 1 < valid_pages.size() && valid_pages[i + 1] == end_page + 1) {
            i++;
            end_page++;
        }
        
        // 计算连续范围的偏移量和大小
        off_t offset = static_cast<off_t>(start_page) * PAGE_SIZE;
        off_t size = static_cast<off_t>(end_page - start_page + 1) * PAGE_SIZE;
        
        // 使用posix_fadvise建议操作系统预读连续页面范围
        int result = posix_fadvise(fd, offset, size, POSIX_FADV_WILLNEED);
        if (result != 0) {
            success = false;
        }
        
        i++; // 移动到下一个不连续的页面
    }
    
    close(fd);
    return success;
}

// 配置变更回调处理实现
// Why: 需要响应配置变更，动态调整磁盘管理器行为
// What: OnConfigChange方法处理配置变更事件，根据变更的配置项调整相应的磁盘管理器参数
// How: 根据配置键判断变更类型，执行相应的调整操作
void DiskManager::OnConfigChange(const std::string& key, const ConfigValue& value) {
    std::lock_guard<std::recursive_mutex> lock(io_mutex_);
    
    if (key == "disk_manager.enable_direct_io") {
        // 处理直接I/O开关变更
        if (std::holds_alternative<bool>(value)) {
            bool enable_direct_io = std::get<bool>(value);
            // 这里可以添加启用/禁用直接I/O的逻辑
            SQLCC_LOG_INFO("Direct I/O " + std::string(enable_direct_io ? "enabled" : "disabled"));
        }
    }
    else if (key == "disk_manager.io_queue_depth") {
        // 处理I/O队列深度变更
        if (std::holds_alternative<int>(value)) {
            int queue_depth = std::get<int>(value);
            // 这里可以添加I/O队列深度变更的逻辑
            SQLCC_LOG_INFO("I/O queue depth updated to: " + std::to_string(queue_depth));
        }
    }
    else if (key == "disk_manager.enable_async_io") {
        // 处理异步I/O开关变更
        if (std::holds_alternative<bool>(value)) {
            bool enable_async_io = std::get<bool>(value);
            // 这里可以添加启用/禁用异步I/O的逻辑
            SQLCC_LOG_INFO("Async I/O " + std::string(enable_async_io ? "enabled" : "disabled"));
        }
    }
    else if (key == "disk_manager.batch_io_size") {
        // 处理批量I/O大小变更
        if (std::holds_alternative<int>(value)) {
            int batch_size = std::get<int>(value);
            // 这里可以添加批量I/O大小变更的逻辑
            SQLCC_LOG_INFO("Batch I/O size updated to: " + std::to_string(batch_size));
        }
    }
    else if (key == "disk_manager.sync_strategy") {
        // 处理同步策略变更
        if (std::holds_alternative<std::string>(value)) {
            std::string strategy = std::get<std::string>(value);
            // 这里可以添加同步策略变更的逻辑
            SQLCC_LOG_INFO("Sync strategy updated to: " + strategy);
        }
    }
    else if (key == "disk_manager.sync_interval") {
        // 处理同步间隔变更
        if (std::holds_alternative<int>(value)) {
            int sync_interval = std::get<int>(value);
            // 这里可以添加同步间隔变更的逻辑
            SQLCC_LOG_INFO("Sync interval updated to: " + std::to_string(sync_interval));
        }
    }
}

// 同步文件到磁盘实现
// Why: 确保所有写入操作都已被持久化到磁盘，保证数据持久性
// What: Sync方法将文件缓冲区的内容强制写入磁盘
// How: 调用文件流的flush方法，强制将缓冲区数据写入磁盘
bool DiskManager::Sync() {
    std::lock_guard<std::recursive_mutex> lock(io_mutex_);
    
    // 检查文件流是否打开
    if (!db_io_.is_open()) {
        SQLCC_LOG_ERROR("Cannot sync: database file is not open");
        return false;
    }
    
    // 模拟刷新失败（仅用于测试）
    if (simulate_flush_failure_) {
        SQLCC_LOG_ERROR("Simulated flush failure");
        return false;
    }
    
    // 刷新文件流缓冲区到磁盘
    // Why: 确保所有缓冲的数据都被写入磁盘，保证数据持久性
    // What: 调用文件流的flush方法，强制刷新缓冲区
    // How: 使用fstream的flush方法将缓冲区数据同步到磁盘
    db_io_.flush();
    
    // 检查刷新是否成功
    if (db_io_.fail()) {
        std::string error_msg = "Failed to sync database file: " + db_file_name_;
        SQLCC_LOG_ERROR(error_msg);
        db_io_.clear();
        return false;
    }
    
    // 更新统计信息
    io_stats_.total_syncs++;
    
    SQLCC_LOG_DEBUG("Successfully synced database file: " + db_file_name_);
    return true;
}


}  // namespace sqlcc