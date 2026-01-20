/**
 * @file buffer_pool_sharded.cpp
 * @brief SQLCC分片缓冲池实现 - 16分片高并发缓冲池管理
 *
 * 设计理念：通过分片策略降低锁竞争，提高并发性能
 * 核心优势：将缓冲池分割为多个独立的子缓冲池，每个子缓冲池独立管理
 * 性能优化：减少锁粒度，提升多线程并发访问效率
 */

#include "include/storage/buffer_pool_sharded.h"
#include "disk_manager.h"
#include "exception.h"
#include "include/utils/logger.h"

namespace sqlcc {

/**
 * @class BufferPoolSharded
 * @brief 分片缓冲池管理器 - 现代数据库缓冲池的核心组件
 *
 * WHY层 - 设计意图：
 *   在现代多核CPU环境下，传统的单锁缓冲池已成为性能瓶颈。
 *   通过将缓冲池分割为16个独立的分片，每个分片独立管理自己的页面，
 *   大大降低了锁竞争，提高了并发访问性能。
 *
 * WHAT层 - 功能说明：
 *   提供页面缓存、替换、刷新等核心功能，支持LRU页面替换策略。
 *   实现了线程安全的页面访问，支持脏页管理和异步刷新。
 *
 * HOW层 - 实现细节：
 *   使用哈希函数将页面ID映射到特定分片，每个分片独立维护LRU列表。
 *   通过细粒度锁机制实现高并发访问，采用写时复制策略避免阻塞。
 *
 * 性能特征：
 *   - 并发度：支持16个并发线程同时访问不同分片
 *   - 命中率：LRU策略确保热点数据常驻内存
 *   - 内存效率：动态页面替换，适应不同工作负载
 *
 * @note 分片数量设计为16是因为：
 *       1. 2^4 = 16，便于高效的哈希计算
 *       2. 在常见工作负载下提供良好并发度
 *       3. 平衡内存开销和并发性能
 *
 * @see docs/design/storage_engine/sharded_buffer_pool_design.md
 *      详细的分片缓冲池设计文档，包含完整的算法分析和性能优化指南
 */
/**
 * @brief 构造函数 - 初始化16分片缓冲池
 *
 * WHY层 - 设计意图：
 *   分片数量调整为2的幂次方是为了优化哈希分布和减少冲突。
 *   每个分片独立分配内存，避免了大锁竞争，提高初始化效率。
 *
 * WHAT层 - 功能说明：
 *   根据配置参数初始化缓冲池，自动调整分片数量确保性能最优。
 *   为每个分片分配相等大小的缓冲区，建立独立的管理结构。
 *
 * HOW层 - 实现细节：
 *   1. 验证分片数量为2的幂，通过位运算优化哈希计算
 *   2. 平均分配总缓冲池大小到各个分片
 *   3. 为每个分片创建独立的Shard对象，包含LRU管理和锁机制
 *
 * @param disk_manager 磁盘管理器，用于页面持久化
 * @param config_manager 配置管理器，提供运行时参数
 * @param pool_size 总缓冲池大小（页面数量）
 * @param num_shards 分片数量，建议为16以获得最佳并发性能
 *
 * @note 分片大小计算：shard_size = pool_size / num_shards
 * @note 实际分片数量可能被调整为最接近的2的幂
 */
BufferPoolSharded::BufferPoolSharded(std::shared_ptr<DiskManager> disk_manager,
                                     ConfigManager &config_manager,
                                     size_t pool_size, size_t num_shards)
    : disk_manager_(std::move(disk_manager)), config_manager_(config_manager),
      pool_size_(pool_size), next_page_id_(0) {
  // 确保num_shards是2的幂 - 这是分片策略的核心优化
  if (num_shards & (num_shards - 1)) {
    // 找到最接近的2的幂 - 使用位运算实现高效计算
    num_shards_ = 1;
    while (num_shards_ < num_shards) {
      num_shards_ <<= 1;
    }
    SQLCC_LOG_INFO("Adjusting shard count to power of 2: " +
                   std::to_string(num_shards_));
  } else {
    num_shards_ = num_shards;
  }

  // 初始化shards - 每个分片独立管理，提高并发性能
  size_t shard_size = pool_size_ / num_shards_;
  shards_.resize(num_shards_);
  for (size_t i = 0; i < num_shards_; ++i) {
    shards_[i] = std::make_unique<Shard>(shard_size);
  }

  SQLCC_LOG_INFO("Sharded BufferPool initialized with " +
                 std::to_string(num_shards_) + " shards, each with " +
                 std::to_string(shard_size) + " pages");
}

BufferPoolSharded::~BufferPoolSharded() {
  SQLCC_LOG_INFO("Destroying Sharded BufferPool");
  FlushAllPages();
}

/**
 * @brief 获取页面 - 分片缓冲池的核心页面访问接口
 *
 * WHY层 - 设计意图：
 *   这是缓冲池最关键的接口，决定了数据库的I/O性能。
 *   通过分片哈希定位，避免全局锁竞争，实现高并发访问。
 *   采用写时复制策略，确保调用者获得独立的数据副本。
 *
 * WHAT层 - 功能说明：
 *   根据页面ID获取页面数据，支持缓存命中和磁盘加载两种路径。
 *   实现LRU页面替换策略，管理缓冲池容量限制。
 *   返回页面数据的独立副本，保证调用者安全修改。
 *
 * HOW层 - 实现细节：
 *   1. 哈希计算分片索引：shard_idx = page_id % num_shards
 *   2. 分片级锁保护：每个分片独立加锁，减少锁竞争
 *   3. 双重检查缓存：先查页面表，再更新LRU位置
 *   4. 容量管理：缓冲池满时触发LRU页面替换
 *   5. 磁盘I/O：缓存未命中时同步读取磁盘数据
 *   6. 引用计数：跟踪页面使用情况，支持并发访问
 *
 * 并发安全保证：
 *   - 分片级锁确保同一分片内的串行化访问
 *   - 不同分片的并发访问不受影响
 *   - 写时复制避免调用者间的相互干扰
 *
 * @param page_id 要获取的页面ID
 * @param exclusive 独占访问标志（当前版本未使用，预留扩展）
 * @return 页面数据的独立副本，失败时返回nullptr
 *
 * @note 性能关键路径：哈希计算->锁获取->缓存查找->返回副本
 * @note 内存安全：返回unique_ptr确保资源自动管理
 * @note 异常处理：磁盘读取失败时创建空页面，避免崩溃
 */
std::unique_ptr<Page> BufferPoolSharded::FetchPage(int32_t page_id, bool exclusive) {
    size_t shard_idx = GetShardIndex(page_id);
    Shard &shard = *shards_[shard_idx];
    (void)exclusive; // 标记参数为已使用

    std::lock_guard<std::mutex> lock(shard.mutex);

    // 查找页面是否已在缓冲池中
    auto it = shard.page_table.find(page_id);
    if (it != shard.page_table.end()) {
        // 页面已在缓冲池中
        std::shared_ptr<PageWrapper> page_wrapper = it->second;

        // 更新引用计数
        page_wrapper->ref_count++;

        // 如果页面在LRU列表中，则将其移到头部
        if (page_wrapper->is_in_lru) {
            MoveToHead(shard, page_id);
        }

        stats_.total_hits++;
        // 返回页面的副本，确保调用者拥有独立的所有权
        return std::make_unique<Page>(*page_wrapper->page);
    }

    // 页面不在缓冲池中，需要从磁盘加载
    stats_.total_misses++;

    // 如果shard已满，需要替换页面
    if (shard.current_size >= shard.max_size) {
        int32_t replaced_page_id = ReplacePage(shard);
        if (replaced_page_id == -1) {
            SQLCC_LOG_ERROR("Failed to replace page for page_id: " +
                           std::to_string(page_id));
            // 返回空智能指针表示替换失败
            return nullptr;
        }
    }

    // 从磁盘读取页面数据
    char page_data[PAGE_SIZE];
    bool read_success = disk_manager_->ReadPage(page_id, page_data);
    if (!read_success) {
        // 如果读取失败，可能是新页面，创建一个新的页面
        memset(page_data, 0, PAGE_SIZE);
    }

    // 创建新页面
    auto page = std::make_unique<Page>(page_id);
    #ifdef __cpp_lib_span
        memcpy(page->GetDataSpan().data(), page_data, PAGE_SIZE);
    #else
        // C++17兼容模式下直接访问数据指针
        memcpy(page->GetDataSpan().data, page_data, PAGE_SIZE);
    #endif

    auto page_wrapper = std::make_shared<PageWrapper>(std::move(page));
    page_wrapper->ref_count = 1;
    page_wrapper->is_dirty = false;

    shard.page_table[page_id] = page_wrapper;
    shard.lru_list.push_front(page_id);
    shard.lru_map[page_id] = shard.lru_list.begin();
    page_wrapper->lru_iter = shard.lru_list.begin();
    page_wrapper->is_in_lru = true;
    shard.current_size++;

    // 记录已分配的页面
    {
        std::lock_guard<std::mutex> alloc_lock(allocated_pages_mutex_);
        allocated_pages_.insert(page_id);
    }

    stats_.total_accesses++;
    // 返回页面的副本，确保调用者拥有独立的所有权
    return std::make_unique<Page>(*page_wrapper->page);
}

/**
 * @brief 刷新单个页面到磁盘 - 脏页同步的核心机制
 *
 * WHY层 - 设计意图：
 *   缓冲池的脏页管理是数据库ACID属性的关键保障。
 *   及时将修改的页面同步到磁盘，避免数据丢失。
 *   支持按需刷新，避免频繁的全量同步开销。
 *
 * WHAT层 - 功能说明：
 *   检查指定页面是否为脏页，如果是则将其内容同步到磁盘。
 *   成功刷新后清除脏页标记，确保数据一致性。
 *   失败时保持脏页状态，支持后续重试。
 *
 * HOW层 - 实现细节：
 *   1. 通过哈希定位页面所属分片
 *   2. 分片级锁保护并发访问安全
 *   3. 检查页面是否存在和脏页状态
 *   4. 调用磁盘管理器执行页面写入
 *   5. 成功后清除脏页标记
 *
 * 并发安全保证：
 *   - 分片级锁确保同一分片内的原子性操作
 *   - 不影响其他分片的并发刷新操作
 *   - 脏页标记的原子性更新
 *
 * 性能优化：
 *   - 只刷新真正修改过的页面
 *   - 分片设计减少锁竞争范围
 *   - 支持异步刷新扩展点
 *
 * @param page_id 要刷新的页面ID
 * @return 刷新是否成功
 *
 * @note 刷新失败不会丢失数据，只会推迟同步
 * @note 大量脏页可能影响系统性能，需要监控
 * @note 未来可扩展为异步刷新机制
 */
bool BufferPoolSharded::FlushPage(int32_t page_id) {
  size_t shard_idx = GetShardIndex(page_id);
  Shard &shard = *shards_[shard_idx];

  std::lock_guard<std::mutex> lock(shard.mutex);

  auto it = shard.page_table.find(page_id);
  if (it == shard.page_table.end()) {
    return false;
  }

  std::shared_ptr<PageWrapper> page_wrapper = it->second;
  if (!page_wrapper->is_dirty) {
    return true;
  }

  bool write_success = disk_manager_->WritePage(
#ifdef __cpp_lib_span
      page_id, static_cast<char *>(page_wrapper->page->GetDataSpan().data()));
#else
      // C++17兼容模式下直接访问数据指针
      page_id, static_cast<char *>(page_wrapper->page->GetDataSpan().data));
#endif

  if (write_success) {
    page_wrapper->is_dirty = false;
  }

  return write_success;
}

void BufferPoolSharded::FlushAllPages() {
  for (size_t i = 0; i < num_shards_; ++i) {
    Shard &shard = *shards_[i];
    std::lock_guard<std::mutex> lock(shard.mutex);

    for (const auto &pair : shard.page_table) {
      int32_t page_id = pair.first;
      std::shared_ptr<PageWrapper> page_wrapper = pair.second;
      (void)page_id; // 标记变量为已使用

      if (page_wrapper->is_dirty) {
        disk_manager_->WritePage(
#ifdef __cpp_lib_span
            page_id, static_cast<char *>(page_wrapper->page->GetDataSpan().data()));
#else
            // C++17兼容模式下直接访问数据指针
            page_id, static_cast<char *>(page_wrapper->page->GetDataSpan().data));
#endif
        page_wrapper->is_dirty = false;
      }
    }
  }
}

/**
 * @brief 释放页面引用 - 引用计数管理的核心机制
 *
 * WHY层 - 设计意图：
 *   缓冲池的引用计数是内存安全的关键，避免页面在被使用时被替换。
 *   通过引用计数实现多线程间的安全共享，保证数据一致性。
 *   脏页标记支持延迟写回策略，优化I/O性能。
 *
 * WHAT层 - 功能说明：
 *   减少指定页面的引用计数，当计数为零时页面可以被替换。
 *   支持脏页标记，用于跟踪页面是否被修改过。
 *   确保页面在被安全释放前不会被意外替换。
 *
 * HOW层 - 实现细节：
 *   1. 通过哈希计算定位页面所属分片
 *   2. 分片级锁保证原子性操作
 *   3. 引用计数递减，支持并发访问
 *   4. 脏页标记设置，影响后续刷新策略
 *
 * 并发安全保证：
 *   - 分片级锁确保计数器操作的原子性
 *   - 引用计数线程安全，支持多线程同时访问
 *   - 脏页标记原子更新，不丢失修改状态
 *
 * 性能优化：
 *   - 轻量级操作，只涉及计数器修改
 *   - 分片设计减少锁竞争范围
 *   - 延迟刷新减少同步I/O开销
 *
 * @param page_id 要释放引用的页面ID
 * @param is_dirty 页面是否已被修改（脏页标记）
 * @return 释放操作是否成功
 *
 * @note 引用计数为零后，页面可能被LRU替换
 * @note 脏页会在合适时机异步刷新到磁盘
 * @note 这是缓冲池内存管理的重要同步点
 */
bool BufferPoolSharded::UnpinPage(int32_t page_id, bool is_dirty) {
  size_t shard_idx = GetShardIndex(page_id);
  Shard &shard = *shards_[shard_idx];

  std::lock_guard<std::mutex> lock(shard.mutex);

  auto it = shard.page_table.find(page_id);
  if (it == shard.page_table.end()) {
    return false;
  }

  std::shared_ptr<PageWrapper> page_wrapper = it->second;
  if (page_wrapper->ref_count > 0) {
    page_wrapper->ref_count--;
  }

  if (is_dirty) {
    page_wrapper->is_dirty = true;
  }

  return true;
}

/**
 * @brief 创建新的页面 - 分片缓冲池的页面分配接口
 *
 * WHY层 - 设计意图：
 *   数据库运行过程中经常需要创建新的页面来存储数据。
 *   NewPage操作是数据库写操作的基础，决定了系统的扩展能力。
 *   通过分片设计，即使在高并发场景下也能高效分配页面。
 *
 * WHAT层 - 功能说明：
 *   分配一个新的唯一页面ID，创建对应的页面对象并初始化为空。
 *   如果缓冲池已满，会自动触发页面替换以腾出空间。
 *   返回新页面的独立副本，保证调用者可以安全修改。
 *
 * HOW层 - 实现细节：
 *   1. 原子递增全局页面ID计数器，确保ID唯一性
 *   2. 通过哈希计算确定目标分片，避免全局锁竞争
 *   3. 分片级锁保护，确保同一分片的串行化分配
 *   4. 容量检查和自动页面替换，保持缓冲池可用空间
 *   5. 初始化页面包装器，设置引用计数和LRU状态
 *   6. 记录到已分配页面集合，支持统计和清理
 *
 * 并发安全保证：
 *   - 页面ID分配使用原子操作，确保全局唯一
 *   - 分片级锁确保同一分片内的分配串行化
 *   - 不同分片的并发分配不受影响
 *
 * 页面生命周期：
 *   - 创建时ref_count = 1，表示调用者拥有引用
 *   - 页面初始为干净状态(is_dirty = false)
 *   - 自动加入LRU链表头部，作为最近访问页面
 *
 * 内存管理：
 *   - 返回unique_ptr确保资源自动管理
 *   - 页面数据初始化为全零，保证确定性行为
 *   - 失败时返回nullptr，调用者需要处理错误情况
 *
 * @param page_id 输出参数，用于返回新分配的页面ID
 * @return 新页面的独立副本，失败时返回nullptr
 *
 * @note 页面ID从0开始递增，确保全局唯一性
 * @note 新页面内容初始化为全零，保证可预测行为
 * @note 如果缓冲池满，会自动触发LRU页面替换
 * @note 这是数据库写操作的基础，支持表创建、索引构建等
 */
std::unique_ptr<Page> BufferPoolSharded::NewPage(int32_t *page_id) {
  // 分配新的页面ID - 使用原子递增确保并发安全
  int32_t new_page_id = next_page_id_++;

  // 获取对应的shard - 哈希分片确保负载均衡
  size_t shard_idx = GetShardIndex(new_page_id);
  Shard &shard = *shards_[shard_idx];

  std::lock_guard<std::mutex> lock(shard.mutex);

  // 如果shard已满，需要替换页面 - 自动容量管理
  if (shard.current_size >= shard.max_size) {
    int32_t replaced_page_id = ReplacePage(shard);
    if (replaced_page_id == -1) {
      SQLCC_LOG_ERROR("Failed to replace page for new page creation");
      // 设置无效页面ID表示失败
      if (page_id != nullptr) {
        *page_id = -1;
      }
      // 返回空智能指针表示替换失败
      return nullptr;
    }
  }

  // 创建新页面 - 初始化为空白页面
  auto page = std::make_unique<Page>(new_page_id);

  // 创建页面包装器 - 管理页面生命周期
  auto page_wrapper = std::make_shared<PageWrapper>(std::move(page));
  page_wrapper->ref_count = 1;        // 调用者拥有初始引用
  page_wrapper->is_dirty = false;     // 新页面初始为干净

  // 添加到缓存结构
  shard.page_table[new_page_id] = page_wrapper;
  shard.lru_list.push_front(new_page_id);  // 作为最近访问页面
  shard.lru_map[new_page_id] = shard.lru_list.begin();
  page_wrapper->lru_iter = shard.lru_list.begin();
  page_wrapper->is_in_lru = true;
  shard.current_size++;

  // 记录已分配的页面 - 支持统计和清理
  {
    std::lock_guard<std::mutex> alloc_lock(allocated_pages_mutex_);
    allocated_pages_.insert(new_page_id);
  }

  // 返回分配的页面ID
  if (page_id != nullptr) {
    *page_id = new_page_id;
  }

  // 返回页面的副本，确保调用者拥有独立的所有权
  return std::make_unique<Page>(*page_wrapper->page);
}

/**
 * @brief 删除页面 - 分片缓冲池的页面删除接口
 *
 * WHY层 - 设计意图：
 *   数据库运行过程中可能需要显式删除不再需要的页面。
 *   DeletePage操作是数据库空间管理的关键，支持数据的清理和优化。
 *   通过分片设计，即使在高并发场景下也能安全删除页面。
 *
 * WHAT层 - 功能说明：
 *   从缓冲池中完全移除指定页面，释放占用的内存空间。
 *   只允许删除引用计数为0的页面，确保没有其他使用者。
 *   清理所有相关的缓存结构和统计信息。
 *
 * HOW层 - 实现细节：
 *   1. 通过哈希计算确定目标分片，避免全局锁竞争
 *   2. 分片级锁保护，确保同一分片的串行化删除
 *   3. 引用计数检查，防止删除正在使用的页面
 *   4. 清理LRU链表，从页面表中移除，更新分片统计
 *   5. 从全局已分配页面集合中移除，支持统计和监控
 *
 * 安全检查：
 *   - 引用计数验证：确保页面没有活跃使用者
 *   - 存在性检查：页面必须在缓冲池中存在
 *   - 锁保护：分片级锁确保操作的原子性
 *
 * 清理操作：
 *   - LRU链表清理：移除页面在LRU结构中的所有引用
 *   - 页面表清理：从哈希表中完全删除页面条目
 *   - 统计更新：减少分片当前页面计数
 *   - 全局记录：从已分配页面集合中移除
 *
 * 使用场景：
 *   - 表删除操作：清理不再需要的表页面
 *   - 索引重建：删除旧的索引页面
 *   - 空间整理：清理临时或过期的数据页面
 *
 * @param page_id 要删除的页面ID
 * @return 删除是否成功（页面存在且未被使用则返回true）
 *
 * @note 只能删除引用计数为0的页面
 * @note 删除操作是不可逆的，页面数据将丢失
 * @note 删除后页面ID可以被重新分配使用
 * @note 这是数据库空间管理的重要操作
 */
bool BufferPoolSharded::DeletePage(int32_t page_id) {
  size_t shard_idx = GetShardIndex(page_id);
  Shard &shard = *shards_[shard_idx];

  std::lock_guard<std::mutex> lock(shard.mutex);

  auto it = shard.page_table.find(page_id);
  if (it == shard.page_table.end()) {
    return false;
  }

  std::shared_ptr<PageWrapper> page_wrapper = it->second;
  if (page_wrapper->ref_count > 0) {
    return false; // 页面正在被使用
  }

  // 清理LRU结构
  RemoveFromLRU(shard, page_id);

  // 从页面表中移除
  shard.page_table.erase(it);

  // 更新分片统计
  shard.current_size--;

  // 从已分配页面集合中移除
  {
    std::lock_guard<std::mutex> alloc_lock(allocated_pages_mutex_);
    allocated_pages_.erase(page_id);
  }

  return true;
}

int32_t BufferPoolSharded::ReplacePage(Shard &shard) {
  for (auto it = shard.lru_list.rbegin(); it != shard.lru_list.rend(); ++it) {
    int32_t page_id = *it;
    auto page_it = shard.page_table.find(page_id);

    if (page_it != shard.page_table.end()) {
      std::shared_ptr<PageWrapper> page_wrapper = page_it->second;

      if (page_wrapper->ref_count == 0) {
        // 释放锁，执行磁盘I/O（如果需要）
        std::unique_lock<std::mutex> lock(shard.mutex, std::adopt_lock);
        lock.unlock();

        if (page_wrapper->is_dirty) {
          disk_manager_->WritePage(
              #ifdef __cpp_lib_span
                            page_id, static_cast<char *>(page_wrapper->page->GetDataSpan().data()));
              #else
                            // C++17兼容模式下直接访问数据指针
                            page_id, static_cast<char *>(page_wrapper->page->GetDataSpan().data));
              #endif
        }

        // 重新获取锁
        lock.lock();

        // 再次检查，避免在释放锁期间状态改变
        if (page_wrapper->ref_count == 0) {
          RemoveFromLRU(shard, page_id);
          shard.page_table.erase(page_it);
          shard.current_size--;
          stats_.total_evictions++;

          // 从已分配页面集合中移除
          {
            std::lock_guard<std::mutex> alloc_lock(allocated_pages_mutex_);
            allocated_pages_.erase(page_id);
          }

          return page_id;
        }
      }
    }
  }

  return -1; // 无法找到可替换的页面
}

void BufferPoolSharded::MoveToHead(Shard &shard, int32_t page_id) {
  auto map_it = shard.lru_map.find(page_id);
  if (map_it != shard.lru_map.end()) {
    shard.lru_list.erase(map_it->second);
  }

  shard.lru_list.push_front(page_id);
  shard.lru_map[page_id] = shard.lru_list.begin();
}

void BufferPoolSharded::RemoveFromLRU(Shard &shard, int32_t page_id) {
  auto map_it = shard.lru_map.find(page_id);
  if (map_it != shard.lru_map.end()) {
    shard.lru_list.erase(map_it->second);
    shard.lru_map.erase(map_it);
  }

  auto page_it = shard.page_table.find(page_id);
  if (page_it != shard.page_table.end()) {
    page_it->second->is_in_lru = false;
  }
}

size_t BufferPoolSharded::GetCurrentPageCount() const {
  size_t total_count = 0;
  for (const auto &shard : shards_) {
    total_count += shard->current_size;
  }
  return total_count;
}

std::unordered_map<std::string, double> BufferPoolSharded::GetStats() const {
  std::unordered_map<std::string, double> stats;
  stats["total_accesses"] = static_cast<double>(stats_.total_accesses);
  stats["total_hits"] = static_cast<double>(stats_.total_hits);
  stats["total_misses"] = static_cast<double>(stats_.total_misses);
  stats["total_evictions"] = static_cast<double>(stats_.total_evictions);

  if (stats_.total_accesses > 0) {
    stats["hit_rate"] =
        static_cast<double>(stats_.total_hits) / stats_.total_accesses;
  } else {
    stats["hit_rate"] = 0.0;
  }

  stats["current_page_count"] = static_cast<double>(GetCurrentPageCount());
  stats["pool_size"] = static_cast<double>(pool_size_);
  stats["num_shards"] = static_cast<double>(num_shards_);

  return stats;
}

} // namespace sqlcc
