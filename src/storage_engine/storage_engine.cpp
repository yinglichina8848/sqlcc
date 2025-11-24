#include "storage_engine.h"
#include "../include/sql_executor.h"
#include "logger.h"
#include <memory>

namespace sqlcc {

// 存储引擎构造函数实现
// Why: 需要初始化存储引擎，从配置管理器获取参数创建磁盘管理器和缓冲池实例
// What: 构造函数接收配置管理器引用，从中获取参数初始化存储引擎的各个组件
// How:
// 从配置管理器获取数据库文件路径和缓冲池大小，创建DiskManager和BufferPool对象
StorageEngine::StorageEngine(ConfigManager &config_manager)
    : config_manager_(config_manager) {
  // 从配置管理器获取数据库文件路径
  std::string db_file_path =
      config_manager_.GetString("database.db_file_path", "./sqlcc.db");

  // 从配置管理器获取缓冲池大小
  size_t pool_size =
      static_cast<size_t>(config_manager_.GetInt("buffer_pool.pool_size", 64));

  // 记录初始化信息，便于调试和监控
  // Why: 日志记录有助于系统运行状态的监控和问题排查
  // What: 记录正在初始化的数据库文件名和缓冲池大小
  // How: 使用SQLCC_LOG_INFO宏记录信息级别日志
  SQLCC_LOG_INFO("Initializing StorageEngine with database file: " +
                 db_file_path + " and pool size: " + std::to_string(pool_size));

  // 创建磁盘管理器
  // Why: 需要一个组件负责磁盘I/O操作，管理数据库文件的读写
  // What: 创建DiskManager对象，负责页面的磁盘读写操作
  // How: 使用std::make_unique创建DiskManager对象，传入数据库文件名和配置管理器
  disk_manager_ = std::make_unique<DiskManager>(db_file_path, config_manager_);

  // 创建缓冲池管理器
  // Why: 需要一个组件负责内存中的页面缓存，提高数据库性能
  // What: 创建BufferPool对象，负责内存中的页面管理
  // How:
  // 使用std::make_unique创建BufferPool对象，传入磁盘管理器指针、缓冲池大小和配置管理器
  buffer_pool_ = std::make_unique<BufferPool>(disk_manager_.get(), pool_size,
                                              config_manager_);

  // 注意：配置回调功能已禁用，不再注册配置变更回调

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

  // 在销毁前刷新所有脏页到磁盘
  // Why: 确保所有修改过的数据都被持久化到磁盘，避免数据丢失
  // What: 调用FlushAllPages方法将所有脏页写回磁盘
  // How: 直接调用FlushAllPages方法
  FlushAllPages();

  // 析构函数会自动清理资源
  // Why: 使用智能指针管理资源，析构时会自动释放
  // What: 智能指针会自动调用析构函数释放资源
  // How: std::unique_ptr在析构时会自动释放所管理的对象
}

// 创建新页面实现
// Why: 数据库需要新的存储空间来存储数据，例如插入新记录或创建索引
// What:
// NewPage方法在数据库中分配一个新的页面，返回指向该页面的指针，并通过输出参数返回页面ID
// How: 通过磁盘管理器分配新的页面ID，通过缓冲池创建页面对象，将页面标记为脏页
Page *StorageEngine::NewPage(int32_t *page_id) {
  // 记录创建页面操作，便于调试
  // Why: 日志记录有助于系统运行状态的监控和问题排查
  // What: 记录正在创建新页面
  // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
  SQLCC_LOG_DEBUG("Creating new page");
  // 调用缓冲池创建新页面
  // Why: 缓冲池负责管理内存中的页面，包括创建新页面
  // What: 调用BufferPool的NewPage方法创建新页面
  // How: 直接调用buffer_pool_的NewPage方法，传入页面ID指针，添加异常处理
  Page *page = nullptr;
  try {
    page = buffer_pool_->NewPage(page_id);
  } catch (const LockTimeoutException &e) {
    SQLCC_LOG_ERROR("Lock timeout when creating new page: " +
                    std::string(e.what()));
    // 可以尝试重试策略或者直接返回nullptr
  }
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
Page *StorageEngine::FetchPage(int32_t page_id) {
  // 记录获取页面操作，便于调试
  // Why: 日志记录有助于系统运行状态的监控和问题排查
  // What: 记录正在获取的页面ID
  // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
  SQLCC_LOG_DEBUG("Fetching page ID: " + std::to_string(page_id));
  // 调用缓冲池获取页面
  // Why: 缓冲池负责管理内存中的页面，包括从磁盘加载页面
  // What: 调用BufferPool的FetchPage方法获取页面
  // How: 直接调用buffer_pool_的FetchPage方法，传入页面ID，添加异常处理
  Page *page = nullptr;
  try {
    page = buffer_pool_->FetchPage(page_id);
  } catch (const LockTimeoutException &e) {
    SQLCC_LOG_ERROR("Lock timeout when fetching page: " +
                    std::string(e.what()));
    // 可以尝试重试策略或者直接返回nullptr
  }
  // 检查获取结果，记录日志
  // Why: 需要检查操作是否成功，并记录结果
  // What: 根据页面指针是否为空判断获取是否成功，记录相应日志
  // How: 使用if语句检查页面指针，使用SQLCC_LOG_DEBUG或SQLCC_LOG_WARN记录日志
  if (page != nullptr) {
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) +
                    " fetched successfully");
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
  SQLCC_LOG_DEBUG("Unpinning page ID: " + std::to_string(page_id) +
                  ", is_dirty: " + std::to_string(is_dirty));
  // 调用缓冲池取消固定页面
  // Why: 缓冲池负责管理内存中的页面，包括取消固定页面
  // What: 调用BufferPool的UnpinPage方法取消固定页面
  // How:
  // 直接调用buffer_pool_的UnpinPage方法，传入页面ID和脏页标记，添加异常处理
  bool result = false;
  try {
    result = buffer_pool_->UnpinPage(page_id, is_dirty);
  } catch (const LockTimeoutException &e) {
    SQLCC_LOG_ERROR("Lock timeout when unpinning page: " +
                    std::string(e.what()));
    // 锁超时导致操作失败
  }
  // 检查操作结果，记录日志
  // Why: 需要检查操作是否成功，并记录结果
  // What: 根据返回值判断操作是否成功，记录相应日志
  // How: 使用if语句检查返回值，使用SQLCC_LOG_DEBUG或SQLCC_LOG_WARN记录日志
  if (result) {
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) +
                    " unpinned successfully");
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
  // How: 直接调用buffer_pool_的FlushPage方法，传入页面ID，添加异常处理
  bool result = false;
  try {
    result = buffer_pool_->FlushPage(page_id);
  } catch (const LockTimeoutException &e) {
    SQLCC_LOG_ERROR("Lock timeout when flushing page: " +
                    std::string(e.what()));
    // 锁超时导致操作失败
  }
  // 检查操作结果，记录日志
  // Why: 需要检查操作是否成功，并记录结果
  // What: 根据返回值判断操作是否成功，记录相应日志
  // How: 使用if语句检查返回值，使用SQLCC_LOG_DEBUG或SQLCC_LOG_WARN记录日志
  if (result) {
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) +
                    " flushed successfully");
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
  // How: 直接调用buffer_pool_的DeletePage方法，传入页面ID，添加异常处理
  bool result = false;
  try {
    result = buffer_pool_->DeletePage(page_id);
  } catch (const LockTimeoutException &e) {
    SQLCC_LOG_ERROR("Lock timeout when deleting page: " +
                    std::string(e.what()));
    // 锁超时导致操作失败
  }
  // 检查操作结果，记录日志
  // Why: 需要检查操作是否成功，并记录结果
  // What: 根据返回值判断操作是否成功，记录相应日志
  // How: 使用if语句检查返回值，使用SQLCC_LOG_DEBUG或SQLCC_LOG_WARN记录日志
  if (result) {
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) +
                    " deleted successfully");
  } else {
    SQLCC_LOG_WARN("Failed to delete page ID " + std::to_string(page_id));
  }
  return result;
}

// 刷新所有页面到磁盘实现
// Why: 需要将所有修改过的页面数据持久化到磁盘，保证数据的一致性和持久性
// What: FlushAllPages方法将所有脏页写入磁盘文件
// How: 调用缓冲池的FlushAllPages方法，由缓冲池协调与磁盘管理器的交互
void StorageEngine::FlushAllPages() {
  // 记录刷新所有页面操作，便于调试
  // Why: 日志记录有助于系统运行状态的监控和问题排查
  // What: 记录正在刷新所有页面
  // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
  SQLCC_LOG_DEBUG("Flushing all pages");

  // 直接调用缓冲池的FlushAllPages方法，不需要额外加锁
  // 注意：这里不再使用storage_engine的锁来保护，因为BufferPool::FlushAllPages内部已经实现了正确的锁管理策略
  // 外部再加锁会导致死锁问题
  try {
    buffer_pool_->FlushAllPages();
  } catch (const LockTimeoutException &e) {
    SQLCC_LOG_ERROR("Lock timeout when flushing all pages: " +
                    std::string(e.what()));
    // 锁超时，可能需要重新尝试或通知用户
  }

  // 记录操作完成，便于调试
  // Why: 日志记录有助于系统运行状态的监控和问题排查
  // What: 记录所有页面刷新完成
  // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
  SQLCC_LOG_DEBUG("All pages flushed successfully");
}

// 获取数据库统计信息实现
// Why: 需要了解数据库的运行状态和性能指标，便于监控和优化
// What: GetStats方法返回数据库的统计信息，包括页面数量、缓冲池状态等
// How: 调用缓冲池的GetStats方法获取缓冲池统计信息，将map转换为字符串格式
std::string StorageEngine::GetStats() const {
  // 记录获取统计信息操作，便于调试
  // Why: 日志记录有助于系统运行状态的监控和问题排查
  // What: 记录正在获取数据库统计信息
  // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
  SQLCC_LOG_DEBUG("Getting database statistics");
  // 获取缓冲池统计信息
  // Why: 缓冲池是数据库的核心组件，其统计信息反映了数据库的运行状态
  // What: 调用BufferPool的GetStats方法获取缓冲池统计信息
  // How: 直接调用buffer_pool_的GetStats方法，获取map格式的统计信息
  std::unordered_map<std::string, double> stats_map = buffer_pool_->GetStats();

  // 将统计信息map转换为字符串格式
  // Why: 需要将map格式的统计信息转换为字符串以便返回和显示
  // What: 遍历map中的键值对，构建格式化的字符串
  // How: 使用字符串流拼接键值对，添加适当的分隔符
  std::string stats = "Buffer Pool Stats: ";
  for (const auto &pair : stats_map) {
    stats += pair.first + "=" + std::to_string(pair.second) + " ";
  }

  // 记录操作完成，便于调试
  // Why: 日志记录有助于系统运行状态的监控和问题排查
  // What: 记录获取统计信息完成
  // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
  SQLCC_LOG_DEBUG("Database statistics retrieved successfully");
  return stats;
}

} // namespace sqlcc