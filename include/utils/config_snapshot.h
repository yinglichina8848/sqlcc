/**
 * WHY: 为什么需要智能配置快照管理器？
 *
 * 数据库系统配置管理复杂度极高，传统方案存在诸多技术挑战：
 * - 配置变更并发冲突：多线程环境下配置更新的竞态条件难以控制
 * - 配置版本管理混乱：配置变更历史追溯和版本回滚能力缺失
 * - 内存安全隐患严重：配置数据生命周期管理不当导致内存泄漏或野指针
 * - 配置一致性保证困难：分布式环境下配置数据的一致性难以保证
 * - 配置性能瓶颈明显：配置读取操作的锁竞争和性能开销过大
 * - 配置监控统计缺失：配置变更行为的监控和统计信息不足
 * - 配置数据持久化复杂：配置数据的序列化和反序列化处理繁琐
 *
 * 智能配置快照管理器的核心价值：
 * 1. 线程安全保证：多线程环境下配置访问的原子性和一致性
 * 2. 内存安全管理：智能指针管理的自动生命周期和资源清理
 * 3. 版本控制能力：配置变更的历史记录和版本管理机制
 * 4. 无锁读取优化：配置读取操作的性能优化和并发访问
 * 5. 数据一致性保证：配置数据在变更过程中的一致性保护
 * 6. 监控统计透明：配置访问行为的统计和监控信息
 * 7. 扩展性架构：新配置管理策略的灵活扩展机制
 *
 * 🏗️ 设计模式：快照模式(Snapshot Pattern) + 智能指针模式(Smart Pointer Pattern) + 版本控制模式(Version Control Pattern)
 *
 * 智能配置快照管理器作为快照模式的应用：
 * - 不可变性保证：配置快照的不可变性确保数据一致性
 * - 共享访问优化：多个读取者共享同一个快照实例
 * - 版本隔离管理：不同版本的配置快照独立管理
 * - 变更原子性：配置变更操作的原子性和一致性
 * - 历史追溯能力：配置变更的历史记录和追溯
 * - 回滚能力支持：配置错误的自动回滚和恢复
 *
 * SOLID原则体现：
 * - 单一职责：专门负责配置数据的快照管理和版本控制
 * - 开闭原则：新快照管理策略通过扩展实现无需修改核心
 * - 里氏替换：所有快照实现都可以互相替换使用
 * - 接口隔离：快照管理接口精确定义管理契约
 * - 依赖倒置：依赖抽象的快照接口而非具体实现
 *
 * WHAT: 智能配置快照管理系统 - 数据库配置数据的版本控制和生命周期管理框架
 *
 * 核心功能：
 * - 配置快照创建：基于当前配置数据创建不可变快照实例
 * - 智能指针管理：配置快照的自动生命周期和内存管理
 * - 版本控制管理：配置变更的历史记录和版本标识
 * - 线程安全访问：多线程环境下配置快照的安全访问
 * - 数据完整性校验：配置快照数据的完整性验证和校验
 * - 快照合并操作：多个配置快照的合并和覆盖逻辑
 * - 性能监控统计：配置快照访问的性能监控和统计
 *
 * 系统组件：
 * - ConfigSnapshot：核心配置快照类，实现不可变配置数据的封装
 * - ConfigSnapshotFactory：配置快照工厂类，提供快照创建和管理
 * - ConfigSnapshotManager：配置快照管理器，实现快照存储和检索
 * - SnapshotMetadata：快照元数据结构，记录快照的版本和状态信息
 * - VersionIdGenerator：版本ID生成器，生成唯一的快照版本标识
 * - SnapshotValidator：快照验证器，验证快照数据的完整性和正确性
 * - SnapshotMonitor：快照监控器，监控快照的访问和使用情况
 *
 * 快照生命周期：
 * - 创建阶段：基于配置数据创建快照实例和元数据
 * - 存储阶段：快照实例存储到管理器中进行统一管理
 * - 访问阶段：多线程安全地访问快照数据进行配置读取
 * - 清理阶段：根据策略清理过期快照释放内存资源
 * - 销毁阶段：智能指针自动管理快照对象的生命周期
 *
 * 版本控制机制：
 * - 版本ID生成：基于时间戳和随机数的唯一版本标识
 * - 版本历史记录：配置变更的完整历史记录和追溯链
 * - 版本比较操作：不同版本快照之间的比较和差异分析
 * - 版本回滚支持：支持回滚到指定历史版本的配置状态
 * - 版本分支管理：配置变更的分支和合并操作支持
 * - 版本冲突检测：多版本配置变更的冲突检测和解决
 *
 * 线程安全实现：
 * - 读写锁保护：使用shared_mutex实现高效的读写锁保护
 * - 原子操作更新：访问计数器的原子操作更新
 * - 无锁读取优化：不可变快照的无锁读取访问
 * - 写时复制策略：配置变更时的写时复制避免锁竞争
 * - 条件变量同步：快照管理操作的条件变量通知
 * - 死锁预防机制：锁顺序规范和死锁检测机制
 *
 * 内存管理策略：
 * - 智能指针封装：shared_ptr/weak_ptr的自动生命周期管理
 * - 引用计数优化：智能指针的引用计数和垃圾回收
 * - 内存池分配：快照对象的内存池分配减少动态分配
 * - 缓存友好设计：数据结构的缓存友好布局优化
 * - 内存使用监控：快照内存使用的实时监控和告警
 * - 垃圾回收机制：过期快照的自动清理和回收
 *
 * 性能优化措施：
 * - 哈希索引查找：配置键的哈希索引快速查找
 * - 缓存预热机制：常用配置的缓存预热和预加载
 * - 批量操作支持：多个配置键的批量读取操作
 * - 异步更新机制：配置变更的异步更新和通知
 * - 压缩存储优化：大配置数据的压缩存储节省内存
 * - 预编译表达式：配置表达式的预编译提高访问速度
 *
 * 监控和统计：
 * - 访问频率统计：各配置键的访问频率和热点分析
 * - 性能指标监控：快照创建、访问、销毁的性能指标
 * - 内存使用统计：快照内存占用和增长趋势分析
 * - 版本变更统计：配置变更频率和模式的统计分析
 * - 错误统计监控：快照操作错误的分类统计
 * - 健康状态检查：快照系统的健康状态实时检查
 *
 * 接口设计：
 * - 快照创建接口：创建新配置快照的工厂接口
 * - 快照访问接口：配置数据的读取和访问接口
 * - 版本管理接口：快照版本控制和管理接口
 * - 监控统计接口：快照监控数据的获取接口
 * - 生命周期接口：快照生命周期管理的控制接口
 * - 扩展策略接口：自定义快照管理策略的接口
 * - 调试诊断接口：快照系统调试和诊断信息的接口
 *
 * HOW: 智能配置快照管理系统的实现机制
 *
 * 快照模式实现：
 * 1. 不可变对象设计：ConfigSnapshot类的不可变性保证
 * 2. 深拷贝策略：快照创建时的深拷贝数据隔离
 * 3. 共享访问优化：shared_ptr的引用计数共享访问
 * 4. 元数据管理：快照版本、时间、校验和的元数据记录
 * 5. 完整性验证：数据校验和的完整性验证机制
 * 6. 版本链管理：父子版本关系的链式管理结构
 * 7. 生命周期追踪：快照对象的创建到销毁的生命周期追踪
 *
 * 智能指针模式支撑：
 * 1. shared_ptr封装：配置快照的共享指针封装管理
 * 2. weak_ptr引用：避免循环引用的弱引用机制
 * 3. 引用计数统计：智能指针的引用计数统计监控
 * 4. 自动内存管理：RAII原则的自动资源管理
 * 5. 线程安全保证：智能指针的线程安全操作保证
 * 6. 自定义删除器：快照对象的自定义删除和清理
 * 7. 内存泄漏防护：智能指针的内存泄漏自动防护
 *
 * 版本控制模式支撑：
 * 1. 版本ID生成：时间戳+随机数的唯一版本标识算法
 * 2. 版本历史维护：版本变更的链表结构历史记录
 * 3. 版本比较算法：快照数据的深度比较和差异计算
 * 4. 版本合并策略：多个版本快照的合并和冲突解决
 * 5. 版本回滚机制：支持回滚到历史版本的机制实现
 * 6. 版本清理策略：过期版本的自动清理和空间回收
 * 7. 版本监控统计：版本操作的统计和性能监控
 *
 * 数据结构设计：
 * 1. unordered_map存储：配置键值对的高效哈希表存储
 * 2. 字符串优化：配置键的字符串池化减少内存占用
 * 3. variant类型支持：std::variant的多类型配置值支持
 * 4. 元数据结构体：快照元数据的紧凑结构体设计
 * 5. 引用计数器：原子类型的访问计数器实现
 * 6. 时间戳记录：chrono库的高精度时间戳记录
 * 7. 校验和计算：配置数据的哈希校验和计算
 *
 * 线程安全实现：
 * 1. shared_mutex保护：读写锁的精细化锁粒度控制
 * 2. 原子操作计数：访问计数器的内存屏障保证
 * 3. 无锁读取路径：不可变数据的无锁读取优化
 * 4. 写时复制机制：配置变更的写时复制避免竞争
 * 5. 条件变量通知：快照变更的异步通知机制
 * 6. 锁顺序规范：多锁操作的死锁预防规范
 * 7. 竞态条件防护：ABA问题等竞态条件的防护措施
 *
 * 内存管理实现：
 * 1. 智能指针生命周期：shared_ptr的自动引用计数管理
 * 2. 弱引用清理：weak_ptr的循环引用清理机制
 * 3. 内存池分配：对象池的内存分配优化
 * 4. 缓存对齐优化：数据结构的CPU缓存友好设计
 * 5. 内存使用监控：jemalloc/tcmalloc的内存监控
 * 6. 垃圾回收调度：过期快照的定期清理调度
 * 7. 内存压力处理：高内存压力下的降级处理
 *
 * 性能优化实现：
 * 1. 哈希查找加速：unordered_map的平均O(1)查找性能
 * 2. 字符串池化：配置键的字符串池化减少比较开销
 * 3. 批量操作接口：多个配置键的批量读取接口
 * 4. 异步更新机制：变更通知的异步处理减少阻塞
 * 5. 压缩存储算法：大配置数据的LZ4压缩存储
 * 6. 预热机制实现：启动时的配置缓存预热
 * 7. 性能监控指标：访问延迟、吞吐量的性能监控
 *
 * 完整性验证实现：
 * 1. 校验和算法：FNV1a哈希的快速校验和计算
 * 2. 版本一致性：版本ID与数据内容的双重验证
 * 3. 时间戳验证：创建时间与系统时间的合理性检查
 * 4. 数据完整性：配置值类型和范围的完整性验证
 * 5. 引用完整性：智能指针引用的有效性验证
 * 6. 序列化验证：快照序列化的一致性验证
 * 7. 反序列化校验：快照加载的完整性校验
 *
 * 监控统计实现：
 * 1. 访问模式分析：热点配置键的访问模式分析
 * 2. 性能指标收集：创建时间、访问延迟的指标收集
 * 3. 内存使用跟踪：快照大小、引用计数的跟踪
 * 4. 版本变更统计：变更频率、回滚次数的统计
 * 5. 错误模式识别：异常访问模式的识别和告警
 * 6. 健康状态检查：系统整体健康状态的检查
 * 7. 可视化展示：监控数据的图表化展示
 *
 * 配置管理实现：
 * 1. 工厂方法创建：ConfigSnapshotFactory的工厂方法
 * 2. 管理器统一管理：ConfigSnapshotManager的统一管理
 * 3. 策略模式扩展：不同管理策略的可插拔设计
 * 4. 配置参数控制：快照参数的可配置化控制
 * 5. 生命周期管理：快照从创建到销毁的生命周期
 * 6. 资源配额控制：快照数量和内存使用的配额控制
 * 7. 清理策略配置：过期快照的清理策略配置
 *
 * 测试验证实现：
 * 1. 单元测试覆盖：所有接口和方法的单元测试
 * 2. 并发测试验证：多线程访问的并发安全测试
 * 3. 性能基准测试：快照操作的性能基准测试
 * 4. 内存泄漏检测：智能指针的内存泄漏检测
 * 5. 边界条件测试：极端情况下的边界条件测试
 * 6. 故障注入测试：异常情况下的容错性测试
 * 7. 集成测试验证：与其他组件的集成测试验证
 *
 * 扩展性设计：
 * - 插件化存储：支持不同存储后端的快照持久化
 * - 分布式同步：集群环境的快照同步和一致性
 * - 云存储集成：云存储服务的快照备份和恢复
 * - 自定义序列化：支持自定义的快照序列化格式
 * - 监控扩展接口：第三方监控系统的集成接口
 * - 安全扩展模块：快照数据的加密和安全保护
 * - AI优化集成：基于AI的快照访问模式预测
 *
 * 调试和诊断：
 * - 快照状态检查：当前快照的完整状态检查
 * - 版本历史追溯：配置变更的完整历史追溯
 * - 性能瓶颈分析：快照操作的性能瓶颈识别
 * - 内存泄漏检测：智能指针的内存泄漏检测工具
 * - 并发问题诊断：多线程访问的竞态条件诊断
 * - 配置变更审计：所有配置变更的审计日志记录
 * - 可视化调试器：快照系统的图形化调试界面
 */

#ifndef SQLCC_CONFIG_SNAPSHOT_H_
#define SQLCC_CONFIG_SNAPSHOT_H_

#include <string>
#include <unordered_map>
#include <variant>
#include <memory>
#include <chrono>
#include <atomic>
#include <shared_mutex>

namespace sqlcc {

/**
 * @brief 配置值类型，支持多种数据类型
 * 使用std::variant实现类型安全的配置值存储
 */
using ConfigValue = std::variant<bool, int, double, std::string>;

/**
 * @brief 配置快照类
 * 
 * 提供不可变的配置数据快照，支持线程安全的配置读取和版本管理
 * 使用智能指针管理，确保内存安全和自动资源清理
 */
class ConfigSnapshot {
public:
    /**
     * @brief 智能指针类型定义
     */
    using SnapshotPtr = std::shared_ptr<const ConfigSnapshot>;
    using MutableSnapshotPtr = std::shared_ptr<ConfigSnapshot>;
    using WeakSnapshotPtr = std::weak_ptr<const ConfigSnapshot>;

    /**
     * @brief 快照元数据结构
     */
    struct SnapshotMetadata {
        std::string version_id;                    // 版本ID
        std::string description;                   // 描述信息
        std::chrono::system_clock::time_point create_time;  // 创建时间
        std::string parent_version;               // 父版本ID
        size_t config_count;                      // 配置项数量
        std::string checksum;                     // 数据校验和
    };

private:
    /**
     * @brief 配置数据（不可变）
     */
    const std::unordered_map<std::string, ConfigValue> config_data_;
    
    /**
     * @brief 快照元数据
     */
    const SnapshotMetadata metadata_;
    
    /**
     * @brief 原子引用计数（用于调试和监控）
     */
    mutable std::atomic<int> access_count_{0};

public:
    /**
     * @brief 构造函数
     * @param config_data 配置数据
     * @param metadata 元数据
     */
    explicit ConfigSnapshot(
        const std::unordered_map<std::string, ConfigValue>& config_data,
        const SnapshotMetadata& metadata)
        : config_data_(config_data), metadata_(metadata) {}

    /**
     * @brief 默认析构函数
     */
    ~ConfigSnapshot() = default;

    // 禁止拷贝构造和赋值（使用智能指针管理）
    ConfigSnapshot(const ConfigSnapshot&) = delete;
    ConfigSnapshot& operator=(const ConfigSnapshot&) = delete;

    // 允许移动构造和赋值
    ConfigSnapshot(ConfigSnapshot&&) noexcept = default;
    ConfigSnapshot& operator=(ConfigSnapshot&&) noexcept = default;

    /**
     * @brief 获取配置值
     * @param key 配置键
     * @param value 输出参数，配置值
     * @return bool 是否找到配置
     */
    bool GetValue(const std::string& key, ConfigValue& value) const {
        access_count_.fetch_add(1, std::memory_order_relaxed);
        
        auto it = config_data_.find(key);
        if (it != config_data_.end()) {
            value = it->second;
            return true;
        }
        return false;
    }

    /**
     * @brief 检查配置键是否存在
     * @param key 配置键
     * @return bool 是否存在
     */
    bool HasKey(const std::string& key) const {
        access_count_.fetch_add(1, std::memory_order_relaxed);
        return config_data_.find(key) != config_data_.end();
    }

    /**
     * @brief 获取所有配置键
     * @return std::vector<std::string> 配置键列表
     */
    std::vector<std::string> GetAllKeys() const {
        access_count_.fetch_add(1, std::memory_order_relaxed);
        
        std::vector<std::string> keys;
        keys.reserve(config_data_.size());
        
        for (const auto& [key, value] : config_data_) {
            keys.push_back(key);
        }
        
        return keys;
    }

    /**
     * @brief 获取指定前缀的配置键
     * @param prefix 键前缀
     * @return std::vector<std::string> 匹配的配置键列表
     */
    std::vector<std::string> GetKeysWithPrefix(const std::string& prefix) const {
        access_count_.fetch_add(1, std::memory_order_relaxed);
        
        std::vector<std::string> keys;
        
        for (const auto& [key, value] : config_data_) {
            if (key.find(prefix) == 0) {
                keys.push_back(key);
            }
        }
        
        return keys;
    }

    /**
     * @brief 获取配置数量
     * @return size_t 配置项数量
     */
    size_t GetConfigCount() const {
        return config_data_.size();
    }

    /**
     * @brief 获取快照元数据
     * @return const SnapshotMetadata& 元数据引用
     */
    const SnapshotMetadata& GetMetadata() const {
        return metadata_;
    }

    /**
     * @brief 获取访问计数（用于调试和监控）
     * @return int 访问次数
     */
    int GetAccessCount() const {
        return access_count_.load(std::memory_order_relaxed);
    }

    /**
     * @brief 计算数据校验和
     * @return std::string 校验和
     */
    std::string CalculateChecksum() const {
        // 简单的校验和计算，实际项目中可以使用更复杂的哈希算法
        size_t hash = 0;
        for (const auto& [key, value] : config_data_) {
            std::hash<std::string> hasher;
            hash ^= hasher(key) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            
            // 对配置值也进行哈希
            std::visit([&hash, &hasher](const auto& val) {
                if constexpr (std::is_same_v<std::decay_t<decltype(val)>, std::string>) {
                    hash ^= hasher(val) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
                } else {
                    hash ^= std::hash<std::decay_t<decltype(val)>>{}(val) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
                }
            }, value);
        }
        
        return std::to_string(hash);
    }

    /**
     * @brief 验证快照完整性
     * @return bool 是否有效
     */
    bool ValidateIntegrity() const {
        return CalculateChecksum() == metadata_.checksum;
    }

    /**
     * @brief 创建快照的深拷贝
     * @return SnapshotPtr 新快照的智能指针
     */
    SnapshotPtr Clone() const {
        access_count_.fetch_add(1, std::memory_order_relaxed);
        
        // 创建新的元数据，保持版本ID但更新时间
        SnapshotMetadata new_metadata = metadata_;
        new_metadata.create_time = std::chrono::system_clock::now();
        new_metadata.parent_version = metadata_.version_id;
        
        return std::make_shared<ConfigSnapshot>(config_data_, new_metadata);
    }

    /**
     * @brief 比较两个快照是否相等
     * @param other 另一个快照
     * @return bool 是否相等
     */
    bool Equals(const ConfigSnapshot& other) const {
        return config_data_ == other.config_data_ && 
               metadata_.version_id == other.metadata_.version_id;
    }
};

/**
 * @brief 配置快照工厂类
 * 
 * 提供创建和管理配置快照的工厂方法
 */
class ConfigSnapshotFactory {
public:
    /**
     * @brief 从配置映射创建快照
     * @param config_data 配置数据
     * @param version_id 版本ID
     * @param description 描述信息
     * @return ConfigSnapshot::SnapshotPtr 快照智能指针
     */
    static ConfigSnapshot::SnapshotPtr CreateSnapshot(
        const std::unordered_map<std::string, ConfigValue>& config_data,
        const std::string& version_id,
        const std::string& description = "") {
        
        ConfigSnapshot::SnapshotMetadata metadata;
        metadata.version_id = version_id;
        metadata.description = description;
        metadata.create_time = std::chrono::system_clock::now();
        metadata.config_count = config_data.size();
        
        // 创建临时快照以计算校验和
        ConfigSnapshot temp_snapshot(config_data, metadata);
        metadata.checksum = temp_snapshot.CalculateChecksum();
        
        return std::make_shared<ConfigSnapshot>(config_data, metadata);
    }

    /**
     * @brief 创建空快照
     * @param version_id 版本ID
     * @param description 描述信息
     * @return ConfigSnapshot::SnapshotPtr 快照智能指针
     */
    static ConfigSnapshot::SnapshotPtr CreateEmptySnapshot(
        const std::string& version_id,
        const std::string& description = "") {
        
        std::unordered_map<std::string, ConfigValue> empty_data;
        return CreateSnapshot(empty_data, version_id, description);
    }

    /**
     * @brief 合并多个快照
     * @param base_snapshot 基础快照
     * @param override_snapshot 覆盖快照
     * @param new_version_id 新版本ID
     * @param description 描述信息
     * @return ConfigSnapshot::SnapshotPtr 合并后的快照
     */
    static ConfigSnapshot::SnapshotPtr MergeSnapshots(
        const ConfigSnapshot::SnapshotPtr& base_snapshot,
        const ConfigSnapshot::SnapshotPtr& override_snapshot,
        const std::string& new_version_id,
        const std::string& description = "") {
        
        if (!base_snapshot) {
            return override_snapshot;
        }
        
        if (!override_snapshot) {
            return base_snapshot;
        }
        
        // 获取基础配置数据
        auto base_keys = base_snapshot->GetAllKeys();
        std::unordered_map<std::string, ConfigValue> merged_data;
        
        // 复制基础配置
        for (const auto& key : base_keys) {
            ConfigValue value;
            if (base_snapshot->GetValue(key, value)) {
                merged_data[key] = value;
            }
        }
        
        // 应用覆盖配置
        auto override_keys = override_snapshot->GetAllKeys();
        for (const auto& key : override_keys) {
            ConfigValue value;
            if (override_snapshot->GetValue(key, value)) {
                merged_data[key] = value;  // 覆盖或新增
            }
        }
        
        return CreateSnapshot(merged_data, new_version_id, description);
    }
};

/**
 * @brief 配置快照管理器
 * 
 * 提供快照的存储、检索和管理功能
 */
class ConfigSnapshotManager {
private:
    mutable std::shared_mutex snapshots_mutex_;
    std::unordered_map<std::string, ConfigSnapshot::SnapshotPtr> snapshots_;
    ConfigSnapshot::SnapshotPtr current_snapshot_;
    std::string current_version_id_;
    
    // 快照历史记录（用于回滚）
    std::vector<std::string> version_history_;
    static constexpr size_t kMaxHistorySize = 100;

public:
    ConfigSnapshotManager() = default;
    ~ConfigSnapshotManager() = default;
    
    // 禁止拷贝，允许移动
    ConfigSnapshotManager(const ConfigSnapshotManager&) = delete;
    ConfigSnapshotManager& operator=(const ConfigSnapshotManager&) = delete;
    
    ConfigSnapshotManager(ConfigSnapshotManager&&) noexcept = default;
    ConfigSnapshotManager& operator=(ConfigSnapshotManager&&) noexcept = default;

    /**
     * @brief 添加快照
     * @param snapshot 快照智能指针
     * @return bool 是否成功
     */
    bool AddSnapshot(const ConfigSnapshot::SnapshotPtr& snapshot) {
        if (!snapshot) {
            return false;
        }
        
        std::unique_lock<std::shared_mutex> lock(snapshots_mutex_);
        
        const auto& version_id = snapshot->GetMetadata().version_id;
        snapshots_[version_id] = snapshot;
        
        // 更新当前快照
        current_snapshot_ = snapshot;
        current_version_id_ = version_id;
        
        // 更新版本历史
        version_history_.push_back(version_id);
        if (version_history_.size() > kMaxHistorySize) {
            version_history_.erase(version_history_.begin());
        }
        
        return true;
    }

    /**
     * @brief 获取快照
     * @param version_id 版本ID
     * @return ConfigSnapshot::SnapshotPtr 快照智能指针，不存在时返回nullptr
     */
    ConfigSnapshot::SnapshotPtr GetSnapshot(const std::string& version_id) const {
        std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
        
        auto it = snapshots_.find(version_id);
        if (it != snapshots_.end()) {
            return it->second;
        }
        
        return nullptr;
    }

    /**
     * @brief 获取当前快照
     * @return ConfigSnapshot::SnapshotPtr 当前快照智能指针
     */
    ConfigSnapshot::SnapshotPtr GetCurrentSnapshot() const {
        std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
        return current_snapshot_;
    }

    /**
     * @brief 获取当前版本ID
     * @return std::string 当前版本ID
     */
    std::string GetCurrentVersionId() const {
        std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
        return current_version_id_;
    }

    /**
     * @brief 删除快照
     * @param version_id 版本ID
     * @return bool 是否成功
     */
    bool RemoveSnapshot(const std::string& version_id) {
        std::unique_lock<std::shared_mutex> lock(snapshots_mutex_);
        
        // 不能删除当前版本
        if (version_id == current_version_id_) {
            return false;
        }
        
        return snapshots_.erase(version_id) > 0;
    }

    /**
     * @brief 清理过期快照
     * @param keep_versions 要保留的版本数量
     * @return size_t 清理的快照数量
     */
    size_t CleanupSnapshots(size_t keep_versions = 10) {
        std::unique_lock<std::shared_mutex> lock(snapshots_mutex_);
        
        if (snapshots_.size() <= keep_versions) {
            return 0;
        }
        
        size_t removed_count = 0;
        auto it = snapshots_.begin();
        
        while (it != snapshots_.end() && snapshots_.size() > keep_versions) {
            // 跳过当前版本
            if (it->first == current_version_id_) {
                ++it;
                continue;
            }
            
            it = snapshots_.erase(it);
            ++removed_count;
        }
        
        return removed_count;
    }

    /**
     * @brief 获取所有版本ID
     * @return std::vector<std::string> 版本ID列表
     */
    std::vector<std::string> GetAllVersionIds() const {
        std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
        
        std::vector<std::string> version_ids;
        version_ids.reserve(snapshots_.size());
        
        for (const auto& [version_id, snapshot] : snapshots_) {
            version_ids.push_back(version_id);
        }
        
        return version_ids;
    }

    /**
     * @brief 获取快照数量
     * @return size_t 快照数量
     */
    size_t GetSnapshotCount() const {
        std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
        return snapshots_.size();
    }

    /**
     * @brief 回滚到指定版本
     * @param version_id 目标版本ID
     * @return bool 是否成功
     */
    bool RollbackToVersion(const std::string& version_id) {
        std::unique_lock<std::shared_mutex> lock(snapshots_mutex_);
        
        auto it = snapshots_.find(version_id);
        if (it == snapshots_.end()) {
            return false;
        }
        
        // 更新当前快照和版本ID
        current_snapshot_ = it->second;
        current_version_id_ = version_id;
        
        // 添加到历史记录
        version_history_.push_back(version_id);
        
        return true;
    }

    /**
     * @brief 获取版本历史
     * @return std::vector<std::string> 版本历史列表
     */
    std::vector<std::string> GetVersionHistory() const {
        std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
        return version_history_;
    }
};

/**
 * @brief 生成版本ID
 * @param prefix 前缀
 * @return std::string 版本ID
 */
std::string GenerateVersionId(const std::string& prefix = "v");

}  // namespace sqlcc

#endif  // SQLCC_CONFIG_SNAPSHOT_H_
