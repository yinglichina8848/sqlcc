#include "page.h"
#include "exception.h"
#include "logger.h"
#include <cstring>

namespace sqlcc {

Page::Page() = default;

Page::Page(int32_t page_id) : page_id_(page_id) {
    SQLCC_LOG_DEBUG("Creating page with ID: " + std::to_string(page_id_));
    
    // 初始化页面数据为0
    memset(data_, 0, PAGE_SIZE);
}

Page::~Page() {
    SQLCC_LOG_DEBUG("Destroying page with ID: " + std::to_string(page_id_));
}

void Page::WriteData(size_t offset, const char* src, size_t size) {
    SQLCC_LOG_DEBUG("Writing data to page ID " + std::to_string(page_id_) + 
                   " at offset " + std::to_string(offset) + " with size " + std::to_string(size));
    
    // 检查边界
    if (offset + size > PAGE_SIZE) {
        std::string error_msg = "WriteData out of bounds: offset=" + std::to_string(offset) + 
                               ", size=" + std::to_string(size) + ", page_size=" + std::to_string(PAGE_SIZE);
        SQLCC_LOG_ERROR(error_msg);
        throw PageException(error_msg);
    }
    
    // 复制数据
    memcpy(data_ + offset, src, size);
    SQLCC_LOG_DEBUG("Successfully wrote data to page ID " + std::to_string(page_id_));
}

void Page::ReadData(size_t offset, char* dest, size_t size) const {
    SQLCC_LOG_DEBUG("Reading data from page ID " + std::to_string(page_id_) + 
                   " at offset " + std::to_string(offset) + " with size " + std::to_string(size));
    
    // 检查边界
    if (offset + size > PAGE_SIZE) {
        std::string error_msg = "ReadData out of bounds: offset=" + std::to_string(offset) + 
                               ", size=" + std::to_string(size) + ", page_size=" + std::to_string(PAGE_SIZE);
        SQLCC_LOG_ERROR(error_msg);
        throw PageException(error_msg);
    }
    
    // 复制数据
    memcpy(dest, data_ + offset, size);
    SQLCC_LOG_DEBUG("Successfully read data from page ID " + std::to_string(page_id_));
}

}  // namespace sqlcc