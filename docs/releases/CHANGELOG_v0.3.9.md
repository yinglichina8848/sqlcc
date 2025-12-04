# SQLCC v0.3.9 ChangeLog

## [v0.3.9] - 2025-11-25

### 修复
- **BufferPool磁盘I/O死锁问题**
  - 修复了BatchPrefetchPages和PrefetchPage方法中的死锁问题
  - 在执行磁盘I/O前释放锁，完成后重新获取锁
  - 更新了版本号为0.3.9
  - 更新了ChangeLog.md文档，记录了v0.3.9版本的变更