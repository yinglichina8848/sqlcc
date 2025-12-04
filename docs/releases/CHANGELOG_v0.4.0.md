# SQLCC v0.4.0 ChangeLog

## [v0.4.0] - 2025-11-12

### 修复
- **BufferPool死锁问题修复**
  - 解决了在执行磁盘I/O操作时持有锁导致的死锁问题
  - BufferPool::BatchPrefetchPages和PrefetchPage方法修改
  - 在执行磁盘I/O前释放缓冲池锁，操作完成后重新获取
  - 锁顺序优化，遵循一致的锁获取顺序

### 优化
- 并发性能优化：8线程并发下吞吐量达到2044.99 ops/sec
- 性能稳定，平均延迟保持在3.5-4.5ms范围内