#include "disk_manager.h"
#include "exception.h"
#include "logger.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>

namespace sqlcc {

DiskManager::DiskManager(const std::string& db_file) 
    : db_file_(db_file), file_size_(0), next_page_id_(0) {
    SQLCC_LOG_INFO("Initializing DiskManager for database file: " + db_file_);
    
    // 以读写模式打开文件，如果文件不存在则创建
    db_io_.open(db_file_, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
    
    // 如果文件不存在，创建一个空文件
    if (!db_io_.good()) {
        SQLCC_LOG_INFO("Database file does not exist, creating new file: " + db_file_);
        db_io_.clear();
        db_io_.open(db_file_, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
    }
    
    // 获取文件大小
    if (db_io_.good()) {
        db_io_.seekg(0, std::ios::end);
        file_size_ = db_io_.tellg();
        next_page_id_ = static_cast<int32_t>(file_size_ / PAGE_SIZE);
        SQLCC_LOG_INFO("Opened database file: " + db_file_ + ", file size: " + 
                      std::to_string(file_size_) + ", next page ID: " + std::to_string(next_page_id_));
    } else {
        std::string error_msg = "Failed to open database file: " + db_file_;
        SQLCC_LOG_ERROR(error_msg);
        throw DiskManagerException(error_msg);
    }
}

DiskManager::~DiskManager() {
    if (db_io_.is_open()) {
        SQLCC_LOG_INFO("Closing database file: " + db_file_);
        db_io_.close();
    }
}

bool DiskManager::WritePage(const Page& page) {
    int32_t page_id = page.GetPageId();
    if (page_id < 0) {
        std::string error_msg = "Invalid page ID: " + std::to_string(page_id);
        SQLCC_LOG_ERROR(error_msg);
        return false;
    }
    
    // 计算页面在文件中的偏移量
    size_t offset = static_cast<size_t>(page_id) * PAGE_SIZE;
    
    SQLCC_LOG_DEBUG("Writing page ID " + std::to_string(page_id) + " at offset " + std::to_string(offset));
    
    // 定位到页面位置
    db_io_.seekp(offset, std::ios::beg);
    if (db_io_.fail()) {
        std::string error_msg = "Failed to seek to page " + std::to_string(page_id);
        SQLCC_LOG_ERROR(error_msg);
        db_io_.clear();
        return false;
    }
    
    // 写入页面数据
    db_io_.write(page.GetData(), PAGE_SIZE);
    if (db_io_.fail()) {
        std::string error_msg = "Failed to write page " + std::to_string(page_id);
        SQLCC_LOG_ERROR(error_msg);
        db_io_.clear();
        return false;
    }
    
    // 更新文件大小
    size_t new_size = offset + PAGE_SIZE;
    if (new_size > file_size_) {
        file_size_ = new_size;
        SQLCC_LOG_DEBUG("Updated file size to " + std::to_string(file_size_));
    }
    
    // 刷新到磁盘
    db_io_.flush();
    if (db_io_.fail()) {
        std::string error_msg = "Failed to flush page " + std::to_string(page_id) + " to disk";
        SQLCC_LOG_ERROR(error_msg);
        db_io_.clear();
        return false;
    }
    
    SQLCC_LOG_DEBUG("Successfully wrote page ID " + std::to_string(page_id));
    return true;
}

bool DiskManager::ReadPage(int32_t page_id, Page* page) {
    if (page_id < 0 || page == nullptr) {
        std::string error_msg = "Invalid page ID: " + std::to_string(page_id) + " or null page pointer";
        SQLCC_LOG_ERROR(error_msg);
        return false;
    }
    
    // 计算页面在文件中的偏移量
    size_t offset = static_cast<size_t>(page_id) * PAGE_SIZE;
    
    SQLCC_LOG_DEBUG("Reading page ID " + std::to_string(page_id) + " at offset " + std::to_string(offset));
    
    // 检查页面是否在文件范围内
    if (offset >= file_size_) {
        std::string warn_msg = "Page " + std::to_string(page_id) + " does not exist in file";
        SQLCC_LOG_WARN(warn_msg);
        return false;
    }
    
    // 定位到页面位置
    db_io_.seekg(offset, std::ios::beg);
    if (db_io_.fail()) {
        std::string error_msg = "Failed to seek to page " + std::to_string(page_id);
        SQLCC_LOG_ERROR(error_msg);
        db_io_.clear();
        return false;
    }
    
    // 读取页面数据
    db_io_.read(page->GetData(), PAGE_SIZE);
    if (db_io_.fail()) {
        std::string error_msg = "Failed to read page " + std::to_string(page_id);
        SQLCC_LOG_ERROR(error_msg);
        db_io_.clear();
        return false;
    }
    
    // 设置页面ID
    page->SetPageId(page_id);
    SQLCC_LOG_DEBUG("Successfully read page ID " + std::to_string(page_id));
    return true;
}

int32_t DiskManager::AllocatePage() {
    int32_t page_id = next_page_id_++;
    SQLCC_LOG_DEBUG("Allocated new page ID: " + std::to_string(page_id));
    return page_id;
}

size_t DiskManager::GetFileSize() const {
    return file_size_;
}

}  // namespace sqlcc