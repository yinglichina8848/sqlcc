#include "disk_manager.h"
#include "exception.h"
#include "logger.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>

namespace sqlcc {

// 磁盘管理器构造函数实现
// Why: 需要初始化磁盘管理器，打开数据库文件并准备进行I/O操作
// What: 构造函数接收数据库文件路径作为参数，打开文件流，初始化文件大小和页面计数器
// How: 使用fstream打开文件，如果文件不存在则创建新文件，获取文件大小并计算页面数量
DiskManager::DiskManager(const std::string& db_file) 
    : db_file_(db_file), file_size_(0), next_page_id_(0) {
    // 记录初始化信息，便于调试和监控
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在初始化的数据库文件路径
    // How: 使用SQLCC_LOG_INFO宏记录信息级别日志
    SQLCC_LOG_INFO("Initializing DiskManager for database file: " + db_file_);
    
    // 以读写模式打开文件，如果文件不存在则创建
    // Why: 需要打开数据库文件进行读写操作，如果文件不存在则需要创建
    // What: 使用fstream的open方法打开文件，指定二进制模式和读写权限
    // How: 使用std::ios::binary指定二进制模式，std::ios::in|std::ios::out指定读写权限，std::ios::app允许追加
    db_io_.open(db_file_, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
    
    // 如果文件不存在，创建一个空文件
    // Why: 如果文件不存在，第一次打开会失败，需要创建新文件
    // What: 检查文件流状态，如果失败则清除错误状态并重新创建文件
    // How: 使用good()方法检查文件流状态，使用clear()清除错误状态，使用trunc模式创建空文件
    if (!db_io_.good()) {
        SQLCC_LOG_INFO("Database file does not exist, creating new file: " + db_file_);
        db_io_.clear();
        db_io_.open(db_file_, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
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
        SQLCC_LOG_INFO("Opened database file: " + db_file_ + ", file size: " + 
                      std::to_string(file_size_) + ", next page ID: " + std::to_string(next_page_id_));
    } else {
        // 文件打开失败，抛出异常
        // Why: 如果无法打开数据库文件，磁盘管理器无法正常工作，需要抛出异常
        // What: 创建错误消息，记录错误日志，然后抛出DiskManagerException异常
        // How: 使用SQLCC_LOG_ERROR记录错误级别日志，然后抛出异常
        std::string error_msg = "Failed to open database file: " + db_file_;
        SQLCC_LOG_ERROR(error_msg);
        throw DiskManagerException(error_msg);
    }
}

// 磁盘管理器析构函数实现
// Why: 需要释放文件流资源，确保文件正确关闭，防止数据丢失
// What: 析构函数负责关闭数据库文件流，释放系统资源
// How: 检查文件流是否打开，如果打开则调用close方法关闭
DiskManager::~DiskManager() {
    // 检查文件流是否打开
    // Why: 只有在文件流打开的情况下才需要关闭
    // What: 使用is_open方法检查文件流状态
    // How: 如果文件流打开，则调用close方法关闭
    if (db_io_.is_open()) {
        SQLCC_LOG_INFO("Closing database file: " + db_file_);
        db_io_.close();
    }
}

// 写入页面到磁盘实现
// Why: 数据库需要将修改后的页面持久化到磁盘，以保证数据的持久性和一致性
// What: WritePage方法接收一个页面对象，将其内容写入到磁盘文件的对应位置
// How: 计算页面在文件中的偏移量，使用seekp定位到该位置，然后调用write方法写入页面数据
bool DiskManager::WritePage(const Page& page) {
    // 获取页面ID
    // Why: 需要页面ID来计算页面在文件中的位置
    // What: 调用页面对象的GetPageId方法获取页面ID
    // How: 直接调用Page类的GetPageId方法
    int32_t page_id = page.GetPageId();
    // 验证页面ID的有效性
    // Why: 页面ID必须是非负数，负数是无效的页面ID
    // What: 检查页面ID是否小于0
    // How: 使用if语句检查页面ID，如果无效则记录错误并返回false
    if (page_id < 0) {
        std::string error_msg = "Invalid page ID: " + std::to_string(page_id);
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
    // How: 使用页面对象的GetData方法获取数据指针，写入PAGE_SIZE字节
    db_io_.write(page.GetData(), PAGE_SIZE);
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
// What: ReadPage方法接收一个页面ID和一个页面对象指针，从磁盘文件中读取对应页面的数据
// How: 计算页面在文件中的偏移量，使用seekg定位到该位置，然后调用read方法读取页面数据
bool DiskManager::ReadPage(int32_t page_id, Page* page) {
    // 验证参数的有效性
    // Why: 页面ID必须是非负数，页面对象指针不能为空
    // What: 检查页面ID是否小于0，页面对象指针是否为空
    // How: 使用if语句检查参数，如果无效则记录错误并返回false
    if (page_id < 0 || page == nullptr) {
        std::string error_msg = "Invalid page ID: " + std::to_string(page_id) + " or null page pointer";
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
    
    // 读取页面数据
    // Why: 需要从磁盘文件中读取页面数据到内存中
    // What: 调用read方法从文件中读取页面数据
    // How: 使用页面对象的GetData方法获取数据指针，读取PAGE_SIZE字节
    db_io_.read(page->GetData(), PAGE_SIZE);
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
    
    // 设置页面ID
    // Why: 需要设置页面对象的ID，以便后续操作知道这是哪个页面
    // What: 调用页面对象的SetPageId方法设置页面ID
    // How: 直接调用Page类的SetPageId方法
    page->SetPageId(page_id);
    // 记录读取成功，便于调试
    SQLCC_LOG_DEBUG("Successfully read page ID " + std::to_string(page_id));
    return true;
}

// 分配新页面实现
// Why: 当数据库需要存储新数据时，需要分配新的页面空间
// What: AllocatePage方法分配一个新的页面ID，并扩展数据库文件大小以容纳新页面
// How: 使用原子递增next_page_id_计数器生成新的页面ID，返回新分配的页面ID
int32_t DiskManager::AllocatePage() {
    // 分配新的页面ID
    // Why: 需要生成唯一的页面ID来标识新页面
    // What: 使用next_page_id_计数器生成新的页面ID，然后递增计数器
    // How: 使用后缀递增操作符，先返回当前值，然后递增
    int32_t page_id = next_page_id_++;
    // 记录页面分配，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录新分配的页面ID
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Allocated new page ID: " + std::to_string(page_id));
    return page_id;
}

// 获取文件大小实现
// Why: 需要知道数据库文件的当前大小，以便进行空间管理和计算页面位置
// What: GetFileSize方法返回数据库文件的当前大小，以字节为单位
// How: 直接返回file_size_成员变量
size_t DiskManager::GetFileSize() const {
    return file_size_;
}

size_t DiskManager::BatchReadPages(const std::vector<int32_t>& page_ids, std::vector<char*>& data_buffers) {
    if (page_ids.empty() || data_buffers.empty() || page_ids.size() != data_buffers.size()) {
        SQLCC_LOG_ERROR("Invalid parameters for batch read pages");
        return 0;
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
        return 0;
    }
    
    // 按页面ID排序，优化磁盘访问模式
    std::sort(page_pairs.begin(), page_pairs.end());
    
    // 使用文件描述符进行更高效的读取
    int fd = open(db_file_.c_str(), O_RDONLY);
    if (fd == -1) {
        SQLCC_LOG_ERROR("Failed to open database file for batch read: " + db_file_);
        return 0;
    }
    
    size_t success_count = 0;
    
    // 批量读取页面
    for (const auto& pair : page_pairs) {
        int32_t page_id = pair.first;
        char* data = pair.second;
        
        // 计算页面偏移量
        off_t offset = static_cast<off_t>(page_id) * PAGE_SIZE;
        
        // 定位到页面位置
        if (lseek(fd, offset, SEEK_SET) == -1) {
            SQLCC_LOG_ERROR("Failed to seek to page " + std::to_string(page_id) + " during batch read");
            continue;
        }
        
        // 读取页面数据
        ssize_t bytes_read = read(fd, data, PAGE_SIZE);
        if (bytes_read == -1) {
            SQLCC_LOG_ERROR("Failed to read page " + std::to_string(page_id) + " during batch read");
            continue;
        } else if (bytes_read < PAGE_SIZE) {
            // 如果读取的字节数小于页面大小，可能是文件末尾，填充剩余部分为0
            memset(data + bytes_read, 0, PAGE_SIZE - bytes_read);
        }
        
        success_count++;
    }
    
    close(fd);
    return success_count;
}

bool DiskManager::PrefetchPage(int32_t page_id) {
    if (page_id < 0) {
        return false;
    }
    
    // 使用文件描述符进行预读
    int fd = open(db_file_.c_str(), O_RDONLY);
    if (fd == -1) {
        SQLCC_LOG_ERROR("Failed to open database file for prefetch: " + db_file_);
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

size_t DiskManager::BatchPrefetchPages(const std::vector<int32_t>& page_ids) {
    if (page_ids.empty()) {
        return 0;
    }
    
    // 过滤有效的页面ID并排序
    std::vector<int32_t> valid_pages;
    for (int32_t page_id : page_ids) {
        if (page_id >= 0) {
            valid_pages.push_back(page_id);
        }
    }
    
    if (valid_pages.empty()) {
        return 0;
    }
    
    // 按页面ID排序，优化磁盘访问模式
    std::sort(valid_pages.begin(), valid_pages.end());
    
    // 使用文件描述符进行批量预读
    int fd = open(db_file_.c_str(), O_RDONLY);
    if (fd == -1) {
        SQLCC_LOG_ERROR("Failed to open database file for batch prefetch: " + db_file_);
        return 0;
    }
    
    size_t success_count = 0;
    
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
        if (result == 0) {
            success_count += (end_page - start_page + 1);
        }
        
        i++; // 移动到下一个不连续的页面
    }
    
    close(fd);
    return success_count;
}

}  // namespace sqlcc