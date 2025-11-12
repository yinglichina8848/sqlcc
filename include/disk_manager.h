#pragma once

#include <fstream>
#include <memory>
#include <mutex>
#include <string>

#include "config_manager.h"

namespace sqlcc {

// 磁盘管理器类，负责数据库文件的读写操作
// Why: 数据库系统需要持久化存储数据，磁盘管理器负责处理底层的文件I/O操作
// What: DiskManager类提供了页面的读写、分配、预取等基本磁盘操作
// How: 使用文件流操作数据库文件，通过互斥锁保证线程安全
class DiskManager {
public:
    // 构造函数，初始化磁盘管理器
    // Why: 需要创建磁盘管理器实例，打开数据库文件，初始化内部状态
    // What: 构造函数接收数据库文件名和配置管理器引用，打开文件并初始化成员变量
    // How: 打开数据库文件，初始化文件流，设置读写位置，初始化互斥锁
    explicit DiskManager(const std::string& db_file, ConfigManager& config_manager);

    // 析构函数，清理资源析构函数，关闭文件
    // Why: 需要确保文件被正确关闭，避免数据丢失
    // What: 析构函数负责关闭数据库文件，释放资源
    // How: 调用文件流的close方法关闭文件
    ~DiskManager();

    // 禁止拷贝构造和赋值操作
    // Why: 磁盘管理器管理文件资源，拷贝可能导致资源重复释放或文件句柄冲突
    // What: 通过将拷贝构造函数和赋值操作符标记为delete，防止编译器自动生成
    // How: 使用= delete语法显式禁用拷贝构造函数和赋值操作符
    DiskManager(const DiskManager&) = delete;
    DiskManager& operator=(const DiskManager&) = delete;

    // 写入页面到磁盘
    // Why: 需要将内存中的页面数据持久化到磁盘，保证数据的持久性
    // What: WritePage方法将指定页面的数据写入磁盘文件
    // How: 定位到页面对应的文件位置，写入页面数据，刷新文件缓冲区
    bool WritePage(int32_t page_id, const char* page_data);

    // 从磁盘读取页面
    // Why: 需要从磁盘加载页面数据到内存，供数据库操作使用
    // What: ReadPage方法从磁盘文件读取指定页面的数据
    // How: 定位到页面对应的文件位置，读取页面数据到内存缓冲区
    bool ReadPage(int32_t page_id, char* page_data);

    // 批量读取页面，优化多个页面的读取性能
    // Why: 某些操作需要同时访问多个页面，批量读取可以提高性能
    // What: BatchReadPages方法根据页面ID列表批量读取多个页面
    // How: 对页面ID进行排序以优化磁盘访问，使用批量I/O操作读取页面
    bool BatchReadPages(const std::vector<int32_t>& page_ids, std::vector<char*>& page_data);

    // 预取页面到缓冲区
    // Why: 预取可以提前加载可能需要的页面，减少未来的磁盘I/O延迟
    // What: PrefetchPage方法将指定页面预加载到内部缓冲区
    // How: 使用异步I/O或内部缓冲区预取页面数据
    bool PrefetchPage(int32_t page_id);

    // 批量预取页面到缓冲区
    // Why: 批量预取可以一次性加载多个页面，提高预取效率
    // What: BatchPrefetchPages方法将多个页面预加载到内部缓冲区
    // How: 对页面ID进行排序以优化磁盘访问，使用批量预取策略
    bool BatchPrefetchPages(const std::vector<int32_t>& page_ids);

    // 分配新页面
    // Why: 数据库需要新的存储空间来存储数据，例如插入新记录或创建索引
    // What: AllocatePage方法分配一个新的页面ID
    // How: 增加页面计数器，返回新的页面ID
    int32_t AllocatePage();

    // 释放页面
    // Why: 当数据不再需要时，需要释放页面空间，例如删除记录或索引
    // What: DeallocatePage方法释放指定页面，使其可以被重新使用
    // How: 将页面ID添加到空闲页面列表，供后续分配使用
    bool DeallocatePage(int32_t page_id);

    // 获取数据库文件大小（页面数）
    // Why: 需要了解数据库文件的当前大小，用于监控和调试
    // What: GetFileSize方法返回数据库文件包含的页面数量
    // How: 通过文件大小除以页面大小计算页面数量
    int32_t GetFileSize() const;

    // 同步文件到磁盘
    // Why: 确保所有写入操作都已被持久化到磁盘，保证数据持久性
    // What: Sync方法将文件缓冲区的内容强制写入磁盘
    // How: 调用文件流的sync方法或操作系统提供的同步函数
    bool Sync();

    // 获取磁盘I/O统计信息
    // Why: 监控磁盘I/O性能有助于系统调优和问题诊断
    // What: GetIOStats方法返回磁盘I/O的统计信息，如读写次数等
    // How: 收集并返回各种I/O统计指标
    std::unordered_map<std::string, uint64_t> GetIOStats() const;

    // 重置磁盘I/O统计信息
    // Why: 需要定期重置统计信息，便于监控特定时间段的I/O性能
    // What: ResetIOStats方法重置所有I/O统计计数器
    // How: 将所有统计计数器重置为0
    void ResetIOStats();

    /**
     * @brief 设置是否模拟读取失败（仅用于测试）
     * @param simulate 是否模拟读取失败
     */
    void SetSimulateReadFailure(bool simulate) { simulate_read_failure_ = simulate; }

    /**
     * @brief 设置是否模拟写入失败（仅用于测试）
     * @param simulate 是否模拟写入失败
     */
    void SetSimulateWriteFailure(bool simulate) { simulate_write_failure_ = simulate; }

    /**
     * @brief 设置是否模拟寻道失败（仅用于测试）
     * @param simulate 是否模拟寻道失败
     */
    void SetSimulateSeekFailure(bool simulate) { simulate_seek_failure_ = simulate; }

    /**
     * @brief 设置是否模拟刷新失败（仅用于测试）
     * @param simulate 是否模拟刷新失败
     */
    void SetSimulateFlushFailure(bool simulate) { simulate_flush_failure_ = simulate; }

private:
    // 打开数据库文件
    // Why: 需要打开数据库文件进行读写操作
    // What: OpenFile方法打开数据库文件，初始化文件流
    // How: 使用std::fstream打开文件，设置适当的打开模式
    bool OpenFile();

    // 关闭数据库文件
    // Why: 需要关闭数据库文件，释放文件句柄
    // What: CloseFile方法关闭数据库文件
    // How: 调用文件流的close方法
    void CloseFile();

    // 初始化文件（如果不存在）
    // Why: 如果数据库文件不存在，需要创建新文件并初始化
    // What: InitializeFile方法创建新数据库文件并写入初始化数据
    // How: 创建新文件，写入文件头信息，初始化页面分配信息
    bool InitializeFile();

    // 读取文件头
    // Why: 文件头包含数据库的元数据，需要在启动时读取
    // What: ReadFileHeader方法读取数据库文件的头部信息
    // How: 定位到文件开头，读取固定大小的文件头数据
    bool ReadFileHeader();

    // 写入文件头
    // Why: 文件头包含数据库的元数据，需要在修改后写入
    // What: WriteFileHeader方法写入数据库文件的头部信息
    // How: 定位到文件开头，写入固定大小的文件头数据
    bool WriteFileHeader();

    // 配置变更回调处理
    // Why: 需要响应配置变更，动态调整磁盘管理器行为
    // What: OnConfigChange方法处理配置变更事件
    // How: 根据变更的配置项调整相应的磁盘管理器参数
    void OnConfigChange(const std::string& key, const ConfigValue& value);

    // 数据库文件名
    // Why: 需要存储数据库文件名，用于打开和操作文件
    // What: db_file_name_存储数据库文件的完整路径
    // How: 通过构造函数初始化，用于OpenFile方法
    std::string db_file_name_;

    // 配置管理器引用，用于获取配置参数
    // Why: 磁盘管理器需要从配置管理器获取配置参数，如I/O策略等
    // What: config_manager_引用ConfigManager对象，提供配置访问接口
    // How: 通过构造函数初始化，在需要获取配置时调用相应方法
    ConfigManager& config_manager_;

    // 数据库文件流
    // Why: 需要文件流对象来读写数据库文件
    // What: db_io_是文件流对象，负责数据库文件的I/O操作
    // How: 使用std::fstream实现，通过OpenFile方法初始化
    std::fstream db_io_;

    // 文件大小（字节）
    // Why: 需要记录文件大小，用于边界检查和页面分配
    // What: file_size_存储数据库文件的当前大小（字节）
    // How: 在打开文件时初始化，在写入新页面时更新
    size_t file_size_;

    // 递归互斥锁，保护文件访问
    // Why: 磁盘管理器可能被多个线程同时访问，需要同步机制保证数据一致性
    // What: io_mutex_是递归互斥锁，保护所有文件I/O操作，支持同一线程多次锁定
    // How: 在进行文件I/O操作前加锁，操作完成后解锁，支持递归锁定避免死锁
    mutable std::recursive_mutex io_mutex_;

    // 下一个要分配的页面ID
    // Why: 需要跟踪下一个可分配的页面ID，确保页面ID的唯一性
    // What: next_page_id_存储下一个可分配的页面ID
    // How: 初始化时从文件头读取，分配页面时递增
    int32_t next_page_id_;

    // 测试用失败模拟标志
    bool simulate_write_failure_ = false;
    bool simulate_flush_failure_ = false;
    bool simulate_seek_failure_ = false;
    bool simulate_read_failure_ = false;

    // 空闲页面列表
    // Why: 需要记录被释放的页面，以便重新使用
    // What: free_pages_是存储空闲页面ID的列表
    // How: 释放页面时添加到列表，分配页面时优先从列表获取
    std::vector<int32_t> free_pages_;

    // 统计信息，记录磁盘I/O的使用情况
    // Why: 监控磁盘I/O的使用情况有助于性能调优和问题诊断
    // What: io_stats_记录各种I/O统计指标，如读写次数、字节数等
    // How: 在相应操作时更新统计信息
    struct IOStats {
        uint64_t total_reads = 0;       // 总读取次数
        uint64_t total_writes = 0;      // 总写入次数
        uint64_t total_bytes_read = 0;  // 总读取字节数
        uint64_t total_bytes_written = 0; // 总写入字节数
        uint64_t total_syncs = 0;       // 总同步次数
        uint64_t total_allocations = 0; // 总分配次数
        uint64_t total_deallocations = 0; // 总释放次数
    } io_stats_;

    // 配置变更回调ID，用于在析构函数中注销回调
    // Why: 需要在析构函数中注销构造函数中注册的回调，避免悬垂指针
    // What: 存储各个配置变更回调的注册ID
    // How: 在构造函数中注册时获取ID，在析构函数中使用ID注销
    int direct_io_callback_id_ = -1;
    int io_queue_depth_callback_id_ = -1;
    int async_io_callback_id_ = -1;
    int batch_io_size_callback_id_ = -1;
    int sync_strategy_callback_id_ = -1;
    int sync_interval_callback_id_ = -1;
};

}  // namespace sqlcc