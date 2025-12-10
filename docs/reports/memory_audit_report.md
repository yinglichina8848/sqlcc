# SQLCC项目内存审计报告

## 基本信息

- **生成时间**: 2025-12-11 05:46:58
- **审计文件数**: 663
- **项目根目录**: /home/liying/sqlcc

## 审计结果

发现 663 个潜在的内存管理问题:

### 问题 1

```
/home/liying/sqlcc/include/page.h:79: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     inline char* GetData() { return data_; }
```

### 问题 2

```
/home/liying/sqlcc/include/page.h:113: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     void ReadData(size_t offset, char* data, size_t size) const;
```

### 问题 3

```
/home/liying/sqlcc/include/network/network.h:183: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     void SetTLS(struct ssl_st* ssl, bool enabled);
```

### 问题 4

```
/home/liying/sqlcc/include/core/unified_executor.h:176: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeCreate(sql_parser::CreateStatement *stmt,
```

### 问题 5

```
/home/liying/sqlcc/include/core/unified_executor.h:179: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeDrop(sql_parser::DropStatement *stmt,
```

### 问题 6

```
/home/liying/sqlcc/include/core/unified_executor.h:182: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeAlter(sql_parser::AlterStatement *stmt,
```

### 问题 7

```
/home/liying/sqlcc/include/core/unified_executor.h:185: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeCreateIndex(sql_parser::CreateIndexStatement *stmt,
```

### 问题 8

```
/home/liying/sqlcc/include/core/unified_executor.h:188: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeDropIndex(sql_parser::DropIndexStatement *stmt,
```

### 问题 9

```
/home/liying/sqlcc/include/core/unified_executor.h:214: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeInsert(sql_parser::InsertStatement *stmt,
```

### 问题 10

```
/home/liying/sqlcc/include/core/unified_executor.h:217: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeUpdate(sql_parser::UpdateStatement *stmt,
```

### 问题 11

```
/home/liying/sqlcc/include/core/unified_executor.h:220: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeDelete(sql_parser::DeleteStatement *stmt,
```

### 问题 12

```
/home/liying/sqlcc/include/core/unified_executor.h:223: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeSelect(sql_parser::SelectStatement *stmt,
```

### 问题 13

```
/home/liying/sqlcc/include/core/unified_executor.h:242: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeCreateUser(sql_parser::CreateUserStatement *stmt,
```

### 问题 14

```
/home/liying/sqlcc/include/core/unified_executor.h:245: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeDropUser(sql_parser::DropUserStatement *stmt,
```

### 问题 15

```
/home/liying/sqlcc/include/core/unified_executor.h:248: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeGrant(sql_parser::GrantStatement *stmt,
```

### 问题 16

```
/home/liying/sqlcc/include/core/unified_executor.h:251: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeRevoke(sql_parser::RevokeStatement *stmt,
```

### 问题 17

```
/home/liying/sqlcc/include/core/unified_executor.h:270: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeUse(sql_parser::UseStatement *stmt,
```

### 问题 18

```
/home/liying/sqlcc/include/core/unified_executor.h:273: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeShow(sql_parser::ShowStatement *stmt,
```

### 问题 19

```
/home/liying/sqlcc/include/core/unified_executor.h:456: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionStrategy *getStrategy(sql_parser::Statement::Type type);
```

### 问题 20

```
/home/liying/sqlcc/include/core/unified_executor.h:483: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeJoinQuery(sql_parser::SelectStatement *stmt);
```

### 问题 21

```
/home/liying/sqlcc/include/core/unified_executor.h:486: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeSubquery(sql_parser::SelectStatement *stmt);
```

### 问题 22

```
/home/liying/sqlcc/include/core/unified_executor.h:489: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeWindowFunction(sql_parser::SelectStatement *stmt);
```

### 问题 23

```
/home/liying/sqlcc/include/core/user_manager.h:94: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   void SetSystemDatabase(SystemDatabase *sys_db);
```

### 问题 24

```
/home/liying/sqlcc/include/core/user_manager.h:168: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   SystemDatabase *sys_db_;   // SystemDatabase引用（用于权限同步）
```

### 问题 25

```
/home/liying/sqlcc/include/storage/prefetcher.h:89: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       return *this;
```

### 问题 26

```
/home/liying/sqlcc/include/storage/prefetcher.h:101: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       return *this;
```

### 问题 27

```
/home/liying/sqlcc/include/storage/buffer_pool.h:145: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     explicit BufferPool(DiskManager* disk_manager, size_t pool_size, ConfigManager& config_manager);
```

### 问题 28

```
/home/liying/sqlcc/include/storage/buffer_pool.h:162: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* FetchPage(int32_t page_id);
```

### 问题 29

```
/home/liying/sqlcc/include/storage/buffer_pool.h:181: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* NewPage(int32_t* page_id);
```

### 问题 30

```
/home/liying/sqlcc/include/storage/performance_monitor.h:58: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     return *this;
```

### 问题 31

```
/home/liying/sqlcc/include/storage/performance_monitor.h:67: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     return *this;
```

### 问题 32

```
/home/liying/sqlcc/include/storage/buffer_pool_sharded.h:38: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     BufferPoolSharded(DiskManager* disk_manager, ConfigManager& config_manager, 
```

### 问题 33

```
/home/liying/sqlcc/include/storage/buffer_pool_sharded.h:52: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* FetchPage(int32_t page_id, bool exclusive = false);
```

### 问题 34

```
/home/liying/sqlcc/include/storage/buffer_pool_sharded.h:86: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* NewPage(int32_t* page_id);
```

### 问题 35

```
/home/liying/sqlcc/include/storage/buffer_pool_sharded.h:149: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     DiskManager* disk_manager_;
```

### 问题 36

```
/home/liying/sqlcc/include/storage/b_plus_tree.h:132: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     void Merge(BPlusTreeLeafNode* right_node);
```

### 问题 37

```
/home/liying/sqlcc/include/storage/table_storage.h:111: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     bool DeleteRecordInPage(class Page* page, size_t offset);
```

### 问题 38

```
/home/liying/sqlcc/include/storage/table_storage.h:112: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::vector<std::string> GetRecordFromPage(class Page* page, size_t offset) const;
```

### 问题 39

```
/home/liying/sqlcc/include/storage/table_storage.h:116: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     PageHeader ReadPageHeader(class Page* page) const;
```

### 问题 40

```
/home/liying/sqlcc/include/storage/replace_strategy.h:67: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       return *this;
```

### 问题 41

```
/home/liying/sqlcc/include/storage/buffer_pool_v2.h:17: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     explicit BufferPoolV2(DiskManager* disk_manager, size_t pool_size);
```

### 问题 42

```
/home/liying/sqlcc/include/storage/buffer_pool_v2.h:23: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* FetchPage(int32_t page_id);
```

### 问题 43

```
/home/liying/sqlcc/include/storage/buffer_pool_v2.h:25: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* NewPage(int32_t* page_id);
```

### 问题 44

```
/home/liying/sqlcc/include/storage/buffer_pool_v2.h:58: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     DiskManager* disk_manager_;
```

### 问题 45

```
/home/liying/sqlcc/include/storage/buffer_pool_new.h:47: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     explicit BufferPool(DiskManager* disk_manager, size_t pool_size, ConfigManager& config_manager);
```

### 问题 46

```
/home/liying/sqlcc/include/storage/buffer_pool_new.h:51: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* FetchPage(int32_t page_id);
```

### 问题 47

```
/home/liying/sqlcc/include/storage/buffer_pool_new.h:53: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* NewPage(int32_t* page_id);
```

### 问题 48

```
/home/liying/sqlcc/include/execution_engine.h:76: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeCreate(sqlcc::sql_parser::CreateStatement *stmt);
```

### 问题 49

```
/home/liying/sqlcc/include/execution_engine.h:77: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeDrop(sqlcc::sql_parser::DropStatement *stmt);
```

### 问题 50

```
/home/liying/sqlcc/include/execution_engine.h:78: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeAlter(sqlcc::sql_parser::AlterStatement *stmt);
```

### 问题 51

```
/home/liying/sqlcc/include/execution_engine.h:80: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   executeCreateIndex(sqlcc::sql_parser::CreateIndexStatement *stmt);
```

### 问题 52

```
/home/liying/sqlcc/include/execution_engine.h:81: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeDropIndex(sqlcc::sql_parser::DropIndexStatement *stmt);
```

### 问题 53

```
/home/liying/sqlcc/include/execution_engine.h:116: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeInsert(sqlcc::sql_parser::InsertStatement *stmt);
```

### 问题 54

```
/home/liying/sqlcc/include/execution_engine.h:117: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeUpdate(sqlcc::sql_parser::UpdateStatement *stmt);
```

### 问题 55

```
/home/liying/sqlcc/include/execution_engine.h:118: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeDelete(sqlcc::sql_parser::DeleteStatement *stmt);
```

### 问题 56

```
/home/liying/sqlcc/include/execution_engine.h:178: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeCreateUser(sql_parser::CreateUserStatement *stmt);
```

### 问题 57

```
/home/liying/sqlcc/include/execution_engine.h:179: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeDropUser(sql_parser::DropUserStatement *stmt);
```

### 问题 58

```
/home/liying/sqlcc/include/execution_engine.h:180: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeGrant(sql_parser::GrantStatement *stmt);
```

### 问题 59

```
/home/liying/sqlcc/include/execution_engine.h:181: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeRevoke(sql_parser::RevokeStatement *stmt);
```

### 问题 60

```
/home/liying/sqlcc/include/execution_engine.h:198: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult executeShow(sql_parser::ShowStatement *stmt);
```

### 问题 61

```
/home/liying/sqlcc/include/utils/file_descriptor.h:44: 文件描述符直接使用 - 建议封装为RAII类
  代码:     explicit FileDescriptor(int fd) noexcept : fd_(fd) {}
```

### 问题 62

```
/home/liying/sqlcc/include/utils/file_descriptor.h:77: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         return *this;
```

### 问题 63

```
/home/liying/sqlcc/include/utils/file_descriptor.h:84: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int get() const noexcept { return fd_; }
```

### 问题 64

```
/home/liying/sqlcc/include/utils/file_descriptor.h:102: 文件描述符直接使用 - 建议封装为RAII类
  代码:     void reset(int fd = -1) noexcept {
```

### 问题 65

```
/home/liying/sqlcc/include/utils/file_descriptor.h:113: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int temp = fd_;
```

### 问题 66

```
/home/liying/sqlcc/include/utils/file_descriptor.h:129: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int fd_;  ///< 文件描述符
```

### 问题 67

```
/home/liying/sqlcc/include/utils/file_descriptor.h:146: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int fd = ::socket(AF_INET, SOCK_STREAM, 0);
```

### 问题 68

```
/home/liying/sqlcc/include/utils/file_descriptor.h:155: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
```

### 问题 69

```
/home/liying/sqlcc/include/sql_parser/set_operation_node.h:48: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SelectStatement* getLeftOperand() const;
```

### 问题 70

```
/home/liying/sqlcc/include/sql_parser/set_operation_node.h:49: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SelectStatement* getRightOperand() const;
```

### 问题 71

```
/home/liying/sqlcc/include/disk_manager.h:52: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   bool ReadPage(int32_t page_id, char *page_data);
```

### 问题 72

```
/home/liying/sqlcc/include/database_manager.h:80: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   bool WritePage(TransactionId txn_id, int32_t page_id, Page *page);
```

### 问题 73

```
/home/liying/sqlcc/include/execution/set_operation_executor.h:81: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult execute_subquery(SelectStatement *subquery);
```

### 问题 74

```
/home/liying/sqlcc/include/storage_engine.h:26: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:  * Page* page = storage_engine.NewPage();
```

### 问题 75

```
/home/liying/sqlcc/include/storage_engine.h:91: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *FetchPage(int32_t page_id);
```

### 问题 76

```
/home/liying/sqlcc/include/sql_executor/index_manager.h:20: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   IndexManager(StorageEngine *storage_engine, ConfigManager &config_manager);
```

### 问题 77

```
/home/liying/sqlcc/include/sql_executor/index_manager.h:45: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   StorageEngine *storage_engine_; // 存储引擎指针
```

### 问题 78

```
/home/liying/sqlcc/examples/aes_demo.cpp:66: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string plaintext = "SELECT * FROM users WHERE id = 1;";
```

### 问题 79

```
/home/liying/sqlcc/examples/aes_demo.cpp:118: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM users WHERE name LIKE 'A%';"
```

### 问题 80

```
/home/liying/sqlcc/examples/transaction_manager_quick_test.cpp:232: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   int expected_successes = NUM_THREADS * TXNS_PER_THREAD;
```

### 问题 81

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:28: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     void expectStatementType(Statement* stmt, Statement::Type expectedType) {
```

### 问题 82

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:41: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* stmt = statements[0].get();
```

### 问题 83

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:44: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
```

### 问题 84

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:54: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* setOp = operations[0].get();
```

### 问题 85

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:66: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* stmt = statements[0].get();
```

### 问题 86

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:69: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
```

### 问题 87

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:75: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* setOp = operations[0].get();
```

### 问题 88

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:87: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* stmt = statements[0].get();
```

### 问题 89

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:90: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
```

### 问题 90

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:96: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* setOp = operations[0].get();
```

### 问题 91

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:108: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* stmt = statements[0].get();
```

### 问题 92

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:111: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
```

### 问题 93

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:117: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* setOp = operations[0].get();
```

### 问题 94

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:129: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* stmt = statements[0].get();
```

### 问题 95

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:132: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
```

### 问题 96

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:155: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* stmt = statements[0].get();
```

### 问题 97

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:158: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* selectStmt = dynamic_cast<SelectStatement*>(stmt);
```

### 问题 98

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:171: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* stmt = statements[0].get();
```

### 问题 99

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:174: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
```

### 问题 100

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:181: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* firstSelect = selectStmts[0].get();
```

### 问题 101

```
/home/liying/sqlcc/test/sql_parser/set_operation_parser_test.cpp:186: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* secondSelect = selectStmts[1].get();
```

### 问题 102

```
/home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp:75: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   bool ReadPage(int32_t page_id, char *page_data) {
```

### 问题 103

```
/home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp:136: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   static MockConfigManager *mock_config_manager_;
```

### 问题 104

```
/home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp:268: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::vector<int32_t> page_ids(num_threads * operations_per_thread / 2);
```

### 问题 105

```
/home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp:271: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   for (int i = 0; i < num_threads * operations_per_thread / 2; ++i) {
```

### 问题 106

```
/home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp:309: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   EXPECT_EQ(success_count.load(), num_threads * operations_per_thread);
```

### 问题 107

```
/home/liying/sqlcc/tests/test_disk_manager.h:84: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     bool TestReadPage(int32_t page_id, char* page_data) {
```

### 问题 108

```
/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_simple.cpp:5: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string input = "SELECT * FROM users WHERE id = 1;";
```

### 问题 109

```
/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_output.cpp:5: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string input = "SELECT * FROM users WHERE id = 1;";
```

### 问题 110

```
/home/liying/sqlcc/tests/network/aes_encryption_test.cc:153: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM users WHERE id = 1;",
```

### 问题 111

```
/home/liying/sqlcc/tests/network/aes_encryption_test.cc:187: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::vector<uint8_t> large_data(100 * 1024);
```

### 问题 112

```
/home/liying/sqlcc/tests/network/tls_e2e_test.cc:18: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     EVP_PKEY* pkey = EVP_PKEY_new();
```

### 问题 113

```
/home/liying/sqlcc/tests/network/tls_e2e_test.cc:19: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     RSA* rsa = RSA_new();
```

### 问题 114

```
/home/liying/sqlcc/tests/network/tls_e2e_test.cc:20: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     BIGNUM* e = BN_new();
```

### 问题 115

```
/home/liying/sqlcc/tests/network/tls_e2e_test.cc:26: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     X509* x509 = X509_new();
```

### 问题 116

```
/home/liying/sqlcc/tests/network/tls_e2e_test.cc:31: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     X509_NAME* name = X509_get_subject_name(x509);
```

### 问题 117

```
/home/liying/sqlcc/tests/network/tls_e2e_test.cc:38: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     FILE* cf = fopen(cert_path.c_str(), "wb");
```

### 问题 118

```
/home/liying/sqlcc/tests/network/tls_e2e_test.cc:43: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     FILE* kf = fopen(key_path.c_str(), "wb");
```

### 问题 119

```
/home/liying/sqlcc/tests/network/tls_e2e_test.cc:100: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* ack_hdr = reinterpret_cast<MessageHeader*>(conn_ack.data());
```

### 问题 120

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:93: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int ret = recv(sock_fd_, buffer, sizeof(buffer), 0);
```

### 问题 121

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:113: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             MessageHeader* resp_header = reinterpret_cast<MessageHeader*>(buffer);
```

### 问题 122

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:136: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         size_t body_len = 2 * sizeof(uint32_t) + username_len + password_len;
```

### 问题 123

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:139: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* header = reinterpret_cast<MessageHeader*>(message.data());
```

### 问题 124

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:154: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         char* body = message.data() + sizeof(MessageHeader);
```

### 问题 125

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:157: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         memcpy(body + 2 * sizeof(uint32_t), username.c_str(), username_len);
```

### 问题 126

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:158: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         memcpy(body + 2 * sizeof(uint32_t) + username_len, password.c_str(), password_len);
```

### 问题 127

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:167: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int ret = recv(sock_fd_, buffer, sizeof(buffer), 0);
```

### 问题 128

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:172: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* resp_header = reinterpret_cast<MessageHeader*>(buffer);
```

### 问题 129

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:181: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* header = reinterpret_cast<MessageHeader*>(message.data());
```

### 问题 130

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:197: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int ret = recv(sock_fd_, buffer, sizeof(buffer), 0);
```

### 问题 131

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:202: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* resp_header = reinterpret_cast<MessageHeader*>(buffer);
```

### 问题 132

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:224: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int sock_fd_;
```

### 问题 133

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:354: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     result = client.ExecuteQuery("SELECT * FROM network_test_users WHERE id = 1");
```

### 问题 134

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:371: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     result = client.ExecuteQuery("SELECT * FROM network_test_users WHERE id = 2");
```

### 问题 135

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:411: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     string result = client.ExecuteQuery("SELECT * FROM network_test_products WHERE price > 4000");
```

### 问题 136

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:102: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string query = "SELECT * FROM users WHERE id = 1;";
```

### 问题 137

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:129: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM users;",
```

### 问题 138

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:169: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::vector<bool> results(num_threads * iterations_per_thread, false);
```

### 问题 139

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:185: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:                 results[t * iterations_per_thread + i] = success;
```

### 问题 140

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:234: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::cout << "  Data Size: " << data_size / (1024.0 * 1024.0) << " MB" << std::endl;
```

### 问题 141

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:239: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double encrypt_throughput = (data_size / (1024.0 * 1024.0)) / (encrypt_time / 1000.0);
```

### 问题 142

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:244: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double decrypt_throughput = (data_size / (1024.0 * 1024.0)) / (decrypt_time / 1000.0);
```

### 问题 143

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:357: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string query = "SELECT * FROM users WHERE id = 1;";
```

### 问题 144

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:384: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM users;",
```

### 问题 145

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:424: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::vector<bool> results(num_threads * iterations_per_thread, false);
```

### 问题 146

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:440: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:                 results[t * iterations_per_thread + i] = success;
```

### 问题 147

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:489: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::cout << "  Data Size: " << data_size / (1024.0 * 1024.0) << " MB" << std::endl;
```

### 问题 148

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:494: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double encrypt_throughput = (data_size / (1024.0 * 1024.0)) / (encrypt_time / 1000.0);
```

### 问题 149

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:499: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double decrypt_throughput = (data_size / (1024.0 * 1024.0)) / (decrypt_time / 1000.0);
```

### 问题 150

```
/home/liying/sqlcc/tests/integration/advanced_sql/isql_integration_test.cpp:133: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     FILE *pipe = popen(command.c_str(), "r");
```

### 问题 151

```
/home/liying/sqlcc/tests/integration/advanced_sql/isql_integration_test.cpp:206: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "SELECT * FROM test_table;\n"
```

### 问题 152

```
/home/liying/sqlcc/tests/integration/advanced_sql/sql_executor_integration_test.cpp:91: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql = "SELECT * FROM test_users;";
```

### 问题 153

```
/home/liying/sqlcc/tests/integration/advanced_sql/sql_executor_integration_test.cpp:159: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql = "SELECT * FROM non_existent_table;";
```

### 问题 154

```
/home/liying/sqlcc/tests/integration/advanced_sql/sql_executor_integration_test.cpp:201: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql3 = "SELECT * FROM multi_test;";
```

### 问题 155

```
/home/liying/sqlcc/tests/integration/isql_integration_test.cpp:133: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     FILE *pipe = popen(command.c_str(), "r");
```

### 问题 156

```
/home/liying/sqlcc/tests/integration/isql_integration_test.cpp:206: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "SELECT * FROM test_table;\n"
```

### 问题 157

```
/home/liying/sqlcc/tests/integration/simple_sql_test.cpp:31: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string sql = "SELECT * FROM users;";
```

### 问题 158

```
/home/liying/sqlcc/tests/integration/basic_sql/simple_sql_test.cpp:31: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string sql = "SELECT * FROM users;";
```

### 问题 159

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:166: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM products");
```

### 问题 160

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:172: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM products WHERE price BETWEEN 50 AND 200");
```

### 问题 161

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:173: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM products WHERE name LIKE '%board%'");
```

### 问题 162

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:174: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM products WHERE stock > 0 AND price < 100");
```

### 问题 163

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:235: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM persistent_data WHERE id = 5");
```

### 问题 164

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:268: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:                          std::to_string(i * 10) + ")");
```

### 问题 165

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:272: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM indexed_table WHERE name = 'Item50'");
```

### 问题 166

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:273: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM indexed_table WHERE category = 'Category5'");
```

### 问题 167

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:274: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM indexed_table WHERE category = 'Category3' AND value > 500");
```

### 问题 168

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:350: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM non_existent_table", "错误");
```

### 问题 169

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:404: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM perf_table WHERE id = 500");
```

### 问题 170

```
/home/liying/sqlcc/tests/integration/sql_executor_integration_test.cpp:91: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql = "SELECT * FROM test_users;";
```

### 问题 171

```
/home/liying/sqlcc/tests/integration/sql_executor_integration_test.cpp:159: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql = "SELECT * FROM non_existent_table;";
```

### 问题 172

```
/home/liying/sqlcc/tests/integration/sql_executor_integration_test.cpp:201: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql3 = "SELECT * FROM multi_test;";
```

### 问题 173

```
/home/liying/sqlcc/tests/unsupported_commands_test.cpp:48: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       executor.Execute("CREATE VIEW v1 AS SELECT * FROM users;");
```

### 问题 174

```
/home/liying/sqlcc/tests/components/network/network_unit_test.cpp:11: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Session *session_;
```

### 问题 175

```
/home/liying/sqlcc/tests/components/network/network_unit_test.cpp:13: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   void SetUp() override { session_ = new Session(1); }
```

### 问题 176

```
/home/liying/sqlcc/tests/components/network/network_unit_test.cpp:15: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:   void TearDown() override { delete session_; }
```

### 问题 177

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_test.cpp:45: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = buffer_pool_->FetchPage(page_id);
```

### 问题 178

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_test.cpp:64: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = buffer_pool_->FetchPage(page_id);
```

### 问题 179

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_test.cpp:84: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = buffer_pool_->FetchPage(page_id);
```

### 问题 180

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_test.cpp:93: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page2 = buffer_pool_->FetchPage(page_id);
```

### 问题 181

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_test.cpp:114: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page *page = buffer_pool_->FetchPage(page_id);
```

### 问题 182

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_test.cpp:123: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page *page = buffer_pool_->FetchPage(page_ids[i]);
```

### 问题 183

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_test.cpp:143: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page *page = buffer_pool_->FetchPage(page_id);
```

### 问题 184

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_test.cpp:154: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page *page = buffer_pool_->FetchPage(page_id);
```

### 问题 185

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_test.cpp:171: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = buffer_pool_->FetchPage(page_id);
```

### 问题 186

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_test.cpp:179: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page2 = buffer_pool_->FetchPage(page_id);
```

### 问题 187

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:83: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string select_sql = "SELECT * FROM users WHERE id = 1;";
```

### 问题 188

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:102: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string select_sql = "SELECT * FROM users WHERE name = 'Bob';";
```

### 问题 189

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:128: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string select_sql = "SELECT * FROM users WHERE id = 4;";
```

### 问题 190

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:150: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string select_sql = "SELECT * FROM users WHERE age = 36;";
```

### 问题 191

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:172: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string select_sql = "SELECT * FROM users WHERE id = 7;";
```

### 问题 192

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:190: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string select_sql = "SELECT * FROM users WHERE age > 20 AND age < 50;";
```

### 问题 193

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:209: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "SELECT * FROM users WHERE id = 9;",
```

### 问题 194

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:210: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "SELECT * FROM users WHERE name = 'Ivan';",
```

### 问题 195

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:211: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "SELECT * FROM users WHERE age = 50;"};
```

### 问题 196

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:241: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string select_sql = "SELECT * FROM users WHERE id = 10;";
```

### 问题 197

```
/home/liying/sqlcc/tests/components/storage/disk_manager_test.cpp:103: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   // 文件大小应该至少是num_pages * page_size_
```

### 问题 198

```
/home/liying/sqlcc/tests/components/storage/disk_manager_test.cpp:104: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   EXPECT_GE(file_size, num_pages * page_size_);
```

### 问题 199

```
/home/liying/sqlcc/tests/components/storage/disk_manager_test.cpp:126: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: //   EXPECT_EQ(new_page_id, 3); // 应该从3开始分配，因为文件大小已经是3 * PAGE_SIZE
```

### 问题 200

```
/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp:40: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int temp_fd_ = -1;
```

### 问题 201

```
/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp:64: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int raw_fd = static_cast<int>(fd3);
```

### 问题 202

```
/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp:72: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int original_fd;
```

### 问题 203

```
/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp:80: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int flags = fcntl(fd.get(), F_GETFL);
```

### 问题 204

```
/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp:85: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int result = fcntl(original_fd, F_GETFL);
```

### 问题 205

```
/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp:119: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int released_fd = fd.release();
```

### 问题 206

```
/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp:140: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int result = getsockopt(tcp_socket.get(), SOL_SOCKET, SO_TYPE, &type, &len);
```

### 问题 207

```
/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp:193: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int result = fcntl(temp_fd_, F_GETFL);
```

### 问题 208

```
/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp:86: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         // 模式1: 裸指针声明 (Type* var)
```

### 问题 209

```
/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp:97: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:         if (line.find("new ") != std::string::npos &&
```

### 问题 210

```
/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp:105: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         if (line.find("delete ") != std::string::npos &&
```

### 问题 211

```
/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp:277: 文件描述符直接使用 - 建议封装为RAII类
  代码:         explicit SafeFileDescriptor(int fd) : fd_(fd) {}
```

### 问题 212

```
/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp:284: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int get() const { return fd_; }
```

### 问题 213

```
/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp:296: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int fd_;
```

### 问题 214

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:202: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* page = buffer_pool_->NewPage(&page_id);
```

### 问题 215

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:228: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* page = buffer_pool_->NewPage(&page_id);
```

### 问题 216

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:249: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* page = buffer_pool_->NewPage(&page_id);
```

### 问题 217

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:270: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* page = buffer_pool_->NewPage(&page_id);
```

### 问题 218

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:275: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* fetched_page = buffer_pool_->FetchPage(page_id);
```

### 问题 219

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:300: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* page = buffer_pool_sharded_->NewPage(&page_id);
```

### 问题 220

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:305: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* fetched = buffer_pool_sharded_->FetchPage(page_id);
```

### 问题 221

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:322: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* page = buffer_pool_sharded_->NewPage(&page_id);
```

### 问题 222

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:337: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         Page* page = buffer_pool_sharded_->FetchPage(page_ids[j]);
```

### 问题 223

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:370: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* page = buffer_pool_new_->NewPage(&page_id);
```

### 问题 224

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:375: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* fetched = buffer_pool_new_->FetchPage(page_id);
```

### 问题 225

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:392: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* page = buffer_pool_new_->NewPage(&page_id);
```

### 问题 226

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:519: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* page = buffer_pool->NewPage(&page_id);
```

### 问题 227

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:571: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       Page* page = buffer_pool->NewPage(&page_id);
```

### 问题 228

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:615: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* page = buffer_pool->NewPage(&page_id);
```

### 问题 229

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:632: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* page = buffer_pool->FetchPage(page_id);
```

### 问题 230

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:663: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* page = small_pool->NewPage(&page_id);
```

### 问题 231

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:668: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* page2 = small_pool->NewPage(&page_id2);
```

### 问题 232

```
/home/liying/sqlcc/tests/components/debug/buffer_pool_smart_pointer_test.cpp:697: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* page = buffer_pool->NewPage(&page_id);
```

### 问题 233

```
/home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp:155: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:                       std::to_string(i) + ", " + std::to_string(i * 100) + ");";
```

### 问题 234

```
/home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp:162: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   // std::string select_sql = "SELECT * FROM transactions WHERE amount BETWEEN
```

### 问题 235

```
/home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp:190: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   // std::string select_sql = "SELECT * FROM users WHERE email LIKE
```

### 问题 236

```
/home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp:221: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   // std::string select_sql = "SELECT * FROM employees WHERE (age >= 25 AND
```

### 问题 237

```
/home/liying/sqlcc/tests/components/executor/set_operation_test.cpp:33: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   set_executor_->set_memory_limit(1024 * 1024 * 100); // 100MB
```

### 问题 238

```
/home/liying/sqlcc/tests/components/executor/set_operation_test.cpp:71: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   set_executor_->set_memory_limit(1024 * 1024);       // 1MB
```

### 问题 239

```
/home/liying/sqlcc/tests/components/executor/set_operation_test.cpp:72: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   set_executor_->set_memory_limit(1024 * 1024 * 500); // 500MB
```

### 问题 240

```
/home/liying/sqlcc/tests/components/executor/unified_executor_test.cpp:227: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ParserNew select_parser("SELECT * FROM test_table;");
```

### 问题 241

```
/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp:130: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   EXPECT_EQ(stats.rows_processed, 9); // 3*3=9行处理
```

### 问题 242

```
/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp:157: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   EXPECT_EQ(stats.rows_processed, 9); // 3*3=9行处理
```

### 问题 243

```
/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp:185: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   EXPECT_EQ(stats.rows_processed, 9); // 3*3=9行处理
```

### 问题 244

```
/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp:205: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   EXPECT_EQ(result.rows.size(), 9); // 3*3=9行交叉连接
```

### 问题 245

```
/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp:211: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   EXPECT_EQ(stats.rows_processed, 9); // 3*3=9行处理
```

### 问题 246

```
/home/liying/sqlcc/tests/test_revoke_persistence.cpp:65: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string result = sql_exec.Execute("SELECT * FROM sys_privileges WHERE grantee_name = 'alice'");
```

### 问题 247

```
/home/liying/sqlcc/tests/test_revoke_persistence.cpp:87: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string result_before = sql_exec.Execute("SELECT * FROM sys_privileges WHERE grantee_name = 'alice'");
```

### 问题 248

```
/home/liying/sqlcc/tests/test_revoke_persistence.cpp:99: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string result_after = sql_exec.Execute("SELECT * FROM sys_privileges WHERE grantee_name = 'alice'");
```

### 问题 249

```
/home/liying/sqlcc/tests/test_revoke_persistence.cpp:121: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string final_result = sql_exec.Execute("SELECT * FROM sys_privileges WHERE grantee_name = 'alice'");
```

### 问题 250

```
/home/liying/sqlcc/tests/sql_parser/error_integration_test.cpp:417: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "SELCT * FROM users",  // Typo in SELECT
```

### 问题 251

```
/home/liying/sqlcc/tests/sql_parser/error_integration_test.cpp:418: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "SELECT * FROM users WHERE id = 'string'",  // Type mismatch
```

### 问题 252

```
/home/liying/sqlcc/tests/sql_parser/error_integration_test.cpp:419: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "SELECT * FROM unknown_table",  // Undefined table
```

### 问题 253

```
/home/liying/sqlcc/tests/sql_parser/error_integration_test.cpp:420: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "SELECT * FROM users  ",  // Missing semicolon (will be detected)
```

### 问题 254

```
/home/liying/sqlcc/tests/sql_parser/error_integration_test.cpp:421: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "SELECT * FROM users\x01",  // Invalid character
```

### 问题 255

```
/home/liying/sqlcc/tests/sql_parser/error_integration_test.cpp:422: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "SELECT * FROM users WHERE name = 'unterminated"  // Unterminated string
```

### 问题 256

```
/home/liying/sqlcc/tests/sql_parser/token_new_unit_test.cpp:380: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql = "SELECT * FROM users WHERE id = 1;";
```

### 问题 257

```
/home/liying/sqlcc/tests/sql_parser/token_new_unit_test.cpp:474: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM users -- This is a comment\nWHERE id = 1;";
```

### 问题 258

```
/home/liying/sqlcc/tests/sql_parser/token_new_unit_test.cpp:495: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT    *    FROM     users    WHERE   id   =   1;";
```

### 问题 259

```
/home/liying/sqlcc/tests/sql_parser/lexer_new_test.cpp:23: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     LexerNew lexer("SELECT * FROM users WHERE id = 123;");
```

### 问题 260

```
/home/liying/sqlcc/tests/sql_parser/simple_parser_test.cpp:235: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string sql1 = "SELECT * FROM users;";
```

### 问题 261

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:11: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:  * Compares the performance of the new DFA-based parser system
```

### 问题 262

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:146: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "UPDATE products SET price = price * 1.1 WHERE category = 'electronics'",
```

### 问题 263

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:155: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         for (int i = 1; i <= 20 * complexity; ++i) {
```

### 问题 264

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:163: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         for (int i = 2; i <= 5 * complexity; ++i) {
```

### 问题 265

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:170: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         for (int i = 2; i <= 3 * complexity; ++i) {
```

### 问题 266

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:218: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:             // Test new parser
```

### 问题 267

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:273: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double oldThroughput = (oldTotalChars * 1000.0) / oldTotalTime;
```

### 问题 268

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:274: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double newThroughput = (newTotalChars * 1000.0) / newTotalTime;
```

### 问题 269

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:312: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:             // Test new parser
```

### 问题 270

```
/home/liying/sqlcc/tests/sql_parser/lexer_new_benchmark_test.cpp:11: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码: // Benchmark test for comparing old and new lexers
```

### 问题 271

```
/home/liying/sqlcc/tests/sql_parser/lexer_new_benchmark_test.cpp:19: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:           "SELECT * FROM users;", // Simple query
```

### 问题 272

```
/home/liying/sqlcc/tests/sql_parser/lexer_new_benchmark_test.cpp:46: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     // Benchmark new DFA lexer
```

### 问题 273

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:26: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   void expectStatementNotNull(Statement *stmt) {
```

### 问题 274

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:31: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   template <typename T> T *expectExpressionType(Expression *expr) {
```

### 问题 275

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:75: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string sql = "SELECT * FROM users LIMIT 10 OFFSET 20;";
```

### 问题 276

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:133: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string sql = "SELECT name FROM users WHERE EXISTS (SELECT * FROM orders "
```

### 问题 277

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:200: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   CreateStatement *createStmt = static_cast<CreateStatement *>(stmt.get());
```

### 问题 278

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:261: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   CreateStatement *createStmt = static_cast<CreateStatement *>(stmt.get());
```

### 问题 279

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:329: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   UseStatement *useStmt = static_cast<UseStatement *>(stmt.get());
```

### 问题 280

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:335: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string sql = "SELECT * FROM users WHERE age > 18 AND (name LIKE "
```

### 问题 281

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:354: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string sql = "SELECT * FROM users; INSERT INTO logs VALUES (NOW());";
```

### 问题 282

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:395: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string sql = "SELECT * FROM users -- This is a comment\nWHERE age > 18;";
```

### 问题 283

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:617: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "employees GROUP BY department) SELECT * FROM dept_summary;";
```

### 问题 284

```
/home/liying/sqlcc/tests/sql_parser/lexer_new_unit_test.cpp:25: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string input = "SELECT * FROM users WHERE id = 1;";
```

### 问题 285

```
/home/liying/sqlcc/tests/sql_parser/lexer_new_unit_test.cpp:112: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "SELECT * FROM users WHERE id > 10 AND name LIKE 'John%';";
```

### 问题 286

```
/home/liying/sqlcc/tests/sql_parser/lexer_new_unit_test.cpp:205: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string input = "SELECT * FROM users -- This is a comment\nWHERE id = 1;";
```

### 问题 287

```
/home/liying/sqlcc/tests/sql_parser/lexer_new_unit_test.cpp:250: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string input = "SELECT    *    FROM     users    WHERE   id   =   1;";
```

### 问题 288

```
/home/liying/sqlcc/tests/sql_parser/statement_node_test.cpp:149: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         return "SELECT * FROM " + tableName_;
```

### 问题 289

```
/home/liying/sqlcc/tests/sql_parser/statement_node_test.cpp:243: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "SELECT * FROM test WHERE id > 100",
```

### 问题 290

```
/home/liying/sqlcc/tests/sql_parser/lexer_test.cpp:18: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   LexerNew lexer("SELECT * FROM table;");
```

### 问题 291

```
/home/liying/sqlcc/tests/sql_parser/lexer_test.cpp:58: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   LexerNew lexer("SELECT * FROM table WHERE id = 123;");
```

### 问题 292

```
/home/liying/sqlcc/tests/sql_parser/lexer_test.cpp:60: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   // Skip SELECT * FROM table WHERE id =
```

### 问题 293

```
/home/liying/sqlcc/tests/sql_parser/lexer_test.cpp:71: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   LexerNew lexer("SELECT * FROM table WHERE name = 'test';");
```

### 问题 294

```
/home/liying/sqlcc/tests/sql_parser/lexer_test.cpp:73: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   // Skip SELECT * FROM table WHERE name =
```

### 问题 295

```
/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp:85: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   assertParseSuccess("SELECT * FROM users;", "Simple SELECT");
```

### 问题 296

```
/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp:179: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "-- This is a comment\nSELECT * FROM users; -- Another comment",
```

### 问题 297

```
/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp:199: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   assertParseSuccess("SELECT * FROM users WHERE name = 'John';",
```

### 问题 298

```
/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp:203: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   assertParseSuccess("SELECT * FROM products WHERE price > 99.99;",
```

### 问题 299

```
/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp:207: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   assertParseSuccess("SELECT * FROM data WHERE value > 1.23e10;",
```

### 问题 300

```
/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp:222: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   assertParseFailure("SELECT * FROM;", "Missing table name");
```

### 问题 301

```
/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp:225: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   assertParseFailure("SELECT * FROM users WHERE name = 'unterminated;",
```

### 问题 302

```
/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp:229: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   assertParseFailure("SELET * FROM users;", "Typo in keyword");
```

### 问题 303

```
/home/liying/sqlcc/tests/sql_parser/expression_test.cpp:195: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         // Complex expression: 2 * 3 + 5
```

### 问题 304

```
/home/liying/sqlcc/tests/sql_parser/expression_test.cpp:206: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         // 2 + 3 * 4 (should be 2 + (3 * 4))
```

### 问题 305

```
/home/liying/sqlcc/tests/sql_parser/expression_test.cpp:212: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::cout << "✅ 2 + 3 * 4 = " << precExpr1->toString() << std::endl;
```

### 问题 306

```
/home/liying/sqlcc/tests/sql_parser/parser_integration_test.cpp:10: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:  * Tests the integration of the new DFA lexer, ParserNew, and AST system
```

### 问题 307

```
/home/liying/sqlcc/tests/sql_parser/parser_integration_test.cpp:358: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM products WHERE price = 100",
```

### 问题 308

```
/home/liying/sqlcc/tests/sql_parser/parser_new_integration_test.cpp:677: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   MockParserNew parser("SELECT * FROM users;");
```

### 问题 309

```
/home/liying/sqlcc/tests/sql_parser/parser_new_integration_test.cpp:718: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "SELECT * FROM users; INSERT INTO logs VALUES ('test');");
```

### 问题 310

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:55: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "SELECT * FROM users WHERE id IN (SELECT user_id FROM active_users WHERE last_login > '2024-01-01')",
```

### 问题 311

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:80: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         for (int i = 1; i <= complexity * 3; ++i) {
```

### 问题 312

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:109: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             query += " LIMIT " + std::to_string(complexity * 10);
```

### 问题 313

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:287: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double throughput = (total_chars * 1000.0) / total_time; // 字符/秒
```

### 问题 314

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:345: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         if (time_growth <= size_growth * 1.5) {
```

### 问题 315

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:347: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         } else if (time_growth <= size_growth * 2.0) {
```

### 问题 316

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:386: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     double success_rate = (successful_parses * 100.0) / num_queries;
```

### 问题 317

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:432: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         {"SELECT * FROM users WHERE id IN (SELECT user_id FROM active_users)", "IN子查询", true},
```

### 问题 318

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:433: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         {"SELECT * FROM users WHERE EXISTS (SELECT 1 FROM posts WHERE user_id = users.id)", "EXISTS子查询", true},
```

### 问题 319

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:436: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         {"SELECT * FROM users WHERE age BETWEEN 18 AND 65", "BETWEEN表达式", true},
```

### 问题 320

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:437: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         {"SELECT * FROM users WHERE name LIKE 'John%'", "LIKE表达式", true},
```

### 问题 321

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:443: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         {"SELECT * FROM", "不完整FROM", false}
```

### 问题 322

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:479: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     double pass_rate = (passed * 100.0) / (passed + failed);
```

### 问题 323

```
/home/liying/sqlcc/tests/performance/million_insert_test.h:168: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     size_t start_id = thread_id * records_per_thread;
```

### 问题 324

```
/home/liying/sqlcc/tests/performance/disk_io_performance_test.h:73: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     bool SimulatePageRead(int32_t page_id, char* buffer, size_t page_size);
```

### 问题 325

```
/home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test_main.cc:10: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     sql_executor_ = new SqlExecutor();
```

### 问题 326

```
/home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test_main.cc:29: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         delete sql_executor_;
```

### 问题 327

```
/home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test.h:57: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SqlExecutor* sql_executor_;
```

### 问题 328

```
/home/liying/sqlcc/tests/performance/concurrency/concurrency_performance_test.h:171: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SimpleBarrier* start_barrier_;
```

### 问题 329

```
/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h:34: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     static constexpr size_t kMaxMemoryAllocation = 1024 * 1024 * 100; // 100MB
```

### 问题 330

```
/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h:36: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     static constexpr size_t kMediumAllocationSize = 1024 * 10; // 10KB
```

### 问题 331

```
/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h:37: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     static constexpr size_t kLargeAllocationSize = 1024 * 100; // 100KB
```

### 问题 332

```
/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h:40: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     static constexpr size_t kMediumBlockSize = 1024 * 10; // 10KB  
```

### 问题 333

```
/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h:41: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     static constexpr size_t kLargeBlockSize = 1024 * 100; // 100KB
```

### 问题 334

```
/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h:45: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SqlExecutor* sql_executor_;
```

### 问题 335

```
/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.cc:9: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     sql_executor_ = new SqlExecutor();
```

### 问题 336

```
/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.cc:26: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         delete sql_executor_;
```

### 问题 337

```
/home/liying/sqlcc/tests/performance/cpu_test/cpu_intensive_performance_test.h:52: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SqlExecutor* sql_executor_;
```

### 问题 338

```
/home/liying/sqlcc/tests/performance/index_performance_test.h:53: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     IndexManager* index_manager_;
```

### 问题 339

```
/home/liying/sqlcc/tests/performance/buffer_pool_performance_test.h:115: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     sqlcc::SqlExecutor* sql_executor_; // SQL执行器指针
```

### 问题 340

```
/home/liying/sqlcc/tests/performance/basic/cpu_intensive_performance_test.cc:9: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     sql_executor_ = new SqlExecutor();
```

### 问题 341

```
/home/liying/sqlcc/tests/performance/basic/cpu_intensive_performance_test.cc:28: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         delete sql_executor_;
```

### 问题 342

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:77: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string query = "SELECT * FROM " + std::string(kTestDatabase) + "." + std::string(kTestTable) + 
```

### 问题 343

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:109: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string query = "SELECT * FROM " + std::string(kTestDatabase) + "." + std::string(kTestTable) + 
```

### 问题 344

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:160: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         result.p95_latency = result.avg_latency * 1.5; // 占位值
```

### 问题 345

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:161: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         result.p99_latency = result.avg_latency * 2.0; // 占位值
```

### 问题 346

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:190: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         result.p95_latency = result.avg_latency * 1.8; // 占位值
```

### 问题 347

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:191: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         result.p99_latency = result.avg_latency * 2.5; // 占位值
```

### 问题 348

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:224: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         result.p95_latency = result.avg_latency * 1.6; // 占位值
```

### 问题 349

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:225: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         result.p99_latency = result.avg_latency * 2.2; // 占位值
```

### 问题 350

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:253: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         result.p95_latency = result.avg_latency * 2.0; // 占位值
```

### 问题 351

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:254: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         result.p99_latency = result.avg_latency * 3.0; // 占位值
```

### 问题 352

```
/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc:203: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     double throughput = (successful_operations * 1000.0) / duration.count();
```

### 问题 353

```
/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc:261: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     double throughput = (successful_operations * 1000.0) / duration.count();
```

### 问题 354

```
/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc:320: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     double throughput = (successful_operations * 1000.0) / duration.count();
```

### 问题 355

```
/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc:378: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     double throughput = (successful_operations * 1000.0) / duration.count();
```

### 问题 356

```
/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc:436: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     double throughput = (successful_operations * 1000.0) / duration.count();
```

### 问题 357

```
/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc:493: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string select_sql = "SELECT * FROM " + std::string(kTestTable) + 
```

### 问题 358

```
/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc:514: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string select_sql = "SELECT * FROM " + std::string(kTestTable) + 
```

### 问题 359

```
/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc:638: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:               << (static_cast<double>(passed_tests) / total_tests * 100) << "%\n";
```

### 问题 360

```
/home/liying/sqlcc/tests/performance/basic/batch_prefetch_performance_test.cc:121: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             static_cast<int32_t>(region * working_set / 10),
```

### 问题 361

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:149: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             int id = batch * BATCH_SIZE + i + 1;
```

### 问题 362

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:183: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::cout << "Rate: " << (float)LARGE_DATA_SIZE * 1000 / duration.count() 
```

### 问题 363

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:230: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::cout << "Rate: " << (float)QUERIES_TO_RUN * 1000 / duration.count() 
```

### 问题 364

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:334: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::cout << "Rate: " << (float)UPDATES_TO_RUN * 1000 / duration.count() 
```

### 问题 365

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:350: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:     // Measure time for delete operations
```

### 问题 366

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:368: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:     std::cout << "Executed " << DELETES_TO_RUN << " delete operations in " 
```

### 问题 367

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:372: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::cout << "Rate: " << (float)DELETES_TO_RUN * 1000 / duration.count() 
```

### 问题 368

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:399: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM large_test WHERE salary > (SELECT AVG(salary) FROM large_test)",
```

### 问题 369

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:402: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM large_test WHERE name LIKE '%John%' OR email LIKE '%john%' ORDER BY salary DESC LIMIT 100",
```

### 问题 370

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:404: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM large_test WHERE created_at >= DATE_SUB(NOW(), INTERVAL 1 DAY) ORDER BY created_at DESC"
```

### 问题 371

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:102: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     size_t current_batch_size = std::min(BATCH_SIZE, data_size - batch * BATCH_SIZE);
```

### 问题 372

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:108: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       size_t id = batch * BATCH_SIZE + i + 1;
```

### 问题 373

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:196: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   double throughput = (operations_completed * 1000.0) / total_duration.count();
```

### 问题 374

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:228: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql = "SELECT * FROM performance_test_table WHERE id = " + std::to_string(id);
```

### 问题 375

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:251: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   double throughput = (operations_completed * 1000.0) / total_duration.count();
```

### 问题 376

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:284: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql = "SELECT * FROM performance_test_table WHERE id BETWEEN " + 
```

### 问题 377

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:308: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   double throughput = (operations_completed * 1000.0) / total_duration.count();
```

### 问题 378

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:365: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   double throughput = (operations_completed * 1000.0) / total_duration.count();
```

### 问题 379

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:420: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   double throughput = (operations_completed * 1000.0) / total_duration.count();
```

### 问题 380

```
/home/liying/sqlcc/tests/performance/concurrency_test/concurrency_performance_test.h:171: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SimpleBarrier* start_barrier_;
```

### 问题 381

```
/home/liying/sqlcc/tests/legacy/test_dcl_ddl_persistence.cpp:66: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string result10 = executor2.Execute("SELECT * FROM users;");
```

### 问题 382

```
/home/liying/sqlcc/tests/legacy/test_dcl_ddl_persistence.cpp:67: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::cout << "SELECT * FROM users: " << result10 << std::endl;
```

### 问题 383

```
/home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp:433: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: char const* info_compiler = "INFO" ":" "compiler[" COMPILER_ID "]";
```

### 问题 384

```
/home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp:435: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: char const* info_simulate = "INFO" ":" "simulate[" SIMULATE_ID "]";
```

### 问题 385

```
/home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp:439: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: char const* qnxnto = "INFO" ":" "qnxnto[]";
```

### 问题 386

```
/home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp:742: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: char const* info_version = "INFO" ":" "compiler_version[" COMPILER_VERSION "]";
```

### 问题 387

```
/home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp:770: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: char const* info_version_internal = "INFO" ":" "compiler_version_internal[" COMPILER_VERSION_INTERNAL_STR "]";
```

### 问题 388

```
/home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp:795: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: char const* info_platform = "INFO" ":" "platform[" PLATFORM_ID "]";
```

### 问题 389

```
/home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp:796: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: char const* info_arch = "INFO" ":" "arch[" ARCHITECTURE_ID "]";
```

### 问题 390

```
/home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp:844: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char* argv[])
```

### 问题 391

```
/home/liying/sqlcc/tests/sql_executor/sql_executor_comprehensive_test.cpp:28: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string result = executor.Execute("SELECT * FROM test_table");
```

### 问题 392

```
/home/liying/sqlcc/tests/sql_executor/sql_executor_comprehensive_test.cpp:170: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         file << "SELECT * FROM test_table;\n";
```

### 问题 393

```
/home/liying/sqlcc/tests/sql_executor/sql_executor_unit_test.cpp:47: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   result = executor.Execute("SELECT * FROM users");
```

### 问题 394

```
/home/liying/sqlcc/tests/sql_executor/sql_executor_minimal_test.cpp:41: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string result = executor.Execute("SELECT * FROM test_table");
```

### 问题 395

```
/home/liying/sqlcc/tests/client_server/client_test.cpp:34: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   FILE *pipe = popen(command.c_str(), "r");
```

### 问题 396

```
/home/liying/sqlcc/tests/client_server/client_test.cpp:138: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "INSERT INTO test_table VALUES (1, 'test')", "SELECT * FROM test_table",
```

### 问题 397

```
/home/liying/sqlcc/tests/client_server/client_test.cpp:164: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   FILE *pipe = popen(command.c_str(), "r");
```

### 问题 398

```
/home/liying/sqlcc/tests/client_server/encrypted_test_runner.cpp:24: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     FILE* pipe = popen(cmd.c_str(), "r");
```

### 问题 399

```
/home/liying/sqlcc/tests/client_server/encrypted_test_runner.cpp:75: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char* argv[]) {
```

### 问题 400

```
/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp:11: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   static ServerManager *server_manager_;
```

### 问题 401

```
/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp:12: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   static ClientTest *client_test_;
```

### 问题 402

```
/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp:48: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       server_manager_ = new ServerManager(server_path_, port_);
```

### 问题 403

```
/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp:56: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         delete server_manager_;
```

### 问题 404

```
/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp:65: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     client_test_ = new ClientTest(client_path_, "127.0.0.1", port_);
```

### 问题 405

```
/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp:74: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:       delete server_manager_;
```

### 问题 406

```
/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp:81: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:       delete client_test_;
```

### 问题 407

```
/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp:130: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       client_test_->TestQuery(username_, password_, "SELECT * FROM test_table"))
```

### 问题 408

```
/home/liying/sqlcc/tests/client_server/encrypted_integration_test.cpp:19: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   static ServerManager *server_manager_;
```

### 问题 409

```
/home/liying/sqlcc/tests/client_server/encrypted_integration_test.cpp:59: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       server_manager_ = new ServerManager(server_path_, port_);
```

### 问题 410

```
/home/liying/sqlcc/tests/client_server/encrypted_integration_test.cpp:71: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         delete server_manager_;
```

### 问题 411

```
/home/liying/sqlcc/tests/client_server/encrypted_integration_test.cpp:86: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:       delete server_manager_;
```

### 问题 412

```
/home/liying/sqlcc/tests/client_server/encrypted_integration_test.cpp:102: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     FILE *pipe = popen(command.c_str(), "r");
```

### 问题 413

```
/home/liying/sqlcc/comprehensive_test.cpp:50: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     testInput("SELECT * FROM users;", "Basic SELECT statement");
```

### 问题 414

```
/home/liying/sqlcc/test_performance_real.cpp:30: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double throughput = (rows * 1000.0) / duration.count();
```

### 问题 415

```
/home/liying/sqlcc/test_performance_real.cpp:64: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double throughput = (queries * 1000.0) / duration.count();
```

### 问题 416

```
/home/liying/sqlcc/test_performance_real.cpp:89: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double throughput = (updates * 1000.0) / duration.count();
```

### 问题 417

```
/home/liying/sqlcc/test_performance_real.cpp:114: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double throughput = (deletes * 1000.0) / duration.count();
```

### 问题 418

```
/home/liying/sqlcc/test_performance_real.cpp:156: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double throughput = (totalOps * 1000.0) / duration.count();
```

### 问题 419

```
/home/liying/sqlcc/test_performance_real.cpp:158: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double successRate = (successOps * 100.0) / totalOps;
```

### 问题 420

```
/home/liying/sqlcc/test_performance_real.cpp:271: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         volatile int result = id * 2;
```

### 问题 421

```
/home/liying/sqlcc/test_performance_real.cpp:281: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             value = value * 2 + i;
```

### 问题 422

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:15: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: BufferPool::BufferPool(DiskManager *disk_manager, size_t pool_size,
```

### 问题 423

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:70: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: Page *BufferPool::FetchPage(int32_t page_id) {
```

### 问题 424

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:116: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   void *page_data = page->GetData();
```

### 问题 425

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:168: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page_ptr = page.release(); // 释放所有权，获取原始指针
```

### 问题 426

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:195: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* page_ptr = FetchPage(page_id);
```

### 问题 427

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:210: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* page_ptr = NewPage(page_id);
```

### 问题 428

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:286: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = page_it->second.get();
```

### 问题 429

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:287: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   void *page_data = page->GetData();
```

### 问题 430

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:378: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page *page = page_it->second.get();
```

### 问题 431

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:379: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     void *page_data = page->GetData();
```

### 问题 432

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:414: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = page_it->second.get();
```

### 问题 433

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:443: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:   delete page;
```

### 问题 434

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:598: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       Page *page = page_it->second.get();
```

### 问题 435

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:599: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       void *page_data = page->GetData();
```

### 问题 436

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:632: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:               std::chrono::milliseconds(50 * relock_attempts));
```

### 问题 437

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:762: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: Page *BufferPool::NewPage(int32_t *page_id) {
```

### 问题 438

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:767: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     SQLCC_LOG_WARN("Failed to acquire buffer pool lock for creating new page, "
```

### 问题 439

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:783: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:         "Buffer pool is full, replacing page for new page allocation");
```

### 问题 440

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:796: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:           "Failed to replace page in buffer pool for new page allocation";
```

### 问题 441

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:815: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     std::string error_msg = "Failed to allocate new page from disk manager";
```

### 问题 442

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:821: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   SQLCC_LOG_DEBUG("Allocated new page ID " + std::to_string(*page_id) +
```

### 问题 443

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:839: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page_ptr = page.release(); // 释放所有权，获取原始指针
```

### 问题 444

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:896: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       Page *page = page_it->second.get();
```

### 问题 445

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:968: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page *page_ptr = new_pages[i].release(); // 释放所有权，获取原始指针
```

### 问题 446

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:1024: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page *page = pair.second.get();
```

### 问题 447

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:1247: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = new Page();
```

### 问题 448

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:1247: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   Page *page = new Page();
```

### 问题 449

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:1259: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:     delete page;
```

### 问题 450

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:1270: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:     delete page;
```

### 问题 451

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:1277: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:     delete page;
```

### 问题 452

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:1285: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:     delete page;
```

### 问题 453

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:1375: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:            : (dirty_pages * 100) / static_cast<int32_t>(page_table_.size()));
```

### 问题 454

```
/home/liying/sqlcc/src/storage_engine/storage_engine.cpp:40: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: Page *StorageEngine::NewPage(int32_t *page_id) {
```

### 问题 455

```
/home/liying/sqlcc/src/storage_engine/storage_engine.cpp:47: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* page = buffer_pool_->NewPage(&new_page_id);
```

### 问题 456

```
/home/liying/sqlcc/src/storage_engine/storage_engine.cpp:56: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: Page *StorageEngine::FetchPage(int32_t page_id) {
```

### 问题 457

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:57: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     SQLCC_LOG_INFO("Database file does not exist, creating new file: " +
```

### 问题 458

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:125: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:  *       2. 计算偏移量：page_id * PAGE_SIZE
```

### 问题 459

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:279: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: bool DiskManager::ReadPage(int32_t page_id, char *page_data) {
```

### 问题 460

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:409: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   SQLCC_LOG_DEBUG("Allocated new page ID: " + std::to_string(page_id) +
```

### 问题 461

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:510: 文件描述符直接使用 - 建议封装为RAII类
  代码:   int fd = open(db_file_name_.c_str(), O_RDONLY);
```

### 问题 462

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:522: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     char *data = pair.second;
```

### 问题 463

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:566: 文件描述符直接使用 - 建议封装为RAII类
  代码:   int fd = open(db_file_name_.c_str(), O_RDONLY);
```

### 问题 464

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:577: 文件描述符直接使用 - 建议封装为RAII类
  代码:   int result = posix_fadvise(fd, offset, PAGE_SIZE, POSIX_FADV_WILLNEED);
```

### 问题 465

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:617: 文件描述符直接使用 - 建议封装为RAII类
  代码:   int fd = open(db_file_name_.c_str(), O_RDONLY);
```

### 问题 466

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:642: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int result = posix_fadvise(fd, offset, size, POSIX_FADV_WILLNEED);
```

### 问题 467

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_sharded.cpp:7: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: BufferPoolSharded::BufferPoolSharded(DiskManager *disk_manager,
```

### 问题 468

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_sharded.cpp:42: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: Page *BufferPoolSharded::FetchPage(int32_t page_id, bool exclusive) {
```

### 问题 469

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_sharded.cpp:180: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: Page *BufferPoolSharded::NewPage(int32_t *page_id) {
```

### 问题 470

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_sharded.cpp:194: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       SQLCC_LOG_ERROR("Failed to replace page for new page creation");
```

### 问题 471

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:184: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:  * BPlusTreeNode *node = new BPlusTreeNode(storage_engine, page_id, is_leaf);
```

### 问题 472

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:184: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:  * BPlusTreeNode *node = new BPlusTreeNode(storage_engine, page_id, is_leaf);
```

### 问题 473

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:213: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* raw_page = storage_engine_->FetchPage(page_id);
```

### 问题 474

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:266: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:  * BPlusTreeInternalNode *internal_node = new
```

### 问题 475

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:330: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page_->GetData();
```

### 问题 476

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:381: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page_->GetData();
```

### 问题 477

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:613: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   new_node = new BPlusTreeInternalNode(storage_engine_, new_page_id);
```

### 问题 478

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:634: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: void BPlusTreeInternalNode::Merge(BPlusTreeInternalNode *right_node,
```

### 问题 479

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:656: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       Page *temp_page = storage_engine_->FetchPage(child_id);
```

### 问题 480

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:658: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         char *data = temp_page->GetData();
```

### 问题 481

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:664: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:           child_node = new BPlusTreeLeafNode(storage_engine_, child_id);
```

### 问题 482

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:666: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:           child_node = new BPlusTreeInternalNode(storage_engine_, child_id);
```

### 问题 483

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:672: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         delete child_node;
```

### 问题 484

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:702: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:  * BPlusTreeLeafNode *leaf_node = new BPlusTreeLeafNode(storage_engine,
```

### 问题 485

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:702: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:  * BPlusTreeLeafNode *leaf_node = new BPlusTreeLeafNode(storage_engine,
```

### 问题 486

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:765: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page_->GetData();
```

### 问题 487

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:816: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page_->GetData();
```

### 问题 488

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1069: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   new_node = new BPlusTreeLeafNode(storage_engine_, new_page_id);
```

### 问题 489

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1117: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: void BPlusTreeLeafNode::Merge(BPlusTreeLeafNode *right_node) {
```

### 问题 490

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1151: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:  * BPlusTreeIndex *index = new BPlusTreeIndex(storage_engine, table_name,
```

### 问题 491

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1151: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:  * BPlusTreeIndex *index = new BPlusTreeIndex(storage_engine, table_name,
```

### 问题 492

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1240: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   BPlusTreeLeafNode *root_node =
```

### 问题 493

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1241: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       new BPlusTreeLeafNode(storage_engine_, root_page_id_);
```

### 问题 494

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1258: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:   delete root_node;
```

### 问题 495

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1361: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     BPlusTreeInternalNode *new_root =
```

### 问题 496

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1362: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:         new BPlusTreeInternalNode(storage_engine_, new_root_page_id);
```

### 问题 497

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1383: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:     delete new_root;
```

### 问题 498

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1649: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     BPlusTreeLeafNode *leaf = dynamic_cast<BPlusTreeLeafNode *>(node.get());
```

### 问题 499

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1652: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     BPlusTreeInternalNode *internal =
```

### 问题 500

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1666: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     BPlusTreeLeafNode *leaf = dynamic_cast<BPlusTreeLeafNode *>(current_node.get());
```

### 问题 501

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1681: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     BPlusTreeInternalNode *internal =
```

### 问题 502

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1712: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     BPlusTreeLeafNode *leaf = dynamic_cast<BPlusTreeLeafNode *>(current_node.get());
```

### 问题 503

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1716: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     BPlusTreeInternalNode *internal =
```

### 问题 504

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1737: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     BPlusTreeLeafNode *leaf = dynamic_cast<BPlusTreeLeafNode *>(current_node.get());
```

### 问题 505

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1761: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       BPlusTreeNode *next_node = next_node_unique.get();
```

### 问题 506

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1765: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       BPlusTreeLeafNode *next_leaf =
```

### 问题 507

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1768: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         delete next_node;
```

### 问题 508

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1776: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         delete next_leaf;
```

### 问题 509

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1783: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         delete next_leaf;
```

### 问题 510

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1799: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         delete next_leaf;
```

### 问题 511

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1805: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         delete next_leaf;
```

### 问题 512

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1811: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:       delete next_leaf;
```

### 问题 513

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1818: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     BPlusTreeInternalNode *internal =
```

### 问题 514

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1840: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     BPlusTreeLeafNode *leaf = dynamic_cast<BPlusTreeLeafNode *>(current_node.get());
```

### 问题 515

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1863: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     BPlusTreeInternalNode *internal =
```

### 问题 516

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1913: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *temp_page = storage_engine_->FetchPage(page_id);
```

### 问题 517

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:1917: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = temp_page->GetData();
```

### 问题 518

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:115: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = AllocateNewPage(table_name);
```

### 问题 519

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:117: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     SQLCC_LOG_ERROR("Failed to allocate new page for table: " + table_name);
```

### 问题 520

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:142: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = storage_engine_->FetchPage(page_id);
```

### 问题 521

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:167: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = storage_engine_->FetchPage(page_id);
```

### 问题 522

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:193: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = storage_engine_->FetchPage(page_id);
```

### 问题 523

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:241: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page *page = storage_engine_->FetchPage(page_id);
```

### 问题 524

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:262: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = storage_engine_->NewPage(&page_id);
```

### 问题 525

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:273: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: bool TableStorageManager::InitializePage(Page *page,
```

### 问题 526

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:292: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

### 问题 527

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:345: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

### 问题 528

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:360: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: bool TableStorageManager::DeleteRecordInPage(Page *page, size_t offset) {
```

### 问题 529

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:361: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

### 问题 530

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:381: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

### 问题 531

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:412: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

### 问题 532

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:420: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(&header.next_page_id, data + sizeof(PageType) + 2 * sizeof(int32_t),
```

### 问题 533

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:423: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:          data + sizeof(PageType) + 3 * sizeof(int32_t), sizeof(uint16_t));
```

### 问题 534

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:425: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:          data + sizeof(PageType) + 3 * sizeof(int32_t) + sizeof(uint16_t),
```

### 问题 535

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:428: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:          data + sizeof(PageType) + 3 * sizeof(int32_t) + 2 * sizeof(uint16_t),
```

### 问题 536

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:431: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:          data + sizeof(PageType) + 3 * sizeof(int32_t) + 3 * sizeof(uint16_t),
```

### 问题 537

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:437: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: void TableStorageManager::WritePageHeader(Page *page,
```

### 问题 538

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:439: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

### 问题 539

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:446: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(data + sizeof(PageType) + 2 * sizeof(int32_t), &header.next_page_id,
```

### 问题 540

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:448: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(data + sizeof(PageType) + 3 * sizeof(int32_t),
```

### 问题 541

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:450: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(data + sizeof(PageType) + 3 * sizeof(int32_t) + sizeof(uint16_t),
```

### 问题 542

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:452: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(data + sizeof(PageType) + 3 * sizeof(int32_t) + 2 * sizeof(uint16_t),
```

### 问题 543

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:454: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(data + sizeof(PageType) + 3 * sizeof(int32_t) + 3 * sizeof(uint16_t),
```

### 问题 544

```
/home/liying/sqlcc/src/storage_engine/replace_strategy.cpp:140: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     a1in_capacity_ = static_cast<size_t>(a1in_capacity_ * scale);
```

### 问题 545

```
/home/liying/sqlcc/src/storage_engine/replace_strategy.cpp:141: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     am_capacity_ = static_cast<size_t>(am_capacity_ * scale);
```

### 问题 546

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:39: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: BufferPool::BufferPool(DiskManager *disk_manager, size_t pool_size,
```

### 问题 547

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:56: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page *page = pair.second.get();
```

### 问题 548

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:66: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:     delete page;
```

### 问题 549

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:79: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: Page *BufferPool::FetchPage(int32_t page_id) {
```

### 问题 550

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:116: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   // Create new page and load from disk
```

### 问题 551

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:117: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = new Page(page_id);
```

### 问题 552

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:117: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   Page *page = new Page(page_id);
```

### 问题 553

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:127: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:     delete page;
```

### 问题 554

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:137: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:       delete page;
```

### 问题 555

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:143: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:     delete page;
```

### 问题 556

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:187: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码: // Create a new page
```

### 问题 557

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:188: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: Page *BufferPool::NewPage(int32_t *page_id) {
```

### 问题 558

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:191: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     SQLCC_LOG_WARN("Failed to acquire buffer pool lock for creating new page");
```

### 问题 559

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:199: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       SQLCC_LOG_ERROR("No pages available for eviction when creating new page");
```

### 问题 560

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:203: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     if (!ReplacePage(victim_id, -1)) { // -1 means we'll allocate a new page ID
```

### 问题 561

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:210: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     SQLCC_LOG_ERROR("Failed to allocate new page ID from disk manager");
```

### 问题 562

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:214: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = new Page(*page_id);
```

### 问题 563

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:214: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   Page *page = new Page(*page_id);
```

### 问题 564

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:252: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = it->second;
```

### 问题 565

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:366: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page *page = it->second;
```

### 问题 566

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:385: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = it->second;
```

### 问题 567

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:386: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:   delete page;
```

### 问题 568

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:481: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码: // Replace a page with a new one
```

### 问题 569

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:499: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *victim_page = victim_it->second;
```

### 问题 570

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:534: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:   delete victim_page;
```

### 问题 571

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:540: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   // If new_page_id is valid (not -1), we need to load the new page
```

### 问题 572

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:542: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     // Create new page and load from disk
```

### 问题 573

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:543: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page *new_page = new Page(new_page_id);
```

### 问题 574

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:543: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     Page *new_page = new Page(new_page_id);
```

### 问题 575

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:552: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       SQLCC_LOG_ERROR("Failed to reacquire lock after loading new page");
```

### 问题 576

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:553: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:       delete new_page;
```

### 问题 577

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:558: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       SQLCC_LOG_ERROR("Failed to read new page " + std::to_string(new_page_id) +
```

### 问题 578

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:560: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:       delete new_page;
```

### 问题 579

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:564: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     // Add new page to buffer pool
```

### 问题 580

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:20: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char* argv[]) {
```

### 问题 581

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:82: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string query = "SELECT * FROM test_table";
```

### 问题 582

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:113: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* response_header = reinterpret_cast<MessageHeader*>(response.data());
```

### 问题 583

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:143: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* header = reinterpret_cast<MessageHeader*>(connect_msg.data());
```

### 问题 584

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:162: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* resp_header = reinterpret_cast<MessageHeader*>(connect_resp.data());
```

### 问题 585

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:171: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         size_t msg_len = 2 * sizeof(uint32_t) + user_len + pass_len;
```

### 问题 586

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:181: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         char* body = auth_msg.data() + sizeof(MessageHeader);
```

### 问题 587

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:184: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::memcpy(body + 2 * sizeof(uint32_t), username.c_str(), user_len);
```

### 问题 588

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:185: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::memcpy(body + 2 * sizeof(uint32_t) + user_len, password.c_str(), pass_len);
```

### 问题 589

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:199: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* auth_resp_header = reinterpret_cast<MessageHeader*>(auth_resp.data());
```

### 问题 590

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:23: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char* argv[]) {
```

### 问题 591

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:85: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string query = "SELECT * FROM test_table";
```

### 问题 592

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:116: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* response_header = reinterpret_cast<MessageHeader*>(response.data());
```

### 问题 593

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:146: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* header = reinterpret_cast<MessageHeader*>(connect_msg.data());
```

### 问题 594

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:165: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* resp_header = reinterpret_cast<MessageHeader*>(connect_resp.data());
```

### 问题 595

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:174: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         size_t msg_len = 2 * sizeof(uint32_t) + user_len + pass_len;
```

### 问题 596

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:184: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         char* body = auth_msg.data() + sizeof(MessageHeader);
```

### 问题 597

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:187: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::memcpy(body + 2 * sizeof(uint32_t), username.c_str(), user_len);
```

### 问题 598

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:188: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::memcpy(body + 2 * sizeof(uint32_t) + user_len, password.c_str(), pass_len);
```

### 问题 599

```
/home/liying/sqlcc/src/network/encryption.cpp:104: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
```

### 问题 600

```
/home/liying/sqlcc/src/network/encryption.cpp:150: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
```

### 问题 601

```
/home/liying/sqlcc/src/network/network.cpp:146: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         struct hostent* he = gethostbyname(host_.c_str());
```

### 问题 602

```
/home/liying/sqlcc/src/network/network.cpp:314: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* header = reinterpret_cast<MessageHeader*>(msg.data());
```

### 问题 603

```
/home/liying/sqlcc/src/network/network.cpp:333: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* header = reinterpret_cast<MessageHeader*>(resp.data());
```

### 问题 604

```
/home/liying/sqlcc/src/network/network.cpp:359: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     size_t body_size = 2 * sizeof(uint32_t) + username_len + password_len;
```

### 问题 605

```
/home/liying/sqlcc/src/network/network.cpp:363: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* header = reinterpret_cast<MessageHeader*>(message.data());
```

### 问题 606

```
/home/liying/sqlcc/src/network/network.cpp:371: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     char* body = message.data() + sizeof(MessageHeader);
```

### 问题 607

```
/home/liying/sqlcc/src/network/network.cpp:374: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::memcpy(body + 2 * sizeof(uint32_t), username.c_str(), username_len);
```

### 问题 608

```
/home/liying/sqlcc/src/network/network.cpp:375: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::memcpy(body + 2 * sizeof(uint32_t) + username_len, password.c_str(), password_len);
```

### 问题 609

```
/home/liying/sqlcc/src/network/network.cpp:408: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* resp_header = reinterpret_cast<MessageHeader*>(response.data());
```

### 问题 610

```
/home/liying/sqlcc/src/network/network.cpp:506: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int flags = fcntl(fd_.get(), F_GETFL, 0);
```

### 问题 611

```
/home/liying/sqlcc/src/network/network.cpp:517: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: void ConnectionHandler::SetTLS(struct ssl_st* ssl, bool enabled) {
```

### 问题 612

```
/home/liying/sqlcc/src/network/network.cpp:606: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* msg_header = reinterpret_cast<MessageHeader*>(to_send.data());
```

### 问题 613

```
/home/liying/sqlcc/src/network/network.cpp:608: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* header = reinterpret_cast<MessageHeader*>(to_send.data());
```

### 问题 614

```
/home/liying/sqlcc/src/network/network.cpp:651: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
```

### 问题 615

```
/home/liying/sqlcc/src/network/network.cpp:704: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
```

### 问题 616

```
/home/liying/sqlcc/src/network/network.cpp:738: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
```

### 问题 617

```
/home/liying/sqlcc/src/network/network.cpp:746: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     if (header->length < 2 * sizeof(uint32_t)) {
```

### 问题 618

```
/home/liying/sqlcc/src/network/network.cpp:753: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     if (header->length != 2 * sizeof(uint32_t) + username_len + password_len) {
```

### 问题 619

```
/home/liying/sqlcc/src/network/network.cpp:757: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string username(body + 2 * sizeof(uint32_t), username_len);
```

### 问题 620

```
/home/liying/sqlcc/src/network/network.cpp:758: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string password(body + 2 * sizeof(uint32_t) + username_len, password_len);
```

### 问题 621

```
/home/liying/sqlcc/src/network/network.cpp:789: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
```

### 问题 622

```
/home/liying/sqlcc/src/network/network.cpp:821: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
```

### 问题 623

```
/home/liying/sqlcc/src/network/network.cpp:1006: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int nfds = epoll_wait(epoll_fd_.get(), events, 64, 0);
```

### 问题 624

```
/home/liying/sqlcc/src/network/network.cpp:1008: 文件描述符直接使用 - 建议封装为RAII类
  代码:     for (int i = 0; i < nfds; i++) {
```

### 问题 625

```
/home/liying/sqlcc/src/network/network.cpp:1014: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             ConnectionHandler* handler = static_cast<ConnectionHandler*>(events[i].data.ptr);
```

### 问题 626

```
/home/liying/sqlcc/src/network/network.cpp:1021: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:                 delete handler;
```

### 问题 627

```
/home/liying/sqlcc/src/network/network.cpp:1067: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int client_fd = accept4(listen_fd_.get(), (struct sockaddr*)&client_addr, &client_len, SOCK_NONBLOCK);
```

### 问题 628

```
/home/liying/sqlcc/src/network/network.cpp:1074: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ConnectionHandler* handler = new ConnectionHandler(std::move(client_fd_raii), session_manager_, sql_executor_);
```

### 问题 629

```
/home/liying/sqlcc/src/network/network.cpp:1074: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     ConnectionHandler* handler = new ConnectionHandler(std::move(client_fd_raii), session_manager_, sql_executor_);
```

### 问题 630

```
/home/liying/sqlcc/src/network/network.cpp:1078: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         SSL* ssl = SSL_new(ssl_ctx_);
```

### 问题 631

```
/home/liying/sqlcc/src/network/network.cpp:1080: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int flags = fcntl(client_fd, F_GETFL, 0);
```

### 问题 632

```
/home/liying/sqlcc/src/network/network.cpp:1086: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:             delete handler;
```

### 问题 633

```
/home/liying/sqlcc/src/network/network.cpp:1104: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         delete handler;
```

### 问题 634

```
/home/liying/sqlcc/src/sqlcc_server/server_main.cpp:29: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char* argv[]) {
```

### 问题 635

```
/home/liying/sqlcc/src/sqlcc_server/demo_server.cpp:36: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char* argv[]) {
```

### 问题 636

```
/home/liying/sqlcc/src/unified_executor.cpp:513: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: DDLExecutionStrategy::executeCreate(sql_parser::CreateStatement *stmt,
```

### 问题 637

```
/home/liying/sqlcc/src/unified_executor.cpp:550: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: DDLExecutionStrategy::executeDrop(sql_parser::DropStatement *stmt,
```

### 问题 638

```
/home/liying/sqlcc/src/unified_executor.cpp:580: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: DDLExecutionStrategy::executeAlter(sql_parser::AlterStatement *stmt,
```

### 问题 639

```
/home/liying/sqlcc/src/unified_executor.cpp:698: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: DDLExecutionStrategy::executeCreateIndex(sql_parser::CreateIndexStatement *stmt,
```

### 问题 640

```
/home/liying/sqlcc/src/unified_executor.cpp:706: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: DDLExecutionStrategy::executeDropIndex(sql_parser::DropIndexStatement *stmt,
```

### 问题 641

```
/home/liying/sqlcc/src/unified_executor.cpp:828: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: DMLExecutionStrategy::executeInsert(sql_parser::InsertStatement *stmt,
```

### 问题 642

```
/home/liying/sqlcc/src/unified_executor.cpp:931: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: DMLExecutionStrategy::executeUpdate(sql_parser::UpdateStatement *stmt,
```

### 问题 643

```
/home/liying/sqlcc/src/unified_executor.cpp:1013: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: DMLExecutionStrategy::executeDelete(sql_parser::DeleteStatement *stmt,
```

### 问题 644

```
/home/liying/sqlcc/src/unified_executor.cpp:1067: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: DMLExecutionStrategy::executeSelect(sql_parser::SelectStatement *stmt,
```

### 问题 645

```
/home/liying/sqlcc/src/unified_executor.cpp:1196: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: DCLExecutionStrategy::executeCreateUser(sql_parser::CreateUserStatement *stmt,
```

### 问题 646

```
/home/liying/sqlcc/src/unified_executor.cpp:1215: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: DCLExecutionStrategy::executeDropUser(sql_parser::DropUserStatement *stmt,
```

### 问题 647

```
/home/liying/sqlcc/src/unified_executor.cpp:1233: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: DCLExecutionStrategy::executeGrant(sql_parser::GrantStatement *stmt,
```

### 问题 648

```
/home/liying/sqlcc/src/unified_executor.cpp:1246: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: DCLExecutionStrategy::executeRevoke(sql_parser::RevokeStatement *stmt,
```

### 问题 649

```
/home/liying/sqlcc/src/unified_executor.cpp:1288: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: UtilityExecutionStrategy::executeUse(sql_parser::UseStatement *stmt,
```

### 问题 650

```
/home/liying/sqlcc/src/unified_executor.cpp:1302: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: UtilityExecutionStrategy::executeShow(sql_parser::ShowStatement *stmt,
```

### 问题 651

```
/home/liying/sqlcc/src/unified_executor.cpp:1445: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionStrategy *strategy = getStrategy(stmt->getType());
```

### 问题 652

```
/home/liying/sqlcc/src/unified_executor.cpp:1676: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: AdvancedExecutor::executeJoinQuery(sql_parser::SelectStatement *stmt) {
```

### 问题 653

```
/home/liying/sqlcc/src/unified_executor.cpp:1683: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: AdvancedExecutor::executeSubquery(sql_parser::SelectStatement *stmt) {
```

### 问题 654

```
/home/liying/sqlcc/src/unified_executor.cpp:1690: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: AdvancedExecutor::executeWindowFunction(sql_parser::SelectStatement *stmt) {
```

### 问题 655

```
/home/liying/sqlcc/src/core/database_manager.cpp:412: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:                                 Page *page) {
```

### 问题 656

```
/home/liying/sqlcc/src/sql_parser/ast/source_location.cpp:20: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     if (!other.isValid()) return *this;
```

### 问题 657

```
/home/liying/sqlcc/src/sql_parser/lexer_new.cpp:402: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       // Transition to new state
```

### 问题 658

```
/home/liying/sqlcc/src/execution/set_operation_executor.cpp:16: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       memory_limit_(1024 * 1024 * 1024) { // 默认1GB内存限制
```

### 问题 659

```
/home/liying/sqlcc/src/execution/set_operation_executor.cpp:126: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: SetOperationExecutor::execute_subquery(SelectStatement *subquery) {
```

### 问题 660

```
/home/liying/sqlcc/src/bin/isql_main.cpp:155: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char *argv[]) {
```

### 问题 661

```
/home/liying/sqlcc/src/bin/isql_main.cpp:213: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       char *line = readline("");
```

### 问题 662

```
/home/liying/sqlcc/src/sql_executor/user_manager.cpp:38: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: void UserManager::SetSystemDatabase(SystemDatabase* sys_db) {
```

### 问题 663

```
/home/liying/sqlcc/src/sql_executor/index_manager.cpp:8: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: IndexManager::IndexManager(StorageEngine *storage_engine, ConfigManager &)
```

## 建议

1. 使用 `std::unique_ptr` 替代裸指针进行独占所有权管理
2. 使用 `std::shared_ptr` 替代裸指针进行共享所有权管理
3. 使用 `std::make_unique` 和 `std::make_shared` 替代直接使用 `new`
4. 使用 RAII 模式管理资源，避免直接使用 `delete`
5. 将文件描述符等系统资源封装为 RAII 类

