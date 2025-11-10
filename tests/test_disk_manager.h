#pragma once

#include "disk_manager.h"
#include "config_manager.h"
#include "page.h"
#include "logger.h"

namespace sqlcc {

/**
 * @brief 测试专用的磁盘管理器类，可以模拟文件操作失败
 */
class TestDiskManager : public DiskManager {
public:
    /**
     * @brief 构造函数
     * @param db_file 数据库文件路径
     * @param config_manager 配置管理器
     */
    explicit TestDiskManager(const std::string& db_file, ConfigManager& config_manager) : DiskManager(db_file, config_manager) {}

    /**
     * @brief 设置是否模拟写入失败
     * @param simulate_failure 是否模拟写入失败
     */
    void SetSimulateWriteFailure(bool simulate_failure) {
        simulate_write_failure_ = simulate_failure;
    }

    /**
     * @brief 设置是否模拟刷新失败
     * @param simulate_failure 是否模拟刷新失败
     */
    void SetSimulateFlushFailure(bool simulate_failure) {
        simulate_flush_failure_ = simulate_failure;
    }

    /**
     * @brief 设置是否模拟定位失败
     * @param simulate_failure 是否模拟定位失败
     */
    void SetSimulateSeekFailure(bool simulate_failure) {
        simulate_seek_failure_ = simulate_failure;
    }

    /**
     * @brief 模拟WritePage方法，可以模拟写入失败
     */
    bool TestWritePage(const Page& page) {
        if (simulate_write_failure_) {
            // 模拟写入失败
            return false;
        }

        if (simulate_seek_failure_) {
            // 模拟定位失败
            int32_t page_id = page.GetPageId();
            std::string error_msg = "Failed to seek to page " + std::to_string(page_id);
            SQLCC_LOG_ERROR(error_msg);
            return false;
        }

        if (simulate_flush_failure_) {
            // 模拟刷新失败
            // 先执行正常的写入操作
            bool write_success = DiskManager::WritePage(page.GetPageId(), page.GetData());
            if (write_success) {
                // 模拟刷新失败
                int32_t page_id = page.GetPageId();
                std::string error_msg = "Failed to flush page " + std::to_string(page_id) + " to disk";
                SQLCC_LOG_ERROR(error_msg);
                return false;
            }
            return false;
        }

        // 正常写入
        return DiskManager::WritePage(page.GetPageId(), page.GetData());
    }

    /**
     * @brief 模拟ReadPage方法，可以模拟定位失败
     */
    bool TestReadPage(int32_t page_id, char* page_data) {
        if (simulate_seek_failure_) {
            // 模拟定位失败
            std::string error_msg = "Failed to seek to page " + std::to_string(page_id);
            SQLCC_LOG_ERROR(error_msg);
            return false;
        }

        // 正常读取
        return DiskManager::ReadPage(page_id, page_data);
    }

private:
    // 是否模拟写入失败
    bool simulate_write_failure_ = false;
    
    // 是否模拟刷新失败
    bool simulate_flush_failure_ = false;
    
    // 是否模拟定位失败
    bool simulate_seek_failure_ = false;
};

}  // namespace sqlcc