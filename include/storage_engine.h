#pragma once

#include "disk_manager.h"
#include "buffer_pool.h"
#include "page.h"
#include "config_manager.h"

namespace sqlcc {

/**
 * @brief 存储引擎类，封装磁盘管理和缓冲池功能
 * 
 * Why: 数据库系统需要一个统一的接口来管理数据的存储和访问，隐藏底层实现的复杂性
 * What: StorageEngine类作为存储子系统的顶层接口，封装了磁盘管理和缓冲池功能，提供页面的创建、读取、写入和删除操作
 * How: 通过组合DiskManager和BufferPool类，实现磁盘I/O和内存缓存的协调管理，提供统一的页面操作接口
 */
class StorageEngine {
public:
    /**
     * @brief 构造函数
     * @param config_manager 配置管理器引用
     * 
     * Why: 需要初始化存储引擎，从配置管理器获取参数创建磁盘管理器和缓冲池实例
     * What: 构造函数接收配置管理器引用，从中获取配置参数初始化存储引擎的各个组件
     * How: 从配置管理器获取数据库文件路径和缓冲池大小，创建DiskManager和BufferPool对象
     */
    explicit StorageEngine(ConfigManager& config_manager);

    /**
     * @brief 析构函数
     * 
     * Why: 需要释放存储引擎占用的资源，确保数据正确写入磁盘
     * What: 析构函数负责刷新所有脏页到磁盘，释放缓冲池和磁盘管理器资源
     * How: 调用FlushAllPages方法确保所有数据持久化，智能指针自动释放资源
     */
    ~StorageEngine();

    /**
     * @brief 禁止拷贝构造
     * 
     * Why: 存储引擎管理着重要的系统资源，不应该被随意复制
     * What: 删除拷贝构造函数，防止对象被意外复制
     * How: 使用= delete关键字显式删除拷贝构造函数
     */
    StorageEngine(const StorageEngine&) = delete;

    /**
     * @brief 禁止赋值操作
     * 
     * Why: 存储引擎管理着重要的系统资源，不应该被随意赋值
     * What: 删除赋值操作符，防止对象被意外赋值
     * How: 使用= delete关键字显式删除赋值操作符
     */
    StorageEngine& operator=(const StorageEngine&) = delete;

    /**
     * @brief 创建新页面
     * @param[out] page_id 新页面的ID
     * @return 页面指针，如果失败返回nullptr
     * 
     * Why: 数据库需要新的存储空间来存储数据，例如插入新记录或创建索引
     * What: NewPage方法在数据库中分配一个新的页面，返回指向该页面的指针，并通过输出参数返回页面ID
     * How: 通过磁盘管理器分配新的页面ID，通过缓冲池创建页面对象，将页面标记为脏页
     */
    Page* NewPage(int32_t* page_id);

    /**
     * @brief 获取页面
     * @param page_id 页面ID
     * @return 页面指针，如果失败返回nullptr
     * 
     * Why: 需要访问存储在磁盘上的页面数据，例如查询记录或更新数据
     * What: FetchPage方法根据页面ID获取页面对象，如果页面不在内存中则从磁盘加载
     * How: 通过缓冲池获取页面，如果页面不在内存中则通过磁盘管理器从磁盘加载
     */
    Page* FetchPage(int32_t page_id);

    /**
     * @brief 取消固定页面
     * @param page_id 页面ID
     * @param is_dirty 是否标记为脏页
     * @return 操作是否成功
     * 
     * Why: 当页面使用完毕后，需要通知缓冲池该页面可以被替换，同时标记页面是否被修改
     * What: UnpinPage方法减少页面的固定计数，如果页面被修改则标记为脏页
     * How: 调用缓冲池的UnpinPage方法，传递页面ID和脏页标记
     */
    bool UnpinPage(int32_t page_id, bool is_dirty);

    /**
     * @brief 刷新页面到磁盘
     * @param page_id 页面ID
     * @return 操作是否成功
     * 
     * Why: 需要将修改后的页面数据持久化到磁盘，保证数据的持久性和一致性
     * What: FlushPage方法将指定页面的数据写入磁盘文件
     * How: 调用缓冲池的FlushPage方法，由缓冲池协调与磁盘管理器的交互
     */
    bool FlushPage(int32_t page_id);

    /**
     * @brief 删除页面
     * @param page_id 页面ID
     * @return 操作是否成功
     * 
     * Why: 当数据不再需要时，需要释放页面空间，例如删除记录或索引
     * What: DeletePage方法从缓冲池和磁盘中删除指定页面
     * How: 调用缓冲池的DeletePage方法，由缓冲池协调与磁盘管理器的交互
     */
    bool DeletePage(int32_t page_id);

    /**
     * @brief 刷新所有页面到磁盘
     * 
     * Why: 在系统关闭或检查点时，需要将所有修改过的页面写入磁盘，保证数据持久性
     * What: FlushAllPages方法将缓冲池中所有脏页写入磁盘
     * How: 调用缓冲池的FlushAllPages方法，由缓冲池协调与磁盘管理器的交互
     */
    void FlushAllPages();

private:
    /// 配置管理器引用
    // Why: 需要访问配置参数来初始化和调整存储引擎的行为
    // What: config_manager_是对ConfigManager对象的引用，用于获取配置参数
    // How: 通过引用方式使用ConfigManager，避免所有权问题
    ConfigManager& config_manager_;
    
    /// 磁盘管理器
    // Why: 需要一个组件负责磁盘I/O操作，管理数据库文件的读写
    // What: disk_manager_是一个智能指针，指向DiskManager对象，负责页面的磁盘读写操作
    // How: 使用std::unique_ptr管理DiskManager对象的生命周期，确保资源正确释放
    std::unique_ptr<DiskManager> disk_manager_;

    /// 缓冲池管理器
    // Why: 需要一个组件负责内存中的页面缓存，提高数据库性能
    // What: buffer_pool_是一个智能指针，指向BufferPool对象，负责内存中的页面管理
    // How: 使用std::unique_ptr管理BufferPool对象的生命周期，确保资源正确释放
    std::unique_ptr<BufferPool> buffer_pool_;
};

}  // namespace sqlcc