#ifndef SQLCC_WAL_MANAGER_H
#define SQLCC_WAL_MANAGER_H

#include <atomic>
#include <chrono>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace sqlcc {

// 前向声明
using TransactionId = uint64_t;
enum class WALRecordType {
  BEGIN_TRANSACTION,
  COMMIT_TRANSACTION,
  ABORT_TRANSACTION,
  MODIFY_PAGE,
  CREATE_TABLE,
  DROP_TABLE,
  INSERT_TUPLE,
  UPDATE_TUPLE,
  DELETE_TUPLE,
  CHECKPOINT
};

/**
 * WAL日志记录
 */
struct WALRecord {
  uint64_t lsn; // 日志序列号
  TransactionId txn_id;
  WALRecordType type;
  std::chrono::system_clock::time_point timestamp;
  std::string data;  // 序列化的操作数据
  uint32_t checksum; // 数据校验和

  WALRecord();
  WALRecord(uint64_t lsn, TransactionId txn_id, WALRecordType type,
            const std::string &data);
};

/**
 * WAL管理器 - 实现预写日志系统
 */
class WALManager {
public:
  /**
   * 构造函数
   * @param wal_file WAL日志文件路径
   */
  explicit WALManager(const std::string &wal_file);

  /**
   * 析构函数
   */
  ~WALManager();

  /**
   * 写入事务开始记录
   * @param txn_id 事务ID
   */
  void write_begin_transaction(TransactionId txn_id);

  /**
   * 写入事务提交记录
   * @param txn_id 事务ID
   */
  void write_commit_transaction(TransactionId txn_id);

  /**
   * 写入事务中止记录
   * @param txn_id 事务ID
   */
  void write_abort_transaction(TransactionId txn_id);

  /**
   * 写入数据修改记录
   * @param txn_id 事务ID
   * @param table_name 表名
   * @param page_id 页面ID
   * @param old_data 修改前的数据
   * @param new_data 修改后的数据
   */
  void write_modify_page(TransactionId txn_id, const std::string &table_name,
                         page_id_t page_id, const std::vector<char> &old_data,
                         const std::vector<char> &new_data);

  /**
   * 强制刷新日志到磁盘
   */
  void flush();

  /**
   * 创建检查点
   * @return 检查点LSN
   */
  uint64_t create_checkpoint();

  /**
   * 恢复数据库状态
   */
  void recover();

  /**
   * 获取当前LSN
   */
  uint64_t get_current_lsn() const;

  /**
   * 根据LSN查找记录
   * @param lsn 日志序列号
   * @return WAL记录
   */
  WALRecord get_record(uint64_t lsn) const;

  /**
   * 获取事务的最后LSN
   * @param txn_id 事务ID
   */
  uint64_t get_last_lsn(TransactionId txn_id) const;

private:
  /**
   * 计算校验和
   * @param data 数据
   * @return 校验和
   */
  uint32_t calculate_checksum(const std::string &data) const;

  /**
   * 序列化WAL记录
   * @param record WAL记录
   * @return 序列化后的字符串
   */
  std::string serialize_record(const WALRecord &record) const;

  /**
   * 反序列化WAL记录
   * @param data 序列化数据
   * @return WAL记录
   */
  WALRecord deserialize_record(const std::string &data) const;

  /**
   * 追加记录到日志文件
   * @param record WAL记录
   */
  void append_record(const WALRecord &record);

  /**
   * 读取日志文件中的记录
   * @param lsn 日志序列号
   * @return WAL记录
   */
  WALRecord read_record(uint64_t lsn) const;

  std::string wal_file_;                                     // WAL文件路径
  std::ofstream wal_stream_;                                 // WAL文件输出流
  mutable std::mutex mutex_;                                 // 互斥锁
  std::atomic<uint64_t> current_lsn_;                        // 当前LSN
  std::unordered_map<TransactionId, uint64_t> txn_last_lsn_; // 事务最后LSN

  // 配置参数
  static constexpr size_t BUFFER_SIZE = 64 * 1024; // 64KB缓冲区
  static constexpr size_t MAX_LOG_SEGMENT_SIZE = 100 * 1024 * 1024; // 100MB
};

} // namespace sqlcc

#endif // SQLCC_WAL_MANAGER_H
