#include "storage_engine.h"
#include "logger.h"

namespace sqlcc {

// 存储引擎构造函数实现
// Why: 需要初始化存储引擎，创建磁盘管理器和缓冲池实例
// What: 构造函数接收数据库文件名和缓冲池大小参数，初始化存储引擎的各个组件
// How: 创建DiskManager和BufferPool对象，建立它们之间的关联关系
StorageEngine::StorageEngine(const std::string& db_filename, size_t pool_size) {
    // 记录初始化信息，便于调试和监控
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在初始化的数据库文件名和缓冲池大小
    // How: 使用SQLCC_LOG_INFO宏记录信息级别日志
    SQLCC_LOG_INFO("Initializing StorageEngine with database file: " + db_filename + 
                  " and pool size: " + std::to_string(pool_size));
    
    // 创建磁盘管理器
    // Why: 需要一个组件负责磁盘I/O操作，管理数据库文件的读写
    // What: 创建DiskManager对象，负责页面的磁盘读写操作
    // How: 使用std::make_unique创建DiskManager对象，传入数据库文件名
    disk_manager_ = std::make_unique<DiskManager>(db_filename);
    
    // 创建缓冲池管理器
    // Why: 需要一个组件负责内存中的页面缓存，提高数据库性能
    // What: 创建BufferPool对象，负责内存中的页面管理
    // How: 使用std::make_unique创建BufferPool对象，传入磁盘管理器指针和缓冲池大小
    buffer_pool_ = std::make_unique<BufferPool>(disk_manager_.get(), pool_size);
    
    // 记录初始化成功，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录存储引擎初始化成功
    // How: 使用SQLCC_LOG_INFO宏记录信息级别日志
    SQLCC_LOG_INFO("StorageEngine initialized successfully");
}

// 存储引擎析构函数实现
// Why: 需要释放存储引擎占用的资源，确保数据正确写入磁盘
// What: 析构函数负责刷新所有脏页到磁盘，释放缓冲池和磁盘管理器资源
// How: 调用FlushAllPages方法确保所有数据持久化，智能指针自动释放资源
StorageEngine::~StorageEngine() {
    // 记录销毁信息，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录存储引擎正在销毁
    // How: 使用SQLCC_LOG_INFO宏记录信息级别日志
    SQLCC_LOG_INFO("Destroying StorageEngine");
    // 析构函数会自动清理资源
    // Why: 使用智能指针管理资源，析构时会自动释放
    // What: 智能指针会自动调用析构函数释放资源
    // How: std::unique_ptr在析构时会自动释放所管理的对象
}

// 创建新页面实现
// Why: 数据库需要新的存储空间来存储数据，例如插入新记录或创建索引
// What: NewPage方法在数据库中分配一个新的页面，返回指向该页面的指针，并通过输出参数返回页面ID
// How: 通过磁盘管理器分配新的页面ID，通过缓冲池创建页面对象，将页面标记为脏页
Page* StorageEngine::NewPage(int32_t* page_id) {
    // 记录创建页面操作，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在创建新页面
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Creating new page");
    // 调用缓冲池创建新页面
    // Why: 缓冲池负责管理内存中的页面，包括创建新页面
    // What: 调用BufferPool的NewPage方法创建新页面
    // How: 直接调用buffer_pool_的NewPage方法，传入页面ID指针
    Page* page = buffer_pool_->NewPage(page_id);
    // 检查创建结果，记录日志
    // Why: 需要检查操作是否成功，并记录结果
    // What: 根据页面指针是否为空判断创建是否成功，记录相应日志
    // How: 使用if语句检查页面指针，使用SQLCC_LOG_DEBUG或SQLCC_LOG_ERROR记录日志
    if (page != nullptr) {
        SQLCC_LOG_DEBUG("New page created with ID: " + std::to_string(*page_id));
    } else {
        SQLCC_LOG_ERROR("Failed to create new page");
    }
    return page;
}

// 获取页面实现
// Why: 需要访问存储在磁盘上的页面数据，例如查询记录或更新数据
// What: FetchPage方法根据页面ID获取页面对象，如果页面不在内存中则从磁盘加载
// How: 通过缓冲池获取页面，如果页面不在内存中则通过磁盘管理器从磁盘加载
Page* StorageEngine::FetchPage(int32_t page_id) {
    // 记录获取页面操作，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在获取的页面ID
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Fetching page ID: " + std::to_string(page_id));
    // 调用缓冲池获取页面
    // Why: 缓冲池负责管理内存中的页面，包括从磁盘加载页面
    // What: 调用BufferPool的FetchPage方法获取页面
    // How: 直接调用buffer_pool_的FetchPage方法，传入页面ID
    Page* page = buffer_pool_->FetchPage(page_id);
    // 检查获取结果，记录日志
    // Why: 需要检查操作是否成功，并记录结果
    // What: 根据页面指针是否为空判断获取是否成功，记录相应日志
    // How: 使用if语句检查页面指针，使用SQLCC_LOG_DEBUG或SQLCC_LOG_WARN记录日志
    if (page != nullptr) {
        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " fetched successfully");
    } else {
        SQLCC_LOG_WARN("Failed to fetch page ID " + std::to_string(page_id));
    }
    return page;
}

// 取消固定页面实现
// Why: 当页面使用完毕后，需要通知缓冲池该页面可以被替换，同时标记页面是否被修改
// What: UnpinPage方法减少页面的固定计数，如果页面被修改则标记为脏页
// How: 调用缓冲池的UnpinPage方法，传递页面ID和脏页标记
bool StorageEngine::UnpinPage(int32_t page_id, bool is_dirty) {
    // 记录取消固定页面操作，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在取消固定的页面ID和脏页标记
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Unpinning page ID: " + std::to_string(page_id) + ", is_dirty: " + std::to_string(is_dirty));
    // 调用缓冲池取消固定页面
    // Why: 缓冲池负责管理内存中的页面，包括取消固定页面
    // What: 调用BufferPool的UnpinPage方法取消固定页面
    // How: 直接调用buffer_pool_的UnpinPage方法，传入页面ID和脏页标记
    bool result = buffer_pool_->UnpinPage(page_id, is_dirty);
    // 检查操作结果，记录日志
    // Why: 需要检查操作是否成功，并记录结果
    // What: 根据返回值判断操作是否成功，记录相应日志
    // How: 使用if语句检查返回值，使用SQLCC_LOG_DEBUG或SQLCC_LOG_WARN记录日志
    if (result) {
        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " unpinned successfully");
    } else {
        SQLCC_LOG_WARN("Failed to unpin page ID " + std::to_string(page_id));
    }
    return result;
}

// 刷新页面到磁盘实现
// Why: 需要将修改后的页面数据持久化到磁盘，保证数据的持久性和一致性
// What: FlushPage方法将指定页面的数据写入磁盘文件
// How: 调用缓冲池的FlushPage方法，由缓冲池协调与磁盘管理器的交互
bool StorageEngine::FlushPage(int32_t page_id) {
    // 记录刷新页面操作，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在刷新的页面ID
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Flushing page ID: " + std::to_string(page_id));
    // 调用缓冲池刷新页面
    // Why: 缓冲池负责管理内存中的页面，包括将页面写入磁盘
    // What: 调用BufferPool的FlushPage方法刷新页面
    // How: 直接调用buffer_pool_的FlushPage方法，传入页面ID
    bool result = buffer_pool_->FlushPage(page_id);
    // 检查操作结果，记录日志
    // Why: 需要检查操作是否成功，并记录结果
    // What: 根据返回值判断操作是否成功，记录相应日志
    // How: 使用if语句检查返回值，使用SQLCC_LOG_DEBUG或SQLCC_LOG_WARN记录日志
    if (result) {
        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " flushed successfully");
    } else {
        SQLCC_LOG_WARN("Failed to flush page ID " + std::to_string(page_id));
    }
    return result;
}

// 删除页面实现
// Why: 当数据不再需要时，需要释放页面空间，例如删除记录或索引
// What: DeletePage方法从缓冲池和磁盘中删除指定页面
// How: 调用缓冲池的DeletePage方法，由缓冲池协调与磁盘管理器的交互
bool StorageEngine::DeletePage(int32_t page_id) {
    // 记录删除页面操作，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在删除的页面ID
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Deleting page ID: " + std::to_string(page_id));
    // 调用缓冲池删除页面
    // Why: 缓冲池负责管理内存中的页面，包括删除页面
    // What: 调用BufferPool的DeletePage方法删除页面
    // How: 直接调用buffer_pool_的DeletePage方法，传入页面ID
    bool result = buffer_pool_->DeletePage(page_id);
    // 检查操作结果，记录日志
    // Why: 需要检查操作是否成功，并记录结果
    // What: 根据返回值判断操作是否成功，记录相应日志
    // How: 使用if语句检查返回值，使用SQLCC_LOG_DEBUG或SQLCC_LOG_WARN记录日志
    if (result) {
        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " deleted successfully");
    } else {
        SQLCC_LOG_WARN("Failed to delete page ID " + std::to_string(page_id));
    }
    return result;
}

// 刷新所有页面到磁盘实现
// Why: 在系统关闭或检查点时，需要将所有修改过的页面写入磁盘，保证数据持久性
// What: FlushAllPages方法将缓冲池中所有脏页写入磁盘
// How: 调用缓冲池的FlushAllPages方法，由缓冲池协调与磁盘管理器的交互
void StorageEngine::FlushAllPages() {
    // 记录刷新所有页面操作，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在刷新所有页面
    // How: 使用SQLCC_LOG_INFO宏记录信息级别日志
    SQLCC_LOG_INFO("Flushing all pages");
    // 调用缓冲池刷新所有页面
    // Why: 缓冲池负责管理内存中的页面，包括将所有页面写入磁盘
    // What: 调用BufferPool的FlushAllPages方法刷新所有页面
    // How: 直接调用buffer_pool_的FlushAllPages方法
    buffer_pool_->FlushAllPages();
    // 记录刷新成功，便于调试
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录所有页面刷新成功
    // How: 使用SQLCC_LOG_INFO宏记录信息级别日志
    SQLCC_LOG_INFO("All pages flushed successfully");
}

}  // namespace sqlcc