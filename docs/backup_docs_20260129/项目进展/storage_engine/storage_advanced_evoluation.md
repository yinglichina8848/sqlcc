<!-- StorageEngine_Evolution.md -->
<!-- 一键复制保存即可使用 -->
# StorageEngine 演进设计：多线程性能与数据读写优化

## 一、现状回顾
- 固定 8K 页面
- 单线程 BufferPool + 全局锁
- 无 NUMA/IO 调度感知
- 无压缩/加密/校验
- 无异步 IO

## 二、多线程性能优化

### 2.1 分片缓冲池（ShardedBufferPool）
```cpp
class ShardedBufferPool {
    static constexpr size_t kShards = 64;        // 2^6
    static constexpr size_t kShardMask = kShards - 1;

    struct Shard {
        alignas(64) std::shared_mutex latch;
        BufferPool pool;          // 独立 LRU/替换策略
    };
    std::array<Shard, kShards> shards_;

    size_t Shard(page_id_t id) const { return id & kShardMask; }
public:
    Page* FetchPage(page_id_t id) {
        auto& s = shards_[Shard(id)];
        std::shared_lock lock(s.latch);
        return s.pool.FetchPage(id);
    }
};
```
- 每 Shard ≤ 1GB（64K 页），全局 64GB 可扩展
- 锁竞争降 1/64，NUMA 节点本地分配

### 2.2 每页读写锁 + 版本锁（Lock Striping）
```cpp
struct PageState {
    alignas(64) std::shared_mutex rw_lock;
    std::atomic<uint64_t> version{0};   // 用于 MVCC/乐观读
    std::atomic<uint8_t> dirty{0};
};
```
- 读线程完全无阻塞写（MVCC 快照）
- 写-写仍互斥，但粒度最细

### 2.3 无锁 Ring-Buffer 提交队列
```cpp
class IoQueue {
    using Ring = moodycamel::ConcurrentQueue<IoRequest>;
    Ring ring_{1024};          // 1024 深度
    std::vector<std::thread> workers_;
    void WorkerLoop();
};
```
- 异步 Flush/Prefetch 任务化
- 消费者线程池绑定 NUMA 核心

### 2.4 事务级内存分配器
```cpp
class TxnAllocator {
    thread_local char* bump_ptr = nullptr;
    char* Alloc(size_t n) {
        if (bump_ptr + n > tls_end) NewTlsChunk();
        char* p = bump_ptr; bump_ptr += n; return p;
    }
};
```
- 单线程内无锁分配 Page/Record
- Commit 时一次性写回，Rollback 直接丢弃 Chunk

## 三、页面大小动态策略

### 3.1 多页面大小支持（Multi-Page-Size）
| 大小  | 用途                  | 占比阈值 |
|-------|-----------------------|----------|
| 4KB   | 索引叶、小记录        | ≤ 30%    |
| 8KB   | 默认数据页            | 默认 50% |
| 16KB  | 大对象、批量顺序扫描  | ≤ 15%    |
| 64KB  | LOB、日志、压缩块     | ≤ 5%     |

**实现方式**
```cpp
struct PageClass {
    size_t page_size;
    size_t pool_limit;
    ShardedBufferPool pool;
};
std::array<PageClass, 4> classes_;
```
- 页面 ID 高位编码大小类别（2 bit）
- BufferPool 按类别独立，防止不同大小页互相替换
- 后台监控线程根据访问模式调整各类别上限（自适应）

### 3.2 自适应大小调整算法
```
输入：过去 N 秒访问日志
1. 计算每类页命中率 H_i
2. 计算平均 IO 大小 S_i
3. 若 H_i < 阈值且 S_i 连续增大 → 增大 page_size
4. 若 H_i 高但空间浪费 > 20% → 减小 page_size
5. 在线迁移：拷贝-重定向-释放旧页
```

## 四、数据读写高级策略

### 4.1 压缩页（Per-Page Compression）
```cpp
struct CompressedPage {
    char raw[PAGE_SIZE];
    zstd::CTX* zctx;
    size_t Compress() {
        return ZSTD_compress(dst, dstCap, raw, PAGE_SIZE, 1);
    }
};
```
- 采用 ZSTD 快速级别，单页 < 1ms
- 仅压缩冷页/归档页，热页保持原样
- 节省 30-50% 空间，IO 带宽↑

### 4.2 加密页（AES-256-XTS）
- 每页独立 96-bit IV 存储在页头
- CPU AES-NI 加速，吞吐 > 3GB/s
- 与压缩链：压缩→加密→刷盘

### 4.3 校验与自愈
```cpp
struct PageHeader {
    uint32_t crc32c;
    uint64_t salt;     // 每次写递增
};
```
- 每次 ReadPage 校验 CRC，失败则尝试副本（若启用冗余存储）
- 后台 Scrubber 线程定期全表扫描

### 4.4 异步 IO + IO 调度器
```cpp
class IoScheduler {
    libaio::io_context_t ctx_;
    std::priority_queue<IoRequest, std::vector<IoRequest>, DeadlineCompare> pq_;
    void SubmitBatch();
};
```
- Linux libaio / Windows IOCP 统一封装
- 合并相邻页请求（elevator 算法）
- 支持 deadline + CFQ 双队列，读优先

### 4.5 预取与扫描优化
- 顺序检测：连续命中 ID+1 时触发顺序预取（depth=8）
- 跳跃扫描（Index Range）：记录过去跳转步长，预测下一次跳转页
- 自适应预取深度 = f(带宽, 命中率)

## 五、存储引擎顶层改进汇总

| 模块          | 改进点                              | 预期收益 |
|---------------|---------------------------------------|----------|
| BufferPool    | 分片 + per-page 锁                   | 8-16× TPS|
| IO            | 异步 + 合并 + NUMA 感知              | 延迟↓30%|
| Page Size     | 多类别 + 自适应                      | 命中率↑15%|
| Compression   | ZSTD 单页压缩                        | 空间↓40%|
| Encryption    | AES-NI 硬件加速                      | 零感知|
| CRC           | 每页校验 + 后台 Scrubber             | 数据安全|
| MVCC          | 版本锁 + 无锁读                      | 读写并发↑|

## 六、配置示例（cfg.ini）
```ini
[storage]
page_sizes=4k,8k,16k,64k
pool_limit_mb=1024,2048,512,256
compression=zstd
encryption=aes256
async_io=libaio
deadlock_detection=true
numa_node=0,1
```

## 七、渐进落地路线
1. **阶段 1**：引入分片 BufferPool + per-page 读写锁（立竿见影）
2. **阶段 2**：接入异步 IO 与 IO 合并（延迟优化）
3. **阶段 3**：动态页面大小与压缩（容量/性能双赢）
4. **阶段 4**：加密、CRC、Scrubber（企业级安全）
5. **阶段 5**：MVCC + 无锁数据结构（极致并发）

