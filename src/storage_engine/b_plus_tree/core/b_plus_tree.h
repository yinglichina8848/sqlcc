#ifndef SQLCC_B_PLUS_TREE_H
#define SQLCC_B_PLUS_TREE_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "storage/b_plus_tree_nodes.h"

// 前向声明解决循环依赖
namespace sqlcc {
class StorageEngine;
class Page;
} // namespace sqlcc

namespace sqlcc {

/**
 * WHY: 为什么数据库索引选择B+树而不是B树或哈希表？
 *
 * 数据库索引需要支持：
 * 1. 范围查询：SELECT * FROM t WHERE id BETWEEN 1 AND 100
 * 2. 顺序访问：ORDER BY id
 * 3. 高效的插入/删除：维持平衡结构
 *
 * B+树相对于其他数据结构的优势：
 * - 叶子节点串联，支持高效范围查询和顺序遍历
 * - 所有数据都在叶子节点，内部节点只存键，减少树高度
 * - 磁盘I/O优化：节点大小适配页面大小，减少磁盘访问
 * - 高度平衡，保证查询性能稳定且可预测
 *
 * 性能特点：
 * - 查找复杂度：O(log_m N) - m为阶数，N为记录数
 * - 范围查询：O(log_m N + K) - K为结果集大小
 * - 插入/删除：O(log_m N) - 包含可能的节点分裂/合并
 *
 * WHAT: B+树索引实现类
 *
 * BPlusTreeIndex 提供完整的B+树索引功能，支持：
 * - 单键查找和范围查询
 * - 动态插入和删除操作
 * - 索引的创建和管理
 * - 并发安全的操作接口
 *
 * 核心特性：
 * - 磁盘持久化：通过StorageEngine管理页面存储
 * - 缓存优化：内部节点缓存减少磁盘访问
 * - 事务支持：支持ACID操作的原子性保证
 * - 并发控制：多线程安全访问
 *
 * HOW: B+树的核心实现机制
 *
 * 1. 树结构设计：
 *    - 根节点：树的入口点，可为叶子或内部节点
 *    - 内部节点：只存储键值和子节点指针
 *    - 叶子节点：存储完整的键值对和双向链表指针
 *    - 节点分裂：当节点溢出时自动分裂并调整父节点
 *    - 节点合并：当节点下溢时与兄弟节点合并
 *
 * 2. 磁盘存储策略：
 *    - 页面对齐：节点大小与磁盘页面大小匹配
 *    - 延迟写入：批量写入减少I/O操作
 *    - 预读优化：根据访问模式预读相邻页面
 *
 * 3. 并发控制机制：
 *    - 读写锁分离：读操作不阻塞其他读操作
 *    - 乐观并发：基于版本号的冲突检测
 *    - 死锁避免：固定的锁获取顺序
 *
 * 4. 性能优化技术：
 *    - 缓冲区管理：LRU缓存热点数据
 *    - 批量操作：减少单次操作的开销
 *    - 自适应调整：根据负载动态调整参数
 *
 * 🏗️ 设计模式：组合模式 + 模板方法模式
 *
 * 组合模式应用：
 * - 树节点作为组件，支持递归组合
 * - 统一的操作接口，屏蔽内部差异
 * - 灵活的树结构扩展能力
 *
 * 模板方法模式应用：
 * - 定义操作的通用框架
 * - 子类实现具体的节点操作
 * - 保证操作的一致性和正确性
 *
 * SOLID原则体现：
 *
 * 1. 单一职责原则(SRP)：
 *    - BPlusTreeIndex只负责索引管理
 *    - 节点操作由专门的类负责
 *    - 存储管理委托给StorageEngine
 *
 * 2. 开闭原则(OCP)：
 *    - 支持新的索引类型扩展
 *    - 通过接口隔离实现细节变化
 *    - 不修改现有代码即可扩展功能
 *
 * 3. 里氏替换原则(LSP)：
 *    - 所有节点类型都可以作为树节点使用
 *    - 保证继承关系的正确性
 *
 * 4. 接口隔离原则(ISP)：
 *    - 提供简洁的外部接口
 *    - 内部操作接口与外部分离
 *
 * 5. 依赖倒置原则(DIP)：
 *    - 依赖抽象的StorageEngine接口
 *    - 不依赖具体的存储实现
 *    - 通过依赖注入提高可测试性
 */
class BPlusTreeIndex {
public:
    // WHY: 在多线程数据库环境中，B+树作为共享数据结构，必须保证并发访问的正确性和高效性。
    // WHAT: 本`BPlusTreeIndex`类的并发控制策略是委托给底层的`StorageEngine`和更高层的`TransactionManager`来管理。
    // `BPlusTreeIndex`本身不直接持有细粒度锁，而是依赖于外部组件提供的页面级锁或事务级锁。
    // HOW:
    // 1.  **页面级并发控制**: 当`BPlusTreeIndex`通过`StorageEngine`读取或写入页面时，`StorageEngine`负责获取和释放页面锁（例如，读写锁）。
    //     这确保了对单个页面的并发访问是安全的。
    // 2.  **树结构并发控制**: 对于修改B+树结构的操作（如节点分裂、合并），为了维护树的平衡和一致性，
    //     可能需要更高级别的锁（如锁路径锁存，Latch Coupling，也称为Crabbing Lock）。
    //     然而，在本`BPlusTreeIndex`的公共接口中，这些锁的获取和释放逻辑预期由调用方（例如`TransactionManager`）
    //     或更深层次的辅助方法来管理。`BPlusTreeIndex`的私有递归操作方法会隐含地假定页面锁已被适当持有。
    // 3.  **ACID保证**: 事务的隔离性和原子性最终由`TransactionManager`通过两阶段锁（2PL）或MVCC（多版本并发控制）
    //     等机制来保证。B+树索引作为事务管理的一部分，其操作必须与这些机制协调。

    /**
     * @brief 构造函数
     * @param storage_engine 存储引擎智能指针
     * @param table_name 表名
     * @param column_name 列名
     */
    BPlusTreeIndex(std::shared_ptr<StorageEngine> storage_engine, const std::string& table_name, const std::string& column_name);

    /**
     * @brief 析构函数
     */
    ~BPlusTreeIndex();

    /**
     * @brief 创建索引
     * @return 是否创建成功
     */
    bool Create();

    /**
     * @brief 删除索引
     * @return 是否删除成功
     */
    bool Drop();

    /**
     * WHAT: Insert - B+树插入操作
     *
     * 插入键值对到B+树索引中，维护树的平衡性。
     * 如果节点溢出，会触发分裂操作。
     *
     * HOW: 递归插入算法
     * 1. 从根节点开始，递归查找插入位置
     * 2. 在叶子节点找到插入点，插入键值对
     * 3. 如果叶子节点溢出（超过最大键数），执行分裂：
     *    - 创建新叶子节点
     *    - 将键值对平均分配到两个节点
     *    - 在父节点插入中间键和新节点指针
     * 4. 如果父节点也溢出，递归向上分裂
     * 5. 如果根节点分裂，树高度增加
     *
     * @param key 键
     * @param page_id 页面ID
     * @param offset 偏移量
     * @return 是否插入成功
     */
    bool Insert(const std::string& key, int32_t page_id, size_t offset);

    /**
     * WHAT: Delete - B+树删除操作
     *
     * 从B+树索引中删除指定的键，并维护树的平衡性。
     * 如果节点下溢，会触发合并或重新分配操作。
     *
     * HOW: 递归删除算法
     * 1. 查找目标键所在的叶子节点。
     * 2. 从叶子节点中删除指定的键值对。
     * 3. 如果叶子节点在删除后发生下溢（即键的数量低于最小阈值）：
     *    a. 尝试从左兄弟节点或右兄弟节点重新分配键。
     *    b. 如果无法重新分配（兄弟节点也处于最小键数），则将该叶子节点与一个兄弟节点合并。
     * 4. 如果合并操作导致父节点中的键或指针数量发生变化（例如，某个子节点被移除），
     *    则需要递归地检查父节点是否发生下溢，并重复步骤3。
     * 5. 如果根节点因合并而只剩下一个子节点，则将根节点设置为其唯一子节点，树的高度降低。
     *
     * @param key 键
     * @return 是否删除成功
     */
    bool Delete(const std::string& key);

    /**
     * @brief 查找键
     * @param key 键
     * @param page_id 输出参数：页面ID
     * @param offset 输出参数：偏移量
     * @return 是否找到
     */
    bool Lookup(const std::string& key, int32_t& page_id, size_t& offset) const;

    /**
     * @brief 范围查找
     * @param start_key 起始键
     * @param end_key 结束键
     * @return 查找结果列表
     */
    std::vector<std::pair<int32_t, size_t>> RangeLookup(const std::string& start_key, const std::string& end_key) const;

    /**
     * WHAT: Search - 多路查找和叶子节点搜索
     *
     * 从根节点开始，沿着树结构向下查找指定的键。
     * 在叶子节点中找到匹配的键值对。
     *
     * HOW: 多路查找算法
     * 1. 从根节点开始
     * 2. 在当前节点中进行二分查找，确定子节点索引
     * 3. 递归进入相应的子节点
     * 4. 重复步骤2-3直到到达叶子节点
     * 5. 在叶子节点中顺序查找匹配的键
     * 6. 返回所有匹配的索引条目
     *
     * @param key 要搜索的键
     * @return 匹配的索引条目列表
     */
    std::vector<IndexEntry> Search(const std::string& key) const;

    /**
     * WHAT: SearchRange - 范围查询和叶子链表遍历
     *
     * 执行范围查询，从lower_bound到upper_bound之间的所有键。
     * 利用B+树叶子节点链表的特性，实现高效的范围遍历。
     *
     * HOW: 范围查询算法
     * 1. 找到范围起始位置：Search(lower_bound)
     * 2. 从起始叶子节点开始，沿叶子链表向右遍历
     * 3. 收集所有在[lower_bound, upper_bound]范围内的键值对
     * 4. 当遇到超出upper_bound的键或到达链表末尾时停止
     * 5. 返回所有匹配的索引条目
     *
     * 性能优势：
     * - 一次查找定位起始点：O(log N)
     * - 顺序遍历匹配范围：O(K)
     * - 总复杂度：O(log N + K)
     *
     * @param lower_bound 范围下界
     * @param upper_bound 范围上界
     * @return 匹配的索引条目列表
     */
    std::vector<IndexEntry> SearchRange(const std::string& lower_bound, const std::string& upper_bound) const;

    /**
     * @brief 获取表名
     * @return 表名
     */
    const std::string& GetTableName() const { return table_name_; }

    /**
     * @brief 获取列名
     * @return 列名
     */
    const std::string& GetColumnName() const { return column_name_; }

    /**
     * @brief 检查索引是否存在
     * @return 索引是否存在
     */
    bool Exists() const;

private:
    std::shared_ptr<StorageEngine> storage_engine_;  ///< 存储引擎智能指针
    std::string table_name_;         ///< 表名
    std::string column_name_;        ///< 列名
    int32_t root_page_id_;           ///< 根节点页面ID
    mutable std::shared_ptr<Page> root_page_; ///< 根节点页面（缓存）

    // 内部辅助方法
    /**
     * WHAT: LoadNode - 从磁盘加载B+树节点
     *
     * WHY: B+树节点通常存储在磁盘页面中。为了操作这些节点（例如，在插入/删除/查找过程中），
     * 它们必须从`StorageEngine`中加载到内存。此方法封装了从给定`page_id`获取页面，
     * 并将页面数据解释为正确的B+树节点类型（内部节点或叶子节点）的逻辑。
     * HOW:
     * 1.  **获取页面**: 调用`storage_engine_->GetPage(page_id)`从存储引擎获取指定`page_id`的页面。
     * 2.  **验证页面**: 检查获取到的页面是否有效。
     * 3.  **判断节点类型**: 根据页面头部（例如，通过`Page::GetPageType()`或页面中存储的元数据）
     *     判断该页面是B+树的内部节点还是叶子节点。
     * 4.  **构建节点对象**: 根据节点类型，使用页面数据构造并返回一个`std::unique_ptr<BPlusTreeNode>`
     *     指向正确的具体子类实例（`InternalNode`或`LeafNode`）。
     *
     * @param page_id 待加载节点的页面ID。
     * @return 包含B+树节点的`std::unique_ptr`。
     */
    std::unique_ptr<BPlusTreeNode> LoadNode(int32_t page_id);
    /**
     * WHAT: SaveNode - 将B+树节点写回磁盘
     *
     * WHY: 当B+树节点在内存中被修改后（例如，插入新键、删除旧键、节点分裂或合并），
     * 必须将这些更改持久化到磁盘，以确保数据的完整性和持久性。
     * WHAT: 此方法负责将一个已修改的B+树节点所关联的内存页面标记为脏（dirty），
     * 并指示`StorageEngine`将其内容写回磁盘。
     * HOW:
     * 1.  **标记脏页**: 调用`page->SetDirty(true)`将页面标记为已修改。
     * 2.  **写回磁盘**: `StorageEngine`的缓冲区管理器（Buffer Pool Manager）会周期性地
     *     或在特定事件（例如，LRU替换策略驱逐页面、检查点）时，将所有脏页写回磁盘。
     *     此方法本身通常不直接执行物理I/O，而是委托给`StorageEngine`的策略。
     *
     * @param page 待保存节点的内存页面。
     */
    void SaveNode(std::shared_ptr<Page> page) const;
    /**
     * WHAT: GetNode - 获取B+树节点（可能从缓存）
     *
     * WHY: 为了优化性能，B+树的节点（页面）通常会驻留在内存缓冲区（Buffer Pool）中。
     * 此方法提供了一个统一的接口来获取指定`page_id`的节点，它可能首先尝试从缓存中获取，
     * 如果缓存中没有，则通过`LoadNode`从磁盘加载。
     * WHAT: 此方法负责从内存或磁盘中获取指定`page_id`的B+树节点，并返回其智能指针。
     * HOW:
     * 1.  **缓存查找**: 首先在本地缓存（如果`BPlusTreeIndex`内部有）或`StorageEngine`的Buffer Pool中查找该`page_id`对应的页面。
     * 2.  **加载节点**: 如果节点不在缓存中，则调用`LoadNode(page_id)`从磁盘加载。
     * 3.  **错误处理**: 如果无法加载节点（例如，`page_id`无效），则抛出异常或返回`nullptr`。
     *
     * @param page_id 待获取节点的页面ID。
     * @return 包含B+树节点的`std::unique_ptr`。
     */
    std::unique_ptr<BPlusTreeNode> GetNode(int32_t page_id) const;
    /**
     * WHAT: CreateNewNode - 创建新的B+树节点
     *
     * WHY: 在B+树的动态操作（如插入导致节点分裂，或树的高度增加）中，需要分配新的节点。
     * 此方法封装了从`StorageEngine`获取一个新的空闲页面，并将其初始化为B+树内部节点或叶子节点的逻辑。
     * WHAT: 此方法负责分配一个新的物理页面，并根据`is_leaf`参数将其初始化为`InternalNode`或`LeafNode`。
     * HOW:
     * 1.  **分配新页面**: 调用`storage_engine_->NewPage()`从存储引擎获取一个可用的新页面ID。
     * 2.  **初始化节点**: 根据`is_leaf`的值，构造一个新的`InternalNode`或`LeafNode`对象。
     *     这些新节点将使用刚分配的`page_id`，并设置其初始元数据（例如，父节点ID，是否为根节点等）。
     * 3.  **返回节点**: 返回一个`std::unique_ptr<BPlusTreeNode>`指向新创建的节点。
     *
     * @param is_leaf `true`表示创建叶子节点，`false`表示创建内部节点。
     * @return 包含新创建B+树节点的`std::unique_ptr`。
     */
    std::unique_ptr<BPlusTreeNode> CreateNewNode(bool is_leaf);
    /**
     * WHAT: DeleteNode - 从磁盘删除B+树节点
     *
     * WHY: 当B+树节点因合并（merge）操作而不再需要时，其占用的物理页面应该被回收。
     * 释放这些页面是有效的存储空间管理的关键，可以防止磁盘空间泄露。
     * WHAT: 此方法负责通知`StorageEngine`释放指定`page_id`的物理页面。
     * HOW: 调用`storage_engine_->DeletePage(page_id)`来将该页面标记为可用，
     * 从而允许`StorageEngine`在未来将此页面重新分配给新的节点或数据。
     *
     * @param page_id 待删除节点的页面ID。
     */
    void DeleteNode(int32_t page_id);
    /**
     * WHAT: NeedMerge - 检查B+树节点是否需要合并或重新分配
     *
     * WHY: 在B+树的删除操作中，如果一个节点的键数量过少（低于其最小填充因子），
     * 就会发生“下溢”（underflow）。下溢会导致树的存储效率下降和高度增加的风险。
     * 此函数用于判断一个节点是否处于这种下溢状态，从而触发后续的合并或重新分配操作，
     * 以维护树的平衡性和性能。
     * WHAT: 此方法负责检查给定的B+树节点的键数量是否低于最小阈值。
     * HOW: 比较节点的当前键数量与预定义的最小键数量（例如，`node->GetMinKeys()`）。
     *      对于内部节点和叶子节点，最小键数量的计算规则可能略有不同（例如，通常是阶数m的一半）。
     *
     * @param node 待检查的B+树节点。
     * @return 如果节点下溢则返回`true`，否则返回`false`。
     */
    bool NeedMerge(const std::unique_ptr<BPlusTreeNode>& node);
    /**
     * WHAT: LoadMetadata - 加载B+树索引元数据
     *
     * WHY: B+树索引是持久化的数据结构。当数据库启动或索引被打开时，需要从持久化存储中
     * 加载其关键元数据（例如，根节点的`page_id`、它索引的表名和列名），以便`BPlusTreeIndex`
     * 对象能够正确地初始化和操作。
     * WHAT: 此方法负责从磁盘上的某个位置（例如，系统目录表或一个专用的索引元数据页面）
     * 读取B+树索引的`root_page_id_`、`table_name_`和`column_name_`。
     * HOW:
     * 1.  **确定元数据位置**: 元数据可能存储在`SystemDatabase`的`sys_indexes`表中，
     *     或者在一个由`storage_engine_`管理的特殊页面中。
     * 2.  **读取元数据**: 通过调用`StorageEngine`的接口（或`SystemDatabase`的接口）
     *     来读取这些元数据字段的值。
     * 3.  **初始化成员变量**: 将读取到的值赋给`root_page_id_`、`table_name_`和`column_name_`成员变量。
     *     如果这是第一次创建索引，元数据可能不存在，此时需要初始化为默认值。
     */
    void LoadMetadata();
    /**
     * WHAT: SaveMetadata - 保存B+树索引元数据
     *
     * WHY: B+树索引的关键元数据（例如，根节点的`page_id`，因为树结构变化根节点可能会变）
     * 必须持久化到磁盘，以确保索引在数据库关闭和重启后能够被正确加载和使用。
     * 如果不保存这些元数据，索引的状态将丢失。
     * WHAT: 此方法负责将`BPlusTreeIndex`对象的`root_page_id_`、`table_name_`和`column_name_`等元数据
     * 写入磁盘上的指定位置（例如，系统目录表或一个专用的索引元数据页面）。
     * HOW:
     * 1.  **确定元数据位置**: 与`LoadMetadata`对应，元数据将写入`SystemDatabase`的`sys_indexes`表，
     *     或者由`storage_engine_`管理的特定页面。
     * 2.  **写入元数据**: 通过调用`StorageEngine`的接口（或`SystemDatabase`的接口）
     *     来更新或插入这些元数据字段的值。
     * 3.  **持久化**: 确保这些元数据写入是持久的（例如，通过日志和刷盘机制）。
     */
    void SaveMetadata();

public:  // 添加公共接口用于查询优化
    
    /**
     * WHAT: Insert (Recursive) - B+树递归插入操作
     *
     * WHY: B+树的插入操作通常通过递归方式实现，从根节点向下查找正确的叶子节点进行插入，
     * 并处理可能发生的节点分裂，将分裂信息逐级向上汇报。这个私有方法是公共`Insert`方法
     * 的递归实现核心，它处理了单个节点层面的插入逻辑。
     * WHAT: 此方法负责在给定的`node`中插入`key`和其对应的`page_id/offset`值。
     * 它会确定正确的子节点（如果`node`是内部节点），并递归调用自身。
     * 如果插入导致`node`溢出，它会触发节点分裂，并向上级调用返回分裂信息。
     * HOW:
     * 1.  **查找插入位置**: 在当前`node`中查找`key`的正确插入位置。
     * 2.  **递归向下**: 如果`node`是内部节点，递归调用`Insert`方法处理子节点。
     *     如果`node`是叶子节点，则直接插入键值对。
     * 3.  **处理分裂**: 如果递归调用返回了分裂信息（即子节点发生了分裂），`node`需要处理
     *     中间键的插入和新子节点指针的添加。如果`node`因此也发生分裂，则向上返回分裂信息。
     * 4.  **根节点分裂**: 如果根节点发生分裂，树的高度会增加，`root_page_id_`需要更新。
     *
     * @param key 待插入的键。
     * @param page_id 键对应的数据页面ID。
     * @param offset 键对应的数据在页面中的偏移量。
     * @param node 当前操作的B+树节点。
     * @param recursion_depth 当前递归的深度，用于调试或特定优化。
     * @return 插入操作是否成功。
     */
    bool Insert(const std::string& key, int32_t page_id, size_t offset, std::unique_ptr<BPlusTreeNode>& node, int recursion_depth = 0);
    /**
     * WHAT: Delete (Recursive) - B+树递归删除操作
     *
     * WHY: B+树的删除操作通常通过递归方式实现，从根节点向下查找正确的叶子节点进行删除，
     * 并处理可能发生的节点下溢和合并，将下溢信息逐级向上汇报。这个私有方法是公共`Delete`方法
     * 的递归实现核心，它处理了单个节点层面的删除逻辑。
     * WHAT: 此方法负责在给定的`node`中删除`key`。
     * 它会确定正确的子节点（如果`node`是内部节点），并递归调用自身。
     * 如果删除导致`node`下溢，它会触发节点重新分配或合并，并向上级调用返回下溢信息。
     * HOW:
     * 1.  **查找删除位置**: 在当前`node`中查找`key`的正确删除位置。
     * 2.  **递归向下**: 如果`node`是内部节点，递归调用`Delete`方法处理子节点。
     *     如果`node`是叶子节点，则直接删除键值对。
     * 3.  **处理下溢**: 如果递归调用返回了下溢信息（即子节点发生了下溢），`node`需要处理
     *     对子节点的重新分配或合并操作，并更新其键和子节点指针。如果`node`因此也发生下溢，
     *     则向上返回下溢信息。
     * 4.  **根节点下溢**: 如果根节点因合并而只剩下一个子节点，则将根节点设置为其唯一子节点，
     *     树的高度降低，`root_page_id_`需要更新。
     *
     * @param key 待删除的键。
     * @param node 当前操作的B+树节点。
     * @return 删除操作是否成功。
     */
    bool Delete(const std::string& key, std::unique_ptr<BPlusTreeNode>& node);
    /**
     * WHAT: Lookup (Recursive) - B+树递归查找操作
     *
     * WHY: B+树的核心功能之一是高效的单键查找。此递归方法负责在给定节点及其子树中查找指定的键，
     * 并返回键值对所在的页面ID和偏移量。它支持从根节点到叶子节点的快速导航。
     * WHAT: 此方法负责在给定的`node`中查找`key`。
     * 它会确定正确的子节点（如果`node`是内部节点），并递归调用自身，直到到达包含`key`的叶子节点。
     * HOW:
     * 1.  **在节点中查找**: 在当前`node`中执行二分查找，确定`key`可能存在的子节点范围（对于内部节点）
     *     或`key`在当前节点中的位置（对于叶子节点）。
     * 2.  **递归向下**: 如果`node`是内部节点，递归调用`Lookup`方法处理正确的子节点。
     * 3.  **返回结果**: 如果`node`是叶子节点，且找到匹配的`key`，则设置`page_id`和`offset`输出参数，并返回`true`。
     *     如果未找到，则返回`false`。
     *
     * @param key 待查找的键。
     * @param page_id 输出参数：如果找到，返回键值对所在的页面ID。
     * @param offset 输出参数：如果找到，返回键值对在页面中的偏移量。
     * @param node 当前操作的B+树节点。
     * @return 如果找到键则返回`true`，否则返回`false`。
     */
    bool Lookup(const std::string& key, int32_t& page_id, size_t& offset, std::unique_ptr<BPlusTreeNode>& node) const;
    /**
     * WHAT: Search (Recursive) - B+树递归搜索操作
     *
     * WHY: 在某些场景下，我们可能需要查找所有与给定键匹配的索引条目。
     * 虽然B+树内部节点通常只存储唯一键用于导航，但叶子节点可能指向多个数据记录（例如，如果索引允许重复键，
     * 或者键实际上是组合键的一部分，但搜索只提供了部分键）。此方法用于收集所有匹配的`IndexEntry`。
     * WHAT: 此方法负责在给定的`node`中递归搜索`key`，并返回一个包含所有匹配`IndexEntry`的向量。
     * HOW:
     * 1.  **在节点中搜索**: 在当前`node`中查找`key`。
     * 2.  **递归向下**: 如果`node`是内部节点，递归调用`Search`方法处理正确的子节点。
     * 3.  **收集结果**: 如果`node`是叶子节点，则收集所有与`key`匹配的`IndexEntry`。
     *
     * @param key 待搜索的键。
     * @param node 当前操作的B+树节点。
     * @return 包含所有匹配`IndexEntry`的`std::vector`。
     */
    std::vector<IndexEntry> Search(const std::string& key, std::unique_ptr<BPlusTreeNode>& node) const;
    /**
     * WHAT: SearchRange (Recursive) - B+树递归范围搜索操作
     *
     * WHY: 范围查询是数据库索引最常用的功能之一。B+树的叶子节点通过双向链表连接，
     * 使得一旦找到范围的起始点，就可以非常高效地遍历所有位于指定范围内的键。
     * 此递归方法用于从根节点开始，找到范围的起始叶子节点，然后沿叶子节点链表收集所有匹配的条目。
     * WHAT: 此方法负责在给定的`node`中递归搜索`lower_bound`和`upper_bound`之间的所有`IndexEntry`。
     * HOW:
     * 1.  **查找起始位置**: 在当前`node`中查找`lower_bound`的起始位置。
     * 2.  **递归向下**: 如果`node`是内部节点，递归调用`SearchRange`方法处理正确的子节点，直到到达叶子节点。
     * 3.  **遍历叶子链表**: 从找到的叶子节点开始，沿叶子节点的双向链表向右遍历。
     * 4.  **收集结果**: 收集所有键值在`[lower_bound, upper_bound]`范围内的`IndexEntry`。
     * 5.  **停止条件**: 当遇到超出`upper_bound`的键或到达叶子链表末尾时停止遍历。
     *
     * @param lower_bound 范围的下界键。
     * @param upper_bound 范围的上界键。
     * @param node 当前操作的B+树节点。
     * @return 包含所有匹配`IndexEntry`的`std::vector`。
     */
    std::vector<IndexEntry> SearchRange(const std::string& lower_bound, const std::string& upper_bound, std::unique_ptr<BPlusTreeNode>& node) const;
    /**
     * WHAT: RangeLookup (Recursive) - B+树递归范围查找数据记录指针
     *
     * WHY: 与`SearchRange`类似，此方法也执行范围查询。但是，`RangeLookup`通常用于返回
     * 实际数据记录的物理位置（`page_id`和`offset`），而不是`IndexEntry`对象（它可能包含键）。
     * 这在需要直接访问数据页以检索完整记录的场景中非常有用。
     * WHAT: 此方法负责在给定的`node`中递归搜索`start_key`和`end_key`之间的所有数据记录指针。
     * HOW:
     * 1.  **查找起始位置**: 在当前`node`中查找`start_key`的起始位置。
     * 2.  **递归向下**: 如果`node`是内部节点，递归调用`RangeLookup`方法处理正确的子节点，直到到达叶子节点。
     * 3.  **遍历叶子链表**: 从找到的叶子节点开始，沿叶子节点的双向链表向右遍历。
     * 4.  **收集结果**: 收集所有键值在`[start_key, end_key]`范围内的`std::pair<int32_t, size_t>`。
     * 5.  **停止条件**: 当遇到超出`end_key`的键或到达叶子链表末尾时停止遍历。
     *
     * @param start_key 范围的起始键。
     * @param end_key 范围的结束键。
     * @param node 当前操作的B+树节点。
     * @return 包含所有匹配数据记录指针的`std::vector<std::pair<int32_t, size_t>>`。
     */
    std::vector<std::pair<int32_t, size_t>> RangeLookup(const std::string& start_key, const std::string& end_key, std::unique_ptr<BPlusTreeNode>& node) const;

    // 节点操作方法
    bool IsLeafNode(std::unique_ptr<BPlusTreeNode>& node) const;
    std::vector<std::string> GetKeys(std::unique_ptr<BPlusTreeNode>& node) const;
    std::vector<std::pair<int32_t, size_t>> GetValues(std::unique_ptr<BPlusTreeNode>& node) const;
    std::vector<int32_t> GetChildren(std::unique_ptr<BPlusTreeNode>& node) const;
};

} // namespace sqlcc

#endif // SQLCC_B_PLUS_TREE_H
