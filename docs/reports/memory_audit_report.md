# SQLCC项目内存审计报告

## 基本信息

- **生成时间**: 2025-12-12 04:34:23
- **审计文件总数**: 532
- **影响文件数**: 116
- **项目根目录**: /home/liying/sqlcc

## 审计结果

### 问题统计

| 文件路径 | 问题数量 | 主要问题类型 |
|---------|---------|-------------|
| `/home/liying/sqlcc/examples/aes_demo.cpp` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/examples/transaction_manager_quick_test.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/include/core/unified_executor.h` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/include/database_manager.h` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/include/disk_manager.h` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/include/execution/set_operation_executor.h` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/include/network/network.h` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/include/page.h` | 6 | 裸指针(6)  |
| `/home/liying/sqlcc/include/sql_parser/set_operation_node.h` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/include/storage/buffer_pool_new.h` | 3 | 裸指针(3)  |
| `/home/liying/sqlcc/include/storage/buffer_pool_v2.h` | 4 | 裸指针(4)  |
| `/home/liying/sqlcc/include/storage/performance_monitor.h` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/include/storage/prefetcher.h` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/include/storage/replace_strategy.h` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/include/storage/table_storage.h` | 3 | 裸指针(3)  |
| `/home/liying/sqlcc/include/storage_engine.h` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/include/utils/file_descriptor.h` | 10 | 裸指针(1) 文件描述符(9)  |
| `/home/liying/sqlcc/include/utils/ssl_wrapper.h` | 12 | 裸指针(12)  |
| `/home/liying/sqlcc/src/bin/isql_main.cpp` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/src/core/database_manager.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/src/dcl_executor.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/src/execution/set_operation_executor.cpp` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/src/isql_network/client_main.cpp` | 9 | 裸指针(9)  |
| `/home/liying/sqlcc/src/isql_network/demo_client.cpp` | 10 | 裸指针(10)  |
| `/home/liying/sqlcc/src/network/encryption.cpp` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/src/network/network.cpp` | 22 | 裸指针(16) 文件描述符(6)  |
| `/home/liying/sqlcc/src/sql_parser/ast/source_location.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/src/sql_parser/lexer_new.cpp` | 1 | new/delete(1)  |
| `/home/liying/sqlcc/src/sql_parser/token.cpp` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/src/sql_parser/token_new.cpp` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/src/sqlcc_server/demo_server.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/src/sqlcc_server/server_main.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp` | 11 | 裸指针(8) new/delete(3)  |
| `/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp` | 10 | 裸指针(5) new/delete(5)  |
| `/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp` | 17 | 裸指针(5) new/delete(12)  |
| `/home/liying/sqlcc/src/storage_engine/buffer_pool_sharded.cpp` | 1 | new/delete(1)  |
| `/home/liying/sqlcc/src/storage_engine/disk_manager.cpp` | 10 | 裸指针(3) new/delete(2) 文件描述符(5)  |
| `/home/liying/sqlcc/src/storage_engine/replace_strategy.cpp` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/src/storage_engine/storage_engine.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/src/storage_engine/table_storage.cpp` | 26 | 裸指针(25) new/delete(1)  |
| `/home/liying/sqlcc/src/unified_executor.cpp` | 2 | 裸指针(1) new/delete(1)  |
| `/home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp` | 8 | 裸指针(8)  |
| `/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp` | 8 | 裸指针(3) new/delete(5)  |
| `/home/liying/sqlcc/tests/client_server/client_test.cpp` | 3 | 裸指针(3)  |
| `/home/liying/sqlcc/tests/client_server/encrypted_integration_test.cpp` | 5 | 裸指针(2) new/delete(3)  |
| `/home/liying/sqlcc/tests/client_server/encrypted_test_runner.cpp` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/tests/components/debug/comprehensive_test.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp` | 8 | 文件描述符(8)  |
| `/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp` | 5 | 裸指针(2) new/delete(2) 文件描述符(1)  |
| `/home/liying/sqlcc/tests/components/debug/memory_safety_test.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/components/debug/test_performance_real.cpp` | 8 | 裸指针(8)  |
| `/home/liying/sqlcc/tests/components/executor/index_optimization_test.cpp` | 4 | 裸指针(4)  |
| `/home/liying/sqlcc/tests/components/executor/index_usage_test.cpp` | 3 | 裸指针(2) new/delete(1)  |
| `/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp` | 5 | 裸指针(5)  |
| `/home/liying/sqlcc/tests/components/executor/privilege_consistency_test.cpp` | 6 | 裸指针(6)  |
| `/home/liying/sqlcc/tests/components/executor/set_operation_test.cpp` | 3 | 裸指针(3)  |
| `/home/liying/sqlcc/tests/components/executor/unified_executor_test.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/components/executor/unsupported_commands_test.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp` | 4 | 裸指针(4)  |
| `/home/liying/sqlcc/tests/components/network/network_unit_test.cpp` | 3 | 裸指针(1) new/delete(2)  |
| `/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp` | 21 | 裸指针(21)  |
| `/home/liying/sqlcc/tests/components/security/test_revoke_persistence.cpp` | 4 | 裸指针(4)  |
| `/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp` | 15 | 裸指针(15)  |
| `/home/liying/sqlcc/tests/components/storage/disk_manager_test.cpp` | 3 | 裸指针(3)  |
| `/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp` | 10 | 裸指针(10)  |
| `/home/liying/sqlcc/tests/integration/advanced_sql/isql_integration_test.cpp` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/tests/integration/advanced_sql/sql_executor_integration_test.cpp` | 3 | 裸指针(3)  |
| `/home/liying/sqlcc/tests/integration/basic_sql/simple_sql_test.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/integration/isql_integration_test.cpp` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/tests/integration/simple_sql_test.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp` | 11 | 裸指针(11)  |
| `/home/liying/sqlcc/tests/integration/sql_executor_integration_test.cpp` | 3 | 裸指针(3)  |
| `/home/liying/sqlcc/tests/legacy/test_dcl_ddl_persistence.cpp` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/tests/network/aes_encryption_test.cc` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/tests/network/aes_network_integration_test.cc` | 14 | 裸指针(14)  |
| `/home/liying/sqlcc/tests/network/sql_network_test.cpp` | 16 | 裸指针(12) 文件描述符(4)  |
| `/home/liying/sqlcc/tests/network/tls_e2e_test.cc` | 8 | 裸指针(8)  |
| `/home/liying/sqlcc/tests/performance/basic/batch_prefetch_performance_test.cc` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc` | 10 | 裸指针(10)  |
| `/home/liying/sqlcc/tests/performance/basic/cpu_intensive_performance_test.cc` | 2 | new/delete(2)  |
| `/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc` | 8 | 裸指针(8)  |
| `/home/liying/sqlcc/tests/performance/buffer_pool_performance_test.h` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/performance/concurrency/concurrency_performance_test.h` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/performance/concurrency_test/concurrency_performance_test.h` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/performance/cpu_test/cpu_intensive_performance_test.h` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc` | 10 | 裸指针(8) new/delete(2)  |
| `/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp` | 9 | 裸指针(9)  |
| `/home/liying/sqlcc/tests/performance/disk_io_performance_test.h` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/performance/index_performance_test.h` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.cc` | 2 | new/delete(2)  |
| `/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h` | 6 | 裸指针(6)  |
| `/home/liying/sqlcc/tests/performance/million_insert_test.h` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test.h` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test_main.cc` | 2 | new/delete(2)  |
| `/home/liying/sqlcc/tests/sql_executor/sql_executor_comprehensive_test.cpp` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/tests/sql_executor/sql_executor_minimal_test.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/sql_executor/sql_executor_unit_test.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/sql_parser/error_integration_test.cpp` | 6 | 裸指针(6)  |
| `/home/liying/sqlcc/tests/sql_parser/expression_test.cpp` | 3 | 裸指针(3)  |
| `/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp` | 8 | 裸指针(8)  |
| `/home/liying/sqlcc/tests/sql_parser/lexer_new_benchmark_test.cpp` | 3 | 裸指针(1) new/delete(2)  |
| `/home/liying/sqlcc/tests/sql_parser/lexer_new_test.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/sql_parser/lexer_new_unit_test.cpp` | 4 | 裸指针(4)  |
| `/home/liying/sqlcc/tests/sql_parser/lexer_test.cpp` | 5 | 裸指针(5)  |
| `/home/liying/sqlcc/tests/sql_parser/parser_integration_test.cpp` | 2 | 裸指针(1) new/delete(1)  |
| `/home/liying/sqlcc/tests/sql_parser/parser_new_integration_test.cpp` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp` | 13 | 裸指针(13)  |
| `/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp` | 9 | 裸指针(6) new/delete(3)  |
| `/home/liying/sqlcc/tests/sql_parser/simple_parser_test.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp` | 11 | 裸指针(11)  |
| `/home/liying/sqlcc/tests/sql_parser/statement_node_test.cpp` | 2 | 裸指针(2)  |
| `/home/liying/sqlcc/tests/sql_parser/token_new_unit_test.cpp` | 3 | 裸指针(3)  |
| `/home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp` | 5 | 裸指针(5)  |
| `/home/liying/sqlcc/tests/test_disk_manager.h` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_output.cpp` | 1 | 裸指针(1)  |
| `/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_simple.cpp` | 1 | 裸指针(1)  |

### 详细问题列表

#### 📁 /home/liying/sqlcc/examples/aes_demo.cpp

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/examples/aes_demo.cpp:66: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string plaintext = "SELECT * FROM users WHERE id = 1;";
```

**问题 2**:

```
/home/liying/sqlcc/examples/aes_demo.cpp:118: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM users WHERE name LIKE 'A%';"
```

#### 📁 /home/liying/sqlcc/examples/transaction_manager_quick_test.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/examples/transaction_manager_quick_test.cpp:232: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   int expected_successes = NUM_THREADS * TXNS_PER_THREAD;
```

#### 📁 /home/liying/sqlcc/include/core/unified_executor.h

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/include/core/unified_executor.h:456: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionStrategy *getStrategy(sql_parser::Statement::Type type);
```

#### 📁 /home/liying/sqlcc/include/database_manager.h

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/include/database_manager.h:80: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   bool WritePage(TransactionId txn_id, int32_t page_id, Page *page);
```

#### 📁 /home/liying/sqlcc/include/disk_manager.h

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/include/disk_manager.h:52: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   bool ReadPage(int32_t page_id, char *page_data);
```

#### 📁 /home/liying/sqlcc/include/execution/set_operation_executor.h

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/include/execution/set_operation_executor.h:81: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionResult execute_subquery(SelectStatement *subquery);
```

#### 📁 /home/liying/sqlcc/include/network/network.h

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/include/network/network.h:184: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     void SetTLS(struct ssl_st* ssl, bool enabled);
```

#### 📁 /home/liying/sqlcc/include/page.h

**问题数量**: 6

**问题 1**:

```
/home/liying/sqlcc/include/page.h:81: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     [[deprecated("Use GetDataSpan() for safe access")]] inline char* GetData() { return data_; }
```

**问题 2**:

```
/home/liying/sqlcc/include/page.h:108: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         char* data;
```

**问题 3**:

```
/home/liying/sqlcc/include/page.h:110: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         char* begin() { return data; }
```

**问题 4**:

```
/home/liying/sqlcc/include/page.h:111: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         char* end() { return data + size; }
```

**问题 5**:

```
/home/liying/sqlcc/include/page.h:143: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     [[deprecated("Use ReadDataToSpan() for safe read operations")]] void ReadData(size_t offset, char* data, size_t size) const;
```

**问题 6**:

```
/home/liying/sqlcc/include/page.h:166: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     void ReadDataToSpan(size_t offset, void* output_data, size_t size) const;
```

#### 📁 /home/liying/sqlcc/include/sql_parser/set_operation_node.h

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/include/sql_parser/set_operation_node.h:48: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SelectStatement* getLeftOperand() const;
```

**问题 2**:

```
/home/liying/sqlcc/include/sql_parser/set_operation_node.h:49: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SelectStatement* getRightOperand() const;
```

#### 📁 /home/liying/sqlcc/include/storage/buffer_pool_new.h

**问题数量**: 3

**问题 1**:

```
/home/liying/sqlcc/include/storage/buffer_pool_new.h:47: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     explicit BufferPool(DiskManager* disk_manager, size_t pool_size, ConfigManager& config_manager);
```

**问题 2**:

```
/home/liying/sqlcc/include/storage/buffer_pool_new.h:51: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* FetchPage(int32_t page_id);
```

**问题 3**:

```
/home/liying/sqlcc/include/storage/buffer_pool_new.h:53: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* NewPage(int32_t* page_id);
```

#### 📁 /home/liying/sqlcc/include/storage/buffer_pool_v2.h

**问题数量**: 4

**问题 1**:

```
/home/liying/sqlcc/include/storage/buffer_pool_v2.h:17: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     explicit BufferPoolV2(DiskManager* disk_manager, size_t pool_size);
```

**问题 2**:

```
/home/liying/sqlcc/include/storage/buffer_pool_v2.h:23: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* FetchPage(int32_t page_id);
```

**问题 3**:

```
/home/liying/sqlcc/include/storage/buffer_pool_v2.h:25: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* NewPage(int32_t* page_id);
```

**问题 4**:

```
/home/liying/sqlcc/include/storage/buffer_pool_v2.h:58: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     DiskManager* disk_manager_;
```

#### 📁 /home/liying/sqlcc/include/storage/performance_monitor.h

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/include/storage/performance_monitor.h:58: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     return *this;
```

**问题 2**:

```
/home/liying/sqlcc/include/storage/performance_monitor.h:67: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     return *this;
```

#### 📁 /home/liying/sqlcc/include/storage/prefetcher.h

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/include/storage/prefetcher.h:89: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       return *this;
```

**问题 2**:

```
/home/liying/sqlcc/include/storage/prefetcher.h:101: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       return *this;
```

#### 📁 /home/liying/sqlcc/include/storage/replace_strategy.h

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/include/storage/replace_strategy.h:67: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       return *this;
```

#### 📁 /home/liying/sqlcc/include/storage/table_storage.h

**问题数量**: 3

**问题 1**:

```
/home/liying/sqlcc/include/storage/table_storage.h:111: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     bool DeleteRecordInPage(class Page* page, size_t offset);
```

**问题 2**:

```
/home/liying/sqlcc/include/storage/table_storage.h:112: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::vector<std::string> GetRecordFromPage(class Page* page, size_t offset) const;
```

**问题 3**:

```
/home/liying/sqlcc/include/storage/table_storage.h:116: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     PageHeader ReadPageHeader(class Page* page) const;
```

#### 📁 /home/liying/sqlcc/include/storage_engine.h

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/include/storage_engine.h:31: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:  * Page* page = storage_engine.NewPage();
```

**问题 2**:

```
/home/liying/sqlcc/include/storage_engine.h:105: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* FetchPage(int32_t page_id);
```

#### 📁 /home/liying/sqlcc/include/utils/file_descriptor.h

**问题数量**: 10

**问题 1**:

```
/home/liying/sqlcc/include/utils/file_descriptor.h:29: 文件描述符直接使用 - 建议封装为RAII类
  代码:     explicit FileDescriptor(int fd) noexcept : fd_(fd) {}
```

**问题 2**:

```
/home/liying/sqlcc/include/utils/file_descriptor.h:59: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         return *this;
```

**问题 3**:

```
/home/liying/sqlcc/include/utils/file_descriptor.h:89: 文件描述符直接使用 - 建议封装为RAII类
  代码:     void reset(int fd = -1) noexcept {
```

**问题 4**:

```
/home/liying/sqlcc/include/utils/file_descriptor.h:99: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int temp = fd_;
```

**问题 5**:

```
/home/liying/sqlcc/include/utils/file_descriptor.h:121: 文件描述符直接使用 - 建议封装为RAII类
  代码:     static FileDescriptor create_socket(int domain, int type, int protocol) {
```

**问题 6**:

```
/home/liying/sqlcc/include/utils/file_descriptor.h:122: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int fd = ::socket(domain, type, protocol);
```

**问题 7**:

```
/home/liying/sqlcc/include/utils/file_descriptor.h:151: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int fd = ::epoll_create1(flags);
```

**问题 8**:

```
/home/liying/sqlcc/include/utils/file_descriptor.h:166: 文件描述符直接使用 - 建议封装为RAII类
  代码:     static FileDescriptor accept(int sockfd, struct sockaddr* addr = nullptr,
```

**问题 9**:

```
/home/liying/sqlcc/include/utils/file_descriptor.h:168: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int fd = ::accept4(sockfd, addr, addrlen, flags);
```

**问题 10**:

```
/home/liying/sqlcc/include/utils/file_descriptor.h:176: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int fd_;  ///< File descriptor value
```

#### 📁 /home/liying/sqlcc/include/utils/ssl_wrapper.h

**问题数量**: 12

**问题 1**:

```
/home/liying/sqlcc/include/utils/ssl_wrapper.h:31: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     explicit SSLContext(SSL_CTX* ctx) : ctx_(ctx) {}
```

**问题 2**:

```
/home/liying/sqlcc/include/utils/ssl_wrapper.h:54: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         return *this;
```

**问题 3**:

```
/home/liying/sqlcc/include/utils/ssl_wrapper.h:68: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SSL_CTX* release() {
```

**问题 4**:

```
/home/liying/sqlcc/include/utils/ssl_wrapper.h:69: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         SSL_CTX* ctx = ctx_;
```

**问题 5**:

```
/home/liying/sqlcc/include/utils/ssl_wrapper.h:88: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SSL_CTX* ctx_;
```

**问题 6**:

```
/home/liying/sqlcc/include/utils/ssl_wrapper.h:98: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     explicit SSLSocket(SSL* ssl) : ssl_(ssl) {}
```

**问题 7**:

```
/home/liying/sqlcc/include/utils/ssl_wrapper.h:121: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         return *this;
```

**问题 8**:

```
/home/liying/sqlcc/include/utils/ssl_wrapper.h:135: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SSL* release() {
```

**问题 9**:

```
/home/liying/sqlcc/include/utils/ssl_wrapper.h:136: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         SSL* ssl = ssl_;
```

**问题 10**:

```
/home/liying/sqlcc/include/utils/ssl_wrapper.h:150: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     static SSLSocket create(SSL_CTX* ctx) {
```

**问题 11**:

```
/home/liying/sqlcc/include/utils/ssl_wrapper.h:162: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SSL* ssl_;
```

**问题 12**:

```
/home/liying/sqlcc/include/utils/ssl_wrapper.h:189: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     static SSLSocket create(void* ctx) {
```

#### 📁 /home/liying/sqlcc/src/bin/isql_main.cpp

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/src/bin/isql_main.cpp:155: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char *argv[]) {
```

**问题 2**:

```
/home/liying/sqlcc/src/bin/isql_main.cpp:213: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       char *line = readline("");
```

#### 📁 /home/liying/sqlcc/src/core/database_manager.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/src/core/database_manager.cpp:424: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:                                 Page *page) {
```

#### 📁 /home/liying/sqlcc/src/dcl_executor.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/src/dcl_executor.cpp:130: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: ExecutionResult DCLExecutor::executeRevoke(sql_parser::RevokeStatement* stmt) {
```

#### 📁 /home/liying/sqlcc/src/execution/set_operation_executor.cpp

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/src/execution/set_operation_executor.cpp:16: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       memory_limit_(1024 * 1024 * 1024) { // 默认1GB内存限制
```

**问题 2**:

```
/home/liying/sqlcc/src/execution/set_operation_executor.cpp:126: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: SetOperationExecutor::execute_subquery(SelectStatement *subquery) {
```

#### 📁 /home/liying/sqlcc/src/isql_network/client_main.cpp

**问题数量**: 9

**问题 1**:

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:23: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char* argv[]) {
```

**问题 2**:

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:85: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string query = "SELECT * FROM test_table";
```

**问题 3**:

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:116: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* response_header = reinterpret_cast<MessageHeader*>(response.data());
```

**问题 4**:

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:146: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* header = reinterpret_cast<MessageHeader*>(connect_msg.data());
```

**问题 5**:

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:165: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* resp_header = reinterpret_cast<MessageHeader*>(connect_resp.data());
```

**问题 6**:

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:174: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         size_t msg_len = 2 * sizeof(uint32_t) + user_len + pass_len;
```

**问题 7**:

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:184: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         char* body = auth_msg.data() + sizeof(MessageHeader);
```

**问题 8**:

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:187: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::memcpy(body + 2 * sizeof(uint32_t), username.c_str(), user_len);
```

**问题 9**:

```
/home/liying/sqlcc/src/isql_network/client_main.cpp:188: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::memcpy(body + 2 * sizeof(uint32_t) + user_len, password.c_str(), pass_len);
```

#### 📁 /home/liying/sqlcc/src/isql_network/demo_client.cpp

**问题数量**: 10

**问题 1**:

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:20: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char* argv[]) {
```

**问题 2**:

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:82: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string query = "SELECT * FROM test_table";
```

**问题 3**:

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:113: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* response_header = reinterpret_cast<MessageHeader*>(response.data());
```

**问题 4**:

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:143: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* header = reinterpret_cast<MessageHeader*>(connect_msg.data());
```

**问题 5**:

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:162: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* resp_header = reinterpret_cast<MessageHeader*>(connect_resp.data());
```

**问题 6**:

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:171: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         size_t msg_len = 2 * sizeof(uint32_t) + user_len + pass_len;
```

**问题 7**:

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:181: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         char* body = auth_msg.data() + sizeof(MessageHeader);
```

**问题 8**:

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:184: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::memcpy(body + 2 * sizeof(uint32_t), username.c_str(), user_len);
```

**问题 9**:

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:185: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::memcpy(body + 2 * sizeof(uint32_t) + user_len, password.c_str(), pass_len);
```

**问题 10**:

```
/home/liying/sqlcc/src/isql_network/demo_client.cpp:199: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* auth_resp_header = reinterpret_cast<MessageHeader*>(auth_resp.data());
```

#### 📁 /home/liying/sqlcc/src/network/encryption.cpp

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/src/network/encryption.cpp:104: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
```

**问题 2**:

```
/home/liying/sqlcc/src/network/encryption.cpp:150: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
```

#### 📁 /home/liying/sqlcc/src/network/network.cpp

**问题数量**: 22

**问题 1**:

```
/home/liying/sqlcc/src/network/network.cpp:367: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     size_t body_size = 2 * sizeof(uint32_t) + username_len + password_len;
```

**问题 2**:

```
/home/liying/sqlcc/src/network/network.cpp:383: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::memcpy(body_span.data() + 2 * sizeof(uint32_t), username.c_str(), username_len);
```

**问题 3**:

```
/home/liying/sqlcc/src/network/network.cpp:384: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::memcpy(body_span.data() + 2 * sizeof(uint32_t) + username_len, password.c_str(), password_len);
```

**问题 4**:

```
/home/liying/sqlcc/src/network/network.cpp:516: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int flags = fcntl(fd_.get(), F_GETFL, 0);
```

**问题 5**:

```
/home/liying/sqlcc/src/network/network.cpp:523: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: void ConnectionHandler::SetTLS(struct ssl_st* ssl, bool enabled) {
```

**问题 6**:

```
/home/liying/sqlcc/src/network/network.cpp:622: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* msg_header = reinterpret_cast<MessageHeader*>(to_send.data());
```

**问题 7**:

```
/home/liying/sqlcc/src/network/network.cpp:624: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* header = reinterpret_cast<MessageHeader*>(to_send.data());
```

**问题 8**:

```
/home/liying/sqlcc/src/network/network.cpp:667: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
```

**问题 9**:

```
/home/liying/sqlcc/src/network/network.cpp:720: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
```

**问题 10**:

```
/home/liying/sqlcc/src/network/network.cpp:754: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
```

**问题 11**:

```
/home/liying/sqlcc/src/network/network.cpp:762: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     if (header->length < 2 * sizeof(uint32_t)) {
```

**问题 12**:

```
/home/liying/sqlcc/src/network/network.cpp:769: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     if (header->length != 2 * sizeof(uint32_t) + username_len + password_len) {
```

**问题 13**:

```
/home/liying/sqlcc/src/network/network.cpp:773: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string username(body + 2 * sizeof(uint32_t), username_len);
```

**问题 14**:

```
/home/liying/sqlcc/src/network/network.cpp:774: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string password(body + 2 * sizeof(uint32_t) + username_len, password_len);
```

**问题 15**:

```
/home/liying/sqlcc/src/network/network.cpp:805: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
```

**问题 16**:

```
/home/liying/sqlcc/src/network/network.cpp:837: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
```

**问题 17**:

```
/home/liying/sqlcc/src/network/network.cpp:1017: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int nfds = epoll_wait(epoll_fd_.get(), events, 64, 0);
```

**问题 18**:

```
/home/liying/sqlcc/src/network/network.cpp:1019: 文件描述符直接使用 - 建议封装为RAII类
  代码:     for (int i = 0; i < nfds; i++) {
```

**问题 19**:

```
/home/liying/sqlcc/src/network/network.cpp:1025: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             ConnectionHandler* handler = static_cast<ConnectionHandler*>(events[i].data.ptr);
```

**问题 20**:

```
/home/liying/sqlcc/src/network/network.cpp:1030: 文件描述符直接使用 - 建议封装为RAII类
  代码:                 int fd = handler->GetFd();
```

**问题 21**:

```
/home/liying/sqlcc/src/network/network.cpp:1089: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int fd = handler->GetFd(); // 获取文件描述符值用于epoll
```

**问题 22**:

```
/home/liying/sqlcc/src/network/network.cpp:1098: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int flags = fcntl(fd, F_GETFL, 0);
```

#### 📁 /home/liying/sqlcc/src/sql_parser/ast/source_location.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/src/sql_parser/ast/source_location.cpp:20: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     if (!other.isValid()) return *this;
```

#### 📁 /home/liying/sqlcc/src/sql_parser/lexer_new.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/src/sql_parser/lexer_new.cpp:402: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       // Transition to new state
```

#### 📁 /home/liying/sqlcc/src/sql_parser/token.cpp

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/src/sql_parser/token.cpp:26: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     return *this;
```

**问题 2**:

```
/home/liying/sqlcc/src/sql_parser/token.cpp:51: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     return *this;
```

#### 📁 /home/liying/sqlcc/src/sql_parser/token_new.cpp

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/src/sql_parser/token_new.cpp:26: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     return *this;
```

**问题 2**:

```
/home/liying/sqlcc/src/sql_parser/token_new.cpp:51: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     return *this;
```

#### 📁 /home/liying/sqlcc/src/sqlcc_server/demo_server.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/src/sqlcc_server/demo_server.cpp:36: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char* argv[]) {
```

#### 📁 /home/liying/sqlcc/src/sqlcc_server/server_main.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/src/sqlcc_server/server_main.cpp:29: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char* argv[]) {
```

#### 📁 /home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp

**问题数量**: 11

**问题 1**:

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:44: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:  * BPlusTreeNode *node = new BPlusTreeNode(storage_engine, page_id, is_leaf);
```

**问题 2**:

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:44: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:  * BPlusTreeNode *node = new BPlusTreeNode(storage_engine, page_id, is_leaf);
```

**问题 3**:

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:127: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:  * BPlusTreeInternalNode *internal_node = new
```

**问题 4**:

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:191: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page_->GetData();
```

**问题 5**:

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:242: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page_->GetData();
```

**问题 6**:

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:465: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:  * BPlusTreeLeafNode *leaf_node = new BPlusTreeLeafNode(storage_engine,
```

**问题 7**:

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:465: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:  * BPlusTreeLeafNode *leaf_node = new BPlusTreeLeafNode(storage_engine,
```

**问题 8**:

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:528: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page_->GetData();
```

**问题 9**:

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:579: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page_->GetData();
```

**问题 10**:

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:893: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:  * BPlusTreeIndex *index = new BPlusTreeIndex(storage_engine, table_name,
```

**问题 11**:

```
/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp:893: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:  * BPlusTreeIndex *index = new BPlusTreeIndex(storage_engine, table_name,
```

#### 📁 /home/liying/sqlcc/src/storage_engine/buffer_pool.cpp

**问题数量**: 10

**问题 1**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:116: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char* page_data = static_cast<char*>(page->GetData());
```

**问题 2**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:497: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = page_it->second.get();
```

**问题 3**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:678: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       Page *page = page_it->second.get();
```

**问题 4**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:679: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       void *page_data = page->GetData();
```

**问题 5**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:712: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:               std::chrono::milliseconds(50 * relock_attempts));
```

**问题 6**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:847: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     SQLCC_LOG_WARN("Failed to acquire buffer pool lock for creating new page, "
```

**问题 7**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:863: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:         "Buffer pool is full, replacing page for new page allocation");
```

**问题 8**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:876: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:           "Failed to replace page in buffer pool for new page allocation";
```

**问题 9**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:895: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   std::string error_msg = "Failed to allocate new page from disk manager";
```

**问题 10**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp:904: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   SQLCC_LOG_DEBUG("Allocated new page ID " + std::to_string(*page_id) +
```

#### 📁 /home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp

**问题数量**: 17

**问题 1**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:39: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: BufferPool::BufferPool(DiskManager *disk_manager, size_t pool_size,
```

**问题 2**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:77: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: Page *BufferPool::FetchPage(int32_t page_id) {
```

**问题 3**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:114: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   // Create new page and load from disk
```

**问题 4**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:182: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码: // Create a new page
```

**问题 5**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:183: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: Page *BufferPool::NewPage(int32_t *page_id) {
```

**问题 6**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:186: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     SQLCC_LOG_WARN("Failed to acquire buffer pool lock for creating new page");
```

**问题 7**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:194: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       SQLCC_LOG_ERROR("No pages available for eviction when creating new page");
```

**问题 8**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:198: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     if (!ReplacePage(victim_id, -1)) { // -1 means we'll allocate a new page ID
```

**问题 9**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:205: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     SQLCC_LOG_ERROR("Failed to allocate new page ID from disk manager");
```

**问题 10**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:247: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = it->second;
```

**问题 11**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:473: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码: // Replace a page with a new one
```

**问题 12**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:491: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *victim_page = victim_it->second;
```

**问题 13**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:529: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   // If new_page_id is valid (not -1), we need to load the new page
```

**问题 14**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:531: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     // Create new page and load from disk
```

**问题 15**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:541: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       SQLCC_LOG_ERROR("Failed to reacquire lock after loading new page");
```

**问题 16**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:546: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       SQLCC_LOG_ERROR("Failed to read new page " + std::to_string(new_page_id) +
```

**问题 17**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp:551: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     // Add new page to buffer pool
```

#### 📁 /home/liying/sqlcc/src/storage_engine/buffer_pool_sharded.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/src/storage_engine/buffer_pool_sharded.cpp:198: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       SQLCC_LOG_ERROR("Failed to replace page for new page creation");
```

#### 📁 /home/liying/sqlcc/src/storage_engine/disk_manager.cpp

**问题数量**: 10

**问题 1**:

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:57: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     SQLCC_LOG_INFO("Database file does not exist, creating new file: " +
```

**问题 2**:

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:125: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:  *       2. 计算偏移量：page_id * PAGE_SIZE
```

**问题 3**:

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:279: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: bool DiskManager::ReadPage(int32_t page_id, char *page_data) {
```

**问题 4**:

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:409: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   SQLCC_LOG_DEBUG("Allocated new page ID: " + std::to_string(page_id) +
```

**问题 5**:

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:510: 文件描述符直接使用 - 建议封装为RAII类
  代码:   int fd = open(db_file_name_.c_str(), O_RDONLY);
```

**问题 6**:

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:522: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     char *data = pair.second;
```

**问题 7**:

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:566: 文件描述符直接使用 - 建议封装为RAII类
  代码:   int fd = open(db_file_name_.c_str(), O_RDONLY);
```

**问题 8**:

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:577: 文件描述符直接使用 - 建议封装为RAII类
  代码:   int result = posix_fadvise(fd, offset, PAGE_SIZE, POSIX_FADV_WILLNEED);
```

**问题 9**:

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:617: 文件描述符直接使用 - 建议封装为RAII类
  代码:   int fd = open(db_file_name_.c_str(), O_RDONLY);
```

**问题 10**:

```
/home/liying/sqlcc/src/storage_engine/disk_manager.cpp:642: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int result = posix_fadvise(fd, offset, size, POSIX_FADV_WILLNEED);
```

#### 📁 /home/liying/sqlcc/src/storage_engine/replace_strategy.cpp

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/src/storage_engine/replace_strategy.cpp:140: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     a1in_capacity_ = static_cast<size_t>(a1in_capacity_ * scale);
```

**问题 2**:

```
/home/liying/sqlcc/src/storage_engine/replace_strategy.cpp:141: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     am_capacity_ = static_cast<size_t>(am_capacity_ * scale);
```

#### 📁 /home/liying/sqlcc/src/storage_engine/storage_engine.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/src/storage_engine/storage_engine.cpp:60: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: Page* StorageEngine::FetchPage(int32_t page_id) {
```

#### 📁 /home/liying/sqlcc/src/storage_engine/table_storage.cpp

**问题数量**: 26

**问题 1**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:118: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = AllocateNewPage(table_name);
```

**问题 2**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:120: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     SQLCC_LOG_ERROR("Failed to allocate new page for table: " + table_name);
```

**问题 3**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:145: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = storage_engine_->FetchPage(page_id);
```

**问题 4**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:170: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = storage_engine_->FetchPage(page_id);
```

**问题 5**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:196: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = storage_engine_->FetchPage(page_id);
```

**问题 6**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:244: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page *page = storage_engine_->FetchPage(page_id);
```

**问题 7**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:270: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* page = page_ptr.release();  // 释放unique_ptr的所有权
```

**问题 8**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:278: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: bool TableStorageManager::InitializePage(Page *page,
```

**问题 9**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:297: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

**问题 10**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:350: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

**问题 11**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:365: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: bool TableStorageManager::DeleteRecordInPage(Page *page, size_t offset) {
```

**问题 12**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:366: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

**问题 13**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:386: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

**问题 14**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:417: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

**问题 15**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:425: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(&header.next_page_id, data + sizeof(PageType) + 2 * sizeof(int32_t),
```

**问题 16**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:428: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:          data + sizeof(PageType) + 3 * sizeof(int32_t), sizeof(uint16_t));
```

**问题 17**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:430: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:          data + sizeof(PageType) + 3 * sizeof(int32_t) + sizeof(uint16_t),
```

**问题 18**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:433: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:          data + sizeof(PageType) + 3 * sizeof(int32_t) + 2 * sizeof(uint16_t),
```

**问题 19**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:436: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:          data + sizeof(PageType) + 3 * sizeof(int32_t) + 3 * sizeof(uint16_t),
```

**问题 20**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:442: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: void TableStorageManager::WritePageHeader(Page *page,
```

**问题 21**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:444: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

**问题 22**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:451: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(data + sizeof(PageType) + 2 * sizeof(int32_t), &header.next_page_id,
```

**问题 23**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:453: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(data + sizeof(PageType) + 3 * sizeof(int32_t),
```

**问题 24**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:455: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(data + sizeof(PageType) + 3 * sizeof(int32_t) + sizeof(uint16_t),
```

**问题 25**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:457: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(data + sizeof(PageType) + 3 * sizeof(int32_t) + 2 * sizeof(uint16_t),
```

**问题 26**:

```
/home/liying/sqlcc/src/storage_engine/table_storage.cpp:459: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(data + sizeof(PageType) + 3 * sizeof(int32_t) + 3 * sizeof(uint16_t),
```

#### 📁 /home/liying/sqlcc/src/unified_executor.cpp

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/src/unified_executor.cpp:1232: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:     return {false, "Invalid delete statement"};
```

**问题 2**:

```
/home/liying/sqlcc/src/unified_executor.cpp:1914: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionStrategy *strategy = getStrategy(stmt->getType());
```

#### 📁 /home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp

**问题数量**: 8

**问题 1**:

```
/home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp:433: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: char const* info_compiler = "INFO" ":" "compiler[" COMPILER_ID "]";
```

**问题 2**:

```
/home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp:435: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: char const* info_simulate = "INFO" ":" "simulate[" SIMULATE_ID "]";
```

**问题 3**:

```
/home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp:439: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: char const* qnxnto = "INFO" ":" "qnxnto[]";
```

**问题 4**:

```
/home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp:742: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: char const* info_version = "INFO" ":" "compiler_version[" COMPILER_VERSION "]";
```

**问题 5**:

```
/home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp:770: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: char const* info_version_internal = "INFO" ":" "compiler_version_internal[" COMPILER_VERSION_INTERNAL_STR "]";
```

**问题 6**:

```
/home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp:795: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: char const* info_platform = "INFO" ":" "platform[" PLATFORM_ID "]";
```

**问题 7**:

```
/home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp:796: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: char const* info_arch = "INFO" ":" "arch[" ARCHITECTURE_ID "]";
```

**问题 8**:

```
/home/liying/sqlcc/tests/CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp:844: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char* argv[])
```

#### 📁 /home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp

**问题数量**: 8

**问题 1**:

```
/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp:11: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   static ServerManager *server_manager_;
```

**问题 2**:

```
/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp:12: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   static ClientTest *client_test_;
```

**问题 3**:

```
/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp:48: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       server_manager_ = new ServerManager(server_path_, port_);
```

**问题 4**:

```
/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp:56: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         delete server_manager_;
```

**问题 5**:

```
/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp:65: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     client_test_ = new ClientTest(client_path_, "127.0.0.1", port_);
```

**问题 6**:

```
/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp:74: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:       delete server_manager_;
```

**问题 7**:

```
/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp:81: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:       delete client_test_;
```

**问题 8**:

```
/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp:130: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       client_test_->TestQuery(username_, password_, "SELECT * FROM test_table"))
```

#### 📁 /home/liying/sqlcc/tests/client_server/client_test.cpp

**问题数量**: 3

**问题 1**:

```
/home/liying/sqlcc/tests/client_server/client_test.cpp:34: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   FILE *pipe = popen(command.c_str(), "r");
```

**问题 2**:

```
/home/liying/sqlcc/tests/client_server/client_test.cpp:138: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "INSERT INTO test_table VALUES (1, 'test')", "SELECT * FROM test_table",
```

**问题 3**:

```
/home/liying/sqlcc/tests/client_server/client_test.cpp:164: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   FILE *pipe = popen(command.c_str(), "r");
```

#### 📁 /home/liying/sqlcc/tests/client_server/encrypted_integration_test.cpp

**问题数量**: 5

**问题 1**:

```
/home/liying/sqlcc/tests/client_server/encrypted_integration_test.cpp:19: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   static ServerManager *server_manager_;
```

**问题 2**:

```
/home/liying/sqlcc/tests/client_server/encrypted_integration_test.cpp:59: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       server_manager_ = new ServerManager(server_path_, port_);
```

**问题 3**:

```
/home/liying/sqlcc/tests/client_server/encrypted_integration_test.cpp:71: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         delete server_manager_;
```

**问题 4**:

```
/home/liying/sqlcc/tests/client_server/encrypted_integration_test.cpp:86: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:       delete server_manager_;
```

**问题 5**:

```
/home/liying/sqlcc/tests/client_server/encrypted_integration_test.cpp:102: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     FILE *pipe = popen(command.c_str(), "r");
```

#### 📁 /home/liying/sqlcc/tests/client_server/encrypted_test_runner.cpp

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/tests/client_server/encrypted_test_runner.cpp:24: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     FILE* pipe = popen(cmd.c_str(), "r");
```

**问题 2**:

```
/home/liying/sqlcc/tests/client_server/encrypted_test_runner.cpp:75: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char* argv[]) {
```

#### 📁 /home/liying/sqlcc/tests/components/debug/comprehensive_test.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/components/debug/comprehensive_test.cpp:50: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     testInput("SELECT * FROM users;", "Basic SELECT statement");
```

#### 📁 /home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp

**问题数量**: 8

**问题 1**:

```
/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp:40: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int temp_fd_ = -1;
```

**问题 2**:

```
/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp:64: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int raw_fd = static_cast<int>(fd3);
```

**问题 3**:

```
/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp:72: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int original_fd;
```

**问题 4**:

```
/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp:80: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int flags = fcntl(fd.get(), F_GETFL);
```

**问题 5**:

```
/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp:85: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int result = fcntl(original_fd, F_GETFL);
```

**问题 6**:

```
/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp:119: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int released_fd = fd.release();
```

**问题 7**:

```
/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp:140: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int result = getsockopt(tcp_socket.get(), SOL_SOCKET, SO_TYPE, &type, &len);
```

**问题 8**:

```
/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp:193: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int result = fcntl(temp_fd_, F_GETFL);
```

#### 📁 /home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp

**问题数量**: 5

**问题 1**:

```
/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp:86: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         // 模式1: 裸指针声明 (Type* var)
```

**问题 2**:

```
/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp:97: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:         if (line.find("new ") != std::string::npos &&
```

**问题 3**:

```
/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp:105: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         if (line.find("delete ") != std::string::npos &&
```

**问题 4**:

```
/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp:171: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char* argv[]) {
```

**问题 5**:

```
/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp:242: 文件描述符直接使用 - 建议封装为RAII类
  代码:             int fd_count = 0;
```

#### 📁 /home/liying/sqlcc/tests/components/debug/memory_safety_test.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/components/debug/memory_safety_test.cpp:78: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         return *this;
```

#### 📁 /home/liying/sqlcc/tests/components/debug/test_performance_real.cpp

**问题数量**: 8

**问题 1**:

```
/home/liying/sqlcc/tests/components/debug/test_performance_real.cpp:30: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double throughput = (rows * 1000.0) / duration.count();
```

**问题 2**:

```
/home/liying/sqlcc/tests/components/debug/test_performance_real.cpp:64: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double throughput = (queries * 1000.0) / duration.count();
```

**问题 3**:

```
/home/liying/sqlcc/tests/components/debug/test_performance_real.cpp:89: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double throughput = (updates * 1000.0) / duration.count();
```

**问题 4**:

```
/home/liying/sqlcc/tests/components/debug/test_performance_real.cpp:114: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double throughput = (deletes * 1000.0) / duration.count();
```

**问题 5**:

```
/home/liying/sqlcc/tests/components/debug/test_performance_real.cpp:156: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double throughput = (totalOps * 1000.0) / duration.count();
```

**问题 6**:

```
/home/liying/sqlcc/tests/components/debug/test_performance_real.cpp:158: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double successRate = (successOps * 100.0) / totalOps;
```

**问题 7**:

```
/home/liying/sqlcc/tests/components/debug/test_performance_real.cpp:271: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         volatile int result = id * 2;
```

**问题 8**:

```
/home/liying/sqlcc/tests/components/debug/test_performance_real.cpp:281: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             value = value * 2 + i;
```

#### 📁 /home/liying/sqlcc/tests/components/executor/index_optimization_test.cpp

**问题数量**: 4

**问题 1**:

```
/home/liying/sqlcc/tests/components/executor/index_optimization_test.cpp:80: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     sql_parser::Parser parser("SELECT * FROM test_table WHERE id = 2");
```

**问题 2**:

```
/home/liying/sqlcc/tests/components/executor/index_optimization_test.cpp:104: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     sql_parser::Parser parser("SELECT * FROM test_table WHERE id > 1");
```

**问题 3**:

```
/home/liying/sqlcc/tests/components/executor/index_optimization_test.cpp:128: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     sql_parser::Parser parser("SELECT * FROM test_table WHERE name = 'Alice'");
```

**问题 4**:

```
/home/liying/sqlcc/tests/components/executor/index_optimization_test.cpp:152: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     sql_parser::Parser parser("SELECT * FROM test_table");
```

#### 📁 /home/liying/sqlcc/tests/components/executor/index_usage_test.cpp

**问题数量**: 3

**问题 1**:

```
/home/liying/sqlcc/tests/components/executor/index_usage_test.cpp:150: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:     auto stmts_delete = parser_delete.parse();
```

**问题 2**:

```
/home/liying/sqlcc/tests/components/executor/index_usage_test.cpp:172: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     sqlcc::sql_parser::ParserNew parser_select("SELECT * FROM employees WHERE id = 3;");
```

**问题 3**:

```
/home/liying/sqlcc/tests/components/executor/index_usage_test.cpp:195: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     sqlcc::sql_parser::ParserNew parser_select("SELECT * FROM employees WHERE salary > 50000.00;");
```

#### 📁 /home/liying/sqlcc/tests/components/executor/join_executor_test.cpp

**问题数量**: 5

**问题 1**:

```
/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp:130: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   EXPECT_EQ(stats.rows_processed, 9); // 3*3=9行处理
```

**问题 2**:

```
/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp:157: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   EXPECT_EQ(stats.rows_processed, 9); // 3*3=9行处理
```

**问题 3**:

```
/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp:185: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   EXPECT_EQ(stats.rows_processed, 9); // 3*3=9行处理
```

**问题 4**:

```
/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp:205: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   EXPECT_EQ(result.rows.size(), 9); // 3*3=9行交叉连接
```

**问题 5**:

```
/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp:211: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   EXPECT_EQ(stats.rows_processed, 9); // 3*3=9行处理
```

#### 📁 /home/liying/sqlcc/tests/components/executor/privilege_consistency_test.cpp

**问题数量**: 6

**问题 1**:

```
/home/liying/sqlcc/tests/components/executor/privilege_consistency_test.cpp:51: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string result = sql_exec.Execute("SELECT * FROM sys_users WHERE username = 'testuser'");
```

**问题 2**:

```
/home/liying/sqlcc/tests/components/executor/privilege_consistency_test.cpp:80: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string result = sql_exec.Execute("SELECT * FROM sys_users WHERE username = 'testuser'");
```

**问题 3**:

```
/home/liying/sqlcc/tests/components/executor/privilege_consistency_test.cpp:105: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string result = sql_exec.Execute("SELECT * FROM sys_roles WHERE role_name = 'testrole'");
```

**问题 4**:

```
/home/liying/sqlcc/tests/components/executor/privilege_consistency_test.cpp:134: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string result = sql_exec.Execute("SELECT * FROM sys_roles WHERE role_name = 'testrole'");
```

**问题 5**:

```
/home/liying/sqlcc/tests/components/executor/privilege_consistency_test.cpp:163: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string result = sql_exec.Execute("SELECT * FROM sys_privileges WHERE grantee_name = 'testuser' AND privilege = 'SELECT'");
```

**问题 6**:

```
/home/liying/sqlcc/tests/components/executor/privilege_consistency_test.cpp:196: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string result = sql_exec.Execute("SELECT * FROM sys_privileges WHERE grantee_name = 'testuser' AND privilege = 'SELECT'");
```

#### 📁 /home/liying/sqlcc/tests/components/executor/set_operation_test.cpp

**问题数量**: 3

**问题 1**:

```
/home/liying/sqlcc/tests/components/executor/set_operation_test.cpp:33: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   set_executor_->set_memory_limit(1024 * 1024 * 100); // 100MB
```

**问题 2**:

```
/home/liying/sqlcc/tests/components/executor/set_operation_test.cpp:71: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   set_executor_->set_memory_limit(1024 * 1024);       // 1MB
```

**问题 3**:

```
/home/liying/sqlcc/tests/components/executor/set_operation_test.cpp:72: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   set_executor_->set_memory_limit(1024 * 1024 * 500); // 500MB
```

#### 📁 /home/liying/sqlcc/tests/components/executor/unified_executor_test.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/components/executor/unified_executor_test.cpp:227: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ParserNew select_parser("SELECT * FROM test_table;");
```

#### 📁 /home/liying/sqlcc/tests/components/executor/unsupported_commands_test.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/components/executor/unsupported_commands_test.cpp:48: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       executor.Execute("CREATE VIEW v1 AS SELECT * FROM users;");
```

#### 📁 /home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp

**问题数量**: 4

**问题 1**:

```
/home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp:155: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:                       std::to_string(i) + ", " + std::to_string(i * 100) + ");";
```

**问题 2**:

```
/home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp:162: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   // std::string select_sql = "SELECT * FROM transactions WHERE amount BETWEEN
```

**问题 3**:

```
/home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp:190: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   // std::string select_sql = "SELECT * FROM users WHERE email LIKE
```

**问题 4**:

```
/home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp:221: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   // std::string select_sql = "SELECT * FROM employees WHERE (age >= 25 AND
```

#### 📁 /home/liying/sqlcc/tests/components/network/network_unit_test.cpp

**问题数量**: 3

**问题 1**:

```
/home/liying/sqlcc/tests/components/network/network_unit_test.cpp:11: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Session *session_;
```

**问题 2**:

```
/home/liying/sqlcc/tests/components/network/network_unit_test.cpp:13: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   void SetUp() override { session_ = new Session(1); }
```

**问题 3**:

```
/home/liying/sqlcc/tests/components/network/network_unit_test.cpp:15: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:   void TearDown() override { delete session_; }
```

#### 📁 /home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp

**问题数量**: 21

**问题 1**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:28: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     void expectStatementType(Statement* stmt, Statement::Type expectedType) {
```

**问题 2**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:41: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* stmt = statements[0].get();
```

**问题 3**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:44: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
```

**问题 4**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:54: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* setOp = operations[0].get();
```

**问题 5**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:66: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* stmt = statements[0].get();
```

**问题 6**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:69: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
```

**问题 7**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:75: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* setOp = operations[0].get();
```

**问题 8**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:87: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* stmt = statements[0].get();
```

**问题 9**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:90: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
```

**问题 10**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:96: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* setOp = operations[0].get();
```

**问题 11**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:108: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* stmt = statements[0].get();
```

**问题 12**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:111: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
```

**问题 13**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:117: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* setOp = operations[0].get();
```

**问题 14**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:129: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* stmt = statements[0].get();
```

**问题 15**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:132: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
```

**问题 16**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:155: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* stmt = statements[0].get();
```

**问题 17**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:158: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* selectStmt = dynamic_cast<SelectStatement*>(stmt);
```

**问题 18**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:171: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* stmt = statements[0].get();
```

**问题 19**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:174: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
```

**问题 20**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:181: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* firstSelect = selectStmts[0].get();
```

**问题 21**:

```
/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp:186: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* secondSelect = selectStmts[1].get();
```

#### 📁 /home/liying/sqlcc/tests/components/security/test_revoke_persistence.cpp

**问题数量**: 4

**问题 1**:

```
/home/liying/sqlcc/tests/components/security/test_revoke_persistence.cpp:65: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string result = sql_exec.Execute("SELECT * FROM sys_privileges WHERE grantee_name = 'alice'");
```

**问题 2**:

```
/home/liying/sqlcc/tests/components/security/test_revoke_persistence.cpp:87: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string result_before = sql_exec.Execute("SELECT * FROM sys_privileges WHERE grantee_name = 'alice'");
```

**问题 3**:

```
/home/liying/sqlcc/tests/components/security/test_revoke_persistence.cpp:99: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string result_after = sql_exec.Execute("SELECT * FROM sys_privileges WHERE grantee_name = 'alice'");
```

**问题 4**:

```
/home/liying/sqlcc/tests/components/security/test_revoke_persistence.cpp:121: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string final_result = sql_exec.Execute("SELECT * FROM sys_privileges WHERE grantee_name = 'alice'");
```

#### 📁 /home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp

**问题数量**: 15

**问题 1**:

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp:158: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* page = buffer_pool_->NewPage(&page_id);
```

**问题 2**:

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp:184: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* page = buffer_pool_->NewPage(&page_id);
```

**问题 3**:

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp:205: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* page = buffer_pool_->NewPage(&page_id);
```

**问题 4**:

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp:226: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* page = buffer_pool_->NewPage(&page_id);
```

**问题 5**:

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp:231: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* fetched_page = buffer_pool_->FetchPage(page_id);
```

**问题 6**:

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp:256: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* page = buffer_pool_sharded_->NewPage(&page_id);
```

**问题 7**:

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp:261: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* fetched = buffer_pool_sharded_->FetchPage(page_id);
```

**问题 8**:

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp:278: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* page = buffer_pool_sharded_->NewPage(&page_id);
```

**问题 9**:

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp:348: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* page = buffer_pool->NewPage(&page_id);
```

**问题 10**:

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp:400: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       Page* page = buffer_pool->NewPage(&page_id);
```

**问题 11**:

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp:444: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* page = buffer_pool->NewPage(&page_id);
```

**问题 12**:

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp:461: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page* page = buffer_pool->FetchPage(page_id);
```

**问题 13**:

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp:492: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* page = small_pool->NewPage(&page_id);
```

**问题 14**:

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp:497: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* page2 = small_pool->NewPage(&page_id2);
```

**问题 15**:

```
/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp:528: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* page = buffer_pool->NewPage(&page_id);
```

#### 📁 /home/liying/sqlcc/tests/components/storage/disk_manager_test.cpp

**问题数量**: 3

**问题 1**:

```
/home/liying/sqlcc/tests/components/storage/disk_manager_test.cpp:103: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   // 文件大小应该至少是num_pages * page_size_
```

**问题 2**:

```
/home/liying/sqlcc/tests/components/storage/disk_manager_test.cpp:104: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   EXPECT_GE(file_size, num_pages * page_size_);
```

**问题 3**:

```
/home/liying/sqlcc/tests/components/storage/disk_manager_test.cpp:126: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: //   EXPECT_EQ(new_page_id, 3); // 应该从3开始分配，因为文件大小已经是3 * PAGE_SIZE
```

#### 📁 /home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp

**问题数量**: 10

**问题 1**:

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:85: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string select_sql = "SELECT * FROM users WHERE id = 1;";
```

**问题 2**:

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:104: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string select_sql = "SELECT * FROM users WHERE name = 'Bob';";
```

**问题 3**:

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:130: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string select_sql = "SELECT * FROM users WHERE id = 4;";
```

**问题 4**:

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:152: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string select_sql = "SELECT * FROM users WHERE age = 36;";
```

**问题 5**:

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:174: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string select_sql = "SELECT * FROM users WHERE id = 7;";
```

**问题 6**:

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:192: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string select_sql = "SELECT * FROM users WHERE age > 20 AND age < 50;";
```

**问题 7**:

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:211: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "SELECT * FROM users WHERE id = 9;",
```

**问题 8**:

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:212: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "SELECT * FROM users WHERE name = 'Ivan';",
```

**问题 9**:

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:213: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "SELECT * FROM users WHERE age = 50;"};
```

**问题 10**:

```
/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp:243: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string select_sql = "SELECT * FROM users WHERE id = 10;";
```

#### 📁 /home/liying/sqlcc/tests/integration/advanced_sql/isql_integration_test.cpp

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/tests/integration/advanced_sql/isql_integration_test.cpp:133: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     FILE *pipe = popen(command.c_str(), "r");
```

**问题 2**:

```
/home/liying/sqlcc/tests/integration/advanced_sql/isql_integration_test.cpp:206: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "SELECT * FROM test_table;\n"
```

#### 📁 /home/liying/sqlcc/tests/integration/advanced_sql/sql_executor_integration_test.cpp

**问题数量**: 3

**问题 1**:

```
/home/liying/sqlcc/tests/integration/advanced_sql/sql_executor_integration_test.cpp:91: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql = "SELECT * FROM test_users;";
```

**问题 2**:

```
/home/liying/sqlcc/tests/integration/advanced_sql/sql_executor_integration_test.cpp:159: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql = "SELECT * FROM non_existent_table;";
```

**问题 3**:

```
/home/liying/sqlcc/tests/integration/advanced_sql/sql_executor_integration_test.cpp:201: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql3 = "SELECT * FROM multi_test;";
```

#### 📁 /home/liying/sqlcc/tests/integration/basic_sql/simple_sql_test.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/integration/basic_sql/simple_sql_test.cpp:31: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string sql = "SELECT * FROM users;";
```

#### 📁 /home/liying/sqlcc/tests/integration/isql_integration_test.cpp

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/tests/integration/isql_integration_test.cpp:133: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     FILE *pipe = popen(command.c_str(), "r");
```

**问题 2**:

```
/home/liying/sqlcc/tests/integration/isql_integration_test.cpp:206: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "SELECT * FROM test_table;\n"
```

#### 📁 /home/liying/sqlcc/tests/integration/simple_sql_test.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/integration/simple_sql_test.cpp:31: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string sql = "SELECT * FROM users;";
```

#### 📁 /home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp

**问题数量**: 11

**问题 1**:

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:166: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM products");
```

**问题 2**:

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:172: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM products WHERE price BETWEEN 50 AND 200");
```

**问题 3**:

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:173: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM products WHERE name LIKE '%board%'");
```

**问题 4**:

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:174: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM products WHERE stock > 0 AND price < 100");
```

**问题 5**:

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:235: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM persistent_data WHERE id = 5");
```

**问题 6**:

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:268: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:                          std::to_string(i * 10) + ")");
```

**问题 7**:

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:272: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM indexed_table WHERE name = 'Item50'");
```

**问题 8**:

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:273: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM indexed_table WHERE category = 'Category5'");
```

**问题 9**:

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:274: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM indexed_table WHERE category = 'Category3' AND value > 500");
```

**问题 10**:

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:350: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM non_existent_table", "错误");
```

**问题 11**:

```
/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp:404: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     ExecuteAndVerify("SELECT * FROM perf_table WHERE id = 500");
```

#### 📁 /home/liying/sqlcc/tests/integration/sql_executor_integration_test.cpp

**问题数量**: 3

**问题 1**:

```
/home/liying/sqlcc/tests/integration/sql_executor_integration_test.cpp:91: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql = "SELECT * FROM test_users;";
```

**问题 2**:

```
/home/liying/sqlcc/tests/integration/sql_executor_integration_test.cpp:159: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql = "SELECT * FROM non_existent_table;";
```

**问题 3**:

```
/home/liying/sqlcc/tests/integration/sql_executor_integration_test.cpp:201: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql3 = "SELECT * FROM multi_test;";
```

#### 📁 /home/liying/sqlcc/tests/legacy/test_dcl_ddl_persistence.cpp

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/tests/legacy/test_dcl_ddl_persistence.cpp:66: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string result10 = executor2.Execute("SELECT * FROM users;");
```

**问题 2**:

```
/home/liying/sqlcc/tests/legacy/test_dcl_ddl_persistence.cpp:67: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::cout << "SELECT * FROM users: " << result10 << std::endl;
```

#### 📁 /home/liying/sqlcc/tests/network/aes_encryption_test.cc

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/tests/network/aes_encryption_test.cc:153: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM users WHERE id = 1;",
```

**问题 2**:

```
/home/liying/sqlcc/tests/network/aes_encryption_test.cc:187: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::vector<uint8_t> large_data(100 * 1024);
```

#### 📁 /home/liying/sqlcc/tests/network/aes_network_integration_test.cc

**问题数量**: 14

**问题 1**:

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:102: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string query = "SELECT * FROM users WHERE id = 1;";
```

**问题 2**:

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:129: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM users;",
```

**问题 3**:

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:169: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::vector<bool> results(num_threads * iterations_per_thread, false);
```

**问题 4**:

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:185: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:                 results[t * iterations_per_thread + i] = success;
```

**问题 5**:

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:234: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::cout << "  Data Size: " << data_size / (1024.0 * 1024.0) << " MB" << std::endl;
```

**问题 6**:

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:239: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double encrypt_throughput = (data_size / (1024.0 * 1024.0)) / (encrypt_time / 1000.0);
```

**问题 7**:

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:244: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double decrypt_throughput = (data_size / (1024.0 * 1024.0)) / (decrypt_time / 1000.0);
```

**问题 8**:

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:357: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string query = "SELECT * FROM users WHERE id = 1;";
```

**问题 9**:

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:384: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM users;",
```

**问题 10**:

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:424: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::vector<bool> results(num_threads * iterations_per_thread, false);
```

**问题 11**:

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:440: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:                 results[t * iterations_per_thread + i] = success;
```

**问题 12**:

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:489: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::cout << "  Data Size: " << data_size / (1024.0 * 1024.0) << " MB" << std::endl;
```

**问题 13**:

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:494: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double encrypt_throughput = (data_size / (1024.0 * 1024.0)) / (encrypt_time / 1000.0);
```

**问题 14**:

```
/home/liying/sqlcc/tests/network/aes_network_integration_test.cc:499: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double decrypt_throughput = (data_size / (1024.0 * 1024.0)) / (decrypt_time / 1000.0);
```

#### 📁 /home/liying/sqlcc/tests/network/sql_network_test.cpp

**问题数量**: 16

**问题 1**:

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:94: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int ret = recv(sock_fd_, buffer, sizeof(buffer), 0);
```

**问题 2**:

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:114: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             MessageHeader* resp_header = reinterpret_cast<MessageHeader*>(buffer);
```

**问题 3**:

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:137: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         size_t body_len = 2 * sizeof(uint32_t) + username_len + password_len;
```

**问题 4**:

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:140: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* header = reinterpret_cast<MessageHeader*>(message.data());
```

**问题 5**:

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:155: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         char* body = message.data() + sizeof(MessageHeader);
```

**问题 6**:

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:158: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         memcpy(body + 2 * sizeof(uint32_t), username.c_str(), username_len);
```

**问题 7**:

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:159: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         memcpy(body + 2 * sizeof(uint32_t) + username_len, password.c_str(), password_len);
```

**问题 8**:

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:168: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int ret = recv(sock_fd_, buffer, sizeof(buffer), 0);
```

**问题 9**:

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:173: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* resp_header = reinterpret_cast<MessageHeader*>(buffer);
```

**问题 10**:

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:182: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* header = reinterpret_cast<MessageHeader*>(message.data());
```

**问题 11**:

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:198: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int ret = recv(sock_fd_, buffer, sizeof(buffer), 0);
```

**问题 12**:

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:203: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* resp_header = reinterpret_cast<MessageHeader*>(buffer);
```

**问题 13**:

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:225: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int sock_fd_;
```

**问题 14**:

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:355: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     result = client.ExecuteQuery("SELECT * FROM network_test_users WHERE id = 1");
```

**问题 15**:

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:372: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     result = client.ExecuteQuery("SELECT * FROM network_test_users WHERE id = 2");
```

**问题 16**:

```
/home/liying/sqlcc/tests/network/sql_network_test.cpp:412: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     string result = client.ExecuteQuery("SELECT * FROM network_test_products WHERE price > 4000");
```

#### 📁 /home/liying/sqlcc/tests/network/tls_e2e_test.cc

**问题数量**: 8

**问题 1**:

```
/home/liying/sqlcc/tests/network/tls_e2e_test.cc:18: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     EVP_PKEY* pkey = EVP_PKEY_new();
```

**问题 2**:

```
/home/liying/sqlcc/tests/network/tls_e2e_test.cc:19: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     RSA* rsa = RSA_new();
```

**问题 3**:

```
/home/liying/sqlcc/tests/network/tls_e2e_test.cc:20: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     BIGNUM* e = BN_new();
```

**问题 4**:

```
/home/liying/sqlcc/tests/network/tls_e2e_test.cc:26: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     X509* x509 = X509_new();
```

**问题 5**:

```
/home/liying/sqlcc/tests/network/tls_e2e_test.cc:31: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     X509_NAME* name = X509_get_subject_name(x509);
```

**问题 6**:

```
/home/liying/sqlcc/tests/network/tls_e2e_test.cc:38: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     FILE* cf = fopen(cert_path.c_str(), "wb");
```

**问题 7**:

```
/home/liying/sqlcc/tests/network/tls_e2e_test.cc:43: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     FILE* kf = fopen(key_path.c_str(), "wb");
```

**问题 8**:

```
/home/liying/sqlcc/tests/network/tls_e2e_test.cc:100: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     auto* ack_hdr = reinterpret_cast<MessageHeader*>(conn_ack.data());
```

#### 📁 /home/liying/sqlcc/tests/performance/basic/batch_prefetch_performance_test.cc

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/performance/basic/batch_prefetch_performance_test.cc:121: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             static_cast<int32_t>(region * working_set / 10),
```

#### 📁 /home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc

**问题数量**: 10

**问题 1**:

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:77: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string query = "SELECT * FROM " + std::string(kTestDatabase) + "." + std::string(kTestTable) + 
```

**问题 2**:

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:109: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string query = "SELECT * FROM " + std::string(kTestDatabase) + "." + std::string(kTestTable) + 
```

**问题 3**:

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:160: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         result.p95_latency = result.avg_latency * 1.5; // 占位值
```

**问题 4**:

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:161: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         result.p99_latency = result.avg_latency * 2.0; // 占位值
```

**问题 5**:

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:190: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         result.p95_latency = result.avg_latency * 1.8; // 占位值
```

**问题 6**:

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:191: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         result.p99_latency = result.avg_latency * 2.5; // 占位值
```

**问题 7**:

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:224: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         result.p95_latency = result.avg_latency * 1.6; // 占位值
```

**问题 8**:

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:225: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         result.p99_latency = result.avg_latency * 2.2; // 占位值
```

**问题 9**:

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:253: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         result.p95_latency = result.avg_latency * 2.0; // 占位值
```

**问题 10**:

```
/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc:254: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         result.p99_latency = result.avg_latency * 3.0; // 占位值
```

#### 📁 /home/liying/sqlcc/tests/performance/basic/cpu_intensive_performance_test.cc

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/tests/performance/basic/cpu_intensive_performance_test.cc:9: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     sql_executor_ = new SqlExecutor();
```

**问题 2**:

```
/home/liying/sqlcc/tests/performance/basic/cpu_intensive_performance_test.cc:28: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         delete sql_executor_;
```

#### 📁 /home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc

**问题数量**: 8

**问题 1**:

```
/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc:203: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     double throughput = (successful_operations * 1000.0) / duration.count();
```

**问题 2**:

```
/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc:261: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     double throughput = (successful_operations * 1000.0) / duration.count();
```

**问题 3**:

```
/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc:320: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     double throughput = (successful_operations * 1000.0) / duration.count();
```

**问题 4**:

```
/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc:378: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     double throughput = (successful_operations * 1000.0) / duration.count();
```

**问题 5**:

```
/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc:436: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     double throughput = (successful_operations * 1000.0) / duration.count();
```

**问题 6**:

```
/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc:493: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string select_sql = "SELECT * FROM " + std::string(kTestTable) + 
```

**问题 7**:

```
/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc:514: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::string select_sql = "SELECT * FROM " + std::string(kTestTable) + 
```

**问题 8**:

```
/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc:638: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:               << (static_cast<double>(passed_tests) / total_tests * 100) << "%\n";
```

#### 📁 /home/liying/sqlcc/tests/performance/buffer_pool_performance_test.h

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/performance/buffer_pool_performance_test.h:115: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     sqlcc::SqlExecutor* sql_executor_; // SQL执行器指针
```

#### 📁 /home/liying/sqlcc/tests/performance/concurrency/concurrency_performance_test.h

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/performance/concurrency/concurrency_performance_test.h:171: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SimpleBarrier* start_barrier_;
```

#### 📁 /home/liying/sqlcc/tests/performance/concurrency_test/concurrency_performance_test.h

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/performance/concurrency_test/concurrency_performance_test.h:171: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SimpleBarrier* start_barrier_;
```

#### 📁 /home/liying/sqlcc/tests/performance/cpu_test/cpu_intensive_performance_test.h

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/performance/cpu_test/cpu_intensive_performance_test.h:52: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SqlExecutor* sql_executor_;
```

#### 📁 /home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc

**问题数量**: 10

**问题 1**:

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:149: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             int id = batch * BATCH_SIZE + i + 1;
```

**问题 2**:

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:183: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::cout << "Rate: " << (float)LARGE_DATA_SIZE * 1000 / duration.count() 
```

**问题 3**:

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:230: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::cout << "Rate: " << (float)QUERIES_TO_RUN * 1000 / duration.count() 
```

**问题 4**:

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:334: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::cout << "Rate: " << (float)UPDATES_TO_RUN * 1000 / duration.count() 
```

**问题 5**:

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:350: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:     // Measure time for delete operations
```

**问题 6**:

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:368: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:     std::cout << "Executed " << DELETES_TO_RUN << " delete operations in " 
```

**问题 7**:

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:372: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::cout << "Rate: " << (float)DELETES_TO_RUN * 1000 / duration.count() 
```

**问题 8**:

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:399: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM large_test WHERE salary > (SELECT AVG(salary) FROM large_test)",
```

**问题 9**:

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:402: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM large_test WHERE name LIKE '%John%' OR email LIKE '%john%' ORDER BY salary DESC LIMIT 100",
```

**问题 10**:

```
/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc:404: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM large_test WHERE created_at >= DATE_SUB(NOW(), INTERVAL 1 DAY) ORDER BY created_at DESC"
```

#### 📁 /home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp

**问题数量**: 9

**问题 1**:

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:102: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     size_t current_batch_size = std::min(BATCH_SIZE, data_size - batch * BATCH_SIZE);
```

**问题 2**:

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:108: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       size_t id = batch * BATCH_SIZE + i + 1;
```

**问题 3**:

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:196: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   double throughput = (operations_completed * 1000.0) / total_duration.count();
```

**问题 4**:

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:228: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql = "SELECT * FROM performance_test_table WHERE id = " + std::to_string(id);
```

**问题 5**:

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:251: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   double throughput = (operations_completed * 1000.0) / total_duration.count();
```

**问题 6**:

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:284: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql = "SELECT * FROM performance_test_table WHERE id BETWEEN " + 
```

**问题 7**:

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:308: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   double throughput = (operations_completed * 1000.0) / total_duration.count();
```

**问题 8**:

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:365: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   double throughput = (operations_completed * 1000.0) / total_duration.count();
```

**问题 9**:

```
/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp:420: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   double throughput = (operations_completed * 1000.0) / total_duration.count();
```

#### 📁 /home/liying/sqlcc/tests/performance/disk_io_performance_test.h

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/performance/disk_io_performance_test.h:73: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     bool SimulatePageRead(int32_t page_id, char* buffer, size_t page_size);
```

#### 📁 /home/liying/sqlcc/tests/performance/index_performance_test.h

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/performance/index_performance_test.h:53: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     IndexManager* index_manager_;
```

#### 📁 /home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.cc

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.cc:9: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     sql_executor_ = new SqlExecutor();
```

**问题 2**:

```
/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.cc:26: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         delete sql_executor_;
```

#### 📁 /home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h

**问题数量**: 6

**问题 1**:

```
/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h:34: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     static constexpr size_t kMaxMemoryAllocation = 1024 * 1024 * 100; // 100MB
```

**问题 2**:

```
/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h:36: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     static constexpr size_t kMediumAllocationSize = 1024 * 10; // 10KB
```

**问题 3**:

```
/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h:37: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     static constexpr size_t kLargeAllocationSize = 1024 * 100; // 100KB
```

**问题 4**:

```
/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h:40: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     static constexpr size_t kMediumBlockSize = 1024 * 10; // 10KB  
```

**问题 5**:

```
/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h:41: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     static constexpr size_t kLargeBlockSize = 1024 * 100; // 100KB
```

**问题 6**:

```
/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h:45: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SqlExecutor* sql_executor_;
```

#### 📁 /home/liying/sqlcc/tests/performance/million_insert_test.h

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/performance/million_insert_test.h:168: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     size_t start_id = thread_id * records_per_thread;
```

#### 📁 /home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test.h

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test.h:57: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     SqlExecutor* sql_executor_;
```

#### 📁 /home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test_main.cc

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test_main.cc:10: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     sql_executor_ = new SqlExecutor();
```

**问题 2**:

```
/home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test_main.cc:29: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:         delete sql_executor_;
```

#### 📁 /home/liying/sqlcc/tests/sql_executor/sql_executor_comprehensive_test.cpp

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/tests/sql_executor/sql_executor_comprehensive_test.cpp:28: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string result = executor.Execute("SELECT * FROM test_table");
```

**问题 2**:

```
/home/liying/sqlcc/tests/sql_executor/sql_executor_comprehensive_test.cpp:170: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         file << "SELECT * FROM test_table;\n";
```

#### 📁 /home/liying/sqlcc/tests/sql_executor/sql_executor_minimal_test.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/sql_executor/sql_executor_minimal_test.cpp:41: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string result = executor.Execute("SELECT * FROM test_table");
```

#### 📁 /home/liying/sqlcc/tests/sql_executor/sql_executor_unit_test.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/sql_executor/sql_executor_unit_test.cpp:47: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   result = executor.Execute("SELECT * FROM users");
```

#### 📁 /home/liying/sqlcc/tests/sql_parser/error_integration_test.cpp

**问题数量**: 6

**问题 1**:

```
/home/liying/sqlcc/tests/sql_parser/error_integration_test.cpp:417: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "SELCT * FROM users",  // Typo in SELECT
```

**问题 2**:

```
/home/liying/sqlcc/tests/sql_parser/error_integration_test.cpp:418: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "SELECT * FROM users WHERE id = 'string'",  // Type mismatch
```

**问题 3**:

```
/home/liying/sqlcc/tests/sql_parser/error_integration_test.cpp:419: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "SELECT * FROM unknown_table",  // Undefined table
```

**问题 4**:

```
/home/liying/sqlcc/tests/sql_parser/error_integration_test.cpp:420: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "SELECT * FROM users  ",  // Missing semicolon (will be detected)
```

**问题 5**:

```
/home/liying/sqlcc/tests/sql_parser/error_integration_test.cpp:421: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "SELECT * FROM users\x01",  // Invalid character
```

**问题 6**:

```
/home/liying/sqlcc/tests/sql_parser/error_integration_test.cpp:422: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "SELECT * FROM users WHERE name = 'unterminated"  // Unterminated string
```

#### 📁 /home/liying/sqlcc/tests/sql_parser/expression_test.cpp

**问题数量**: 3

**问题 1**:

```
/home/liying/sqlcc/tests/sql_parser/expression_test.cpp:195: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         // Complex expression: 2 * 3 + 5
```

**问题 2**:

```
/home/liying/sqlcc/tests/sql_parser/expression_test.cpp:206: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         // 2 + 3 * 4 (should be 2 + (3 * 4))
```

**问题 3**:

```
/home/liying/sqlcc/tests/sql_parser/expression_test.cpp:212: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::cout << "✅ 2 + 3 * 4 = " << precExpr1->toString() << std::endl;
```

#### 📁 /home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp

**问题数量**: 8

**问题 1**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp:85: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   assertParseSuccess("SELECT * FROM users;", "Simple SELECT");
```

**问题 2**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp:179: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "-- This is a comment\nSELECT * FROM users; -- Another comment",
```

**问题 3**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp:199: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   assertParseSuccess("SELECT * FROM users WHERE name = 'John';",
```

**问题 4**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp:203: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   assertParseSuccess("SELECT * FROM products WHERE price > 99.99;",
```

**问题 5**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp:207: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   assertParseSuccess("SELECT * FROM data WHERE value > 1.23e10;",
```

**问题 6**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp:222: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   assertParseFailure("SELECT * FROM;", "Missing table name");
```

**问题 7**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp:225: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   assertParseFailure("SELECT * FROM users WHERE name = 'unterminated;",
```

**问题 8**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp:229: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   assertParseFailure("SELET * FROM users;", "Typo in keyword");
```

#### 📁 /home/liying/sqlcc/tests/sql_parser/lexer_new_benchmark_test.cpp

**问题数量**: 3

**问题 1**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_new_benchmark_test.cpp:11: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码: // Benchmark test for comparing old and new lexers
```

**问题 2**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_new_benchmark_test.cpp:19: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:           "SELECT * FROM users;", // Simple query
```

**问题 3**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_new_benchmark_test.cpp:46: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     // Benchmark new DFA lexer
```

#### 📁 /home/liying/sqlcc/tests/sql_parser/lexer_new_test.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_new_test.cpp:23: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     LexerNew lexer("SELECT * FROM users WHERE id = 123;");
```

#### 📁 /home/liying/sqlcc/tests/sql_parser/lexer_new_unit_test.cpp

**问题数量**: 4

**问题 1**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_new_unit_test.cpp:25: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string input = "SELECT * FROM users WHERE id = 1;";
```

**问题 2**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_new_unit_test.cpp:112: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "SELECT * FROM users WHERE id > 10 AND name LIKE 'John%';";
```

**问题 3**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_new_unit_test.cpp:205: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string input = "SELECT * FROM users -- This is a comment\nWHERE id = 1;";
```

**问题 4**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_new_unit_test.cpp:250: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string input = "SELECT    *    FROM     users    WHERE   id   =   1;";
```

#### 📁 /home/liying/sqlcc/tests/sql_parser/lexer_test.cpp

**问题数量**: 5

**问题 1**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_test.cpp:18: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   LexerNew lexer("SELECT * FROM table;");
```

**问题 2**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_test.cpp:58: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   LexerNew lexer("SELECT * FROM table WHERE id = 123;");
```

**问题 3**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_test.cpp:60: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   // Skip SELECT * FROM table WHERE id =
```

**问题 4**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_test.cpp:71: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   LexerNew lexer("SELECT * FROM table WHERE name = 'test';");
```

**问题 5**:

```
/home/liying/sqlcc/tests/sql_parser/lexer_test.cpp:73: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   // Skip SELECT * FROM table WHERE name =
```

#### 📁 /home/liying/sqlcc/tests/sql_parser/parser_integration_test.cpp

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/tests/sql_parser/parser_integration_test.cpp:10: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:  * Tests the integration of the new DFA lexer, ParserNew, and AST system
```

**问题 2**:

```
/home/liying/sqlcc/tests/sql_parser/parser_integration_test.cpp:358: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM products WHERE price = 100",
```

#### 📁 /home/liying/sqlcc/tests/sql_parser/parser_new_integration_test.cpp

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/tests/sql_parser/parser_new_integration_test.cpp:677: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   MockParserNew parser("SELECT * FROM users;");
```

**问题 2**:

```
/home/liying/sqlcc/tests/sql_parser/parser_new_integration_test.cpp:718: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "SELECT * FROM users; INSERT INTO logs VALUES ('test');");
```

#### 📁 /home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp

**问题数量**: 13

**问题 1**:

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:55: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "SELECT * FROM users WHERE id IN (SELECT user_id FROM active_users WHERE last_login > '2024-01-01')",
```

**问题 2**:

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:80: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         for (int i = 1; i <= complexity * 3; ++i) {
```

**问题 3**:

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:109: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             query += " LIMIT " + std::to_string(complexity * 10);
```

**问题 4**:

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:287: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double throughput = (total_chars * 1000.0) / total_time; // 字符/秒
```

**问题 5**:

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:345: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         if (time_growth <= size_growth * 1.5) {
```

**问题 6**:

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:347: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         } else if (time_growth <= size_growth * 2.0) {
```

**问题 7**:

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:386: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     double success_rate = (successful_parses * 100.0) / num_queries;
```

**问题 8**:

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:432: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         {"SELECT * FROM users WHERE id IN (SELECT user_id FROM active_users)", "IN子查询", true},
```

**问题 9**:

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:433: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         {"SELECT * FROM users WHERE EXISTS (SELECT 1 FROM posts WHERE user_id = users.id)", "EXISTS子查询", true},
```

**问题 10**:

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:436: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         {"SELECT * FROM users WHERE age BETWEEN 18 AND 65", "BETWEEN表达式", true},
```

**问题 11**:

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:437: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         {"SELECT * FROM users WHERE name LIKE 'John%'", "LIKE表达式", true},
```

**问题 12**:

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:443: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         {"SELECT * FROM", "不完整FROM", false}
```

**问题 13**:

```
/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp:479: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     double pass_rate = (passed * 100.0) / (passed + failed);
```

#### 📁 /home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp

**问题数量**: 9

**问题 1**:

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:11: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:  * Compares the performance of the new DFA-based parser system
```

**问题 2**:

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:146: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "UPDATE products SET price = price * 1.1 WHERE category = 'electronics'",
```

**问题 3**:

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:155: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         for (int i = 1; i <= 20 * complexity; ++i) {
```

**问题 4**:

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:163: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         for (int i = 2; i <= 5 * complexity; ++i) {
```

**问题 5**:

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:170: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         for (int i = 2; i <= 3 * complexity; ++i) {
```

**问题 6**:

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:218: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:             // Test new parser
```

**问题 7**:

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:273: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double oldThroughput = (oldTotalChars * 1000.0) / oldTotalTime;
```

**问题 8**:

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:274: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         double newThroughput = (newTotalChars * 1000.0) / newTotalTime;
```

**问题 9**:

```
/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp:312: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:             // Test new parser
```

#### 📁 /home/liying/sqlcc/tests/sql_parser/simple_parser_test.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/sql_parser/simple_parser_test.cpp:235: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string sql1 = "SELECT * FROM users;";
```

#### 📁 /home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp

**问题数量**: 11

**问题 1**:

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:26: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   void expectStatementNotNull(Statement *stmt) {
```

**问题 2**:

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:31: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   template <typename T> T *expectExpressionType(Expression *expr) {
```

**问题 3**:

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:75: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string sql = "SELECT * FROM users LIMIT 10 OFFSET 20;";
```

**问题 4**:

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:133: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string sql = "SELECT name FROM users WHERE EXISTS (SELECT * FROM orders "
```

**问题 5**:

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:200: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   CreateStatement *createStmt = static_cast<CreateStatement *>(stmt.get());
```

**问题 6**:

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:261: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   CreateStatement *createStmt = static_cast<CreateStatement *>(stmt.get());
```

**问题 7**:

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:329: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   UseStatement *useStmt = static_cast<UseStatement *>(stmt.get());
```

**问题 8**:

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:335: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string sql = "SELECT * FROM users WHERE age > 18 AND (name LIKE "
```

**问题 9**:

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:354: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string sql = "SELECT * FROM users; INSERT INTO logs VALUES (NOW());";
```

**问题 10**:

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:395: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::string sql = "SELECT * FROM users -- This is a comment\nWHERE age > 18;";
```

**问题 11**:

```
/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp:617: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       "employees GROUP BY department) SELECT * FROM dept_summary;";
```

#### 📁 /home/liying/sqlcc/tests/sql_parser/statement_node_test.cpp

**问题数量**: 2

**问题 1**:

```
/home/liying/sqlcc/tests/sql_parser/statement_node_test.cpp:149: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         return "SELECT * FROM " + tableName_;
```

**问题 2**:

```
/home/liying/sqlcc/tests/sql_parser/statement_node_test.cpp:243: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             "SELECT * FROM test WHERE id > 100",
```

#### 📁 /home/liying/sqlcc/tests/sql_parser/token_new_unit_test.cpp

**问题数量**: 3

**问题 1**:

```
/home/liying/sqlcc/tests/sql_parser/token_new_unit_test.cpp:380: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string sql = "SELECT * FROM users WHERE id = 1;";
```

**问题 2**:

```
/home/liying/sqlcc/tests/sql_parser/token_new_unit_test.cpp:474: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT * FROM users -- This is a comment\nWHERE id = 1;";
```

**问题 3**:

```
/home/liying/sqlcc/tests/sql_parser/token_new_unit_test.cpp:495: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         "SELECT    *    FROM     users    WHERE   id   =   1;";
```

#### 📁 /home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp

**问题数量**: 5

**问题 1**:

```
/home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp:75: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   bool ReadPage(int32_t page_id, char *page_data) {
```

**问题 2**:

```
/home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp:136: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   static MockConfigManager *mock_config_manager_;
```

**问题 3**:

```
/home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp:268: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   std::vector<int32_t> page_ids(num_threads * operations_per_thread / 2);
```

**问题 4**:

```
/home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp:271: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   for (int i = 0; i < num_threads * operations_per_thread / 2; ++i) {
```

**问题 5**:

```
/home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp:309: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   EXPECT_EQ(success_count.load(), num_threads * operations_per_thread);
```

#### 📁 /home/liying/sqlcc/tests/test_disk_manager.h

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/test_disk_manager.h:84: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     bool TestReadPage(int32_t page_id, char* page_data) {
```

#### 📁 /home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_output.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_output.cpp:5: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string input = "SELECT * FROM users WHERE id = 1;";
```

#### 📁 /home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_simple.cpp

**问题数量**: 1

**问题 1**:

```
/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_simple.cpp:5: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string input = "SELECT * FROM users WHERE id = 1;";
```

## 改进建议

### 1. 智能指针使用原则
- 使用 `std::unique_ptr` 替代裸指针进行独占所有权管理
- 使用 `std::shared_ptr` 替代裸指针进行共享所有权管理
- 使用 `std::make_unique` 和 `std::make_shared` 替代直接使用 `new`

### 2. RAII资源管理
- 使用 RAII 模式管理资源，避免直接使用 `delete`
- 将文件描述符等系统资源封装为 RAII 类
- 实现异常安全的资源管理

### 3. 重构优先级
1. **高优先级**: 裸指针成员变量（内存泄漏风险）
2. **中优先级**: 文件描述符直接使用（资源泄漏风险）
3. **低优先级**: 函数参数裸指针（接口兼容性考虑）

### 4. 验证机制
- 启用 ASan/LSan 进行运行时内存检查
- 定期运行内存审计工具监控改进效果
- 建立自动化测试确保重构质量

