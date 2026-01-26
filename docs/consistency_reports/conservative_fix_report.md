# 文档-代码一致性保守修复报告

## 无效的文件引用清单

以下是在文档中引用但不存在的代码文件:

### docs/stage2_migration_summary_report.md

- `include/utils/logger.h` - 引用形式: ``include/utils/logger.h``
- `include/core/user_manager.h` - 引用形式: ``include/core/user_manager.h``

### docs/stage3_logger_modules_report.md

- `include/utils/logger.h` - 引用形式: ``include/utils/logger.h``
- `src/utils/logger_module_impl.cpp` - 引用形式: ``src/utils/logger_module_impl.cpp``

### docs/stage4_storage_engine_migration_report.md

- `src/storage_engine/storage_engine.cpp` - 引用形式: ``src/storage_engine/storage_engine.cpp``

### docs/project_progress.md

- `/home/liying/sqlcc/include/storage/replace_strategy.h` - 引用形式: ``/home/liying/sqlcc/include/storage/replace_strategy.h``
- `/home/liying/sqlcc/src/storage_engine/replace_strategy.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/replace_strategy.cpp``
- `/home/liying/sqlcc/include/storage/concurrency_control.h` - 引用形式: ``/home/liying/sqlcc/include/storage/concurrency_control.h``
- `/home/liying/sqlcc/include/storage/buffer_pool_v3.h` - 引用形式: ``/home/liying/sqlcc/include/storage/buffer_pool_v3.h``
- `/home/liying/sqlcc/src/storage_engine/buffer_pool_v3.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/buffer_pool_v3.cpp``
- `/home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp``
- `/home/liying/sqlcc/include/storage/performance_monitor.h` - 引用形式: ``/home/liying/sqlcc/include/storage/performance_monitor.h``

### docs/c++20_modules_implementation_summary.md

- `src/utils/logger_module.cpp` - 引用形式: ``src/utils/logger_module.cpp``
- `include/example.h` - 引用形式: ``include/example.h``
- `src/example.cpp` - 引用形式: ``src/example.cpp``

### docs/include_path_fixes_summary.md

- `include/storage/buffer_pool_fixed.h` - 引用形式: ``include/storage/buffer_pool_fixed.h``
- `include/storage/buffer_pool.h` - 引用形式: ``include/storage/buffer_pool.h``
- `tests/storage_engine/storage_engine_comprehensive_test.cpp` - 引用形式: ``tests/storage_engine/storage_engine_comprehensive_test.cpp``

### docs/README.en.md

- `tests/temporary/test_simple.cc` - 引用形式: ``tests/temporary/test_simple.cc``
- `tests/temporary/test_page_id_fix.cc` - 引用形式: ``tests/temporary/test_page_id_fix.cc``
- `tests/temporary/test_sync_functionality.cc` - 引用形式: ``tests/temporary/test_sync_functionality.cc``
- `tests/temporary/test_deadlock_fix_simple.cc` - 引用形式: ``tests/temporary/test_deadlock_fix_simple.cc``

### docs/自助式改进计划.md

- `include/sql_parser/parser.h` - 引用形式: ``include/sql_parser/parser.h``
- `include/storage/b_plus_tree.h` - 引用形式: ``include/storage/b_plus_tree.h``
- `include/storage/buffer_pool_sharded.h` - 引用形式: ``include/storage/buffer_pool_sharded.h``
- `src/storage_engine/buffer_pool_sharded.cpp` - 引用形式: ``src/storage_engine/buffer_pool_sharded.cpp``
- `include/core/core_database_manager.h` - 引用形式: ``include/core/core_database_manager.h``

### docs/final_migration_assessment_report.md

- `include/utils/logger.h` - 引用形式: ``include/utils/logger.h``
- `include/core/user_manager.h` - 引用形式: ``include/core/user_manager.h``
- `src/utils/logger_module_impl.cpp` - 引用形式: ``src/utils/logger_module_impl.cpp``

### docs/migration_execution_plan.md

- `include/core/user_manager.h` - 引用形式: ``include/core/user_manager.h``
- `include/core/session_manager.h` - 引用形式: ``include/core/session_manager.h``
- `include/core/permission_manager.h` - 引用形式: ``include/core/permission_manager.h``
- `include/utils/logger.h` - 引用形式: ``include/utils/logger.h``
- `include/utils/logger.h` - 引用形式: ``include/utils/logger.h``

### docs/SQLCC_1.2.x_综合覆盖率分析报告.md

- `src/sql_parser/lexer_new.cpp` - 引用形式: ``src/sql_parser/lexer_new.cpp``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``
- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `src/storage_engine/table_storage.cpp` - 引用形式: ``src/storage_engine/table_storage.cpp``
- `src/storage_engine/buffer_pool_v3.cpp` - 引用形式: ``src/storage_engine/buffer_pool_v3.cpp``

### docs/c++20_modules_evaluation_report.md

- `src/utils/logger_module.cpp` - 引用形式: ``src/utils/logger_module.cpp``

### docs/stage2_utils_migration_report.md

- `include/utils/logger.h` - 引用形式: ``include/utils/logger.h``

### docs/stage2_core_migration_report.md

- `include/core/user_manager.h` - 引用形式: ``include/core/user_manager.h``

### docs/coverage/COVERAGE_REPORT.md

- `src/sql_parser/lexer_new.cpp` - 引用形式: ``src/sql_parser/lexer_new.cpp``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``
- `src/sql_parser/token_new.cpp` - 引用形式: ``src/sql_parser/token_new.cpp``
- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `src/storage_engine/table_storage.cpp` - 引用形式: ``src/storage_engine/table_storage.cpp``
- `src/storage_engine/buffer_pool_v3.cpp` - 引用形式: ``src/storage_engine/buffer_pool_v3.cpp``

### docs/consistency_reports/problematic_refs_summary.md

- `/home/liying/sqlcc/include/storage/buffer_pool_v3.h` - 引用形式: ``/home/liying/sqlcc/include/storage/buffer_pool_v3.h``
- `/home/liying/sqlcc/src/storage_engine/buffer_pool_v3.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/buffer_pool_v3.cpp``
- `include/example.h` - 引用形式: ``include/example.h``
- `src/example.cpp` - 引用形式: ``src/example.cpp``
- `include/storage/buffer_pool_fixed.h` - 引用形式: ``include/storage/buffer_pool_fixed.h``
- `tests/temporary/test_simple.cc` - 引用形式: ``tests/temporary/test_simple.cc``
- `tests/temporary/test_page_id_fix.cc` - 引用形式: ``tests/temporary/test_page_id_fix.cc``
- `tests/temporary/test_sync_functionality.cc` - 引用形式: ``tests/temporary/test_sync_functionality.cc``
- `tests/temporary/test_deadlock_fix_simple.cc` - 引用形式: ``tests/temporary/test_deadlock_fix_simple.cc``
- `include/core/permission_manager.h` - 引用形式: ``include/core/permission_manager.h``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``
- `src/storage_engine/table_storage.cpp` - 引用形式: ``src/storage_engine/table_storage.cpp``
- `src/storage_engine/buffer_pool_v3.cpp` - 引用形式: ``src/storage_engine/buffer_pool_v3.cpp``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``
- `src/sql_parser/token_new.cpp` - 引用形式: ``src/sql_parser/token_new.cpp``
- `src/storage_engine/table_storage.cpp` - 引用形式: ``src/storage_engine/table_storage.cpp``
- `src/storage_engine/buffer_pool_v3.cpp` - 引用形式: ``src/storage_engine/buffer_pool_v3.cpp``
- `include/path/file.h` - 引用形式: ``include/path/file.h``
- `src/path/file.cpp` - 引用形式: ``src/path/file.cpp``
- `include/path/file.h` - 引用形式: ``include/path/file.h``
- `/home/liying/sqlcc/include/storage/buffer_pool_v3.h` - 引用形式: ``/home/liying/sqlcc/include/storage/buffer_pool_v3.h``
- `src/sql_parser/token_new.cpp` - 引用形式: ``src/sql_parser/token_new.cpp``
- `src/storage_engine/buffer_pool.cpp` - 引用形式: ``src/storage_engine/buffer_pool.cpp``
- `src/storage_engine/buffer_pool_new.cpp` - 引用形式: ``src/storage_engine/buffer_pool_new.cpp``
- `src/storage_engine/table_storage.cpp` - 引用形式: ``src/storage_engine/table_storage.cpp``
- `storage_engine/index_manager/smart_ptr_lifetime_manager.h` - 引用形式: ``storage_engine/index_manager/smart_ptr_lifetime_manager.h``
- `storage/buffer_pool_fixed.h` - 引用形式: ``storage/buffer_pool_fixed.h``
- `storage/buffer_pool_v2.h` - 引用形式: ``storage/buffer_pool_v2.h``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `tests/unit/storage/page_test.cpp` - 引用形式: ``tests/unit/storage/page_test.cpp``
- `tests/unit/sql_parser/parser_new_test.cpp` - 引用形式: ``tests/unit/sql_parser/parser_new_test.cpp``
- `tests/unit/execution/join_executor_test.cpp` - 引用形式: ``tests/unit/execution/join_executor_test.cpp``
- `tests/unit/execution/set_operation_executor_test.cpp` - 引用形式: ``tests/unit/execution/set_operation_executor_test.cpp``
- `tests/performance/buffer_pool_performance_test.cpp` - 引用形式: ``tests/performance/buffer_pool_performance_test.cpp``
- `tests/performance/index_performance_test.cpp` - 引用形式: ``tests/performance/index_performance_test.cpp``
- `tests/performance/network_performance_test.cpp` - 引用形式: ``tests/performance/network_performance_test.cpp``
- `tests/performance/crud_performance_test.cpp` - 引用形式: ``tests/performance/crud_performance_test.cpp``
- `tests/integration/transaction_integration_test.cpp` - 引用形式: ``tests/integration/transaction_integration_test.cpp``
- `tests/integration/sql_execution_integration_test.cpp` - 引用形式: ``tests/integration/sql_execution_integration_test.cpp``
- `storage_engine/buffer_pool_v2.h` - 引用形式: ``storage_engine/buffer_pool_v2.h``
- `src/core/sql_executor_interface.cpp` - 引用形式: ``src/core/sql_executor_interface.cpp``
- `src/storage_engine/buffer_pool.cpp` - 引用形式: ``src/storage_engine/buffer_pool.cpp``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `tests/unit/storage/page_test.cpp` - 引用形式: ``tests/unit/storage/page_test.cpp``
- `tests/unit/sql_parser/parser_new_test.cpp` - 引用形式: ``tests/unit/sql_parser/parser_new_test.cpp``
- `tests/unit/execution/join_executor_test.cpp` - 引用形式: ``tests/unit/execution/join_executor_test.cpp``
- `tests/unit/execution/set_operation_executor_test.cpp` - 引用形式: ``tests/unit/execution/set_operation_executor_test.cpp``
- `tests/integration/transaction_integration_test.cpp` - 引用形式: ``tests/integration/transaction_integration_test.cpp``
- `tests/integration/sql_execution_integration_test.cpp` - 引用形式: ``tests/integration/sql_execution_integration_test.cpp``
- `tests/integration/storage_engine_integration_test.cpp` - 引用形式: ``tests/integration/storage_engine_integration_test.cpp``
- `tests/performance/buffer_pool_performance_test.cpp` - 引用形式: ``tests/performance/buffer_pool_performance_test.cpp``
- `tests/performance/index_performance_test.cpp` - 引用形式: ``tests/performance/index_performance_test.cpp``
- `tests/performance/network_performance_test.cpp` - 引用形式: ``tests/performance/network_performance_test.cpp``
- `tests/performance/crud_performance_test.cpp` - 引用形式: ``tests/performance/crud_performance_test.cpp``
- `tests/performance/concurrent_performance_test.cpp` - 引用形式: ``tests/performance/concurrent_performance_test.cpp``
- `storage_engine/table_storage.cpp` - 引用形式: ``storage_engine/table_storage.cpp``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``
- `include/path/file.h` - 引用形式: ``include/path/file.h``
- `src/path/file.cpp` - 引用形式: ``src/path/file.cpp``
- `include/path/file.h` - 引用形式: ``include/path/file.h``
- `tests/unit/unified_executor_test.cpp` - 引用形式: ``tests/unit/unified_executor_test.cpp``
- `/home/liying/sqlcc/include/sql_parser/set_operation_node.h` - 引用形式: ``/home/liying/sqlcc/include/sql_parser/set_operation_node.h``
- `/home/liying/sqlcc/include/storage/buffer_pool_new.h` - 引用形式: ``/home/liying/sqlcc/include/storage/buffer_pool_new.h``
- `/home/liying/sqlcc/include/storage/buffer_pool_v2.h` - 引用形式: ``/home/liying/sqlcc/include/storage/buffer_pool_v2.h``
- `/home/liying/sqlcc/src/sql_parser/token_new.cpp` - 引用形式: ``/home/liying/sqlcc/src/sql_parser/token_new.cpp``
- `/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp``
- `/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp``
- `/home/liying/sqlcc/src/storage_engine/table_storage.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/table_storage.cpp``
- `/home/liying/sqlcc/tests/client_server/client_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/client_server/client_test.cpp``
- `/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp``
- `/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp``
- `/home/liying/sqlcc/tests/components/debug/memory_safety_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/debug/memory_safety_test.cpp``
- `/home/liying/sqlcc/tests/components/debug/test_performance_real.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/debug/test_performance_real.cpp``
- `/home/liying/sqlcc/tests/components/executor/index_optimization_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/index_optimization_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/index_usage_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/index_usage_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/privilege_consistency_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/privilege_consistency_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/set_operation_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/set_operation_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/unified_executor_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/unified_executor_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/unsupported_commands_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/unsupported_commands_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp``
- `/home/liying/sqlcc/tests/components/network/network_unit_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/network/network_unit_test.cpp``
- `/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp``
- `/home/liying/sqlcc/tests/components/security/test_revoke_persistence.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/security/test_revoke_persistence.cpp``
- `/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp``
- `/home/liying/sqlcc/tests/integration/advanced_sql/isql_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/advanced_sql/isql_integration_test.cpp``
- `/home/liying/sqlcc/tests/integration/advanced_sql/sql_executor_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/advanced_sql/sql_executor_integration_test.cpp``
- `/home/liying/sqlcc/tests/integration/basic_sql/simple_sql_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/basic_sql/simple_sql_test.cpp``
- `/home/liying/sqlcc/tests/integration/isql_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/isql_integration_test.cpp``
- `/home/liying/sqlcc/tests/integration/simple_sql_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/simple_sql_test.cpp``
- `/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp``
- `/home/liying/sqlcc/tests/integration/sql_executor_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/sql_executor_integration_test.cpp``
- `/home/liying/sqlcc/tests/legacy/test_dcl_ddl_persistence.cpp` - 引用形式: ``/home/liying/sqlcc/tests/legacy/test_dcl_ddl_persistence.cpp``
- `/home/liying/sqlcc/tests/network/aes_encryption_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/network/aes_encryption_test.cc``
- `/home/liying/sqlcc/tests/network/aes_network_integration_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/network/aes_network_integration_test.cc``
- `/home/liying/sqlcc/tests/network/sql_network_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/network/sql_network_test.cpp``
- `/home/liying/sqlcc/tests/network/tls_e2e_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/network/tls_e2e_test.cc``
- `/home/liying/sqlcc/tests/performance/basic/batch_prefetch_performance_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/basic/batch_prefetch_performance_test.cc``
- `/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc``
- `/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc``
- `/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc``
- `/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp``
- `/home/liying/sqlcc/tests/performance/disk_io_performance_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/disk_io_performance_test.h``
- `/home/liying/sqlcc/tests/performance/million_insert_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/million_insert_test.h``
- `/home/liying/sqlcc/tests/sql_executor/sql_executor_comprehensive_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_executor/sql_executor_comprehensive_test.cpp``
- `/home/liying/sqlcc/tests/sql_executor/sql_executor_minimal_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_executor/sql_executor_minimal_test.cpp``
- `/home/liying/sqlcc/tests/sql_executor/sql_executor_unit_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_executor/sql_executor_unit_test.cpp``
- `/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_output.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_output.cpp``
- `/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_simple.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_simple.cpp``
- `tests/unit/b_plus_tree_core_test.cc` - 引用形式: ``tests/unit/b_plus_tree_core_test.cc``
- `tests/unit/b_plus_tree_test.cc` - 引用形式: ``tests/unit/b_plus_tree_test.cc``
- `tests/unit/b_plus_tree_performance_test.cc` - 引用形式: ``tests/unit/b_plus_tree_performance_test.cc``
- `tests/unit/transaction_manager_test.cc` - 引用形式: ``tests/unit/transaction_manager_test.cc``
- `tests/unit/transaction_functional_test.cc` - 引用形式: ``tests/unit/transaction_functional_test.cc``
- `tests/unit/sql_parser_comprehensive_test.cc` - 引用形式: ``tests/unit/sql_parser_comprehensive_test.cc``
- `tests/integration/isql_integration_test.cpp` - 引用形式: ``tests/integration/isql_integration_test.cpp``
- `tests/unit/b_plus_tree_performance_test.cc` - 引用形式: ``tests/unit/b_plus_tree_performance_test.cc``
- `tests/unit/enterprise_performance_tests.cc` - 引用形式: ``tests/unit/enterprise_performance_tests.cc``
- `tests/unit/sql_parser_comprehensive_test.cc` - 引用形式: ``tests/unit/sql_parser_comprehensive_test.cc``
- `tests/unit/ast_nodes_comprehensive_test.cc` - 引用形式: ``tests/unit/ast_nodes_comprehensive_test.cc``
- `tests/unit/b_plus_tree_core_test.cc` - 引用形式: ``tests/unit/b_plus_tree_core_test.cc``
- `src/b_plus_tree_enhanced.cc` - 引用形式: ``src/b_plus_tree_enhanced.cc``
- `tests/unit/storage_engine_enhanced_test.cc` - 引用形式: ``tests/unit/storage_engine_enhanced_test.cc``
- `/home/liying/sqlcc/test_simple.cc` - 引用形式: ``/home/liying/sqlcc/test_simple.cc``
- `/home/liying/sqlcc/test_page_id_fix.cc` - 引用形式: ``/home/liying/sqlcc/test_page_id_fix.cc``
- `/home/liying/sqlcc/test_sync_functionality.cc` - 引用形式: ``/home/liying/sqlcc/test_sync_functionality.cc``
- `tests/page_enhanced_test.cc` - 引用形式: ``tests/page_enhanced_test.cc``
- `tests/disk_manager_enhanced_test.cc` - 引用形式: ``tests/disk_manager_enhanced_test.cc``
- `tests/buffer_pool_enhanced_test.cc` - 引用形式: ``tests/buffer_pool_enhanced_test.cc``
- `tests/network/aes_encryption_test.cc` - 引用形式: ``tests/network/aes_encryption_test.cc``
- `tests/network/aes_network_integration_test.cc` - 引用形式: ``tests/network/aes_network_integration_test.cc``
- `tests/network/aes_encryption_test.cc` - 引用形式: ``tests/network/aes_encryption_test.cc``
- `tests/network/aes_encryption_test.cc` - 引用形式: ``tests/network/aes_encryption_test.cc``
- `tests/network/aes_network_integration_test.cc` - 引用形式: ``tests/network/aes_network_integration_test.cc``
- `tests/network/aes_encryption_test.cc` - 引用形式: ``tests/network/aes_encryption_test.cc``
- `tests/network/tls_e2e_test.cc` - 引用形式: ``tests/network/tls_e2e_test.cc``
- `tests/unit/parser/json_test.cpp` - 引用形式: ``tests/unit/parser/json_test.cpp``
- `tests/unit/parser/set_operation_test.cpp` - 引用形式: ``tests/unit/parser/set_operation_test.cpp``
- `tests/unit/parser/recursive_query_test.cpp` - 引用形式: ``tests/unit/parser/recursive_query_test.cpp``
- `tests/unit/parser/data_types_test.cpp` - 引用形式: ``tests/unit/parser/data_types_test.cpp``
- `tests/integration/isql_integration_test.cpp` - 引用形式: ``tests/integration/isql_integration_test.cpp``
- `tests/unit/unified_executor_test.cpp` - 引用形式: ``tests/unit/unified_executor_test.cpp``
- `src/b_plus_tree_enhanced.cc` - 引用形式: ``src/b_plus_tree_enhanced.cc``
- `tests/unit/b_plus_tree_core_test.cc` - 引用形式: ``tests/unit/b_plus_tree_core_test.cc``
- `src/b_plus_tree.cc` - 引用形式: ``src/b_plus_tree.cc``
- `tests/integration/isql_integration_test.cpp` - 引用形式: ``tests/integration/isql_integration_test.cpp``
- `tests/unit/ddl/ddl_performance_test.cpp` - 引用形式: ``tests/unit/ddl/ddl_performance_test.cpp``
- `tests/unit/security/dcl_advanced_test.cpp` - 引用形式: ``tests/unit/security/dcl_advanced_test.cpp``
- `include/sql_parser/advanced_ast.h` - 引用形式: ``include/sql_parser/advanced_ast.h``
- `src/sql_parser/advanced_ast.cpp` - 引用形式: ``src/sql_parser/advanced_ast.cpp``
- `tests/sql_parser/advanced_ast_test.cpp` - 引用形式: ``tests/sql_parser/advanced_ast_test.cpp``
- `tests/advanced_sql/test_framework.h` - 引用形式: ``tests/advanced_sql/test_framework.h``
- `tests/unit/ast_nodes_test.cpp` - 引用形式: ``tests/unit/ast_nodes_test.cpp``
- `tests/unit/execution_engine_test.cpp` - 引用形式: ``tests/unit/execution_engine_test.cpp``
- `tests/integration/sql_executor_integration_test.cpp` - 引用形式: ``tests/integration/sql_executor_integration_test.cpp``
- `src/execution_engine/dml_executor.cpp` - 引用形式: ``src/execution_engine/dml_executor.cpp``
- `include/sql_parser/advanced_ast.h` - 引用形式: ``include/sql_parser/advanced_ast.h``
- `include/sql_parser/having_clause_node.h` - 引用形式: ``include/sql_parser/having_clause_node.h``
- `src/sql_parser/having_clause_node.cpp` - 引用形式: ``src/sql_parser/having_clause_node.cpp``
- `tests/advanced_sql/having_clause_test.cpp` - 引用形式: ``tests/advanced_sql/having_clause_test.cpp``
- `include/sql_parser/advanced_ast.h` - 引用形式: ``include/sql_parser/advanced_ast.h``
- `include/sql_parser/having_clause_node.h` - 引用形式: ``include/sql_parser/having_clause_node.h``
- `src/sql_parser/having_clause_node.cpp` - 引用形式: ``src/sql_parser/having_clause_node.cpp``
- `tests/advanced_sql/having_clause_test.cpp` - 引用形式: ``tests/advanced_sql/having_clause_test.cpp``
- `include/sql_parser/advanced_ast.h` - 引用形式: ``include/sql_parser/advanced_ast.h``
- `include/sql_parser/advanced_ast.h` - 引用形式: ``include/sql_parser/advanced_ast.h``
- `include/sql_parser/advanced_ast.h` - 引用形式: ``include/sql_parser/advanced_ast.h``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``
- `/home/liying/sqlcc/include/storage/buffer_pool_v3.h` - 引用形式: ``/home/liying/sqlcc/include/storage/buffer_pool_v3.h``
- `/tests/components/executor/privilege_consistency_test.cpp` - 引用形式: ``/tests/components/executor/privilege_consistency_test.cpp``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``
- `tests/set_operation_executor_test.cpp` - 引用形式: ``tests/set_operation_executor_test.cpp``
- `tests/integration/set_operation_integration_test.cpp` - 引用形式: ``tests/integration/set_operation_integration_test.cpp``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `tests/unit/basic/decimal_test.cpp` - 引用形式: ``tests/unit/basic/decimal_test.cpp``
- `tests/unit/core/error_handler_test.cpp` - 引用形式: ``tests/unit/core/error_handler_test.cpp``
- `tests/unit/core/sql_executor_test.cpp` - 引用形式: ``tests/unit/core/sql_executor_test.cpp``
- `tests/unit/core/transaction_manager_test.cpp` - 引用形式: ``tests/unit/core/transaction_manager_test.cpp``
- `tests/unit/parser/ast_node_test.cpp` - 引用形式: ``tests/unit/parser/ast_node_test.cpp``
- `tests/unit/parser/token_test.cpp` - 引用形式: ``tests/unit/parser/token_test.cpp``
- `tests/unit/parser/function_parser_test.cpp` - 引用形式: ``tests/unit/parser/function_parser_test.cpp``
- `tests/unit/executor/recursive_query_executor_test.cpp` - 引用形式: ``tests/unit/executor/recursive_query_executor_test.cpp``
- `tests/unit/executor/subquery_executor_test.cpp` - 引用形式: ``tests/unit/executor/subquery_executor_test.cpp``
- `tests/unit/executor/aggregate_executor_test.cpp` - 引用形式: ``tests/unit/executor/aggregate_executor_test.cpp``
- `tests/unit/sql_executor_test.cpp` - 引用形式: ``tests/unit/sql_executor_test.cpp``
- `tests/integration/advanced_sql/sql_executor_integration_test.cpp` - 引用形式: ``tests/integration/advanced_sql/sql_executor_integration_test.cpp``
- `tests/unit/execution/window_function_executor_test.cpp` - 引用形式: ``tests/unit/execution/window_function_executor_test.cpp``
- `tests/unit/execution/join_executor_test.cpp` - 引用形式: ``tests/unit/execution/join_executor_test.cpp``
- `tests/unit/execution/set_operation_executor_test.cpp` - 引用形式: ``tests/unit/execution/set_operation_executor_test.cpp``
- `tests/unit/execution/subquery_executor_test.cpp` - 引用形式: ``tests/unit/execution/subquery_executor_test.cpp``
- `tests/unit/execution/recursive_query_executor_test.cpp` - 引用形式: ``tests/unit/execution/recursive_query_executor_test.cpp``
- `tests/unit/execution/function_executor_test.cpp` - 引用形式: ``tests/unit/execution/function_executor_test.cpp``
- `tests/unit/execution/load_data_executor_test.cpp` - 引用形式: ``tests/unit/execution/load_data_executor_test.cpp``
- `tests/unit/execution/join_executor_test.cpp` - 引用形式: ``tests/unit/execution/join_executor_test.cpp``
- `tests/unit/execution/set_operation_executor_test.cpp` - 引用形式: ``tests/unit/execution/set_operation_executor_test.cpp``
- `tests/unit/execution/subquery_executor_test.cpp` - 引用形式: ``tests/unit/execution/subquery_executor_test.cpp``
- `tests/unit/execution/recursive_query_executor_test.cpp` - 引用形式: ``tests/unit/execution/recursive_query_executor_test.cpp``
- `tests/unit/execution/function_executor_test.cpp` - 引用形式: ``tests/unit/execution/function_executor_test.cpp``
- `tests/unit/execution/load_data_executor_test.cpp` - 引用形式: ``tests/unit/execution/load_data_executor_test.cpp``
- `tests/unit/execution/window_function_executor_test.cpp` - 引用形式: ``tests/unit/execution/window_function_executor_test.cpp``
- `tests/unit/sql_executor_test.cpp` - 引用形式: ``tests/unit/sql_executor_test.cpp``
- `tests/integration/advanced_sql/sql_executor_integration_test.cpp` - 引用形式: ``tests/integration/advanced_sql/sql_executor_integration_test.cpp``
- `tests/unit/logger_test.cpp` - 引用形式: ``tests/unit/logger_test.cpp``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `tests/unit/basic/decimal_test.cpp` - 引用形式: ``tests/unit/basic/decimal_test.cpp``
- `tests/unit/logger_test.cpp` - 引用形式: ``tests/unit/logger_test.cpp``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `tests/unit/basic/decimal_test.cpp` - 引用形式: ``tests/unit/basic/decimal_test.cpp``
- `tests/unit/core/stored_procedure_manager_test.cpp` - 引用形式: ``tests/unit/core/stored_procedure_manager_test.cpp``
- `tests/integration/procedure_trigger_integration_test.cpp` - 引用形式: ``tests/integration/procedure_trigger_integration_test.cpp``
- `tests/sql/simple_procedure_test.cpp` - 引用形式: ``tests/sql/simple_procedure_test.cpp``
- `tests/integration/session_manager_real_test.cpp` - 引用形式: ``tests/integration/session_manager_real_test.cpp``
- `tests/unit/core/stored_procedure_manager_test.cpp` - 引用形式: ``tests/unit/core/stored_procedure_manager_test.cpp``
- `tests/integration/procedure_trigger_integration_test.cpp` - 引用形式: ``tests/integration/procedure_trigger_integration_test.cpp``
- `tests/sql/simple_procedure_test.cpp` - 引用形式: ``tests/sql/simple_procedure_test.cpp``
- `tests/integration/session_manager_real_test.cpp` - 引用形式: ``tests/integration/session_manager_real_test.cpp``
- `tests/unit/test_lexer_fix.cpp` - 引用形式: ``tests/unit/test_lexer_fix.cpp``
- `tests/unit/parser_select_test.cpp` - 引用形式: ``tests/unit/parser_select_test.cpp``
- `tests/unit/parser_create_table_test.cpp` - 引用形式: ``tests/unit/parser_create_table_test.cpp``
- `tests/unit/parser_drop_table_test.cpp` - 引用形式: ``tests/unit/parser_drop_table_test.cpp``
- `tests/unit/parser_alter_table_test.cpp` - 引用形式: ``tests/unit/parser_alter_table_test.cpp``
- `tests/unit/parser_select_test.cpp` - 引用形式: ``tests/unit/parser_select_test.cpp``
- `tests/unit/parser_create_table_test.cpp` - 引用形式: ``tests/unit/parser_create_table_test.cpp``
- `tests/unit/parser_drop_table_test.cpp` - 引用形式: ``tests/unit/parser_drop_table_test.cpp``
- `tests/unit/parser_alter_table_test.cpp` - 引用形式: ``tests/unit/parser_alter_table_test.cpp``
- `tests/unit/test_lexer_fix.cpp` - 引用形式: ``tests/unit/test_lexer_fix.cpp``
- `tests/unit/network/network_boundary_test.cpp` - 引用形式: ``tests/unit/network/network_boundary_test.cpp``
- `tests/unit/simple_network_test.cpp` - 引用形式: ``tests/unit/simple_network_test.cpp``
- `tests/integration/client_connection_real_test.cpp` - 引用形式: ``tests/integration/client_connection_real_test.cpp``
- `tests/integration/server_network_manager_real_test.cpp` - 引用形式: ``tests/integration/server_network_manager_real_test.cpp``
- `tests/integration/connection_handler_real_test.cpp` - 引用形式: ``tests/integration/connection_handler_real_test.cpp``
- `tests/integration/sql_network_test.cpp` - 引用形式: ``tests/integration/sql_network_test.cpp``
- `tests/integration/client_test.cpp` - 引用形式: ``tests/integration/client_test.cpp``
- `tests/integration/server_manager.cpp` - 引用形式: ``tests/integration/server_manager.cpp``
- `tests/unit/network/network_boundary_test.cpp` - 引用形式: ``tests/unit/network/network_boundary_test.cpp``
- `tests/unit/simple_network_test.cpp` - 引用形式: ``tests/unit/simple_network_test.cpp``
- `tests/integration/client_connection_real_test.cpp` - 引用形式: ``tests/integration/client_connection_real_test.cpp``
- `tests/integration/server_network_manager_real_test.cpp` - 引用形式: ``tests/integration/server_network_manager_real_test.cpp``
- `tests/integration/connection_handler_real_test.cpp` - 引用形式: ``tests/integration/connection_handler_real_test.cpp``
- `tests/integration/sql_network_test.cpp` - 引用形式: ``tests/integration/sql_network_test.cpp``
- `tests/integration/client_test.cpp` - 引用形式: ``tests/integration/client_test.cpp``
- `tests/integration/server_manager.cpp` - 引用形式: ``tests/integration/server_manager.cpp``
- `tests/unit/basic_bplus_tree_test.cpp` - 引用形式: ``tests/unit/basic_bplus_tree_test.cpp``
- `tests/unit/index_maintenance_test.cpp` - 引用形式: ``tests/unit/index_maintenance_test.cpp``
- `src/storage_engine/buffer_pool_new.cpp` - 引用形式: ``src/storage_engine/buffer_pool_new.cpp``
- `tests/components/debug/memory_audit_tool.cpp` - 引用形式: ``tests/components/debug/memory_audit_tool.cpp``
- `src/utils/file_descriptor.cpp` - 引用形式: ``src/utils/file_descriptor.cpp``
- `src/storage/buffer_pool_new.cpp` - 引用形式: ``src/storage/buffer_pool_new.cpp``
- `src/storage_engine/buffer_pool_new.cpp` - 引用形式: ``src/storage_engine/buffer_pool_new.cpp``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``
- `src/storage_engine/buffer_pool.cpp` - 引用形式: ``src/storage_engine/buffer_pool.cpp``
- `src/storage_engine/table_storage.cpp` - 引用形式: ``src/storage_engine/table_storage.cpp``
- `tests/performance/crud/real_crud_performance_test.cpp` - 引用形式: ``tests/performance/crud/real_crud_performance_test.cpp``
- `/home/liying/sqlcc/include/sql_parser/set_operation_node.h` - 引用形式: ``/home/liying/sqlcc/include/sql_parser/set_operation_node.h``
- `/home/liying/sqlcc/include/storage/buffer_pool_new.h` - 引用形式: ``/home/liying/sqlcc/include/storage/buffer_pool_new.h``
- `/home/liying/sqlcc/include/storage/buffer_pool_v2.h` - 引用形式: ``/home/liying/sqlcc/include/storage/buffer_pool_v2.h``
- `/home/liying/sqlcc/src/sql_parser/token_new.cpp` - 引用形式: ``/home/liying/sqlcc/src/sql_parser/token_new.cpp``
- `/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp``
- `/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp``
- `/home/liying/sqlcc/src/storage_engine/table_storage.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/table_storage.cpp``
- `/home/liying/sqlcc/tests/client_server/client_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/client_server/client_test.cpp``
- `/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp``
- `/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp``
- `/home/liying/sqlcc/tests/components/debug/test_performance_real.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/debug/test_performance_real.cpp``
- `/home/liying/sqlcc/tests/components/executor/index_optimization_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/index_optimization_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/index_usage_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/index_usage_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/privilege_consistency_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/privilege_consistency_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/set_operation_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/set_operation_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/unified_executor_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/unified_executor_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp``
- `/home/liying/sqlcc/tests/components/network/network_unit_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/network/network_unit_test.cpp``
- `/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp``
- `/home/liying/sqlcc/tests/integration/advanced_sql/isql_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/advanced_sql/isql_integration_test.cpp``
- `/home/liying/sqlcc/tests/integration/advanced_sql/sql_executor_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/advanced_sql/sql_executor_integration_test.cpp``
- `/home/liying/sqlcc/tests/integration/basic_sql/simple_sql_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/basic_sql/simple_sql_test.cpp``
- `/home/liying/sqlcc/tests/integration/isql_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/isql_integration_test.cpp``
- `/home/liying/sqlcc/tests/integration/simple_sql_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/simple_sql_test.cpp``
- `/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp``
- `/home/liying/sqlcc/tests/integration/sql_executor_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/sql_executor_integration_test.cpp``
- `/home/liying/sqlcc/tests/legacy/test_dcl_ddl_persistence.cpp` - 引用形式: ``/home/liying/sqlcc/tests/legacy/test_dcl_ddl_persistence.cpp``
- `/home/liying/sqlcc/tests/network/aes_encryption_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/network/aes_encryption_test.cc``
- `/home/liying/sqlcc/tests/network/aes_network_integration_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/network/aes_network_integration_test.cc``
- `/home/liying/sqlcc/tests/network/sql_network_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/network/sql_network_test.cpp``
- `/home/liying/sqlcc/tests/network/tls_e2e_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/network/tls_e2e_test.cc``
- `/home/liying/sqlcc/tests/performance/basic/batch_prefetch_performance_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/basic/batch_prefetch_performance_test.cc``
- `/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc``
- `/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc``
- `/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc``
- `/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp``
- `/home/liying/sqlcc/tests/performance/disk_io_performance_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/disk_io_performance_test.h``
- `/home/liying/sqlcc/tests/performance/million_insert_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/million_insert_test.h``
- `/home/liying/sqlcc/tests/sql_executor/sql_executor_comprehensive_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_executor/sql_executor_comprehensive_test.cpp``
- `/home/liying/sqlcc/tests/sql_executor/sql_executor_minimal_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_executor/sql_executor_minimal_test.cpp``
- `/home/liying/sqlcc/tests/sql_executor/sql_executor_unit_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_executor/sql_executor_unit_test.cpp``
- `/home/liying/sqlcc/tests/test_revoke_persistence.cpp` - 引用形式: ``/home/liying/sqlcc/tests/test_revoke_persistence.cpp``
- `/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_output.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_output.cpp``
- `/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_simple.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_simple.cpp``
- `/home/liying/sqlcc/tests/unsupported_commands_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unsupported_commands_test.cpp``
- `src/storage_engine/metadata_manager.cpp` - 引用形式: ``src/storage_engine/metadata_manager.cpp``
- `include/storage/metadata_manager.h` - 引用形式: ``include/storage/metadata_manager.h``
- `src/storage_engine/system_tables.cpp` - 引用形式: ``src/storage_engine/system_tables.cpp``
- `include/core/permissions.h` - 引用形式: ``include/core/permissions.h``
- `src/storage_engine/advanced_index.h` - 引用形式: ``src/storage_engine/advanced_index.h``
- `storage_engine/advanced_index.h` - 引用形式: ``storage_engine/advanced_index.h``
- `/tests/security/memory_safety_framework.cpp` - 引用形式: ``/tests/security/memory_safety_framework.cpp``
- `/tests/security/memory_safety_framework.cpp` - 引用形式: ``/tests/security/memory_safety_framework.cpp``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``
- `tests/unit/storage_engine/lock_manager_test.cpp` - 引用形式: ``tests/unit/storage_engine/lock_manager_test.cpp``
- `tests/unit/executor/query_executor_test.cpp` - 引用形式: ``tests/unit/executor/query_executor_test.cpp``
- `tests/unit/network/connection_test.cpp` - 引用形式: ``tests/unit/network/connection_test.cpp``
- `tests/integration/sql/ddl_test_suite.cpp` - 引用形式: ``tests/integration/sql/ddl_test_suite.cpp``
- `tests/integration/sql/dml_test_suite.cpp` - 引用形式: ``tests/integration/sql/dml_test_suite.cpp``
- `tests/integration/sql/query_test_suite.cpp` - 引用形式: ``tests/integration/sql/query_test_suite.cpp``
- `tests/integration/sql/constraint_test_suite.cpp` - 引用形式: ``tests/integration/sql/constraint_test_suite.cpp``
- `tests/unit/storage_engine/concurrency_test.cpp` - 引用形式: ``tests/unit/storage_engine/concurrency_test.cpp``
- `/tests/security/memory_safety_framework.cpp` - 引用形式: ``/tests/security/memory_safety_framework.cpp``
- `/home/liying/sqlcc/debug_privileges_test.cpp` - 引用形式: ``/home/liying/sqlcc/debug_privileges_test.cpp``
- `basic/data_types_test.cpp` - 引用形式: ``basic/data_types_test.cpp``
- `basic/decimal_test.cpp` - 引用形式: ``basic/decimal_test.cpp``
- `parser/tests_development/debug_token_types.cpp` - 引用形式: ``parser/tests_development/debug_token_types.cpp``
- `parser/tests_development/debug_lexer_simple.cpp` - 引用形式: ``parser/tests_development/debug_lexer_simple.cpp``
- `parser/tests_development/debug_lexer_output.cpp` - 引用形式: ``parser/tests_development/debug_lexer_output.cpp``
- `execution/task_scheduler_test.cpp` - 引用形式: ``execution/task_scheduler_test.cpp``
- `core/stored_procedure_manager_test.cpp` - 引用形式: ``core/stored_procedure_manager_test.cpp``
- `storage/wal_buffer_test.cpp` - 引用形式: ``storage/wal_buffer_test.cpp``
- `storage/lazy_writer_test.cpp` - 引用形式: ``storage/lazy_writer_test.cpp``
- `/home/liying/sqlcc/test_constraint_demo.cpp` - 引用形式: ``/home/liying/sqlcc/test_constraint_demo.cpp``
- `/home/liying/sqlcc/test_join_functionality.cpp` - 引用形式: ``/home/liying/sqlcc/test_join_functionality.cpp``
- `/home/liying/sqlcc/test_dcl_parsing.cpp` - 引用形式: ``/home/liying/sqlcc/test_dcl_parsing.cpp``
- `/home/liying/sqlcc/simple_procedure_test.cpp` - 引用形式: ``/home/liying/sqlcc/simple_procedure_test.cpp``
- `advanced_sql/isql_integration_test.cpp` - 引用形式: ``advanced_sql/isql_integration_test.cpp``
- `advanced_sql/sql_executor_integration_test.cpp` - 引用形式: ``advanced_sql/sql_executor_integration_test.cpp``
- `window/row_number_test.cpp` - 引用形式: ``window/row_number_test.cpp``
- `distinct/select_distinct_test.cpp` - 引用形式: ``distinct/select_distinct_test.cpp``
- `basic_sql/simple_sql_test.cpp` - 引用形式: ``basic_sql/simple_sql_test.cpp``
- `subquery/scalar_subquery_test.cpp` - 引用形式: ``subquery/scalar_subquery_test.cpp``
- `grouping/having_clause_test.cpp` - 引用形式: ``grouping/having_clause_test.cpp``
- `grouping/group_by_test.cpp` - 引用形式: ``grouping/group_by_test.cpp``
- `join/inner_join_test.cpp` - 引用形式: ``join/inner_join_test.cpp``
- `core/test_gtest.cpp` - 引用形式: ``core/test_gtest.cpp``
- `core/system_database_test.cpp` - 引用形式: ``core/system_database_test.cpp``
- `transaction/transaction_manager_test.cpp` - 引用形式: ``transaction/transaction_manager_test.cpp``
- `network/network_unit_test.cpp` - 引用形式: ``network/network_unit_test.cpp``
- `network/connection_state_machine_test.cpp` - 引用形式: ``network/connection_state_machine_test.cpp``
- `network/connection_handler_test.cpp` - 引用形式: ``network/connection_handler_test.cpp``
- `network/data_transmission_validator_test.cpp` - 引用形式: ``network/data_transmission_validator_test.cpp``
- `parser/test_show_grants.cpp` - 引用形式: ``parser/test_show_grants.cpp``
- `parser/test_percent_operator.cpp` - 引用形式: ``parser/test_percent_operator.cpp``
- `parser/test_show_commands.cpp` - 引用形式: ``parser/test_show_commands.cpp``
- `parser/test_colon_simple.cpp` - 引用形式: ``parser/test_colon_simple.cpp``
- `parser/dml_test.cpp` - 引用形式: ``parser/dml_test.cpp``
- `parser/dcl_test.cpp` - 引用形式: ``parser/dcl_test.cpp``
- `parser/ddl_test.cpp` - 引用形式: ``parser/ddl_test.cpp``
- `parser/set_operation_parser_test.cpp` - 引用形式: ``parser/set_operation_parser_test.cpp``
- `parser/ast_nodes_test.cpp` - 引用形式: ``parser/ast_nodes_test.cpp``
- `parser/compare_values_test.cpp` - 引用形式: ``parser/compare_values_test.cpp``
- `parser/test_create_table.cpp` - 引用形式: ``parser/test_create_table.cpp``
- `parser/parser_verification_test.cpp` - 引用形式: ``parser/parser_verification_test.cpp``
- `parser/test_colon_parsing.cpp` - 引用形式: ``parser/test_colon_parsing.cpp``
- `parser/dcl_test_advanced.cpp` - 引用形式: ``parser/dcl_test_advanced.cpp``
- `parser/simple_alter_test.cpp` - 引用形式: ``parser/simple_alter_test.cpp``
- `parser/final_dcl_test.cpp` - 引用形式: ``parser/final_dcl_test.cpp``
- `parser/comprehensive_dcl_test.cpp` - 引用形式: ``parser/comprehensive_dcl_test.cpp``
- `parser/alter_table_parser_test.cpp` - 引用形式: ``parser/alter_table_parser_test.cpp``
- `parser/test_simple_create.cpp` - 引用形式: ``parser/test_simple_create.cpp``
- `parser/test_percent_simple.cpp` - 引用形式: ``parser/test_percent_simple.cpp``
- `parser/dcl_parser_test.cpp` - 引用形式: ``parser/dcl_parser_test.cpp``
- `parser/test_create_view.cpp` - 引用形式: ``parser/test_create_view.cpp``
- `/home/liying/sqlcc/debug_privileges_test.cpp` - 引用形式: ``/home/liying/sqlcc/debug_privileges_test.cpp``
- `/home/liying/sqlcc/test_constraint_demo.cpp` - 引用形式: ``/home/liying/sqlcc/test_constraint_demo.cpp``
- `/home/liying/sqlcc/test_join_functionality.cpp` - 引用形式: ``/home/liying/sqlcc/test_join_functionality.cpp``
- `/home/liying/sqlcc/test_dcl_parsing.cpp` - 引用形式: ``/home/liying/sqlcc/test_dcl_parsing.cpp``
- `/home/liying/sqlcc/simple_procedure_test.cpp` - 引用形式: ``/home/liying/sqlcc/simple_procedure_test.cpp``
- `tests/unit/simple_network_test.cpp` - 引用形式: ``tests/unit/simple_network_test.cpp``
- `tests/unit/advanced_sql92_test.cpp` - 引用形式: ``tests/unit/advanced_sql92_test.cpp``
- `tests/set_operation_test.cpp` - 引用形式: ``tests/set_operation_test.cpp``
- `tests/recursive_query_test.cpp` - 引用形式: ``tests/recursive_query_test.cpp``
- `tests/storage_engine/transaction_manager_test.cpp` - 引用形式: ``tests/storage_engine/transaction_manager_test.cpp``
- `tests/network/protocol_test.cpp` - 引用形式: ``tests/network/protocol_test.cpp``
- `tests/performance/stress_test.cpp` - 引用形式: ``tests/performance/stress_test.cpp``
- `tests/stability/long_running_test.cpp` - 引用形式: ``tests/stability/long_running_test.cpp``
- `tests/unit/network/network_boundary_test.cpp` - 引用形式: ``tests/unit/network/network_boundary_test.cpp``
- `/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp``
- `/home/liying/sqlcc/tests/unit/basic/final_dcl_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unit/basic/final_dcl_test.cpp``
- `/home/liying/sqlcc/tests/unit/basic/comprehensive_dcl_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unit/basic/comprehensive_dcl_test.cpp``
- `/home/liying/sqlcc/tests/unit/basic/test_show_grants.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unit/basic/test_show_grants.cpp``
- `tests/debug_insert.cpp` - 引用形式: ``tests/debug_insert.cpp``
- `tests/unit/database_manager_test_development.cpp` - 引用形式: ``tests/unit/database_manager_test_development.cpp``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``
- `src/storage_engine/table_storage.cpp` - 引用形式: ``src/storage_engine/table_storage.cpp``
- `tests/advanced_sql/join/inner_join_test.cpp` - 引用形式: ``tests/advanced_sql/join/inner_join_test.cpp``
- `/home/liying/sqlcc/tests/network/server_network_manager_real_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/network/server_network_manager_real_test.cpp``
- `src/storage_engine/table_storage.cpp` - 引用形式: ``src/storage_engine/table_storage.cpp``
- `tests/advanced_sql/join/inner_join_test.cpp` - 引用形式: ``tests/advanced_sql/join/inner_join_test.cpp``
- `tests/test_executor.cpp` - 引用形式: ``tests/test_executor.cpp``
- `tests/unit/executor/query_processor_test.cpp` - 引用形式: ``tests/unit/executor/query_processor_test.cpp``
- `tests/test_executor.cpp` - 引用形式: ``tests/test_executor.cpp``
- `tests/unit/logger_test.cpp` - 引用形式: ``tests/unit/logger_test.cpp``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `tests/unit/basic/decimal_test.cpp` - 引用形式: ``tests/unit/basic/decimal_test.cpp``
- `tests/unit/logger_test.cpp` - 引用形式: ``tests/unit/logger_test.cpp``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `tests/unit/basic/decimal_test.cpp` - 引用形式: ``tests/unit/basic/decimal_test.cpp``
- `tests/integration/sql_92_comprehensive_test.cpp` - 引用形式: ``tests/integration/sql_92_comprehensive_test.cpp``
- `tests/integration/session_manager_real_test.cpp` - 引用形式: ``tests/integration/session_manager_real_test.cpp``
- `tests/integration/procedure_trigger_integration_test.cpp` - 引用形式: ``tests/integration/procedure_trigger_integration_test.cpp``
- `tests/performance/million_insert_test.cc` - 引用形式: ``tests/performance/million_insert_test.cc``
- `tests/performance/mixed_workload_test.cc` - 引用形式: ``tests/performance/mixed_workload_test.cc``
- `tests/performance/index_constraint_benchmark.cc` - 引用形式: ``tests/performance/index_constraint_benchmark.cc``
- `tests/performance/large_scale_index_constraint_test.cc` - 引用形式: ``tests/performance/large_scale_index_constraint_test.cc``
- `tests/security/memory_safety_framework.cpp` - 引用形式: ``tests/security/memory_safety_framework.cpp``
- `tests/integration/aes_encryption_test.cc` - 引用形式: ``tests/integration/aes_encryption_test.cc``
- `tests/integration/tls_e2e_test.cc` - 引用形式: ``tests/integration/tls_e2e_test.cc``
- `tests/legacy/simple_db_test.cpp` - 引用形式: ``tests/legacy/simple_db_test.cpp``
- `tests/legacy/simple_dcl_ddl_test.cpp` - 引用形式: ``tests/legacy/simple_dcl_ddl_test.cpp``
- `tests/legacy/persistence_check.cpp` - 引用形式: ``tests/legacy/persistence_check.cpp``
- `tests/legacy/user_persistence_test.cpp` - 引用形式: ``tests/legacy/user_persistence_test.cpp``
- `tests/performance/million_insert_test.cc` - 引用形式: ``tests/performance/million_insert_test.cc``
- `tests/security/memory_safety_framework.cpp` - 引用形式: ``tests/security/memory_safety_framework.cpp``
- `tests/legacy/simple_db_test.cpp` - 引用形式: ``tests/legacy/simple_db_test.cpp``
- `tests/integration/sql_92_comprehensive_test.cpp` - 引用形式: ``tests/integration/sql_92_comprehensive_test.cpp``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `tests/unit/basic/decimal_test.cpp` - 引用形式: ``tests/unit/basic/decimal_test.cpp``
- `tests/unit/logger_test.cpp` - 引用形式: ``tests/unit/logger_test.cpp``
- `tests/unit/storage/disk_io_performance_test.h` - 引用形式: ``tests/unit/storage/disk_io_performance_test.h``
- `tests/unit/storage/index_constraint_benchmark.cc` - 引用形式: ``tests/unit/storage/index_constraint_benchmark.cc``
- `tests/sql/simple_token_test.cpp` - 引用形式: ``tests/sql/simple_token_test.cpp``
- `tests/sql/test_constraint_demo.cpp` - 引用形式: ``tests/sql/test_constraint_demo.cpp``
- `tests/sql/test_dcl_parsing.cpp` - 引用形式: ``tests/sql/test_dcl_parsing.cpp``
- `tests/sql/test_join_functionality.cpp` - 引用形式: ``tests/sql/test_join_functionality.cpp``
- `tests/test_executor.cpp` - 引用形式: ``tests/test_executor.cpp``
- `tests/network/simple_query_test.cpp` - 引用形式: ``tests/network/simple_query_test.cpp``
- `tests/network/client_test_standalone.cpp` - 引用形式: ``tests/network/client_test_standalone.cpp``
- `tests/network/mysql_client_test.cpp` - 引用形式: ``tests/network/mysql_client_test.cpp``
- `tests/network/mysql_server_test.cpp` - 引用形式: ``tests/network/mysql_server_test.cpp``
- `tests/network/concurrent_performance_test.cpp` - 引用形式: ``tests/network/concurrent_performance_test.cpp``
- `tests/integration/sql_92_comprehensive_test.cpp` - 引用形式: ``tests/integration/sql_92_comprehensive_test.cpp``
- `tests/unit/storage/disk_io_performance_test.h` - 引用形式: ``tests/unit/storage/disk_io_performance_test.h``
- `tests/sql/simple_token_test.cpp` - 引用形式: ``tests/sql/simple_token_test.cpp``
- `tests/unit/parser/token_test.cpp` - 引用形式: ``tests/unit/parser/token_test.cpp``
- `tests/sql/test_constraint_demo.cpp` - 引用形式: ``tests/sql/test_constraint_demo.cpp``
- `tests/sql/test_dcl_parsing.cpp` - 引用形式: ``tests/sql/test_dcl_parsing.cpp``
- `tests/legacy/simple_dcl_ddl_test.cpp` - 引用形式: ``tests/legacy/simple_dcl_ddl_test.cpp``
- `tests/sql/test_join_functionality.cpp` - 引用形式: ``tests/sql/test_join_functionality.cpp``
- `tests/sql/simple_token_test.cpp` - 引用形式: ``tests/sql/simple_token_test.cpp``
- `tests/sql/test_constraint_demo.cpp` - 引用形式: ``tests/sql/test_constraint_demo.cpp``
- `tests/sql/test_dcl_parsing.cpp` - 引用形式: ``tests/sql/test_dcl_parsing.cpp``
- `tests/sql/test_join_functionality.cpp` - 引用形式: ``tests/sql/test_join_functionality.cpp``
- `tests/legacy/simple_dcl_ddl_test.cpp` - 引用形式: ``tests/legacy/simple_dcl_ddl_test.cpp``
- `tests/network/client_test_standalone.cpp` - 引用形式: ``tests/network/client_test_standalone.cpp``
- `tests/network/mysql_client_test.cpp` - 引用形式: ``tests/network/mysql_client_test.cpp``
- `tests/integration/client_connection_real_test.cpp` - 引用形式: ``tests/integration/client_connection_real_test.cpp``
- `tests/integration/client_network_manager_real_test.cpp` - 引用形式: ``tests/integration/client_network_manager_real_test.cpp``
- `tests/network/mysql_server_test.cpp` - 引用形式: ``tests/network/mysql_server_test.cpp``
- `tests/integration/server_network_manager_real_test.cpp` - 引用形式: ``tests/integration/server_network_manager_real_test.cpp``
- `tests/integration/connection_handler_real_test.cpp` - 引用形式: ``tests/integration/connection_handler_real_test.cpp``
- `tests/network/simple_query_test.cpp` - 引用形式: ``tests/network/simple_query_test.cpp``
- `tests/integration/tls_e2e_test.cc` - 引用形式: ``tests/integration/tls_e2e_test.cc``
- `tests/integration/aes_encryption_test.cc` - 引用形式: ``tests/integration/aes_encryption_test.cc``
- `tests/network/concurrent_performance_test.cpp` - 引用形式: ``tests/network/concurrent_performance_test.cpp``
- `tests/network/crud_performance_test.cpp` - 引用形式: ``tests/network/crud_performance_test.cpp``
- `tests/network/real_concurrent_test.cpp` - 引用形式: ``tests/network/real_concurrent_test.cpp``
- `tests/network/scalable_performance_test.cpp` - 引用形式: ``tests/network/scalable_performance_test.cpp``
- `tests/network/auth_test_client.cpp` - 引用形式: ``tests/network/auth_test_client.cpp``
- `tests/network/auth_test_server.cpp` - 引用形式: ``tests/network/auth_test_server.cpp``
- `tests/network/simple_auth_test.cpp` - 引用形式: ``tests/network/simple_auth_test.cpp``
- `tests/network/standalone_auth_test.cpp` - 引用形式: ``tests/network/standalone_auth_test.cpp``
- `tests/network/simple_query_test.cpp` - 引用形式: ``tests/network/simple_query_test.cpp``
- `tests/network/client_test_standalone.cpp` - 引用形式: ``tests/network/client_test_standalone.cpp``
- `tests/network/mysql_client_test.cpp` - 引用形式: ``tests/network/mysql_client_test.cpp``
- `tests/network/mysql_server_test.cpp` - 引用形式: ``tests/network/mysql_server_test.cpp``
- `tests/network/concurrent_performance_test.cpp` - 引用形式: ``tests/network/concurrent_performance_test.cpp``
- `tests/network/auth_test_client.cpp` - 引用形式: ``tests/network/auth_test_client.cpp``
- `tests/network/auth_test_server.cpp` - 引用形式: ``tests/network/auth_test_server.cpp``
- `tests/network/simple_auth_test.cpp` - 引用形式: ``tests/network/simple_auth_test.cpp``
- `tests/integration/client_connection_real_test.cpp` - 引用形式: ``tests/integration/client_connection_real_test.cpp``
- `tests/integration/server_network_manager_real_test.cpp` - 引用形式: ``tests/integration/server_network_manager_real_test.cpp``
- `tests/integration/connection_handler_real_test.cpp` - 引用形式: ``tests/integration/connection_handler_real_test.cpp``
- `tests/performance/index_constraint_benchmark.cc` - 引用形式: ``tests/performance/index_constraint_benchmark.cc``
- `tests/performance/large_scale_index_constraint_test.cc` - 引用形式: ``tests/performance/large_scale_index_constraint_test.cc``
- `tests/unit/storage/index_constraint_benchmark.cc` - 引用形式: ``tests/unit/storage/index_constraint_benchmark.cc``
- `tests/performance/index_constraint_benchmark.cc` - 引用形式: ``tests/performance/index_constraint_benchmark.cc``
- `tests/performance/large_scale_index_constraint_test.cc` - 引用形式: ``tests/performance/large_scale_index_constraint_test.cc``
- `src/sql_executor/ddl_executor.cpp` - 引用形式: ``src/sql_executor/ddl_executor.cpp``
- `src/execution/ddl_executor.cpp` - 引用形式: ``src/execution/ddl_executor.cpp``
- `tests/unit/logger_test.cpp` - 引用形式: ``tests/unit/logger_test.cpp``
- `tests/unit/logger_test.cpp` - 引用形式: ``tests/unit/logger_test.cpp``
- `src/dcl_test.cpp` - 引用形式: ``src/dcl_test.cpp``
- `src/ddl_test.cpp` - 引用形式: ``src/ddl_test.cpp``
- `tests/unit/network/network_boundary_test.cpp` - 引用形式: ``tests/unit/network/network_boundary_test.cpp``
- `tests/unit/logger_test.cpp` - 引用形式: ``tests/unit/logger_test.cpp``

### docs/AI-Agent/project_index_usage_guide.md

- `include/path/file.h` - 引用形式: ``include/path/file.h``
- `src/path/file.cpp` - 引用形式: ``src/path/file.cpp``
- `tests/unit/path/test.cpp` - 引用形式: ``tests/unit/path/test.cpp``
- `include/path/file.h` - 引用形式: ``include/path/file.h``

### docs/项目进展/mysql_protocol_evaluation.md

- `/home/liying/sqlcc/include/storage/buffer_pool_v3.h` - 引用形式: `[buffer_pool_v3.h](/home/liying/sqlcc/include/storage/buffer_pool_v3.h)`
- `/home/liying/sqlcc/include/core/unified_executor.h` - 引用形式: `[unified_executor.h](src/core/unified_executor.h)`

### docs/project/memory_audit_report.md

- `src/core/database_manager.cpp` - 引用形式: ``src/core/database_manager.cpp``
- `src/sql_parser/lexer_new.cpp` - 引用形式: ``src/sql_parser/lexer_new.cpp``
- `src/sql_parser/token_new.cpp` - 引用形式: ``src/sql_parser/token_new.cpp``
- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `src/storage_engine/buffer_pool.cpp` - 引用形式: ``src/storage_engine/buffer_pool.cpp``
- `src/storage_engine/buffer_pool_new.cpp` - 引用形式: ``src/storage_engine/buffer_pool_new.cpp``
- `src/storage_engine/buffer_pool_sharded.cpp` - 引用形式: ``src/storage_engine/buffer_pool_sharded.cpp``
- `src/storage_engine/disk_manager.cpp` - 引用形式: ``src/storage_engine/disk_manager.cpp``
- `src/storage_engine/replace_strategy.cpp` - 引用形式: ``src/storage_engine/replace_strategy.cpp``
- `src/storage_engine/storage_engine.cpp` - 引用形式: ``src/storage_engine/storage_engine.cpp``
- `src/storage_engine/table_storage.cpp` - 引用形式: ``src/storage_engine/table_storage.cpp``

### docs/project/header_index.md

- `core/error_handler.h` - 引用形式: ``core/error_handler.h``
- `sql_parser/ast_node.h` - 引用形式: ``sql_parser/ast_node.h``
- `sql_parser/ast_nodes.h` - 引用形式: ``sql_parser/ast_nodes.h``
- `sql_parser/load_data_ast.h` - 引用形式: ``sql_parser/load_data_ast.h``
- `sql_parser/node_visitor.h` - 引用形式: ``sql_parser/node_visitor.h``
- `sql_parser/load_data_ast.h` - 引用形式: ``sql_parser/load_data_ast.h``
- `storage_engine/b_plus_tree_node.h` - 引用形式: ``storage_engine/b_plus_tree_node.h``
- `storage_engine/b_plus_tree_leaf_node.h` - 引用形式: ``storage_engine/b_plus_tree_leaf_node.h``
- `storage_engine/b_plus_tree_internal_node.h` - 引用形式: ``storage_engine/b_plus_tree_internal_node.h``
- `storage_engine/b_plus_tree_index.h` - 引用形式: ``storage_engine/b_plus_tree_index.h``
- `storage_engine/index_manager/smart_ptr_lifetime_manager.h` - 引用形式: ``storage_engine/index_manager/smart_ptr_lifetime_manager.h``
- `storage/buffer_pool_fixed.h` - 引用形式: ``storage/buffer_pool_fixed.h``
- `storage/buffer_pool_v2.h` - 引用形式: ``storage/buffer_pool_v2.h``
- `exception/base_exception.h` - 引用形式: ``exception/base_exception.h``
- `exception/buffer_exception.h` - 引用形式: ``exception/buffer_exception.h``
- `exception/page_exception.h` - 引用形式: ``exception/page_exception.h``
- `exception/disk_exception.h` - 引用形式: ``exception/disk_exception.h``
- `exception/lock_exception.h` - 引用形式: ``exception/lock_exception.h``
- `exception/feature_exception.h` - 引用形式: ``exception/feature_exception.h``
- `exception/argument_exception.h` - 引用形式: ``exception/argument_exception.h``
- `include/sql_parser/data_types.h` - 引用形式: ``include/sql_parser/data_types.h``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `include/sql_parser/ast_node.h` - 引用形式: ``include/sql_parser/ast_node.h``
- `include/storage/buffer_pool.h` - 引用形式: ``include/storage/buffer_pool.h``
- `tests/unit/basic/logger_basic_test.cpp` - 引用形式: ``tests/unit/basic/logger_basic_test.cpp``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `tests/unit/storage/buffer_pool_test.cpp` - 引用形式: ``tests/unit/storage/buffer_pool_test.cpp``
- `tests/unit/storage/page_test.cpp` - 引用形式: ``tests/unit/storage/page_test.cpp``
- `tests/unit/sql_parser/lexer_new_test.cpp` - 引用形式: ``tests/unit/sql_parser/lexer_new_test.cpp``
- `tests/unit/sql_parser/parser_new_test.cpp` - 引用形式: ``tests/unit/sql_parser/parser_new_test.cpp``
- `tests/unit/execution/join_executor_test.cpp` - 引用形式: ``tests/unit/execution/join_executor_test.cpp``
- `tests/unit/execution/set_operation_executor_test.cpp` - 引用形式: ``tests/unit/execution/set_operation_executor_test.cpp``
- `tests/performance/buffer_pool_performance_test.cpp` - 引用形式: ``tests/performance/buffer_pool_performance_test.cpp``
- `tests/performance/index_performance_test.cpp` - 引用形式: ``tests/performance/index_performance_test.cpp``
- `tests/performance/network_performance_test.cpp` - 引用形式: ``tests/performance/network_performance_test.cpp``
- `tests/performance/crud_performance_test.cpp` - 引用形式: ``tests/performance/crud_performance_test.cpp``
- `tests/integration/client_server_integration_test.cpp` - 引用形式: ``tests/integration/client_server_integration_test.cpp``
- `tests/integration/transaction_integration_test.cpp` - 引用形式: ``tests/integration/transaction_integration_test.cpp``
- `tests/integration/sql_execution_integration_test.cpp` - 引用形式: ``tests/integration/sql_execution_integration_test.cpp``
- `core/error_handler.h` - 引用形式: ``core/error_handler.h``
- `sql_parser/ast_nodes.h` - 引用形式: ``sql_parser/ast_nodes.h``
- `network/protocol.h` - 引用形式: ``network/protocol.h``
- `storage_engine/buffer_pool_v2.h` - 引用形式: ``storage_engine/buffer_pool_v2.h``
- `storage_engine/page.h` - 引用形式: ``storage_engine/page.h``
- `storage_engine/disk_manager.h` - 引用形式: ``storage_engine/disk_manager.h``
- `transaction/wal_manager.h` - 引用形式: ``transaction/wal_manager.h``
- `core/error_handler.h` - 引用形式: ``core/error_handler.h``
- `src/core/sql_executor_interface.cpp` - 引用形式: ``src/core/sql_executor_interface.cpp``
- `sql_parser/ast_node.h` - 引用形式: ``sql_parser/ast_node.h``
- `sql_parser/ast_nodes.h` - 引用形式: ``sql_parser/ast_nodes.h``
- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `src/storage_engine/buffer_pool.cpp` - 引用形式: ``src/storage_engine/buffer_pool.cpp``
- `src/storage_engine/buffer_pool_sharded.cpp` - 引用形式: ``src/storage_engine/buffer_pool_sharded.cpp``
- `src/execution/unified_executor.cpp` - 引用形式: ``src/execution/unified_executor.cpp``
- `tests/unit/basic/logger_basic_test.cpp` - 引用形式: ``tests/unit/basic/logger_basic_test.cpp``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `tests/unit/basic/execution_context_test.cpp` - 引用形式: ``tests/unit/basic/execution_context_test.cpp``
- `execution/execution_context.h` - 引用形式: ``execution/execution_context.h``
- `tests/unit/storage/buffer_pool_test.cpp` - 引用形式: ``tests/unit/storage/buffer_pool_test.cpp``
- `tests/unit/storage/page_test.cpp` - 引用形式: ``tests/unit/storage/page_test.cpp``
- `storage_engine/page.h` - 引用形式: ``storage_engine/page.h``
- `tests/unit/sql_parser/lexer_new_test.cpp` - 引用形式: ``tests/unit/sql_parser/lexer_new_test.cpp``
- `sql_parser/lexer_new.h` - 引用形式: ``sql_parser/lexer_new.h``
- `tests/unit/sql_parser/parser_new_test.cpp` - 引用形式: ``tests/unit/sql_parser/parser_new_test.cpp``
- `sql_parser/parser_new.h` - 引用形式: ``sql_parser/parser_new.h``
- `tests/unit/execution/join_executor_test.cpp` - 引用形式: ``tests/unit/execution/join_executor_test.cpp``
- `tests/unit/execution/set_operation_executor_test.cpp` - 引用形式: ``tests/unit/execution/set_operation_executor_test.cpp``
- `tests/integration/client_server_integration_test.cpp` - 引用形式: ``tests/integration/client_server_integration_test.cpp``
- `tests/integration/transaction_integration_test.cpp` - 引用形式: ``tests/integration/transaction_integration_test.cpp``
- `tests/integration/sql_execution_integration_test.cpp` - 引用形式: ``tests/integration/sql_execution_integration_test.cpp``
- `tests/integration/storage_engine_integration_test.cpp` - 引用形式: ``tests/integration/storage_engine_integration_test.cpp``
- `tests/performance/buffer_pool_performance_test.cpp` - 引用形式: ``tests/performance/buffer_pool_performance_test.cpp``
- `tests/performance/index_performance_test.cpp` - 引用形式: ``tests/performance/index_performance_test.cpp``
- `tests/performance/network_performance_test.cpp` - 引用形式: ``tests/performance/network_performance_test.cpp``
- `tests/performance/crud_performance_test.cpp` - 引用形式: ``tests/performance/crud_performance_test.cpp``
- `tests/performance/concurrent_performance_test.cpp` - 引用形式: ``tests/performance/concurrent_performance_test.cpp``

### docs/project/EXPERIMENT_REPORT.md

- `storage_engine/b_plus_tree.cpp` - 引用形式: ``storage_engine/b_plus_tree.cpp``
- `storage_engine/table_storage.cpp` - 引用形式: ``storage_engine/table_storage.cpp``

### docs/versions/BRANCHES.md

- `include/version.h` - 引用形式: ``include/version.h``

### docs/设计/v1.1.0_deep_analysis_report.md

- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `src/sql_executor/index_manager.cpp` - 引用形式: ``src/sql_executor/index_manager.cpp``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``

### docs/ai-agent/project_index_usage_guide.md

- `include/path/file.h` - 引用形式: ``include/path/file.h``
- `src/path/file.cpp` - 引用形式: ``src/path/file.cpp``
- `tests/unit/path/test.cpp` - 引用形式: ``tests/unit/path/test.cpp``
- `include/path/file.h` - 引用形式: ``include/path/file.h``

### docs/ai-agent/sqlcc_standardized_build_directory_and_rules_specification.md

- `include/utils/logger.h` - 引用形式: ``include/utils/logger.h``
- `src/utils/include/logger.h` - 引用形式: ``src/utils/include/logger.h``

### docs/reports/ChangeLog_v1.0.6.md

- `include/unified_executor.h` - 引用形式: ``include/unified_executor.h``
- `tests/unit/unified_executor_test.cpp` - 引用形式: ``tests/unit/unified_executor_test.cpp``
- `include/system_database.h` - 引用形式: ``include/system_database.h``

### docs/reports/memory_audit_report.md

- `/home/liying/sqlcc/include/core/unified_executor.h` - 引用形式: ``/home/liying/sqlcc/include/core/unified_executor.h``
- `/home/liying/sqlcc/include/database_manager.h` - 引用形式: ``/home/liying/sqlcc/include/database_manager.h``
- `/home/liying/sqlcc/include/disk_manager.h` - 引用形式: ``/home/liying/sqlcc/include/disk_manager.h``
- `/home/liying/sqlcc/include/execution/set_operation_executor.h` - 引用形式: ``/home/liying/sqlcc/include/execution/set_operation_executor.h``
- `/home/liying/sqlcc/include/sql_parser/set_operation_node.h` - 引用形式: ``/home/liying/sqlcc/include/sql_parser/set_operation_node.h``
- `/home/liying/sqlcc/include/storage/buffer_pool_new.h` - 引用形式: ``/home/liying/sqlcc/include/storage/buffer_pool_new.h``
- `/home/liying/sqlcc/include/storage/buffer_pool_v2.h` - 引用形式: ``/home/liying/sqlcc/include/storage/buffer_pool_v2.h``
- `/home/liying/sqlcc/include/storage/performance_monitor.h` - 引用形式: ``/home/liying/sqlcc/include/storage/performance_monitor.h``
- `/home/liying/sqlcc/include/storage/prefetcher.h` - 引用形式: ``/home/liying/sqlcc/include/storage/prefetcher.h``
- `/home/liying/sqlcc/include/storage/replace_strategy.h` - 引用形式: ``/home/liying/sqlcc/include/storage/replace_strategy.h``
- `/home/liying/sqlcc/include/storage/table_storage.h` - 引用形式: ``/home/liying/sqlcc/include/storage/table_storage.h``
- `/home/liying/sqlcc/include/utils/file_descriptor.h` - 引用形式: ``/home/liying/sqlcc/include/utils/file_descriptor.h``
- `/home/liying/sqlcc/include/utils/ssl_wrapper.h` - 引用形式: ``/home/liying/sqlcc/include/utils/ssl_wrapper.h``
- `/home/liying/sqlcc/src/core/database_manager.cpp` - 引用形式: ``/home/liying/sqlcc/src/core/database_manager.cpp``
- `/home/liying/sqlcc/src/sql_parser/lexer_new.cpp` - 引用形式: ``/home/liying/sqlcc/src/sql_parser/lexer_new.cpp``
- `/home/liying/sqlcc/src/sql_parser/token_new.cpp` - 引用形式: ``/home/liying/sqlcc/src/sql_parser/token_new.cpp``
- `/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp``
- `/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp``
- `/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp``
- `/home/liying/sqlcc/src/storage_engine/buffer_pool_sharded.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/buffer_pool_sharded.cpp``
- `/home/liying/sqlcc/src/storage_engine/disk_manager.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/disk_manager.cpp``
- `/home/liying/sqlcc/src/storage_engine/replace_strategy.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/replace_strategy.cpp``
- `/home/liying/sqlcc/src/storage_engine/storage_engine.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/storage_engine.cpp``
- `/home/liying/sqlcc/src/storage_engine/table_storage.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/table_storage.cpp``
- `/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp``
- `/home/liying/sqlcc/tests/client_server/client_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/client_server/client_test.cpp``
- `/home/liying/sqlcc/tests/client_server/encrypted_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/client_server/encrypted_integration_test.cpp``
- `/home/liying/sqlcc/tests/client_server/encrypted_test_runner.cpp` - 引用形式: ``/home/liying/sqlcc/tests/client_server/encrypted_test_runner.cpp``
- `/home/liying/sqlcc/tests/components/debug/comprehensive_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/debug/comprehensive_test.cpp``
- `/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp``
- `/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp``
- `/home/liying/sqlcc/tests/components/debug/memory_safety_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/debug/memory_safety_test.cpp``
- `/home/liying/sqlcc/tests/components/debug/test_performance_real.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/debug/test_performance_real.cpp``
- `/home/liying/sqlcc/tests/components/executor/index_optimization_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/index_optimization_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/index_usage_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/index_usage_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/privilege_consistency_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/privilege_consistency_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/set_operation_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/set_operation_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/unified_executor_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/unified_executor_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/unsupported_commands_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/unsupported_commands_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp``
- `/home/liying/sqlcc/tests/components/network/network_unit_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/network/network_unit_test.cpp``
- `/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp``
- `/home/liying/sqlcc/tests/components/security/test_revoke_persistence.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/security/test_revoke_persistence.cpp``
- `/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/storage/buffer_pool_smart_pointer_test.cpp``
- `/home/liying/sqlcc/tests/components/storage/disk_manager_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/storage/disk_manager_test.cpp``
- `/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp``
- `/home/liying/sqlcc/tests/integration/advanced_sql/isql_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/advanced_sql/isql_integration_test.cpp``
- `/home/liying/sqlcc/tests/integration/advanced_sql/sql_executor_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/advanced_sql/sql_executor_integration_test.cpp``
- `/home/liying/sqlcc/tests/integration/basic_sql/simple_sql_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/basic_sql/simple_sql_test.cpp``
- `/home/liying/sqlcc/tests/integration/isql_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/isql_integration_test.cpp``
- `/home/liying/sqlcc/tests/integration/simple_sql_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/simple_sql_test.cpp``
- `/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp``
- `/home/liying/sqlcc/tests/integration/sql_executor_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/sql_executor_integration_test.cpp``
- `/home/liying/sqlcc/tests/legacy/test_dcl_ddl_persistence.cpp` - 引用形式: ``/home/liying/sqlcc/tests/legacy/test_dcl_ddl_persistence.cpp``
- `/home/liying/sqlcc/tests/network/aes_encryption_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/network/aes_encryption_test.cc``
- `/home/liying/sqlcc/tests/network/aes_network_integration_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/network/aes_network_integration_test.cc``
- `/home/liying/sqlcc/tests/network/sql_network_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/network/sql_network_test.cpp``
- `/home/liying/sqlcc/tests/network/tls_e2e_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/network/tls_e2e_test.cc``
- `/home/liying/sqlcc/tests/performance/basic/batch_prefetch_performance_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/basic/batch_prefetch_performance_test.cc``
- `/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc``
- `/home/liying/sqlcc/tests/performance/basic/cpu_intensive_performance_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/basic/cpu_intensive_performance_test.cc``
- `/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc``
- `/home/liying/sqlcc/tests/performance/buffer_pool_performance_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/buffer_pool_performance_test.h``
- `/home/liying/sqlcc/tests/performance/concurrency/concurrency_performance_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/concurrency/concurrency_performance_test.h``
- `/home/liying/sqlcc/tests/performance/concurrency_test/concurrency_performance_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/concurrency_test/concurrency_performance_test.h``
- `/home/liying/sqlcc/tests/performance/cpu_test/cpu_intensive_performance_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/cpu_test/cpu_intensive_performance_test.h``
- `/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc``
- `/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp``
- `/home/liying/sqlcc/tests/performance/disk_io_performance_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/disk_io_performance_test.h``
- `/home/liying/sqlcc/tests/performance/index_performance_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/index_performance_test.h``
- `/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.cc``
- `/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h``
- `/home/liying/sqlcc/tests/performance/million_insert_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/million_insert_test.h``
- `/home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test.h``
- `/home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test_main.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test_main.cc``
- `/home/liying/sqlcc/tests/sql_executor/sql_executor_comprehensive_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_executor/sql_executor_comprehensive_test.cpp``
- `/home/liying/sqlcc/tests/sql_executor/sql_executor_minimal_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_executor/sql_executor_minimal_test.cpp``
- `/home/liying/sqlcc/tests/sql_executor/sql_executor_unit_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_executor/sql_executor_unit_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/lexer_new_benchmark_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/lexer_new_benchmark_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/lexer_new_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/lexer_new_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/lexer_new_unit_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/lexer_new_unit_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/lexer_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/lexer_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/parser_new_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/parser_new_integration_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/simple_parser_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/simple_parser_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/token_new_unit_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/token_new_unit_test.cpp``
- `/home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp``
- `/home/liying/sqlcc/tests/test_disk_manager.h` - 引用形式: ``/home/liying/sqlcc/tests/test_disk_manager.h``
- `/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_output.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_output.cpp``
- `/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_simple.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_simple.cpp``

### docs/reports/SQLCC真实测试覆盖率分析报告.md

- `tests/unit/b_plus_tree_core_test.cc` - 引用形式: ``tests/unit/b_plus_tree_core_test.cc``
- `tests/unit/b_plus_tree_test.cc` - 引用形式: ``tests/unit/b_plus_tree_test.cc``
- `tests/unit/b_plus_tree_performance_test.cc` - 引用形式: ``tests/unit/b_plus_tree_performance_test.cc``
- `tests/unit/transaction_manager_test.cc` - 引用形式: ``tests/unit/transaction_manager_test.cc``
- `tests/unit/transaction_functional_test.cc` - 引用形式: ``tests/unit/transaction_functional_test.cc``
- `tests/unit/sql_parser_comprehensive_test.cc` - 引用形式: ``tests/unit/sql_parser_comprehensive_test.cc``

### docs/reports/ChangeLog_v1.0.2.md

- `tests/integration/isql_integration_test.cpp` - 引用形式: ``tests/integration/isql_integration_test.cpp``

### docs/reports/v1.1.2_raw_allocations_audit.md

- `tests/client_server/client_server_integration_test.cpp` - 引用形式: ``tests/client_server/client_server_integration_test.cpp``

### docs/testing/TESTING_ENHANCEMENT_REPORT_v0.5.3.md

- `tests/unit/b_plus_tree_performance_test.cc` - 引用形式: ``tests/unit/b_plus_tree_performance_test.cc``
- `tests/unit/enterprise_performance_tests.cc` - 引用形式: ``tests/unit/enterprise_performance_tests.cc``
- `tests/unit/sql_parser_comprehensive_test.cc` - 引用形式: ``tests/unit/sql_parser_comprehensive_test.cc``
- `tests/unit/ast_nodes_comprehensive_test.cc` - 引用形式: ``tests/unit/ast_nodes_comprehensive_test.cc``

### docs/testing/TESTING_ENHANCEMENT_REPORT_v0.5.4.md

- `tests/unit/b_plus_tree_core_test.cc` - 引用形式: ``tests/unit/b_plus_tree_core_test.cc``
- `src/b_plus_tree_enhanced.cc` - 引用形式: ``src/b_plus_tree_enhanced.cc``
- `tests/unit/storage_engine_enhanced_test.cc` - 引用形式: ``tests/unit/storage_engine_enhanced_test.cc``

### docs/testing/TEMPORARY_TEST_FILES.md

- `/home/liying/sqlcc/test_simple.cc` - 引用形式: ``/home/liying/sqlcc/test_simple.cc``
- `/home/liying/sqlcc/test_page_id_fix.cc` - 引用形式: ``/home/liying/sqlcc/test_page_id_fix.cc``
- `/home/liying/sqlcc/test_sync_functionality.cc` - 引用形式: ``/home/liying/sqlcc/test_sync_functionality.cc``

### docs/testing/TESTING_IMPROVEMENTS_SUMMARY.md

- `tests/page_enhanced_test.cc` - 引用形式: ``tests/page_enhanced_test.cc``
- `tests/disk_manager_enhanced_test.cc` - 引用形式: ``tests/disk_manager_enhanced_test.cc``
- `tests/buffer_pool_enhanced_test.cc` - 引用形式: ``tests/buffer_pool_enhanced_test.cc``

### docs/1.0.3版本加密通信功能改进/AESE_QUICKSTART.md

- `tests/network/aes_encryption_test.cc` - 引用形式: `[tests/network/aes_encryption_test.cc](tests/network/aes_encryption_test.cc)`
- `tests/network/aes_network_integration_test.cc` - 引用形式: `[tests/network/aes_network_integration_test.cc](tests/network/aes_network_integration_test.cc)`
- `tests/network/aes_encryption_test.cc` - 引用形式: ``tests/network/aes_encryption_test.cc``

### docs/1.0.3版本加密通信功能改进/AESE_ENCRYPTION_GUIDE.md

- `tests/network/aes_encryption_test.cc` - 引用形式: ``tests/network/aes_encryption_test.cc``
- `tests/network/aes_network_integration_test.cc` - 引用形式: ``tests/network/aes_network_integration_test.cc``

### docs/1.0.3版本加密通信功能改进/ENCRYPTED_COMMUNICATION_IMPROVEMENTS.md

- `tests/client_server/encrypted_integration_test.cpp` - 引用形式: ``tests/client_server/encrypted_integration_test.cpp``
- `tests/client_server/encrypted_test_runner.cpp` - 引用形式: ``tests/client_server/encrypted_test_runner.cpp``
- `tests/client_server/encrypted_integration_test.cpp` - 引用形式: ``tests/client_server/encrypted_integration_test.cpp``
- `tests/client_server/encrypted_test_runner.cpp` - 引用形式: ``tests/client_server/encrypted_test_runner.cpp``

### docs/1.0.3版本加密通信功能改进/AESE_IMPLEMENTATION_SUMMARY.md

- `tests/network/aes_encryption_test.cc` - 引用形式: ``tests/network/aes_encryption_test.cc``
- `tests/network/tls_e2e_test.cc` - 引用形式: ``tests/network/tls_e2e_test.cc``

### docs/development/sql_parser_coverage_improvement_plan.md

- `tests/unit/parser/constraint_test.cpp` - 引用形式: ``tests/unit/parser/constraint_test.cpp``
- `tests/unit/parser/window_function_test.cpp` - 引用形式: ``tests/unit/parser/window_function_test.cpp``
- `tests/unit/parser/json_test.cpp` - 引用形式: ``tests/unit/parser/json_test.cpp``
- `tests/unit/parser/set_operation_test.cpp` - 引用形式: ``tests/unit/parser/set_operation_test.cpp``
- `tests/unit/parser/recursive_query_test.cpp` - 引用形式: ``tests/unit/parser/recursive_query_test.cpp``
- `tests/unit/parser/data_types_test.cpp` - 引用形式: ``tests/unit/parser/data_types_test.cpp``

### docs/releases/RELEASE_NOTES_v1.0.2.md

- `tests/integration/isql_integration_test.cpp` - 引用形式: ``tests/integration/isql_integration_test.cpp``

### docs/releases/CHANGELOG_v1.0.6.md

- `include/unified_executor.h` - 引用形式: ``include/unified_executor.h``
- `tests/unit/unified_executor_test.cpp` - 引用形式: ``tests/unit/unified_executor_test.cpp``
- `include/system_database.h` - 引用形式: ``include/system_database.h``

### docs/releases/Release_Notes.md

- `include/sql_parser/parser.h` - 引用形式: ``include/sql_parser/parser.h``

### docs/releases/RELEASE_NOTES_v0.5.4.md

- `src/b_plus_tree_enhanced.cc` - 引用形式: ``src/b_plus_tree_enhanced.cc``
- `tests/unit/b_plus_tree_core_test.cc` - 引用形式: ``tests/unit/b_plus_tree_core_test.cc``
- `src/b_plus_tree.cc` - 引用形式: ``src/b_plus_tree.cc``
- `src/index_manager.cpp` - 引用形式: ``src/index_manager.cpp``
- `src/storage_engine.cpp` - 引用形式: ``src/storage_engine.cpp``

### docs/releases/CHANGELOG_v1.0.2.md

- `tests/integration/isql_integration_test.cpp` - 引用形式: ``tests/integration/isql_integration_test.cpp``

### docs/releases/CHANGELOG_BAZEL_REFACTORING.md

- `include/utils/smart_config_manager.h` - 引用形式: ``include/utils/smart_config_manager.h``

### docs/releases/CHANGELOG_v1.2.11.md

- `tests/unit/parser/ast_comprehensive_test.cpp` - 引用形式: ``tests/unit/parser/ast_comprehensive_test.cpp``
- `tests/unit/parser/expression_parser_test.cpp` - 引用形式: ``tests/unit/parser/expression_parser_test.cpp``

### docs/releases/CHANGELOG_INCLUDE_FIXES.md

- `include/sql_parser/ast_node.h` - 引用形式: ``include/sql_parser/ast_node.h``
- `include/sql_parser/ast_nodes.h` - 引用形式: ``include/sql_parser/ast_nodes.h``
- `include/sql_parser/token.h` - 引用形式: ``include/sql_parser/token.h``
- `include/sql_parser/data_types.h` - 引用形式: ``include/sql_parser/data_types.h``
- `include/sql_parser/node_visitor.h` - 引用形式: ``include/sql_parser/node_visitor.h``

### docs/releases/CHANGELOG_v1.3.3.md

- `tests/unit/ddl/ddl_performance_test.cpp` - 引用形式: ``tests/unit/ddl/ddl_performance_test.cpp``
- `tests/unit/security/dcl_advanced_test.cpp` - 引用形式: ``tests/unit/security/dcl_advanced_test.cpp``

### docs/progress/phased_implementation_plan.md

- `include/sql_parser/advanced_ast.h` - 引用形式: ``include/sql_parser/advanced_ast.h``
- `src/sql_parser/advanced_ast.cpp` - 引用形式: ``src/sql_parser/advanced_ast.cpp``
- `tests/sql_parser/advanced_ast_test.cpp` - 引用形式: ``tests/sql_parser/advanced_ast_test.cpp``
- `src/sql_parser/advanced_parser.cpp` - 引用形式: ``src/sql_parser/advanced_parser.cpp``
- `tests/advanced_sql/test_framework.h` - 引用形式: ``tests/advanced_sql/test_framework.h``

### docs/progress/sql_execution_reconstruction_summary.md

- `include/sql_parser/ast_nodes.h` - 引用形式: ``include/sql_parser/ast_nodes.h``
- `src/sql_executor.cpp` - 引用形式: ``src/sql_executor.cpp``
- `tests/unit/ast_nodes_test.cpp` - 引用形式: ``tests/unit/ast_nodes_test.cpp``
- `tests/unit/execution_engine_test.cpp` - 引用形式: ``tests/unit/execution_engine_test.cpp``
- `tests/integration/sql_executor_integration_test.cpp` - 引用形式: ``tests/integration/sql_executor_integration_test.cpp``

### docs/progress/sql_execution_real_implementation_plan.md

- `src/sql_executor.cpp` - 引用形式: ``src/sql_executor.cpp``
- `include/sql_parser/ast_node.h` - 引用形式: ``include/sql_parser/ast_node.h``
- `src/core/database_manager.cpp` - 引用形式: ``src/core/database_manager.cpp``
- `include/sql_parser/ast_node.h` - 引用形式: ``include/sql_parser/ast_node.h``
- `src/sql_executor.cpp` - 引用形式: ``src/sql_executor.cpp``
- `src/core/database_manager.cpp` - 引用形式: ``src/core/database_manager.cpp``
- `src/execution_engine/dml_executor.cpp` - 引用形式: ``src/execution_engine/dml_executor.cpp``
- `src/execution_engine/query_executor.cpp` - 引用形式: ``src/execution_engine/query_executor.cpp``
- `src/sql_executor.cpp` - 引用形式: ``src/sql_executor.cpp``
- `src/core/transaction_manager.cpp` - 引用形式: ``src/core/transaction_manager.cpp``

### docs/progress/1.0.5项目进展-SQL高级语言增强(2025-12-02-22-16-00).md

- `include/sql_parser/advanced_ast.h` - 引用形式: ``include/sql_parser/advanced_ast.h``
- `include/sql_parser/having_clause_node.h` - 引用形式: ``include/sql_parser/having_clause_node.h``
- `src/sql_parser/having_clause_node.cpp` - 引用形式: ``src/sql_parser/having_clause_node.cpp``
- `tests/advanced_sql/having_clause_test.cpp` - 引用形式: ``tests/advanced_sql/having_clause_test.cpp``

### docs/progress/1.0.5项目进展-SQL高级语言增强(2025-12-02-22:16:00).md

- `include/sql_parser/advanced_ast.h` - 引用形式: ``include/sql_parser/advanced_ast.h``
- `include/sql_parser/having_clause_node.h` - 引用形式: ``include/sql_parser/having_clause_node.h``
- `src/sql_parser/having_clause_node.cpp` - 引用形式: ``src/sql_parser/having_clause_node.cpp``
- `tests/advanced_sql/having_clause_test.cpp` - 引用形式: ``tests/advanced_sql/having_clause_test.cpp``

### docs/progress/SQLCC v1.0.8改进实现计划.md

- `src/permission_validator.cpp` - 引用形式: ``src/permission_validator.cpp``
- `include/sql_parser/advanced_ast.h` - 引用形式: ``include/sql_parser/advanced_ast.h``
- `include/sql_parser/advanced_ast.h` - 引用形式: ``include/sql_parser/advanced_ast.h``
- `include/sql_parser/advanced_ast.h` - 引用形式: ``include/sql_parser/advanced_ast.h``

### docs/evaluation/v1.1.0_deep_analysis_report.md

- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `src/sql_executor/index_manager.cpp` - 引用形式: ``src/sql_executor/index_manager.cpp``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``

### docs/evaluation/mysql_protocol_evaluation.md

- `/home/liying/sqlcc/include/storage/buffer_pool_v3.h` - 引用形式: `[buffer_pool_v3.h](/home/liying/sqlcc/include/storage/buffer_pool_v3.h)`
- `/home/liying/sqlcc/include/core/unified_executor.h` - 引用形式: `[unified_executor.h](src/core/unified_executor.h)`

### docs/evaluation/test_coverage_analysis.md

- `/tests/components/executor/privilege_consistency_test.cpp` - 引用形式: ``/tests/components/executor/privilege_consistency_test.cpp``
- `/src/execution_engine.cpp` - 引用形式: ``/src/execution_engine.cpp``

### docs/evaluation/v1.0.8_improvement_report.md

- `/home/liying/sqlcc_qoder/src/unified_executor.cpp` - 引用形式: ``/home/liying/sqlcc_qoder/src/unified_executor.cpp``
- `/home/liying/sqlcc_qoder/src/bin/isql_main.cpp` - 引用形式: ``/home/liying/sqlcc_qoder/src/bin/isql_main.cpp``
- `/home/liying/sqlcc_qoder/include/sql_executor.h` - 引用形式: ``/home/liying/sqlcc_qoder/include/sql_executor.h``
- `/home/liying/sqlcc_qoder/include/sql_parser/node_visitor.h` - 引用形式: ``/home/liying/sqlcc_qoder/include/sql_parser/node_visitor.h``
- `/home/liying/sqlcc_qoder/include/sql_parser/parser.h` - 引用形式: ``/home/liying/sqlcc_qoder/include/sql_parser/parser.h``
- `/home/liying/sqlcc_qoder/include/sql_parser/token.h` - 引用形式: ``/home/liying/sqlcc_qoder/include/sql_parser/token.h``
- `/home/liying/sqlcc_qoder/include/wal_manager.h` - 引用形式: ``/home/liying/sqlcc_qoder/include/wal_manager.h``

### docs/evaluation/v1.1.2/v1.1.2_improvement_plan.md

- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``

### docs/evaluation/v1.1.2/improvement_plan.md

- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``

### docs/versions/1.0.5版本改进/设计文档/集合操作AST设计.md

- `/home/liying/sqlcc_qoder/include/sql_parser/ast_nodes.h` - 引用形式: ``/home/liying/sqlcc_qoder/include/sql_parser/ast_nodes.h``
- `/home/liying/sqlcc_qoder/src/sql_parser/parser.cpp` - 引用形式: ``/home/liying/sqlcc_qoder/src/sql_parser/parser.cpp``
- `/home/liying/sqlcc_qoder/include/sql_parser/token.h` - 引用形式: ``/home/liying/sqlcc_qoder/include/sql_parser/token.h``
- `/home/liying/sqlcc_qoder/src/sql_parser/lexer.cpp` - 引用形式: ``/home/liying/sqlcc_qoder/src/sql_parser/lexer.cpp``

### docs/versions/1.0.5版本改进/计划文档/阶段2_集合操作功能完善_计划报告.md

- `src/sql_executor/set_operation_executor.cpp` - 引用形式: ``src/sql_executor/set_operation_executor.cpp``
- `src/sql_executor/set_operation_executor.h` - 引用形式: ``src/sql_executor/set_operation_executor.h``
- `tests/set_operation_executor_test.cpp` - 引用形式: ``tests/set_operation_executor_test.cpp``
- `tests/integration/set_operation_integration_test.cpp` - 引用形式: ``tests/integration/set_operation_integration_test.cpp``

### docs/项目进展/v1.2.12/SQLCC测试系统错误分析报告_v1.2.12.md

- `tests/unit/buffer_pool_test.cpp` - 引用形式: ``tests/unit/buffer_pool_test.cpp``
- `tests/unit/parser/lexer_test.cpp` - 引用形式: ``tests/unit/parser/lexer_test.cpp``
- `tests/unit/network/multi_threaded_network_manager_test.cpp` - 引用形式: ``tests/unit/network/multi_threaded_network_manager_test.cpp``
- `tests/unit/executor/load_data_boundary_test.cpp` - 引用形式: ``tests/unit/executor/load_data_boundary_test.cpp``
- `tests/unit/basic/sql_executor_core_test.cpp` - 引用形式: ``tests/unit/basic/sql_executor_core_test.cpp``
- `tests/unit/basic/permission_validator_test.cpp` - 引用形式: ``tests/unit/basic/permission_validator_test.cpp``
- `tests/unit/core/simple_test.cpp` - 引用形式: ``tests/unit/core/simple_test.cpp``

### docs/项目进展/v1.2.12/错误修复工作日记.md

- `tests/unit/buffer_pool_test.cpp` - 引用形式: ``tests/unit/buffer_pool_test.cpp``
- `tests/unit/parser/lexer_test.cpp` - 引用形式: ``tests/unit/parser/lexer_test.cpp``
- `tests/unit/executor/load_data_boundary_test.cpp` - 引用形式: ``tests/unit/executor/load_data_boundary_test.cpp``
- `tests/unit/basic/sql_executor_core_test.cpp` - 引用形式: ``tests/unit/basic/sql_executor_core_test.cpp``
- `tests/unit/basic/permission_validator_test.cpp` - 引用形式: ``tests/unit/basic/permission_validator_test.cpp``
- `tests/unit/core/simple_test.cpp` - 引用形式: ``tests/unit/core/simple_test.cpp``

### docs/项目进展/v1.2.8/SQLCC测试系统完整执行状态报告_20251225.md

- `tests/unit/basic/logger_basic_test.cpp` - 引用形式: ``tests/unit/basic/logger_basic_test.cpp``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `tests/unit/basic/execution_context_test.cpp` - 引用形式: ``tests/unit/basic/execution_context_test.cpp``
- `tests/unit/basic/permission_validator_test.cpp` - 引用形式: ``tests/unit/basic/permission_validator_test.cpp``
- `tests/unit/basic/sql_executor_core_test.cpp` - 引用形式: ``tests/unit/basic/sql_executor_core_test.cpp``
- `tests/unit/basic/decimal_test.cpp` - 引用形式: ``tests/unit/basic/decimal_test.cpp``
- `tests/unit/core/config_manager_test.cpp` - 引用形式: ``tests/unit/core/config_manager_test.cpp``
- `tests/unit/core/user_manager_test.cpp` - 引用形式: ``tests/unit/core/user_manager_test.cpp``
- `tests/unit/core/error_handler_test.cpp` - 引用形式: ``tests/unit/core/error_handler_test.cpp``
- `tests/unit/core/database_manager_test.cpp` - 引用形式: ``tests/unit/core/database_manager_test.cpp``
- `tests/unit/core/sql_executor_test.cpp` - 引用形式: ``tests/unit/core/sql_executor_test.cpp``
- `tests/unit/core/transaction_manager_test.cpp` - 引用形式: ``tests/unit/core/transaction_manager_test.cpp``
- `tests/unit/parser/sql_parser_high_coverage_test.cpp` - 引用形式: ``tests/unit/parser/sql_parser_high_coverage_test.cpp``
- `tests/unit/parser/ast_node_test.cpp` - 引用形式: ``tests/unit/parser/ast_node_test.cpp``
- `tests/unit/parser/token_test.cpp` - 引用形式: ``tests/unit/parser/token_test.cpp``
- `tests/unit/parser/lexer_test.cpp` - 引用形式: ``tests/unit/parser/lexer_test.cpp``
- `tests/unit/parser/parser_test.cpp` - 引用形式: ``tests/unit/parser/parser_test.cpp``
- `tests/unit/parser/constraint_parser_test.cpp` - 引用形式: ``tests/unit/parser/constraint_parser_test.cpp``
- `tests/unit/parser/function_parser_test.cpp` - 引用形式: ``tests/unit/parser/function_parser_test.cpp``
- `tests/unit/executor/task_executor_test.cpp` - 引用形式: ``tests/unit/executor/task_executor_test.cpp``
- `tests/unit/executor/task_executor_comprehensive_test.cpp` - 引用形式: ``tests/unit/executor/task_executor_comprehensive_test.cpp``
- `tests/unit/executor/standalone_test.cpp` - 引用形式: ``tests/unit/executor/standalone_test.cpp``
- `tests/unit/executor/test_runner.cpp` - 引用形式: ``tests/unit/executor/test_runner.cpp``
- `tests/unit/executor/join_executor_boundary_test.cpp` - 引用形式: ``tests/unit/executor/join_executor_boundary_test.cpp``
- `tests/unit/executor/set_operation_boundary_test.cpp` - 引用形式: ``tests/unit/executor/set_operation_boundary_test.cpp``
- `tests/unit/executor/window_function_boundary_test.cpp` - 引用形式: ``tests/unit/executor/window_function_boundary_test.cpp``
- `tests/unit/executor/load_data_boundary_test.cpp` - 引用形式: ``tests/unit/executor/load_data_boundary_test.cpp``
- `tests/unit/executor/recursive_query_executor_test.cpp` - 引用形式: ``tests/unit/executor/recursive_query_executor_test.cpp``
- `tests/unit/executor/subquery_executor_test.cpp` - 引用形式: ``tests/unit/executor/subquery_executor_test.cpp``
- `tests/unit/executor/aggregate_executor_test.cpp` - 引用形式: ``tests/unit/executor/aggregate_executor_test.cpp``

### docs/项目进展/v1.2.10/层次5_执行引擎测试分析报告.md

- `tests/unit/test_task_executor.cpp` - 引用形式: ``tests/unit/test_task_executor.cpp``
- `tests/unit/executor/task_executor_test.cpp` - 引用形式: ``tests/unit/executor/task_executor_test.cpp``
- `tests/unit/executor/task_executor_comprehensive_test.cpp` - 引用形式: ``tests/unit/executor/task_executor_comprehensive_test.cpp``
- `tests/unit/executor/comprehensive_task_executor_test.cpp` - 引用形式: ``tests/unit/executor/comprehensive_task_executor_test.cpp``
- `tests/unit/execution/task_executor_test.cpp` - 引用形式: ``tests/unit/execution/task_executor_test.cpp``
- `tests/unit/basic/sql_executor_core_test.cpp` - 引用形式: ``tests/unit/basic/sql_executor_core_test.cpp``
- `tests/unit/sql_executor_test.cpp` - 引用形式: ``tests/unit/sql_executor_test.cpp``
- `tests/integration/advanced_sql/sql_executor_integration_test.cpp` - 引用形式: ``tests/integration/advanced_sql/sql_executor_integration_test.cpp``
- `tests/unit/test_window_function_executor.cpp` - 引用形式: ``tests/unit/test_window_function_executor.cpp``
- `tests/unit/execution/window_function_executor_test.cpp` - 引用形式: ``tests/unit/execution/window_function_executor_test.cpp``
- `tests/unit/execution/join_executor_test.cpp` - 引用形式: ``tests/unit/execution/join_executor_test.cpp``
- `tests/unit/executor/join_executor_boundary_test.cpp` - 引用形式: ``tests/unit/executor/join_executor_boundary_test.cpp``
- `tests/unit/execution/set_operation_executor_test.cpp` - 引用形式: ``tests/unit/execution/set_operation_executor_test.cpp``
- `tests/unit/executor/set_operation_boundary_test.cpp` - 引用形式: ``tests/unit/executor/set_operation_boundary_test.cpp``
- `tests/unit/execution/subquery_executor_test.cpp` - 引用形式: ``tests/unit/execution/subquery_executor_test.cpp``
- `tests/unit/execution/recursive_query_executor_test.cpp` - 引用形式: ``tests/unit/execution/recursive_query_executor_test.cpp``
- `tests/unit/execution/function_executor_test.cpp` - 引用形式: ``tests/unit/execution/function_executor_test.cpp``
- `tests/unit/execution/load_data_executor_test.cpp` - 引用形式: ``tests/unit/execution/load_data_executor_test.cpp``
- `tests/unit/executor/load_data_boundary_test.cpp` - 引用形式: ``tests/unit/executor/load_data_boundary_test.cpp``
- `tests/unit/basic/execution_context_test.cpp` - 引用形式: ``tests/unit/basic/execution_context_test.cpp``
- `tests/unit/executor/task_executor_test.cpp` - 引用形式: ``tests/unit/executor/task_executor_test.cpp``
- `tests/unit/executor/task_executor_comprehensive_test.cpp` - 引用形式: ``tests/unit/executor/task_executor_comprehensive_test.cpp``
- `tests/unit/executor/comprehensive_task_executor_test.cpp` - 引用形式: ``tests/unit/executor/comprehensive_task_executor_test.cpp``
- `tests/unit/execution/join_executor_test.cpp` - 引用形式: ``tests/unit/execution/join_executor_test.cpp``
- `tests/unit/executor/join_executor_boundary_test.cpp` - 引用形式: ``tests/unit/executor/join_executor_boundary_test.cpp``
- `tests/unit/execution/set_operation_executor_test.cpp` - 引用形式: ``tests/unit/execution/set_operation_executor_test.cpp``
- `tests/unit/executor/set_operation_boundary_test.cpp` - 引用形式: ``tests/unit/executor/set_operation_boundary_test.cpp``
- `tests/unit/execution/subquery_executor_test.cpp` - 引用形式: ``tests/unit/execution/subquery_executor_test.cpp``
- `tests/unit/execution/recursive_query_executor_test.cpp` - 引用形式: ``tests/unit/execution/recursive_query_executor_test.cpp``
- `tests/unit/execution/function_executor_test.cpp` - 引用形式: ``tests/unit/execution/function_executor_test.cpp``
- `tests/unit/execution/load_data_executor_test.cpp` - 引用形式: ``tests/unit/execution/load_data_executor_test.cpp``
- `tests/unit/executor/load_data_boundary_test.cpp` - 引用形式: ``tests/unit/executor/load_data_boundary_test.cpp``
- `tests/unit/test_task_executor.cpp` - 引用形式: ``tests/unit/test_task_executor.cpp``
- `tests/unit/test_window_function_executor.cpp` - 引用形式: ``tests/unit/test_window_function_executor.cpp``
- `tests/unit/execution/window_function_executor_test.cpp` - 引用形式: ``tests/unit/execution/window_function_executor_test.cpp``
- `tests/unit/basic/sql_executor_core_test.cpp` - 引用形式: ``tests/unit/basic/sql_executor_core_test.cpp``
- `tests/unit/sql_executor_test.cpp` - 引用形式: ``tests/unit/sql_executor_test.cpp``
- `tests/unit/basic/execution_context_test.cpp` - 引用形式: ``tests/unit/basic/execution_context_test.cpp``
- `tests/integration/advanced_sql/sql_executor_integration_test.cpp` - 引用形式: ``tests/integration/advanced_sql/sql_executor_integration_test.cpp``

### docs/项目进展/v1.2.10/层次1_基础工具类测试分析报告.md

- `tests/unit/logger_test.cpp` - 引用形式: ``tests/unit/logger_test.cpp``
- `tests/unit/basic/logger_basic_test.cpp` - 引用形式: ``tests/unit/basic/logger_basic_test.cpp``
- `tests/unit/basic/config_manager_test.cpp` - 引用形式: ``tests/unit/basic/config_manager_test.cpp``
- `tests/unit/core/config_manager_test.cpp` - 引用形式: ``tests/unit/core/config_manager_test.cpp``
- `tests/test_smart_config_manager.cpp` - 引用形式: ``tests/test_smart_config_manager.cpp``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `tests/unit/basic/decimal_test.cpp` - 引用形式: ``tests/unit/basic/decimal_test.cpp``
- `tests/unit/basic/execution_context_test.cpp` - 引用形式: ``tests/unit/basic/execution_context_test.cpp``
- `tests/unit/logger_test.cpp` - 引用形式: ``tests/unit/logger_test.cpp``
- `tests/unit/basic/logger_basic_test.cpp` - 引用形式: ``tests/unit/basic/logger_basic_test.cpp``
- `tests/unit/core/config_manager_test.cpp` - 引用形式: ``tests/unit/core/config_manager_test.cpp``
- `tests/test_smart_config_manager.cpp` - 引用形式: ``tests/test_smart_config_manager.cpp``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `tests/unit/basic/decimal_test.cpp` - 引用形式: ``tests/unit/basic/decimal_test.cpp``
- `tests/unit/basic/execution_context_test.cpp` - 引用形式: ``tests/unit/basic/execution_context_test.cpp``

### docs/项目进展/v1.2.10/v1.2.10测试重构改进计划.md

- `tests/storage_engine/storage_engine_boundary_test.cpp` - 引用形式: ``tests/storage_engine/storage_engine_boundary_test.cpp``

### docs/项目进展/v1.2.10/SQLCC测试系统完整索引报告_v1.2.10.md

- `include/execution/task_result.h` - 引用形式: ``include/execution/task_result.h``
- `include/execution/task_result.h` - 引用形式: ``include/execution/task_result.h``

### docs/项目进展/v1.2.10/层次7_高层功能测试分析报告.md

- `tests/unit/core/stored_procedure_manager_test.cpp` - 引用形式: ``tests/unit/core/stored_procedure_manager_test.cpp``
- `tests/integration/procedure_trigger_integration_test.cpp` - 引用形式: ``tests/integration/procedure_trigger_integration_test.cpp``
- `tests/sql/simple_procedure_test.cpp` - 引用形式: ``tests/sql/simple_procedure_test.cpp``
- `tests/unit/basic/permission_validator_test.cpp` - 引用形式: ``tests/unit/basic/permission_validator_test.cpp``
- `tests/unit/core/config_manager_test.cpp` - 引用形式: ``tests/unit/core/config_manager_test.cpp``
- `tests/test_smart_config_manager.cpp` - 引用形式: ``tests/test_smart_config_manager.cpp``
- `tests/integration/session_manager_real_test.cpp` - 引用形式: ``tests/integration/session_manager_real_test.cpp``
- `tests/unit/core/config_manager_test.cpp` - 引用形式: ``tests/unit/core/config_manager_test.cpp``
- `tests/unit/basic/permission_validator_test.cpp` - 引用形式: ``tests/unit/basic/permission_validator_test.cpp``
- `tests/unit/core/stored_procedure_manager_test.cpp` - 引用形式: ``tests/unit/core/stored_procedure_manager_test.cpp``
- `tests/integration/procedure_trigger_integration_test.cpp` - 引用形式: ``tests/integration/procedure_trigger_integration_test.cpp``
- `tests/sql/simple_procedure_test.cpp` - 引用形式: ``tests/sql/simple_procedure_test.cpp``
- `tests/test_smart_config_manager.cpp` - 引用形式: ``tests/test_smart_config_manager.cpp``
- `tests/integration/session_manager_real_test.cpp` - 引用形式: ``tests/integration/session_manager_real_test.cpp``

### docs/项目进展/v1.2.10/层次2_存储引擎基础测试分析报告.md

- `tests/storage_engine/b_plus_tree_core_test.cpp` - 引用形式: ``tests/storage_engine/b_plus_tree_core_test.cpp``
- `tests/storage_engine/comprehensive_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/comprehensive_bplus_tree_test.cpp``
- `tests/storage_engine/final_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/final_bplus_tree_test.cpp``
- `tests/storage_engine/minimal_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/minimal_bplus_tree_test.cpp``
- `tests/storage_engine/simple_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/simple_bplus_tree_test.cpp``
- `tests/storage_engine/test_bplus_tree_fix.cpp` - 引用形式: ``tests/storage_engine/test_bplus_tree_fix.cpp``
- `tests/storage_engine/page_allocator_test.cpp` - 引用形式: ``tests/storage_engine/page_allocator_test.cpp``
- `tests/storage_engine/index_manager_test.cpp` - 引用形式: ``tests/storage_engine/index_manager_test.cpp``
- `tests/storage_engine/wal_system_test.cpp` - 引用形式: ``tests/storage_engine/wal_system_test.cpp``
- `tests/storage_engine/data_integrity_test.cpp` - 引用形式: ``tests/storage_engine/data_integrity_test.cpp``
- `tests/storage_engine/disk_manager_test.cpp` - 引用形式: ``tests/storage_engine/disk_manager_test.cpp``
- `tests/storage_engine/concurrency_control_test.cpp` - 引用形式: ``tests/storage_engine/concurrency_control_test.cpp``
- `tests/storage_engine/storage_engine_comprehensive_test.cpp` - 引用形式: ``tests/storage_engine/storage_engine_comprehensive_test.cpp``
- `tests/storage_engine/storage_engine_boundary_test.cpp` - 引用形式: ``tests/storage_engine/storage_engine_boundary_test.cpp``

### docs/项目进展/v1.2.10/层次4_SQL解析器测试分析报告.md

- `tests/unit/test_lexer_fix.cpp` - 引用形式: ``tests/unit/test_lexer_fix.cpp``
- `tests/unit/minimal_lexer_test.cpp` - 引用形式: ``tests/unit/minimal_lexer_test.cpp``
- `tests/unit/detailed_lexer_test.cpp` - 引用形式: ``tests/unit/detailed_lexer_test.cpp``
- `tests/unit/debug_lexer.cpp` - 引用形式: ``tests/unit/debug_lexer.cpp``
- `tests/unit/debug/debug_lexer.cpp` - 引用形式: ``tests/unit/debug/debug_lexer.cpp``
- `tests/unit/debug/debug_lexer_test.cpp` - 引用形式: ``tests/unit/debug/debug_lexer_test.cpp``
- `tests/unit/parser_select_test.cpp` - 引用形式: ``tests/unit/parser_select_test.cpp``
- `tests/unit/parser_create_table_test.cpp` - 引用形式: ``tests/unit/parser_create_table_test.cpp``
- `tests/unit/parser_drop_table_test.cpp` - 引用形式: ``tests/unit/parser_drop_table_test.cpp``
- `tests/unit/parser_alter_table_test.cpp` - 引用形式: ``tests/unit/parser_alter_table_test.cpp``
- `tests/demo/parser_integration_test.cpp` - 引用形式: ``tests/demo/parser_integration_test.cpp``
- `tests/unit/parser_select_test.cpp` - 引用形式: ``tests/unit/parser_select_test.cpp``
- `tests/unit/parser_create_table_test.cpp` - 引用形式: ``tests/unit/parser_create_table_test.cpp``
- `tests/unit/parser_drop_table_test.cpp` - 引用形式: ``tests/unit/parser_drop_table_test.cpp``
- `tests/unit/parser_alter_table_test.cpp` - 引用形式: ``tests/unit/parser_alter_table_test.cpp``
- `tests/unit/test_lexer_fix.cpp` - 引用形式: ``tests/unit/test_lexer_fix.cpp``
- `tests/unit/minimal_lexer_test.cpp` - 引用形式: ``tests/unit/minimal_lexer_test.cpp``
- `tests/unit/detailed_lexer_test.cpp` - 引用形式: ``tests/unit/detailed_lexer_test.cpp``
- `tests/unit/debug_lexer.cpp` - 引用形式: ``tests/unit/debug_lexer.cpp``
- `tests/unit/debug/debug_lexer.cpp` - 引用形式: ``tests/unit/debug/debug_lexer.cpp``
- `tests/unit/debug/debug_lexer_test.cpp` - 引用形式: ``tests/unit/debug/debug_lexer_test.cpp``
- `tests/demo/parser_integration_test.cpp` - 引用形式: ``tests/demo/parser_integration_test.cpp``

### docs/项目进展/v1.2.10/v1.2.10测试执行总结报告.md

- `tests/storage_engine/index_insert_test.cpp` - 引用形式: ``tests/storage_engine/index_insert_test.cpp``

### docs/项目进展/v1.2.10/层次6_网络通信测试分析报告.md

- `tests/unit/network/multi_threaded_network_manager_test.cpp` - 引用形式: ``tests/unit/network/multi_threaded_network_manager_test.cpp``
- `tests/unit/network/network_boundary_test.cpp` - 引用形式: ``tests/unit/network/network_boundary_test.cpp``
- `tests/unit/simple_network_test.cpp` - 引用形式: ``tests/unit/simple_network_test.cpp``
- `tests/integration/client_connection_real_test.cpp` - 引用形式: ``tests/integration/client_connection_real_test.cpp``
- `tests/integration/server_network_manager_real_test.cpp` - 引用形式: ``tests/integration/server_network_manager_real_test.cpp``
- `tests/integration/connection_handler_real_test.cpp` - 引用形式: ``tests/integration/connection_handler_real_test.cpp``
- `tests/integration/sql_network_test.cpp` - 引用形式: ``tests/integration/sql_network_test.cpp``
- `tests/integration/client_test.cpp` - 引用形式: ``tests/integration/client_test.cpp``
- `tests/integration/client_server_integration_test.cpp` - 引用形式: ``tests/integration/client_server_integration_test.cpp``
- `tests/integration/server_manager.cpp` - 引用形式: ``tests/integration/server_manager.cpp``
- `tests/unit/network/network_boundary_test.cpp` - 引用形式: ``tests/unit/network/network_boundary_test.cpp``
- `tests/integration/client_server_integration_test.cpp` - 引用形式: ``tests/integration/client_server_integration_test.cpp``
- `tests/unit/network/multi_threaded_network_manager_test.cpp` - 引用形式: ``tests/unit/network/multi_threaded_network_manager_test.cpp``
- `tests/unit/simple_network_test.cpp` - 引用形式: ``tests/unit/simple_network_test.cpp``
- `tests/integration/client_connection_real_test.cpp` - 引用形式: ``tests/integration/client_connection_real_test.cpp``
- `tests/integration/server_network_manager_real_test.cpp` - 引用形式: ``tests/integration/server_network_manager_real_test.cpp``
- `tests/integration/connection_handler_real_test.cpp` - 引用形式: ``tests/integration/connection_handler_real_test.cpp``
- `tests/integration/sql_network_test.cpp` - 引用形式: ``tests/integration/sql_network_test.cpp``
- `tests/integration/client_test.cpp` - 引用形式: ``tests/integration/client_test.cpp``
- `tests/integration/server_manager.cpp` - 引用形式: ``tests/integration/server_manager.cpp``

### docs/项目进展/v1.2.10/执行引擎模块BUILD修复总结.md

- `include/execution/task_result.h` - 引用形式: ``include/execution/task_result.h``
- `/home/liying/sqlcc/include/execution/task_result.h` - 引用形式: ``/home/liying/sqlcc/include/execution/task_result.h``
- `/home/liying/sqlcc/include/execution/procedure_trigger_task.h` - 引用形式: ``/home/liying/sqlcc/include/execution/procedure_trigger_task.h``

### docs/项目进展/v1.2.10/v1.2.10工作日记.md

- `src/storage_engine/b_plus_tree_index.cpp` - 引用形式: ``src/storage_engine/b_plus_tree_index.cpp``
- `src/storage_engine/b_plus_tree_leaf_node.cpp` - 引用形式: ``src/storage_engine/b_plus_tree_leaf_node.cpp``
- `src/storage_engine/b_plus_tree_internal_node.cpp` - 引用形式: ``src/storage_engine/b_plus_tree_internal_node.cpp``
- `src/storage_engine/b_plus_tree_node.cpp` - 引用形式: ``src/storage_engine/b_plus_tree_node.cpp``
- `tests/storage_engine/test_bplus_tree_fix.cpp` - 引用形式: ``tests/storage_engine/test_bplus_tree_fix.cpp``

### docs/项目进展/v1.2.10/AI辅助循环依赖分析报告.md

- `include/sql_parser/ast_node.h` - 引用形式: ``include/sql_parser/ast_node.h``

### docs/项目进展/v1.2.10/测试改进验证完成报告.md

- `include/storage_engine/index_manager/smart_index_cache.h` - 引用形式: ``include/storage_engine/index_manager/smart_index_cache.h``
- `include/disk_manager.h` - 引用形式: ``include/disk_manager.h``
- `src/storage_engine/disk_manager.cpp` - 引用形式: ``src/storage_engine/disk_manager.cpp``
- `tests/storage_engine/page_allocator_test.cpp` - 引用形式: ``tests/storage_engine/page_allocator_test.cpp``
- `tests/storage_engine/buffer_pool_test.cpp` - 引用形式: ``tests/storage_engine/buffer_pool_test.cpp``

### docs/项目进展/v1.2.10/v1.2.10详细覆盖率数据报告.md

- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `src/storage_engine/b_plus_tree_node.cpp` - 引用形式: ``src/storage_engine/b_plus_tree_node.cpp``
- `src/storage_engine/b_plus_tree_leaf_node.cpp` - 引用形式: ``src/storage_engine/b_plus_tree_leaf_node.cpp``
- `src/storage_engine/b_plus_tree_internal_node.cpp` - 引用形式: ``src/storage_engine/b_plus_tree_internal_node.cpp``

### docs/项目进展/v1.2.10/B+树测试覆盖率分析报告.md

- `src/storage_engine/b_plus_tree_index.cpp` - 引用形式: ``src/storage_engine/b_plus_tree_index.cpp``
- `src/storage_engine/b_plus_tree_leaf_node.cpp` - 引用形式: ``src/storage_engine/b_plus_tree_leaf_node.cpp``
- `src/storage_engine/b_plus_tree_internal_node.cpp` - 引用形式: ``src/storage_engine/b_plus_tree_internal_node.cpp``
- `src/storage_engine/b_plus_tree_node.cpp` - 引用形式: ``src/storage_engine/b_plus_tree_node.cpp``
- `tests/storage_engine/test_bplus_tree_fix.cpp` - 引用形式: ``tests/storage_engine/test_bplus_tree_fix.cpp``

### docs/项目进展/v1.2.10/层次3_索引系统测试分析报告.md

- `tests/storage_engine/final_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/final_bplus_tree_test.cpp``
- `tests/storage_engine/comprehensive_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/comprehensive_bplus_tree_test.cpp``
- `tests/storage_engine/minimal_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/minimal_bplus_tree_test.cpp``
- `tests/storage_engine/simple_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/simple_bplus_tree_test.cpp``
- `tests/storage_engine/test_bplus_tree_fix.cpp` - 引用形式: ``tests/storage_engine/test_bplus_tree_fix.cpp``
- `tests/unit/basic_bplus_tree_test.cpp` - 引用形式: ``tests/unit/basic_bplus_tree_test.cpp``
- `tests/storage_engine/index_manager_test.cpp` - 引用形式: ``tests/storage_engine/index_manager_test.cpp``
- `tests/storage_engine/index_insert_test.cpp` - 引用形式: ``tests/storage_engine/index_insert_test.cpp``
- `tests/unit/simple_index_test.cpp` - 引用形式: ``tests/unit/simple_index_test.cpp``
- `tests/unit/index_maintenance_test.cpp` - 引用形式: ``tests/unit/index_maintenance_test.cpp``

### docs/项目进展/v1.1.3/memory_refactoring_plan.md

- `include/utils/file_descriptor.h` - 引用形式: ``include/utils/file_descriptor.h``
- `include/storage/b_plus_tree.h` - 引用形式: ``include/storage/b_plus_tree.h``
- `include/storage/page.h` - 引用形式: ``include/storage/page.h``
- `include/storage/buffer_pool.h` - 引用形式: ``include/storage/buffer_pool.h``
- `src/storage_engine/buffer_pool_new.cpp` - 引用形式: ``src/storage_engine/buffer_pool_new.cpp``
- `include/core/unified_executor.h` - 引用形式: ``include/core/unified_executor.h``

### docs/项目进展/v1.1.3/内存审计工具分析报告.md

- `tests/components/debug/memory_audit_tool.cpp` - 引用形式: ``tests/components/debug/memory_audit_tool.cpp``

### docs/项目进展/v1.1.3/memory_refactoring_tasks.md

- `include/utils/file_descriptor.h` - 引用形式: ``include/utils/file_descriptor.h``
- `src/utils/file_descriptor.cpp` - 引用形式: ``src/utils/file_descriptor.cpp``
- `include/storage/b_plus_tree.h` - 引用形式: ``include/storage/b_plus_tree.h``
- `src/storage/b_plus_tree.cpp` - 引用形式: ``src/storage/b_plus_tree.cpp``
- `include/storage/page.h` - 引用形式: ``include/storage/page.h``
- `src/storage/page.cpp` - 引用形式: ``src/storage/page.cpp``
- `include/storage/buffer_pool.h` - 引用形式: ``include/storage/buffer_pool.h``
- `src/storage/buffer_pool_new.cpp` - 引用形式: ``src/storage/buffer_pool_new.cpp``
- `src/storage_engine/buffer_pool_new.cpp` - 引用形式: ``src/storage_engine/buffer_pool_new.cpp``
- `include/core/unified_executor.h` - 引用形式: ``include/core/unified_executor.h``
- `include/core/unified_executor.h` - 引用形式: ``include/core/unified_executor.h``
- `include/sql_parser/ast_nodes.h` - 引用形式: ``include/sql_parser/ast_nodes.h``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``
- `include/sql_parser/ast_nodes.h` - 引用形式: ``include/sql_parser/ast_nodes.h``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``

### docs/项目进展/v1.1.3/技术债务清单.md

- `src/core/database_manager.cpp` - 引用形式: ``src/core/database_manager.cpp``
- `src/core/unified_query_plan.cpp` - 引用形式: ``src/core/unified_query_plan.cpp``
- `src/sql_parser/lexer_new.cpp` - 引用形式: ``src/sql_parser/lexer_new.cpp``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``

### docs/项目进展/v1.1.3/内存安全审计总结.md

- `src/storage_engine/buffer_pool.cpp` - 引用形式: ``src/storage_engine/buffer_pool.cpp``
- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `src/storage_engine/table_storage.cpp` - 引用形式: ``src/storage_engine/table_storage.cpp``

### docs/项目进展/v1.1.3/final_test_summary_report.md

- `tests/performance/crud/real_crud_performance_test.cpp` - 引用形式: ``tests/performance/crud/real_crud_performance_test.cpp``

### docs/项目进展/v1.1.3/memory_audit_report copy.md

- `/home/liying/sqlcc/include/core/unified_executor.h` - 引用形式: ``/home/liying/sqlcc/include/core/unified_executor.h``
- `/home/liying/sqlcc/include/database_manager.h` - 引用形式: ``/home/liying/sqlcc/include/database_manager.h``
- `/home/liying/sqlcc/include/disk_manager.h` - 引用形式: ``/home/liying/sqlcc/include/disk_manager.h``
- `/home/liying/sqlcc/include/execution/set_operation_executor.h` - 引用形式: ``/home/liying/sqlcc/include/execution/set_operation_executor.h``
- `/home/liying/sqlcc/include/sql_parser/set_operation_node.h` - 引用形式: ``/home/liying/sqlcc/include/sql_parser/set_operation_node.h``
- `/home/liying/sqlcc/include/storage/b_plus_tree.h` - 引用形式: ``/home/liying/sqlcc/include/storage/b_plus_tree.h``
- `/home/liying/sqlcc/include/storage/b_plus_tree_nodes.h` - 引用形式: ``/home/liying/sqlcc/include/storage/b_plus_tree_nodes.h``
- `/home/liying/sqlcc/include/storage/buffer_pool_new.h` - 引用形式: ``/home/liying/sqlcc/include/storage/buffer_pool_new.h``
- `/home/liying/sqlcc/include/storage/buffer_pool_v2.h` - 引用形式: ``/home/liying/sqlcc/include/storage/buffer_pool_v2.h``
- `/home/liying/sqlcc/include/storage/performance_monitor.h` - 引用形式: ``/home/liying/sqlcc/include/storage/performance_monitor.h``
- `/home/liying/sqlcc/include/storage/prefetcher.h` - 引用形式: ``/home/liying/sqlcc/include/storage/prefetcher.h``
- `/home/liying/sqlcc/include/storage/replace_strategy.h` - 引用形式: ``/home/liying/sqlcc/include/storage/replace_strategy.h``
- `/home/liying/sqlcc/include/storage/table_storage.h` - 引用形式: ``/home/liying/sqlcc/include/storage/table_storage.h``
- `/home/liying/sqlcc/include/utils/file_descriptor.h` - 引用形式: ``/home/liying/sqlcc/include/utils/file_descriptor.h``
- `/home/liying/sqlcc/include/utils/ssl_wrapper.h` - 引用形式: ``/home/liying/sqlcc/include/utils/ssl_wrapper.h``
- `/home/liying/sqlcc/src/core/database_manager.cpp` - 引用形式: ``/home/liying/sqlcc/src/core/database_manager.cpp``
- `/home/liying/sqlcc/src/sql_parser/lexer_new.cpp` - 引用形式: ``/home/liying/sqlcc/src/sql_parser/lexer_new.cpp``
- `/home/liying/sqlcc/src/sql_parser/token_new.cpp` - 引用形式: ``/home/liying/sqlcc/src/sql_parser/token_new.cpp``
- `/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/b_plus_tree.cpp``
- `/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/buffer_pool.cpp``
- `/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/buffer_pool_new.cpp``
- `/home/liying/sqlcc/src/storage_engine/buffer_pool_sharded.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/buffer_pool_sharded.cpp``
- `/home/liying/sqlcc/src/storage_engine/disk_manager.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/disk_manager.cpp``
- `/home/liying/sqlcc/src/storage_engine/replace_strategy.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/replace_strategy.cpp``
- `/home/liying/sqlcc/src/storage_engine/storage_engine.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/storage_engine.cpp``
- `/home/liying/sqlcc/src/storage_engine/table_storage.cpp` - 引用形式: ``/home/liying/sqlcc/src/storage_engine/table_storage.cpp``
- `/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/client_server/client_server_integration_test.cpp``
- `/home/liying/sqlcc/tests/client_server/client_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/client_server/client_test.cpp``
- `/home/liying/sqlcc/tests/client_server/encrypted_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/client_server/encrypted_integration_test.cpp``
- `/home/liying/sqlcc/tests/client_server/encrypted_test_runner.cpp` - 引用形式: ``/home/liying/sqlcc/tests/client_server/encrypted_test_runner.cpp``
- `/home/liying/sqlcc/tests/components/debug/comprehensive_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/debug/comprehensive_test.cpp``
- `/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/debug/file_descriptor_test.cpp``
- `/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/debug/memory_audit_tool.cpp``
- `/home/liying/sqlcc/tests/components/debug/test_performance_real.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/debug/test_performance_real.cpp``
- `/home/liying/sqlcc/tests/components/executor/index_optimization_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/index_optimization_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/index_usage_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/index_usage_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/join_executor_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/privilege_consistency_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/privilege_consistency_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/set_operation_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/set_operation_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/unified_executor_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/unified_executor_test.cpp``
- `/home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/executor/where_clause_optimization_test.cpp``
- `/home/liying/sqlcc/tests/components/network/network_unit_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/network/network_unit_test.cpp``
- `/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/parser/set_operation_parser_test.cpp``
- `/home/liying/sqlcc/tests/components/storage/buffer_pool_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/storage/buffer_pool_test.cpp``
- `/home/liying/sqlcc/tests/components/storage/disk_manager_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/storage/disk_manager_test.cpp``
- `/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/components/storage/index_system_integration_test.cpp``
- `/home/liying/sqlcc/tests/integration/advanced_sql/isql_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/advanced_sql/isql_integration_test.cpp``
- `/home/liying/sqlcc/tests/integration/advanced_sql/sql_executor_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/advanced_sql/sql_executor_integration_test.cpp``
- `/home/liying/sqlcc/tests/integration/basic_sql/simple_sql_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/basic_sql/simple_sql_test.cpp``
- `/home/liying/sqlcc/tests/integration/isql_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/isql_integration_test.cpp``
- `/home/liying/sqlcc/tests/integration/simple_sql_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/simple_sql_test.cpp``
- `/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp``
- `/home/liying/sqlcc/tests/integration/sql_executor_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/sql_executor_integration_test.cpp``
- `/home/liying/sqlcc/tests/legacy/test_dcl_ddl_persistence.cpp` - 引用形式: ``/home/liying/sqlcc/tests/legacy/test_dcl_ddl_persistence.cpp``
- `/home/liying/sqlcc/tests/network/aes_encryption_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/network/aes_encryption_test.cc``
- `/home/liying/sqlcc/tests/network/aes_network_integration_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/network/aes_network_integration_test.cc``
- `/home/liying/sqlcc/tests/network/sql_network_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/network/sql_network_test.cpp``
- `/home/liying/sqlcc/tests/network/tls_e2e_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/network/tls_e2e_test.cc``
- `/home/liying/sqlcc/tests/performance/basic/batch_prefetch_performance_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/basic/batch_prefetch_performance_test.cc``
- `/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/basic/concurrency_performance_test.cc``
- `/home/liying/sqlcc/tests/performance/basic/cpu_intensive_performance_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/basic/cpu_intensive_performance_test.cc``
- `/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/basic/crud_performance_test.cc``
- `/home/liying/sqlcc/tests/performance/buffer_pool_performance_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/buffer_pool_performance_test.h``
- `/home/liying/sqlcc/tests/performance/concurrency/concurrency_performance_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/concurrency/concurrency_performance_test.h``
- `/home/liying/sqlcc/tests/performance/concurrency_test/concurrency_performance_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/concurrency_test/concurrency_performance_test.h``
- `/home/liying/sqlcc/tests/performance/cpu_test/cpu_intensive_performance_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/cpu_test/cpu_intensive_performance_test.h``
- `/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/crud/large_scale_crud_test.cc``
- `/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/performance/crud/real_crud_performance_test.cpp``
- `/home/liying/sqlcc/tests/performance/disk_io_performance_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/disk_io_performance_test.h``
- `/home/liying/sqlcc/tests/performance/index_performance_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/index_performance_test.h``
- `/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.cc``
- `/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/memory_stress_test/memory_stress_test.h``
- `/home/liying/sqlcc/tests/performance/million_insert_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/million_insert_test.h``
- `/home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test.h` - 引用形式: ``/home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test.h``
- `/home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test_main.cc` - 引用形式: ``/home/liying/sqlcc/tests/performance/stability_test/long_term_stability_test_main.cc``
- `/home/liying/sqlcc/tests/sql_executor/sql_executor_comprehensive_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_executor/sql_executor_comprehensive_test.cpp``
- `/home/liying/sqlcc/tests/sql_executor/sql_executor_minimal_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_executor/sql_executor_minimal_test.cpp``
- `/home/liying/sqlcc/tests/sql_executor/sql_executor_unit_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_executor/sql_executor_unit_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/lexer_integration_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/lexer_new_benchmark_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/lexer_new_benchmark_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/lexer_new_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/lexer_new_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/lexer_new_unit_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/lexer_new_unit_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/lexer_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/lexer_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/parser_new_integration_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/parser_new_integration_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/parser_performance_benchmark_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/performance_comparison_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/simple_parser_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/simple_parser_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/sql_parser_test.cpp``
- `/home/liying/sqlcc/tests/sql_parser/token_new_unit_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/sql_parser/token_new_unit_test.cpp``
- `/home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/storage_engine/buffer_pool_v3_test.cpp``
- `/home/liying/sqlcc/tests/test_disk_manager.h` - 引用形式: ``/home/liying/sqlcc/tests/test_disk_manager.h``
- `/home/liying/sqlcc/tests/test_revoke_persistence.cpp` - 引用形式: ``/home/liying/sqlcc/tests/test_revoke_persistence.cpp``
- `/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_output.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_output.cpp``
- `/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_simple.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unit/parser/tests_development/debug_lexer_simple.cpp``
- `/home/liying/sqlcc/tests/unsupported_commands_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unsupported_commands_test.cpp``

### docs/项目进展/v1.1.5/v1.1.5_存储过程和触发器重构报告.md

- `include/sql_parser/ast_nodes.h` - 引用形式: ``include/sql_parser/ast_nodes.h``
- `include/core/execution_context.h` - 引用形式: ``include/core/execution_context.h``
- `src/core/procedure_executor.cpp` - 引用形式: ``src/core/procedure_executor.cpp``
- `src/core/trigger_executor.cpp` - 引用形式: ``src/core/trigger_executor.cpp``
- `src/storage_engine/metadata_manager.cpp` - 引用形式: ``src/storage_engine/metadata_manager.cpp``
- `include/storage/metadata_manager.h` - 引用形式: ``include/storage/metadata_manager.h``
- `src/storage_engine/system_tables.cpp` - 引用形式: ``src/storage_engine/system_tables.cpp``
- `src/core/transaction_manager.cpp` - 引用形式: ``src/core/transaction_manager.cpp``
- `include/core/transaction_context.h` - 引用形式: ``include/core/transaction_context.h``
- `include/core/permissions.h` - 引用形式: ``include/core/permissions.h``

### docs/项目进展/v1.2.11/SQLCC测试系统完整索引报告_v1.2.11.md

- `include/execution/task_result.h` - 引用形式: ``include/execution/task_result.h``

### docs/项目进展/v1.3.2/测试文件编译运行状态分析报告.md

- `tests/level5_execution/execution_context/execution_context_test.cpp` - 引用形式: ``tests/level5_execution/execution_context/execution_context_test.cpp``
- `tests/level5_execution/json_operations/json_operations_test.cpp` - 引用形式: ``tests/level5_execution/json_operations/json_operations_test.cpp``
- `tests/level5_execution/permission_validator/permission_validator_test.cpp` - 引用形式: ``tests/level5_execution/permission_validator/permission_validator_test.cpp``
- `tests/level5_execution/sql_executor_core/sql_executor_core_test.cpp` - 引用形式: ``tests/level5_execution/sql_executor_core/sql_executor_core_test.cpp``
- `tests/level5_execution/stored_procedure/stored_procedure_test.cpp` - 引用形式: ``tests/level5_execution/stored_procedure/stored_procedure_test.cpp``
- `tests/level5_execution/trigger/trigger_test.cpp` - 引用形式: ``tests/level5_execution/trigger/trigger_test.cpp``

### docs/项目进展/v1.2.15/v1.2.15_SQLCC注释补全项目Phase9-14工作计划文档.md

- `include/core/execution_context.h` - 引用形式: ``include/core/execution_context.h``
- `include/core/execution_result.h` - 引用形式: ``include/core/execution_result.h``
- `include/core/system_database.h` - 引用形式: ``include/core/system_database.h``
- `include/core/core_database_manager.h` - 引用形式: ``include/core/core_database_manager.h``
- `include/storage/buffer_pool.h` - 引用形式: ``include/storage/buffer_pool.h``
- `include/storage_engine/b_plus_tree.h` - 引用形式: ``include/storage_engine/b_plus_tree.h``
- `include/storage_engine/b_plus_tree_index.h` - 引用形式: ``include/storage_engine/b_plus_tree_index.h``
- `include/storage_engine/b_plus_tree_node.h` - 引用形式: ``include/storage_engine/b_plus_tree_node.h``
- `include/storage_engine/b_plus_tree_leaf_node.h` - 引用形式: ``include/storage_engine/b_plus_tree_leaf_node.h``
- `include/storage_engine/b_plus_tree_internal_node.h` - 引用形式: ``include/storage_engine/b_plus_tree_internal_node.h``
- `include/storage_engine/buffer_pool/lru_manager.h` - 引用形式: ``include/storage_engine/buffer_pool/lru_manager.h``
- `include/storage_engine/buffer_pool/statistics_collector.h` - 引用形式: ``include/storage_engine/buffer_pool/statistics_collector.h``
- `include/storage/advanced_lock_manager.h` - 引用形式: ``include/storage/advanced_lock_manager.h``
- `include/storage/concurrency_control.h` - 引用形式: ``include/storage/concurrency_control.h``
- `include/storage/disk_error_handler.h` - 引用形式: ``include/storage/disk_error_handler.h``
- `include/storage/record_boundary_validator.h` - 引用形式: ``include/storage/record_boundary_validator.h``
- `include/storage/cache_consistency_manager.h` - 引用形式: ``include/storage/cache_consistency_manager.h``
- `include/storage/wal_buffer.h` - 引用形式: ``include/storage/wal_buffer.h``
- `include/storage/wal_writer.h` - 引用形式: ``include/storage/wal_writer.h``
- `include/sql_parser/ast_node.h` - 引用形式: ``include/sql_parser/ast_node.h``
- `include/sql_parser/ast_nodes.h` - 引用形式: ``include/sql_parser/ast_nodes.h``
- `include/sql_parser/parser.h` - 引用形式: ``include/sql_parser/parser.h``
- `include/sql_parser/token.h` - 引用形式: ``include/sql_parser/token.h``
- `include/sql_parser/constraint.h` - 引用形式: ``include/sql_parser/constraint.h``
- `include/sql_parser/data_types.h` - 引用形式: ``include/sql_parser/data_types.h``
- `include/sql_parser/window_function.h` - 引用形式: ``include/sql_parser/window_function.h``
- `include/sql_parser/set_operation.h` - 引用形式: ``include/sql_parser/set_operation.h``
- `include/sql_parser/function_ast.h` - 引用形式: ``include/sql_parser/function_ast.h``
- `include/sql_parser/json.h` - 引用形式: ``include/sql_parser/json.h``
- `include/sql_parser/datetime.h` - 引用形式: ``include/sql_parser/datetime.h``
- `include/sql_parser/node_visitor.h` - 引用形式: ``include/sql_parser/node_visitor.h``
- `include/sql_parser/ast_fwd.h` - 引用形式: ``include/sql_parser/ast_fwd.h``
- `include/sql_parser/recursive_query.h` - 引用形式: ``include/sql_parser/recursive_query.h``
- `include/sql_parser/json/json_value.h` - 引用形式: ``include/sql_parser/json/json_value.h``
- `include/sql_parser/json/json_null.h` - 引用形式: ``include/sql_parser/json/json_null.h``
- `include/sql_parser/json/json_boolean.h` - 引用形式: ``include/sql_parser/json/json_boolean.h``
- `include/sql_parser/json/json_number.h` - 引用形式: ``include/sql_parser/json/json_number.h``
- `include/sql_parser/json/json_string.h` - 引用形式: ``include/sql_parser/json/json_string.h``
- `include/sql_parser/json/json_array.h` - 引用形式: ``include/sql_parser/json/json_array.h``
- `include/execution/task_executor.h` - 引用形式: ``include/execution/task_executor.h``
- `include/execution/task_result.h` - 引用形式: ``include/execution/task_result.h``
- `include/execution/query_plan_factory.h` - 引用形式: ``include/execution/query_plan_factory.h``
- `include/execution/unified_query_plan.h` - 引用形式: ``include/execution/unified_query_plan.h``
- `include/execution/cost_estimator.h` - 引用形式: ``include/execution/cost_estimator.h``
- `include/execution/function_executor.h` - 引用形式: ``include/execution/function_executor.h``
- `include/execution/set_operation_executor.h` - 引用形式: ``include/execution/set_operation_executor.h``
- `include/execution/load_data_executor.h` - 引用形式: ``include/execution/load_data_executor.h``
- `include/execution/window_function_executor.h` - 引用形式: ``include/execution/window_function_executor.h``
- `include/execution/recursive_query_executor.h` - 引用形式: ``include/execution/recursive_query_executor.h``
- `include/execution/join_executor.h` - 引用形式: ``include/execution/join_executor.h``
- `include/execution/ddl_execution_strategy.h` - 引用形式: ``include/execution/ddl_execution_strategy.h``
- `include/execution/dml_execution_strategy.h` - 引用形式: ``include/execution/dml_execution_strategy.h``
- `include/execution/execution_strategy.h` - 引用形式: ``include/execution/execution_strategy.h``
- `include/execution/comprehensive_task_executor.h` - 引用形式: ``include/execution/comprehensive_task_executor.h``
- `include/utils/config_manager.h` - 引用形式: ``include/utils/config_manager.h``
- `include/utils/config_snapshot.h` - 引用形式: ``include/utils/config_snapshot.h``
- `include/utils/smart_config_manager.h` - 引用形式: ``include/utils/smart_config_manager.h``
- `include/utils/logger.h` - 引用形式: ``include/utils/logger.h``
- `include/exception.h` - 引用形式: ``include/exception.h``
- `include/exception/base_exception.h` - 引用形式: ``include/exception/base_exception.h``
- `include/exception/io_exception.h` - 引用形式: ``include/exception/io_exception.h``
- `include/exception/buffer_exception.h` - 引用形式: ``include/exception/buffer_exception.h``
- `include/exception/page_exception.h` - 引用形式: ``include/exception/page_exception.h``
- `include/exception/disk_exception.h` - 引用形式: ``include/exception/disk_exception.h``
- `include/exception/lock_exception.h` - 引用形式: ``include/exception/lock_exception.h``
- `include/exception/feature_exception.h` - 引用形式: ``include/exception/feature_exception.h``
- `include/exception/argument_exception.h` - 引用形式: ``include/exception/argument_exception.h``

### docs/项目进展/v1.2.15/v1.2.15_SQLCC注释补全项目Phase9-14工作总结.md

- `src/config_manager.cpp` - 引用形式: ``src/config_manager.cpp``
- `src/transaction_manager.cpp` - 引用形式: ``src/transaction_manager.cpp``
- `src/storage_engine.cpp` - 引用形式: ``src/storage_engine.cpp``
- `src/b_plus_tree.cpp` - 引用形式: ``src/b_plus_tree.cpp``
- `src/index_manager.cpp` - 引用形式: ``src/index_manager.cpp``
- `src/parser.cpp` - 引用形式: ``src/parser.cpp``
- `src/lexer.cpp` - 引用形式: ``src/lexer.cpp``
- `src/ast_node.cpp` - 引用形式: ``src/ast_node.cpp``
- `include/core/execution_context.h` - 引用形式: ``include/core/execution_context.h``
- `include/core/execution_result.h` - 引用形式: ``include/core/execution_result.h``
- `include/core/system_database.h` - 引用形式: ``include/core/system_database.h``
- `include/core/core_database_manager.h` - 引用形式: ``include/core/core_database_manager.h``
- `include/storage/buffer_pool.h` - 引用形式: ``include/storage/buffer_pool.h``

### docs/项目进展/v1.2.15/v1.2.15_SQLCC注释补全项目完整TODO清单.md

- `include/core/execution_context.h` - 引用形式: ``include/core/execution_context.h``
- `include/storage/b_plus_tree.h` - 引用形式: ``include/storage/b_plus_tree.h``
- `include/storage/b_plus_tree_nodes.h` - 引用形式: ``include/storage/b_plus_tree_nodes.h``
- `include/utils/config_snapshot.h` - 引用形式: ``include/utils/config_snapshot.h``
- `include/utils/smart_config_manager.h` - 引用形式: ``include/utils/smart_config_manager.h``
- `include/exception.h` - 引用形式: ``include/exception.h``
- `include/constraint_executor.h` - 引用形式: ``include/constraint_executor.h``
- `include/storage/buffer_pool.h` - 引用形式: ``include/storage/buffer_pool.h``
- `include/storage/advanced_lock_manager.h` - 引用形式: ``include/storage/advanced_lock_manager.h``
- `include/storage/cache_consistency_manager.h` - 引用形式: ``include/storage/cache_consistency_manager.h``
- `include/storage/concurrency_control.h` - 引用形式: ``include/storage/concurrency_control.h``
- `include/storage/data_integrity_validator.h` - 引用形式: ``include/storage/data_integrity_validator.h``
- `include/storage/disk_error_handler.h` - 引用形式: ``include/storage/disk_error_handler.h``
- `include/storage/page_allocator.h` - 引用形式: ``include/storage/page_allocator.h``
- `include/storage/partition_manager.h` - 引用形式: ``include/storage/partition_manager.h``
- `include/storage/record_boundary_validator.h` - 引用形式: ``include/storage/record_boundary_validator.h``
- `include/storage/wal_buffer.h` - 引用形式: ``include/storage/wal_buffer.h``
- `include/storage/wal_writer.h` - 引用形式: ``include/storage/wal_writer.h``
- `include/storage/checkpoint.h` - 引用形式: ``include/storage/checkpoint.h``
- `include/core/core_database_manager.h` - 引用形式: ``include/core/core_database_manager.h``
- `include/core/execution_result.h` - 引用形式: ``include/core/execution_result.h``
- `include/core/permission_validator.h` - 引用形式: ``include/core/permission_validator.h``
- `include/core/schema_manager.h` - 引用形式: ``include/core/schema_manager.h``
- `include/core/sql_executor_interface.h` - 引用形式: ``include/core/sql_executor_interface.h``
- `include/core/stored_procedure_manager.h` - 引用形式: ``include/core/stored_procedure_manager.h``
- `include/core/system_database.h` - 引用形式: ``include/core/system_database.h``
- `include/core/unified_executor.h` - 引用形式: ``include/core/unified_executor.h``
- `include/core/user_manager.h` - 引用形式: ``include/core/user_manager.h``
- `include/execution/task_executor.h` - 引用形式: ``include/execution/task_executor.h``
- `include/execution/task_result.h` - 引用形式: ``include/execution/task_result.h``
- `include/execution/query_plan_factory.h` - 引用形式: ``include/execution/query_plan_factory.h``
- `include/execution/cost_estimator.h` - 引用形式: ``include/execution/cost_estimator.h``
- `include/execution/function_executor.h` - 引用形式: ``include/execution/function_executor.h``
- `include/execution/set_operation_executor.h` - 引用形式: ``include/execution/set_operation_executor.h``
- `include/execution/load_data_executor.h` - 引用形式: ``include/execution/load_data_executor.h``
- `include/execution/window_function_executor.h` - 引用形式: ``include/execution/window_function_executor.h``
- `include/execution/recursive_query_executor.h` - 引用形式: ``include/execution/recursive_query_executor.h``
- `include/execution/join_executor.h` - 引用形式: ``include/execution/join_executor.h``
- `include/execution/ddl_execution_strategy.h` - 引用形式: ``include/execution/ddl_execution_strategy.h``
- `include/execution/dml_execution_strategy.h` - 引用形式: ``include/execution/dml_execution_strategy.h``
- `include/execution/execution_strategy.h` - 引用形式: ``include/execution/execution_strategy.h``
- `include/execution/comprehensive_task_executor.h` - 引用形式: ``include/execution/comprehensive_task_executor.h``
- `include/execution/aggregate_engine.h` - 引用形式: ``include/execution/aggregate_engine.h``
- `include/execution/group_by_executor.h` - 引用形式: ``include/execution/group_by_executor.h``
- `include/execution/test_runner.h` - 引用形式: ``include/execution/test_runner.h``
- `include/execution/standalone_test.h` - 引用形式: ``include/execution/standalone_test.h``
- `include/storage/buffer_pool.h` - 引用形式: ``include/storage/buffer_pool.h``
- `include/storage/advanced_lock_manager.h` - 引用形式: ``include/storage/advanced_lock_manager.h``
- `include/storage/cache_consistency_manager.h` - 引用形式: ``include/storage/cache_consistency_manager.h``
- `include/storage/concurrency_control.h` - 引用形式: ``include/storage/concurrency_control.h``
- `include/storage/data_integrity_validator.h` - 引用形式: ``include/storage/data_integrity_validator.h``
- `include/storage/disk_error_handler.h` - 引用形式: ``include/storage/disk_error_handler.h``
- `include/storage/page_allocator.h` - 引用形式: ``include/storage/page_allocator.h``
- `include/storage/partition_manager.h` - 引用形式: ``include/storage/partition_manager.h``
- `include/storage/record_boundary_validator.h` - 引用形式: ``include/storage/record_boundary_validator.h``
- `include/storage/wal_buffer.h` - 引用形式: ``include/storage/wal_buffer.h``
- `include/storage/wal_writer.h` - 引用形式: ``include/storage/wal_writer.h``
- `include/storage/checkpoint.h` - 引用形式: ``include/storage/checkpoint.h``
- `include/core/core_database_manager.h` - 引用形式: ``include/core/core_database_manager.h``
- `include/core/execution_result.h` - 引用形式: ``include/core/execution_result.h``
- `include/core/permission_validator.h` - 引用形式: ``include/core/permission_validator.h``
- `include/core/schema_manager.h` - 引用形式: ``include/core/schema_manager.h``
- `include/core/sql_executor_interface.h` - 引用形式: ``include/core/sql_executor_interface.h``
- `include/core/stored_procedure_manager.h` - 引用形式: ``include/core/stored_procedure_manager.h``
- `include/core/system_database.h` - 引用形式: ``include/core/system_database.h``
- `include/core/unified_executor.h` - 引用形式: ``include/core/unified_executor.h``
- `include/core/user_manager.h` - 引用形式: ``include/core/user_manager.h``
- `include/execution/task_executor.h` - 引用形式: ``include/execution/task_executor.h``
- `include/execution/task_result.h` - 引用形式: ``include/execution/task_result.h``
- `include/execution/query_plan_factory.h` - 引用形式: ``include/execution/query_plan_factory.h``
- `include/execution/cost_estimator.h` - 引用形式: ``include/execution/cost_estimator.h``
- `include/execution/function_executor.h` - 引用形式: ``include/execution/function_executor.h``
- `include/execution/set_operation_executor.h` - 引用形式: ``include/execution/set_operation_executor.h``
- `include/execution/load_data_executor.h` - 引用形式: ``include/execution/load_data_executor.h``
- `include/execution/window_function_executor.h` - 引用形式: ``include/execution/window_function_executor.h``
- `include/execution/recursive_query_executor.h` - 引用形式: ``include/execution/recursive_query_executor.h``
- `include/execution/join_executor.h` - 引用形式: ``include/execution/join_executor.h``
- `include/execution/ddl_execution_strategy.h` - 引用形式: ``include/execution/ddl_execution_strategy.h``
- `include/execution/dml_execution_strategy.h` - 引用形式: ``include/execution/dml_execution_strategy.h``
- `include/execution/execution_strategy.h` - 引用形式: ``include/execution/execution_strategy.h``
- `include/execution/comprehensive_task_executor.h` - 引用形式: ``include/execution/comprehensive_task_executor.h``
- `include/execution/aggregate_engine.h` - 引用形式: ``include/execution/aggregate_engine.h``
- `include/execution/group_by_executor.h` - 引用形式: ``include/execution/group_by_executor.h``
- `include/execution/test_runner.h` - 引用形式: ``include/execution/test_runner.h``
- `include/execution/standalone_test.h` - 引用形式: ``include/execution/standalone_test.h``
- `include/sql_parser/ast_node.h` - 引用形式: ``include/sql_parser/ast_node.h``
- `include/sql_parser/ast_nodes.h` - 引用形式: ``include/sql_parser/ast_nodes.h``
- `include/sql_parser/data_types.h` - 引用形式: ``include/sql_parser/data_types.h``
- `include/sql_parser/token.h` - 引用形式: ``include/sql_parser/token.h``
- `include/sql_parser/constraint.h` - 引用形式: ``include/sql_parser/constraint.h``
- `include/sql_parser/window_function.h` - 引用形式: ``include/sql_parser/window_function.h``
- `include/sql_parser/set_operation.h` - 引用形式: ``include/sql_parser/set_operation.h``
- `include/sql_parser/parser.h` - 引用形式: ``include/sql_parser/parser.h``
- `include/sql_parser/json.h` - 引用形式: ``include/sql_parser/json.h``
- `include/sql_parser/datetime.h` - 引用形式: ``include/sql_parser/datetime.h``
- `include/sql_parser/recursive_query.h` - 引用形式: ``include/sql_parser/recursive_query.h``
- `include/sql_parser/node_visitor.h` - 引用形式: ``include/sql_parser/node_visitor.h``
- `include/sql_parser/ast_fwd.h` - 引用形式: ``include/sql_parser/ast_fwd.h``
- `include/sql_parser/function_ast.h` - 引用形式: ``include/sql_parser/function_ast.h``
- `include/storage_engine/b_plus_tree_index.h` - 引用形式: ``include/storage_engine/b_plus_tree_index.h``
- `include/storage_engine/b_plus_tree_node.h` - 引用形式: ``include/storage_engine/b_plus_tree_node.h``
- `include/storage_engine/b_plus_tree_leaf_node.h` - 引用形式: ``include/storage_engine/b_plus_tree_leaf_node.h``
- `include/storage_engine/b_plus_tree_internal_node.h` - 引用形式: ``include/storage_engine/b_plus_tree_internal_node.h``
- `include/storage_engine/buffer_pool/lru_manager.h` - 引用形式: ``include/storage_engine/buffer_pool/lru_manager.h``
- `include/storage_engine/buffer_pool/statistics_collector.h` - 引用形式: ``include/storage_engine/buffer_pool/statistics_collector.h``
- `include/storage_engine/index_manager/smart_index_cache.h` - 引用形式: ``include/storage_engine/index_manager/smart_index_cache.h``
- `include/storage_engine/table_storage.h` - 引用形式: ``include/storage_engine/table_storage.h``
- `include/exception/base_exception.h` - 引用形式: ``include/exception/base_exception.h``
- `include/exception/io_exception.h` - 引用形式: ``include/exception/io_exception.h``
- `include/exception/buffer_exception.h` - 引用形式: ``include/exception/buffer_exception.h``
- `include/exception/page_exception.h` - 引用形式: ``include/exception/page_exception.h``
- `include/exception/disk_exception.h` - 引用形式: ``include/exception/disk_exception.h``
- `include/exception/lock_exception.h` - 引用形式: ``include/exception/lock_exception.h``
- `include/exception/feature_exception.h` - 引用形式: ``include/exception/feature_exception.h``
- `include/exception/argument_exception.h` - 引用形式: ``include/exception/argument_exception.h``
- `include/utils/config_manager.h` - 引用形式: ``include/utils/config_manager.h``
- `include/utils/config_lifecycle.h` - 引用形式: ``include/utils/config_lifecycle.h``
- `include/utils/logger.h` - 引用形式: ``include/utils/logger.h``

### docs/项目进展/v1.2.15/v1.2.15_SQLCC注释补全项目核心源文件阶段完成报告.md

- `src/config_manager.cpp` - 引用形式: ``src/config_manager.cpp``
- `src/storage_engine/storage_engine.cpp` - 引用形式: ``src/storage_engine/storage_engine.cpp``
- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `src/storage_engine/buffer_pool_sharded.cpp` - 引用形式: ``src/storage_engine/buffer_pool_sharded.cpp``
- `src/storage_engine/disk_manager.cpp` - 引用形式: ``src/storage_engine/disk_manager.cpp``
- `include/utils/logger.h` - 引用形式: ``include/utils/logger.h``
- `include/exception.h` - 引用形式: ``include/exception.h``
- `include/constraint_executor.h` - 引用形式: ``include/constraint_executor.h``
- `include/disk_manager.h` - 引用形式: ``include/disk_manager.h``
- `include/storage_engine/b_plus_tree_node.h` - 引用形式: ``include/storage_engine/b_plus_tree_node.h``
- `include/storage_engine/b_plus_tree_leaf_node.h` - 引用形式: ``include/storage_engine/b_plus_tree_leaf_node.h``
- `include/storage_engine/b_plus_tree_index.h` - 引用形式: ``include/storage_engine/b_plus_tree_index.h``
- `include/storage_engine/buffer_pool.h` - 引用形式: ``include/storage_engine/buffer_pool.h``
- `include/sql_parser/ast_node.h` - 引用形式: ``include/sql_parser/ast_node.h``
- `include/sql_parser/parser.h` - 引用形式: ``include/sql_parser/parser.h``
- `include/sql_parser/lexer.h` - 引用形式: ``include/sql_parser/lexer.h``
- `include/sql_parser/constraint.h` - 引用形式: ``include/sql_parser/constraint.h``
- `include/sql_parser/function_ast.h` - 引用形式: ``include/sql_parser/function_ast.h``
- `include/execution/task_executor.h` - 引用形式: ``include/execution/task_executor.h``
- `include/execution/task_result.h` - 引用形式: ``include/execution/task_result.h``
- `include/execution/query_plan_factory.h` - 引用形式: ``include/execution/query_plan_factory.h``
- `include/execution/join_executor.h` - 引用形式: ``include/execution/join_executor.h``
- `include/execution/window_function_executor.h` - 引用形式: ``include/execution/window_function_executor.h``

### docs/项目进展/v1.2.6/v1.2.6重构工作日记.md

- `include/sql_parser/recursive_query.h` - 引用形式: ``include/sql_parser/recursive_query.h``
- `include/core/database_manager.h` - 引用形式: ``include/core/database_manager.h``
- `include/database_manager.h` - 引用形式: ``include/database_manager.h``
- `include/core/database_manager.h` - 引用形式: ``include/core/database_manager.h``
- `include/core/core_database_manager.h` - 引用形式: ``include/core/core_database_manager.h``

### docs/项目进展/v1.2.6/package_build_order_analysis.md

- `include/sql_parser/parser.h` - 引用形式: ``include/sql_parser/parser.h``
- `include/database_manager.h` - 引用形式: ``include/database_manager.h``
- `include/utils/config_manager.h` - 引用形式: ``include/utils/config_manager.h``
- `include/utils/logger.h` - 引用形式: ``include/utils/logger.h``
- `include/sql_parser/lexer.h` - 引用形式: ``include/sql_parser/lexer.h``
- `include/sql_parser/ast_nodes.h` - 引用形式: ``include/sql_parser/ast_nodes.h``
- `include/storage/b_plus_tree.h` - 引用形式: ``include/storage/b_plus_tree.h``
- `include/sql_parser/parser.h` - 引用形式: ``include/sql_parser/parser.h``

### docs/项目进展/v1.2.6/phase2_bottom_up_fix_progress_report.md

- `sql_parser/ast_nodes.h` - 引用形式: ``sql_parser/ast_nodes.h``

### docs/项目进展/v1.2.6/package_refactoring_status_report.md

- `include/storage_engine/b_plus_tree_nodes.h` - 引用形式: ``include/storage_engine/b_plus_tree_nodes.h``
- `include/sql_parser/decimal.h` - 引用形式: ``include/sql_parser/decimal.h``

### docs/项目进展/v1.2.6/v1.2.6_TODO.md

- `src/storage_engine/advanced_index.h` - 引用形式: ``src/storage_engine/advanced_index.h``
- `src/procedure/procedure_vm.h` - 引用形式: ``src/procedure/procedure_vm.h``

### docs/项目进展/v1.2.6/comprehensive_package_build_status_report.md

- `storage_engine/advanced_index.h` - 引用形式: ``storage_engine/advanced_index.h``

### docs/项目进展/v1.2.3/v1.2.3_内存安全专项修复总结.md

- `/tests/security/memory_safety_framework.cpp` - 引用形式: ``/tests/security/memory_safety_framework.cpp``
- `/tests/security/memory_safety_framework.cpp` - 引用形式: ``/tests/security/memory_safety_framework.cpp``
- `/src/security/memory_monitor.h` - 引用形式: ``/src/security/memory_monitor.h``
- `/src/security/memory_monitor.cpp` - 引用形式: ``/src/security/memory_monitor.cpp``

### docs/项目进展/v1.2.3/测试改进TODO清单.md

- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `src/sql_parser/lexer_new.cpp` - 引用形式: ``src/sql_parser/lexer_new.cpp``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``
- `tests/unit/storage_engine/b_plus_tree_test.cpp` - 引用形式: ``tests/unit/storage_engine/b_plus_tree_test.cpp``
- `tests/unit/storage_engine/buffer_pool_test.cpp` - 引用形式: ``tests/unit/storage_engine/buffer_pool_test.cpp``
- `tests/unit/storage_engine/lock_manager_test.cpp` - 引用形式: ``tests/unit/storage_engine/lock_manager_test.cpp``
- `tests/unit/executor/task_executor_test.cpp` - 引用形式: ``tests/unit/executor/task_executor_test.cpp``
- `tests/unit/executor/query_executor_test.cpp` - 引用形式: ``tests/unit/executor/query_executor_test.cpp``
- `tests/unit/sql_parser/lexer_test.cpp` - 引用形式: ``tests/unit/sql_parser/lexer_test.cpp``
- `tests/unit/sql_parser/parser_test.cpp` - 引用形式: ``tests/unit/sql_parser/parser_test.cpp``
- `tests/unit/network/connection_test.cpp` - 引用形式: ``tests/unit/network/connection_test.cpp``
- `tests/integration/sql/ddl_test_suite.cpp` - 引用形式: ``tests/integration/sql/ddl_test_suite.cpp``
- `tests/integration/sql/dml_test_suite.cpp` - 引用形式: ``tests/integration/sql/dml_test_suite.cpp``
- `tests/integration/sql/query_test_suite.cpp` - 引用形式: ``tests/integration/sql/query_test_suite.cpp``
- `tests/integration/sql/constraint_test_suite.cpp` - 引用形式: ``tests/integration/sql/constraint_test_suite.cpp``
- `tests/unit/storage_engine/concurrency_test.cpp` - 引用形式: ``tests/unit/storage_engine/concurrency_test.cpp``
- `tests/unit/storage_engine/transaction_test.cpp` - 引用形式: ``tests/unit/storage_engine/transaction_test.cpp``
- `tests/unit/storage_engine/integrity_test.cpp` - 引用形式: ``tests/unit/storage_engine/integrity_test.cpp``

### docs/项目进展/v1.2.3/v1.2.3_view_trigger_improvements.md

- `include/sql_parser/ast_nodes.h` - 引用形式: ``include/sql_parser/ast_nodes.h``
- `tests/view_operations_test.cpp` - 引用形式: ``tests/view_operations_test.cpp``
- `tests/view_operations_test.cpp` - 引用形式: ``tests/view_operations_test.cpp``
- `include/sql_parser/ast_nodes.h` - 引用形式: ``include/sql_parser/ast_nodes.h``

### docs/项目进展/v1.2.3/测试源码整理完成报告.md

- `src/config_manager.cpp` - 引用形式: ``src/config_manager.cpp``

### docs/项目进展/v1.2.3/v1.2.3工作日记.md

- `/tests/security/memory_safety_framework.cpp` - 引用形式: ``/tests/security/memory_safety_framework.cpp``

### docs/项目进展/v1.2.3/TODO.md

- `tests/sql_parser/lexer_new_unit_test.cpp` - 引用形式: ``tests/sql_parser/lexer_new_unit_test.cpp``
- `tests/sql_parser/lexer_new_unit_test.cpp` - 引用形式: ``tests/sql_parser/lexer_new_unit_test.cpp``
- `src/sql_parser/lexer_new.cpp` - 引用形式: ``src/sql_parser/lexer_new.cpp``

### docs/项目进展/v1.2.3/最终项目完成报告.md

- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `tests/storage_engine/comprehensive_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/comprehensive_bplus_tree_test.cpp``

### docs/项目进展/v1.2.3/测试源码详细整理报告.md

- `/home/liying/sqlcc/debug_lexer_test.cpp` - 引用形式: ``/home/liying/sqlcc/debug_lexer_test.cpp``
- `/home/liying/sqlcc/debug_privileges_test.cpp` - 引用形式: ``/home/liying/sqlcc/debug_privileges_test.cpp``
- `basic/execution_context_test.cpp` - 引用形式: ``basic/execution_context_test.cpp``
- `basic/data_types_test.cpp` - 引用形式: ``basic/data_types_test.cpp``
- `basic/permission_validator_test.cpp` - 引用形式: ``basic/permission_validator_test.cpp``
- `basic/sql_executor_core_test.cpp` - 引用形式: ``basic/sql_executor_core_test.cpp``
- `basic/decimal_test.cpp` - 引用形式: ``basic/decimal_test.cpp``
- `parser/test_all_statements.cpp` - 引用形式: ``parser/test_all_statements.cpp``
- `parser/test_direct_keyword.cpp` - 引用形式: ``parser/test_direct_keyword.cpp``
- `parser/test_simple_insert.cpp` - 引用形式: ``parser/test_simple_insert.cpp``
- `parser/tests_development/debug_token_types.cpp` - 引用形式: ``parser/tests_development/debug_token_types.cpp``
- `parser/tests_development/debug_lexer_simple.cpp` - 引用形式: ``parser/tests_development/debug_lexer_simple.cpp``
- `parser/tests_development/debug_lexer_output.cpp` - 引用形式: ``parser/tests_development/debug_lexer_output.cpp``
- `parser/test_keyword.cpp` - 引用形式: ``parser/test_keyword.cpp``
- `parser/test_lexer.cpp` - 引用形式: ``parser/test_lexer.cpp``
- `parser/test_fix.cpp` - 引用形式: ``parser/test_fix.cpp``
- `parser/test_insert_parser.cpp` - 引用形式: ``parser/test_insert_parser.cpp``
- `execution/task_scheduler_test.cpp` - 引用形式: ``execution/task_scheduler_test.cpp``
- `network/multi_threaded_network_manager_test.cpp` - 引用形式: ``network/multi_threaded_network_manager_test.cpp``
- `executor/comprehensive_task_executor_test.cpp` - 引用形式: ``executor/comprehensive_task_executor_test.cpp``
- `executor/task_executor_comprehensive_test.cpp` - 引用形式: ``executor/task_executor_comprehensive_test.cpp``
- `executor/standalone_test.cpp` - 引用形式: ``executor/standalone_test.cpp``
- `executor/task_executor_test.cpp` - 引用形式: ``executor/task_executor_test.cpp``
- `executor/test_runner.cpp` - 引用形式: ``executor/test_runner.cpp``
- `core/stored_procedure_manager_test.cpp` - 引用形式: ``core/stored_procedure_manager_test.cpp``
- `storage/wal_buffer_test.cpp` - 引用形式: ``storage/wal_buffer_test.cpp``
- `storage/lazy_writer_test.cpp` - 引用形式: ``storage/lazy_writer_test.cpp``
- `/home/liying/sqlcc/test_constraint_demo.cpp` - 引用形式: ``/home/liying/sqlcc/test_constraint_demo.cpp``
- `/home/liying/sqlcc/test_join_functionality.cpp` - 引用形式: ``/home/liying/sqlcc/test_join_functionality.cpp``
- `/home/liying/sqlcc/test_dcl_parsing.cpp` - 引用形式: ``/home/liying/sqlcc/test_dcl_parsing.cpp``
- `/home/liying/sqlcc/simple_constraint_test.cpp` - 引用形式: ``/home/liying/sqlcc/simple_constraint_test.cpp``
- `/home/liying/sqlcc/simple_procedure_test.cpp` - 引用形式: ``/home/liying/sqlcc/simple_procedure_test.cpp``
- `advanced_sql/isql_integration_test.cpp` - 引用形式: ``advanced_sql/isql_integration_test.cpp``
- `advanced_sql/comprehensive_test.cpp` - 引用形式: ``advanced_sql/comprehensive_test.cpp``
- `advanced_sql/sql_executor_integration_test.cpp` - 引用形式: ``advanced_sql/sql_executor_integration_test.cpp``
- `window/row_number_test.cpp` - 引用形式: ``window/row_number_test.cpp``
- `distinct/select_distinct_test.cpp` - 引用形式: ``distinct/select_distinct_test.cpp``
- `basic_sql/simple_sql_test.cpp` - 引用形式: ``basic_sql/simple_sql_test.cpp``
- `subquery/scalar_subquery_test.cpp` - 引用形式: ``subquery/scalar_subquery_test.cpp``
- `grouping/having_clause_test.cpp` - 引用形式: ``grouping/having_clause_test.cpp``
- `grouping/group_by_test.cpp` - 引用形式: ``grouping/group_by_test.cpp``
- `join/inner_join_test.cpp` - 引用形式: ``join/inner_join_test.cpp``
- `/home/liying/sqlcc/tests/unit/parser/sql_parser_high_coverage_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unit/parser/sql_parser_high_coverage_test.cpp``
- `core/manual_test_system_database.cpp` - 引用形式: ``core/manual_test_system_database.cpp``
- `core/test_gtest.cpp` - 引用形式: ``core/test_gtest.cpp``
- `core/simple_test.cpp` - 引用形式: ``core/simple_test.cpp``
- `core/system_database_test.cpp` - 引用形式: ``core/system_database_test.cpp``
- `transaction/transaction_manager_test.cpp` - 引用形式: ``transaction/transaction_manager_test.cpp``
- `network/client_network_manager_test.cpp` - 引用形式: ``network/client_network_manager_test.cpp``
- `network/network_unit_test.cpp` - 引用形式: ``network/network_unit_test.cpp``
- `network/server_network_manager_test.cpp` - 引用形式: ``network/server_network_manager_test.cpp``
- `network/connection_state_machine_test.cpp` - 引用形式: ``network/connection_state_machine_test.cpp``
- `network/connection_handler_test.cpp` - 引用形式: ``network/connection_handler_test.cpp``
- `network/data_transmission_validator_test.cpp` - 引用形式: ``network/data_transmission_validator_test.cpp``
- `parser/test_show_grants.cpp` - 引用形式: ``parser/test_show_grants.cpp``
- `parser/test_percent_operator.cpp` - 引用形式: ``parser/test_percent_operator.cpp``
- `parser/test_show_commands.cpp` - 引用形式: ``parser/test_show_commands.cpp``
- `parser/test_colon_simple.cpp` - 引用形式: ``parser/test_colon_simple.cpp``
- `parser/advanced_sql_parser_test.cpp` - 引用形式: ``parser/advanced_sql_parser_test.cpp``
- `parser/dml_test.cpp` - 引用形式: ``parser/dml_test.cpp``
- `parser/dcl_test.cpp` - 引用形式: ``parser/dcl_test.cpp``
- `parser/ddl_test.cpp` - 引用形式: ``parser/ddl_test.cpp``
- `parser/set_operation_parser_test.cpp` - 引用形式: ``parser/set_operation_parser_test.cpp``
- `parser/ast_nodes_test.cpp` - 引用形式: ``parser/ast_nodes_test.cpp``
- `parser/compare_values_test.cpp` - 引用形式: ``parser/compare_values_test.cpp``
- `parser/test_create_table.cpp` - 引用形式: ``parser/test_create_table.cpp``
- `parser/parser_verification_test.cpp` - 引用形式: ``parser/parser_verification_test.cpp``
- `parser/test_colon_parsing.cpp` - 引用形式: ``parser/test_colon_parsing.cpp``
- `parser/dcl_test_advanced.cpp` - 引用形式: ``parser/dcl_test_advanced.cpp``
- `parser/simple_alter_test.cpp` - 引用形式: ``parser/simple_alter_test.cpp``
- `parser/final_dcl_test.cpp` - 引用形式: ``parser/final_dcl_test.cpp``
- `parser/comprehensive_dcl_test.cpp` - 引用形式: ``parser/comprehensive_dcl_test.cpp``
- `parser/alter_table_parser_test.cpp` - 引用形式: ``parser/alter_table_parser_test.cpp``
- `parser/test_simple_create.cpp` - 引用形式: ``parser/test_simple_create.cpp``
- `parser/test_percent_simple.cpp` - 引用形式: ``parser/test_percent_simple.cpp``
- `parser/dcl_parser_test.cpp` - 引用形式: ``parser/dcl_parser_test.cpp``
- `parser/test_create_view.cpp` - 引用形式: ``parser/test_create_view.cpp``

### docs/项目进展/v1.2.3/测试改进变更记录.md

- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `include/sql_parser/ast_nodes.h` - 引用形式: ``include/sql_parser/ast_nodes.h``

### docs/项目进展/v1.2.3/测试源码整理报告.md

- `/home/liying/sqlcc/debug_lexer_test.cpp` - 引用形式: ``/home/liying/sqlcc/debug_lexer_test.cpp``
- `/home/liying/sqlcc/debug_privileges_test.cpp` - 引用形式: ``/home/liying/sqlcc/debug_privileges_test.cpp``
- `/home/liying/sqlcc/test_constraint_demo.cpp` - 引用形式: ``/home/liying/sqlcc/test_constraint_demo.cpp``
- `/home/liying/sqlcc/test_join_functionality.cpp` - 引用形式: ``/home/liying/sqlcc/test_join_functionality.cpp``
- `/home/liying/sqlcc/test_dcl_parsing.cpp` - 引用形式: ``/home/liying/sqlcc/test_dcl_parsing.cpp``
- `/home/liying/sqlcc/simple_constraint_test.cpp` - 引用形式: ``/home/liying/sqlcc/simple_constraint_test.cpp``
- `/home/liying/sqlcc/simple_procedure_test.cpp` - 引用形式: ``/home/liying/sqlcc/simple_procedure_test.cpp``
- `/home/liying/sqlcc/tests/unit/parser/sql_parser_high_coverage_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unit/parser/sql_parser_high_coverage_test.cpp``

### docs/项目进展/v1.2.3/当前覆盖率数据收集报告.md

- `tests/unit/simple_network_test.cpp` - 引用形式: ``tests/unit/simple_network_test.cpp``

### docs/项目进展/v1.2.3/高级SQL-92特性开发完成报告.md

- `include/sql_parser/advanced_sql92_features.h` - 引用形式: ``include/sql_parser/advanced_sql92_features.h``
- `tests/unit/advanced_sql92_test.cpp` - 引用形式: ``tests/unit/advanced_sql92_test.cpp``

### docs/项目进展/v1.2.4/测试改进TODO清单.md

- `include/storage/table_storage.h` - 引用形式: ``include/storage/table_storage.h``
- `tests/query_features_test.cpp` - 引用形式: ``tests/query_features_test.cpp``
- `tests/constraint_advanced_test.cpp` - 引用形式: ``tests/constraint_advanced_test.cpp``
- `tests/view_operations_test.cpp` - 引用形式: ``tests/view_operations_test.cpp``
- `tests/window_function_test.cpp` - 引用形式: ``tests/window_function_test.cpp``
- `tests/set_operation_test.cpp` - 引用形式: ``tests/set_operation_test.cpp``
- `tests/recursive_query_test.cpp` - 引用形式: ``tests/recursive_query_test.cpp``
- `tests/storage_engine/index_manager_test.cpp` - 引用形式: ``tests/storage_engine/index_manager_test.cpp``
- `tests/storage_engine/transaction_manager_test.cpp` - 引用形式: ``tests/storage_engine/transaction_manager_test.cpp``
- `tests/network/protocol_test.cpp` - 引用形式: ``tests/network/protocol_test.cpp``
- `tests/security/security_test.cpp` - 引用形式: ``tests/security/security_test.cpp``
- `tests/performance/benchmark_test.cpp` - 引用形式: ``tests/performance/benchmark_test.cpp``
- `tests/performance/stress_test.cpp` - 引用形式: ``tests/performance/stress_test.cpp``
- `tests/stability/long_running_test.cpp` - 引用形式: ``tests/stability/long_running_test.cpp``
- `tests/stability/error_handling_test.cpp` - 引用形式: ``tests/stability/error_handling_test.cpp``

### docs/项目进展/v1.2.4/测试改进变更记录.md

- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `include/sql_parser/ast_nodes.h` - 引用形式: ``include/sql_parser/ast_nodes.h``

### docs/项目进展/v1.2.4/测试变更记录.md

- `tests/unit/executor/join_executor_boundary_test.cpp` - 引用形式: ``tests/unit/executor/join_executor_boundary_test.cpp``
- `tests/unit/executor/set_operation_boundary_test.cpp` - 引用形式: ``tests/unit/executor/set_operation_boundary_test.cpp``
- `tests/unit/network/network_boundary_test.cpp` - 引用形式: ``tests/unit/network/network_boundary_test.cpp``

### docs/项目进展/v1.2.7/dependency_analysis_report.md

- `src/storage_engine/storage_engine.cpp` - 引用形式: ``src/storage_engine/storage_engine.cpp``
- `src/transaction/transaction_manager.cpp` - 引用形式: ``src/transaction/transaction_manager.cpp``

### docs/项目进展/v1.2.7/comprehensive_test_layer_analysis_final_report.md

- `tests/unit/core/config_manager_test.cpp` - 引用形式: ``tests/unit/core/config_manager_test.cpp``

### docs/项目进展/v1.1.1/v1.1.1SQL-92命令集成情况报告.md

- `/src/unified_executor.cpp` - 引用形式: ``/src/unified_executor.cpp``
- `/include/sql_executor.h` - 引用形式: ``/include/sql_executor.h``

### docs/项目进展/v1.1.1/v1.1.1测试驱动开发计划.md

- `/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/integration/sql_92_comprehensive_test.cpp``

### docs/项目进展/v1.1.1/v1.1.1_working_diary.md

- `/home/liying/sqlcc/tests/unit/basic/final_dcl_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unit/basic/final_dcl_test.cpp``
- `/home/liying/sqlcc/tests/unit/basic/comprehensive_dcl_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unit/basic/comprehensive_dcl_test.cpp``
- `/home/liying/sqlcc/tests/unit/basic/test_show_grants.cpp` - 引用形式: ``/home/liying/sqlcc/tests/unit/basic/test_show_grants.cpp``
- `tests/debug_insert.cpp` - 引用形式: ``tests/debug_insert.cpp``
- `tests/unit/database_manager_test_development.cpp` - 引用形式: ``tests/unit/database_manager_test_development.cpp``

### docs/项目进展/v1.1.0/v1.1.0_deep_analysis_report.md

- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `src/sql_executor/index_manager.cpp` - 引用形式: ``src/sql_executor/index_manager.cpp``
- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``

### docs/项目进展/v1.3.4/LLVM_覆盖率测试集成指南.md

- `tests/unit/core/config_manager_test.cpp` - 引用形式: ``tests/unit/core/config_manager_test.cpp``

### docs/项目进展/v1.1.2/v1.1.2_improvement_plan.md

- `src/sql_parser/parser_new.cpp` - 引用形式: ``src/sql_parser/parser_new.cpp``

### docs/项目进展/v1.1.4/内存安全修复进展报告_20251215_003250.md

- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `src/storage_engine/table_storage.cpp` - 引用形式: ``src/storage_engine/table_storage.cpp``

### docs/项目进展/v1.1.4/内存安全深度修复进展报告_20251215_004600.md

- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``

### docs/项目进展/v1.1.4/测试修复与全面评估完成报告.md

- `tests/advanced_sql/join/inner_join_test.cpp` - 引用形式: ``tests/advanced_sql/join/inner_join_test.cpp``

### docs/项目进展/v1.1.4/v1.1.4工作日记.md

- `/home/liying/sqlcc/tests/network/server_network_manager_real_test.cpp` - 引用形式: ``/home/liying/sqlcc/tests/network/server_network_manager_real_test.cpp``
- `tests/unit/executor/standalone_test.cpp` - 引用形式: ``tests/unit/executor/standalone_test.cpp``

### docs/项目进展/v1.1.4/内存安全改进计划_v1.1.4.md

- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `src/storage_engine/table_storage.cpp` - 引用形式: ``src/storage_engine/table_storage.cpp``

### docs/项目进展/v1.1.4/全面测试报告_20251215.md

- `tests/advanced_sql/join/inner_join_test.cpp` - 引用形式: ``tests/advanced_sql/join/inner_join_test.cpp``

### docs/项目进展/v1.2.5/sqlcc_refactoring_improvement_summary.md

- `include/exception.h` - 引用形式: ``include/exception.h``
- `include/storage/replace_strategy.h` - 引用形式: ``include/storage/replace_strategy.h``
- `include/sql_parser/constraint.h` - 引用形式: ``include/sql_parser/constraint.h``
- `include/sql_parser/function_ast.h` - 引用形式: ``include/sql_parser/function_ast.h``
- `include/execution/join_executor.h` - 引用形式: ``include/execution/join_executor.h``
- `include/execution/task_executor.h` - 引用形式: ``include/execution/task_executor.h``

### docs/项目进展/v1.2.5/class_file_separation_refactoring_progress.md

- `include/exception.h` - 引用形式: ``include/exception.h``
- `include/storage/replace_strategy.h` - 引用形式: ``include/storage/replace_strategy.h``
- `include/sql_parser/constraint.h` - 引用形式: ``include/sql_parser/constraint.h``
- `include/sql_parser/function_ast.h` - 引用形式: ``include/sql_parser/function_ast.h``

### docs/项目进展/v1.3.1/层次5_执行引擎测试分析报告.md

- `tests/execution/test_sql_executor.cpp` - 引用形式: ``tests/execution/test_sql_executor.cpp``
- `tests/test_executor.cpp` - 引用形式: ``tests/test_executor.cpp``
- `tests/execution/simple_execution_context_test.cpp` - 引用形式: ``tests/execution/simple_execution_context_test.cpp``
- `tests/unit/basic/execution_context_test.cpp` - 引用形式: ``tests/unit/basic/execution_context_test.cpp``
- `tests/unit/executor/query_processor_test.cpp` - 引用形式: ``tests/unit/executor/query_processor_test.cpp``
- `tests/execution/test_sql_executor.cpp` - 引用形式: ``tests/execution/test_sql_executor.cpp``
- `tests/execution/simple_execution_context_test.cpp` - 引用形式: ``tests/execution/simple_execution_context_test.cpp``
- `tests/test_executor.cpp` - 引用形式: ``tests/test_executor.cpp``
- `tests/unit/basic/execution_context_test.cpp` - 引用形式: ``tests/unit/basic/execution_context_test.cpp``

### docs/项目进展/v1.3.1/层次1_基础工具类测试分析报告.md

- `tests/unit/logger_test.cpp` - 引用形式: ``tests/unit/logger_test.cpp``
- `tests/unit/basic/logger_basic_test.cpp` - 引用形式: ``tests/unit/basic/logger_basic_test.cpp``
- `tests/unit/basic/config_manager_test.cpp` - 引用形式: ``tests/unit/basic/config_manager_test.cpp``
- `tests/unit/core/config_manager_test.cpp` - 引用形式: ``tests/unit/core/config_manager_test.cpp``
- `tests/test_smart_config_manager.cpp` - 引用形式: ``tests/test_smart_config_manager.cpp``
- `tests/unit/basic/config_manager_test.cpp` - 引用形式: ``tests/unit/basic/config_manager_test.cpp``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `tests/unit/basic/decimal_test.cpp` - 引用形式: ``tests/unit/basic/decimal_test.cpp``
- `tests/unit/basic/execution_context_test.cpp` - 引用形式: ``tests/unit/basic/execution_context_test.cpp``
- `tests/unit/logger_test.cpp` - 引用形式: ``tests/unit/logger_test.cpp``
- `tests/unit/basic/logger_basic_test.cpp` - 引用形式: ``tests/unit/basic/logger_basic_test.cpp``
- `tests/unit/core/config_manager_test.cpp` - 引用形式: ``tests/unit/core/config_manager_test.cpp``
- `tests/test_smart_config_manager.cpp` - 引用形式: ``tests/test_smart_config_manager.cpp``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `tests/unit/basic/decimal_test.cpp` - 引用形式: ``tests/unit/basic/decimal_test.cpp``
- `tests/unit/basic/execution_context_test.cpp` - 引用形式: ``tests/unit/basic/execution_context_test.cpp``
- `tests/unit/basic/config_manager_test.cpp` - 引用形式: ``tests/unit/basic/config_manager_test.cpp``

### docs/项目进展/v1.3.1/层次7_高层功能测试分析报告.md

- `tests/integration/comprehensive_test.cpp` - 引用形式: ``tests/integration/comprehensive_test.cpp``
- `tests/integration/sql_92_comprehensive_test.cpp` - 引用形式: ``tests/integration/sql_92_comprehensive_test.cpp``
- `tests/integration/client_server_integration_test.cpp` - 引用形式: ``tests/integration/client_server_integration_test.cpp``
- `tests/integration/session_manager_real_test.cpp` - 引用形式: ``tests/integration/session_manager_real_test.cpp``
- `tests/integration/procedure_trigger_integration_test.cpp` - 引用形式: ``tests/integration/procedure_trigger_integration_test.cpp``
- `tests/advanced_sql92_test_suite.cpp` - 引用形式: ``tests/advanced_sql92_test_suite.cpp``
- `tests/performance/million_insert_test.cc` - 引用形式: ``tests/performance/million_insert_test.cc``
- `tests/performance/mixed_workload_test.cc` - 引用形式: ``tests/performance/mixed_workload_test.cc``
- `tests/performance/index_constraint_benchmark.cc` - 引用形式: ``tests/performance/index_constraint_benchmark.cc``
- `tests/performance/large_scale_index_constraint_test.cc` - 引用形式: ``tests/performance/large_scale_index_constraint_test.cc``
- `tests/security/memory_safety_framework.cpp` - 引用形式: ``tests/security/memory_safety_framework.cpp``
- `tests/integration/encrypted_integration_test.cpp` - 引用形式: ``tests/integration/encrypted_integration_test.cpp``
- `tests/integration/aes_encryption_test.cc` - 引用形式: ``tests/integration/aes_encryption_test.cc``
- `tests/integration/tls_e2e_test.cc` - 引用形式: ``tests/integration/tls_e2e_test.cc``
- `tests/legacy/simple_db_test.cpp` - 引用形式: ``tests/legacy/simple_db_test.cpp``
- `tests/legacy/simple_dcl_ddl_test.cpp` - 引用形式: ``tests/legacy/simple_dcl_ddl_test.cpp``
- `tests/legacy/persistence_check.cpp` - 引用形式: ``tests/legacy/persistence_check.cpp``
- `tests/legacy/user_persistence_test.cpp` - 引用形式: ``tests/legacy/user_persistence_test.cpp``
- `tests/integration/comprehensive_test.cpp` - 引用形式: ``tests/integration/comprehensive_test.cpp``
- `tests/advanced_sql92_test_suite.cpp` - 引用形式: ``tests/advanced_sql92_test_suite.cpp``
- `tests/performance/million_insert_test.cc` - 引用形式: ``tests/performance/million_insert_test.cc``
- `tests/security/memory_safety_framework.cpp` - 引用形式: ``tests/security/memory_safety_framework.cpp``
- `tests/legacy/simple_db_test.cpp` - 引用形式: ``tests/legacy/simple_db_test.cpp``
- `tests/integration/sql_92_comprehensive_test.cpp` - 引用形式: ``tests/integration/sql_92_comprehensive_test.cpp``
- `tests/integration/client_server_integration_test.cpp` - 引用形式: ``tests/integration/client_server_integration_test.cpp``

### docs/项目进展/v1.3.1/测试程序分类分析.md

- `tests/unit/basic/logger_basic_test.cpp` - 引用形式: ``tests/unit/basic/logger_basic_test.cpp``
- `tests/unit/basic/config_manager_test.cpp` - 引用形式: ``tests/unit/basic/config_manager_test.cpp``
- `tests/unit/basic/data_types_test.cpp` - 引用形式: ``tests/unit/basic/data_types_test.cpp``
- `tests/unit/basic/decimal_test.cpp` - 引用形式: ``tests/unit/basic/decimal_test.cpp``
- `tests/unit/basic/execution_context_test.cpp` - 引用形式: ``tests/unit/basic/execution_context_test.cpp``
- `tests/test_smart_config_manager.cpp` - 引用形式: ``tests/test_smart_config_manager.cpp``
- `tests/unit/logger_test.cpp` - 引用形式: ``tests/unit/logger_test.cpp``
- `tests/storage_engine/buffer_pool_test.cpp` - 引用形式: ``tests/storage_engine/buffer_pool_test.cpp``
- `tests/storage_engine/disk_manager_test.cpp` - 引用形式: ``tests/storage_engine/disk_manager_test.cpp``
- `tests/storage_engine/page_allocator_test.cpp` - 引用形式: ``tests/storage_engine/page_allocator_test.cpp``
- `tests/storage_engine/storage_engine_boundary_test.cpp` - 引用形式: ``tests/storage_engine/storage_engine_boundary_test.cpp``
- `tests/storage_engine/storage_engine_comprehensive_test.cpp` - 引用形式: ``tests/storage_engine/storage_engine_comprehensive_test.cpp``
- `tests/unit/storage/buffer_pool_performance_test.h` - 引用形式: ``tests/unit/storage/buffer_pool_performance_test.h``
- `tests/unit/storage/disk_io_performance_test.h` - 引用形式: ``tests/unit/storage/disk_io_performance_test.h``
- `tests/storage_engine/b_plus_tree_core_test.cpp` - 引用形式: ``tests/storage_engine/b_plus_tree_core_test.cpp``
- `tests/storage_engine/comprehensive_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/comprehensive_bplus_tree_test.cpp``
- `tests/storage_engine/final_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/final_bplus_tree_test.cpp``
- `tests/storage_engine/index_insert_test.cpp` - 引用形式: ``tests/storage_engine/index_insert_test.cpp``
- `tests/storage_engine/index_manager_test.cpp` - 引用形式: ``tests/storage_engine/index_manager_test.cpp``
- `tests/unit/storage/index_constraint_benchmark.cc` - 引用形式: ``tests/unit/storage/index_constraint_benchmark.cc``
- `tests/unit/storage/index_performance_test.h` - 引用形式: ``tests/unit/storage/index_performance_test.h``
- `tests/sql/simple_token_test.cpp` - 引用形式: ``tests/sql/simple_token_test.cpp``
- `tests/sql/simple_constraint_test.cpp` - 引用形式: ``tests/sql/simple_constraint_test.cpp``
- `tests/sql/test_constraint_demo.cpp` - 引用形式: ``tests/sql/test_constraint_demo.cpp``
- `tests/sql/test_dcl_parsing.cpp` - 引用形式: ``tests/sql/test_dcl_parsing.cpp``
- `tests/sql/test_join_functionality.cpp` - 引用形式: ``tests/sql/test_join_functionality.cpp``
- `tests/execution/simple_execution_context_test.cpp` - 引用形式: ``tests/execution/simple_execution_context_test.cpp``
- `tests/execution/test_sql_executor.cpp` - 引用形式: ``tests/execution/test_sql_executor.cpp``
- `tests/test_executor.cpp` - 引用形式: ``tests/test_executor.cpp``
- `tests/network/simple_query_test.cpp` - 引用形式: ``tests/network/simple_query_test.cpp``
- `tests/network/client_test_standalone.cpp` - 引用形式: ``tests/network/client_test_standalone.cpp``
- `tests/network/mysql_client_test.cpp` - 引用形式: ``tests/network/mysql_client_test.cpp``
- `tests/network/mysql_server_test.cpp` - 引用形式: ``tests/network/mysql_server_test.cpp``
- `tests/network/concurrent_performance_test.cpp` - 引用形式: ``tests/network/concurrent_performance_test.cpp``
- `tests/network/crud_performance_test_main.cpp` - 引用形式: ``tests/network/crud_performance_test_main.cpp``
- `tests/integration/comprehensive_test.cpp` - 引用形式: ``tests/integration/comprehensive_test.cpp``
- `tests/integration/sql_92_comprehensive_test.cpp` - 引用形式: ``tests/integration/sql_92_comprehensive_test.cpp``

### docs/项目进展/v1.3.1/层次2_存储引擎基础测试分析报告.md

- `tests/storage_engine/buffer_pool_test.cpp` - 引用形式: ``tests/storage_engine/buffer_pool_test.cpp``
- `tests/storage_engine/buffer_pool_performance_benchmark_test.cpp` - 引用形式: ``tests/storage_engine/buffer_pool_performance_benchmark_test.cpp``
- `tests/storage_engine/buffer_pool_quick_test.cpp` - 引用形式: ``tests/storage_engine/buffer_pool_quick_test.cpp``
- `tests/storage_engine/buffer_pool_v3_test.cpp` - 引用形式: ``tests/storage_engine/buffer_pool_v3_test.cpp``
- `tests/unit/storage/buffer_pool_performance_test.h` - 引用形式: ``tests/unit/storage/buffer_pool_performance_test.h``
- `tests/storage_engine/disk_manager_test.cpp` - 引用形式: ``tests/storage_engine/disk_manager_test.cpp``
- `tests/unit/storage/disk_io_performance_test.h` - 引用形式: ``tests/unit/storage/disk_io_performance_test.h``
- `tests/storage_engine/page_allocator_test.cpp` - 引用形式: ``tests/storage_engine/page_allocator_test.cpp``
- `tests/storage_engine/storage_engine_comprehensive_test.cpp` - 引用形式: ``tests/storage_engine/storage_engine_comprehensive_test.cpp``
- `tests/storage_engine/storage_engine_boundary_test.cpp` - 引用形式: ``tests/storage_engine/storage_engine_boundary_test.cpp``
- `tests/storage_engine/buffer_pool_test.cpp` - 引用形式: ``tests/storage_engine/buffer_pool_test.cpp``
- `tests/storage_engine/disk_manager_test.cpp` - 引用形式: ``tests/storage_engine/disk_manager_test.cpp``
- `tests/storage_engine/page_allocator_test.cpp` - 引用形式: ``tests/storage_engine/page_allocator_test.cpp``
- `tests/storage_engine/storage_engine_comprehensive_test.cpp` - 引用形式: ``tests/storage_engine/storage_engine_comprehensive_test.cpp``
- `tests/storage_engine/storage_engine_boundary_test.cpp` - 引用形式: ``tests/storage_engine/storage_engine_boundary_test.cpp``
- `tests/storage_engine/buffer_pool_performance_benchmark_test.cpp` - 引用形式: ``tests/storage_engine/buffer_pool_performance_benchmark_test.cpp``
- `tests/storage_engine/buffer_pool_quick_test.cpp` - 引用形式: ``tests/storage_engine/buffer_pool_quick_test.cpp``
- `tests/storage_engine/buffer_pool_v3_test.cpp` - 引用形式: ``tests/storage_engine/buffer_pool_v3_test.cpp``

### docs/项目进展/v1.3.1/Phase3.1_变更记录.md

- `include/sql_parser/ast_nodes.h` - 引用形式: ``include/sql_parser/ast_nodes.h``
- `include/sql_parser/parser.h` - 引用形式: ``include/sql_parser/parser.h``
- `tests/unit/parser/parser_error_handling_test.cpp` - 引用形式: ``tests/unit/parser/parser_error_handling_test.cpp``

### docs/项目进展/v1.3.1/层次4_SQL解析器测试分析报告.md

- `tests/sql/simple_token_test.cpp` - 引用形式: ``tests/sql/simple_token_test.cpp``
- `tests/unit/parser/token_test.cpp` - 引用形式: ``tests/unit/parser/token_test.cpp``
- `tests/unit/parser/lexer_test.cpp` - 引用形式: ``tests/unit/parser/lexer_test.cpp``
- `tests/debug/debug_lexer_test.cpp` - 引用形式: ``tests/debug/debug_lexer_test.cpp``
- `tests/unit/parser/parser_test.cpp` - 引用形式: ``tests/unit/parser/parser_test.cpp``
- `tests/demo/parser_integration_test.cpp` - 引用形式: ``tests/demo/parser_integration_test.cpp``
- `tests/sql/simple_constraint_test.cpp` - 引用形式: ``tests/sql/simple_constraint_test.cpp``
- `tests/sql/test_constraint_demo.cpp` - 引用形式: ``tests/sql/test_constraint_demo.cpp``
- `tests/sql/test_dcl_parsing.cpp` - 引用形式: ``tests/sql/test_dcl_parsing.cpp``
- `tests/legacy/simple_dcl_ddl_test.cpp` - 引用形式: ``tests/legacy/simple_dcl_ddl_test.cpp``
- `tests/sql/test_join_functionality.cpp` - 引用形式: ``tests/sql/test_join_functionality.cpp``
- `tests/sql/simple_token_test.cpp` - 引用形式: ``tests/sql/simple_token_test.cpp``
- `tests/sql/simple_constraint_test.cpp` - 引用形式: ``tests/sql/simple_constraint_test.cpp``
- `tests/sql/test_constraint_demo.cpp` - 引用形式: ``tests/sql/test_constraint_demo.cpp``
- `tests/sql/test_dcl_parsing.cpp` - 引用形式: ``tests/sql/test_dcl_parsing.cpp``
- `tests/sql/test_join_functionality.cpp` - 引用形式: ``tests/sql/test_join_functionality.cpp``
- `tests/debug/debug_lexer_test.cpp` - 引用形式: ``tests/debug/debug_lexer_test.cpp``
- `tests/demo/parser_integration_test.cpp` - 引用形式: ``tests/demo/parser_integration_test.cpp``
- `tests/legacy/simple_dcl_ddl_test.cpp` - 引用形式: ``tests/legacy/simple_dcl_ddl_test.cpp``

### docs/项目进展/v1.3.1/层次6_网络通信测试分析报告.md

- `tests/network/client_test_standalone.cpp` - 引用形式: ``tests/network/client_test_standalone.cpp``
- `tests/network/mysql_client_test.cpp` - 引用形式: ``tests/network/mysql_client_test.cpp``
- `tests/integration/client_connection_real_test.cpp` - 引用形式: ``tests/integration/client_connection_real_test.cpp``
- `tests/integration/client_network_manager_real_test.cpp` - 引用形式: ``tests/integration/client_network_manager_real_test.cpp``
- `tests/network/mysql_server_test.cpp` - 引用形式: ``tests/network/mysql_server_test.cpp``
- `tests/integration/server_network_manager_real_test.cpp` - 引用形式: ``tests/integration/server_network_manager_real_test.cpp``
- `tests/integration/connection_handler_real_test.cpp` - 引用形式: ``tests/integration/connection_handler_real_test.cpp``
- `tests/network/test_mysql_protocol.cpp` - 引用形式: ``tests/network/test_mysql_protocol.cpp``
- `tests/network/simple_query_test.cpp` - 引用形式: ``tests/network/simple_query_test.cpp``
- `tests/integration/tls_e2e_test.cc` - 引用形式: ``tests/integration/tls_e2e_test.cc``
- `tests/integration/aes_encryption_test.cc` - 引用形式: ``tests/integration/aes_encryption_test.cc``
- `tests/network/concurrent_performance_test.cpp` - 引用形式: ``tests/network/concurrent_performance_test.cpp``
- `tests/network/crud_performance_test_main.cpp` - 引用形式: ``tests/network/crud_performance_test_main.cpp``
- `tests/network/crud_performance_test.cpp` - 引用形式: ``tests/network/crud_performance_test.cpp``
- `tests/network/real_concurrent_test.cpp` - 引用形式: ``tests/network/real_concurrent_test.cpp``
- `tests/network/scalable_performance_test.cpp` - 引用形式: ``tests/network/scalable_performance_test.cpp``
- `tests/network/auth_test_client.cpp` - 引用形式: ``tests/network/auth_test_client.cpp``
- `tests/network/auth_test_server.cpp` - 引用形式: ``tests/network/auth_test_server.cpp``
- `tests/network/simple_auth_test.cpp` - 引用形式: ``tests/network/simple_auth_test.cpp``
- `tests/network/standalone_auth_test.cpp` - 引用形式: ``tests/network/standalone_auth_test.cpp``
- `tests/network/simple_query_test.cpp` - 引用形式: ``tests/network/simple_query_test.cpp``
- `tests/network/client_test_standalone.cpp` - 引用形式: ``tests/network/client_test_standalone.cpp``
- `tests/network/mysql_client_test.cpp` - 引用形式: ``tests/network/mysql_client_test.cpp``
- `tests/network/mysql_server_test.cpp` - 引用形式: ``tests/network/mysql_server_test.cpp``
- `tests/network/concurrent_performance_test.cpp` - 引用形式: ``tests/network/concurrent_performance_test.cpp``
- `tests/network/crud_performance_test_main.cpp` - 引用形式: ``tests/network/crud_performance_test_main.cpp``
- `tests/network/auth_test_client.cpp` - 引用形式: ``tests/network/auth_test_client.cpp``
- `tests/network/auth_test_server.cpp` - 引用形式: ``tests/network/auth_test_server.cpp``
- `tests/network/simple_auth_test.cpp` - 引用形式: ``tests/network/simple_auth_test.cpp``
- `tests/integration/client_connection_real_test.cpp` - 引用形式: ``tests/integration/client_connection_real_test.cpp``
- `tests/integration/server_network_manager_real_test.cpp` - 引用形式: ``tests/integration/server_network_manager_real_test.cpp``
- `tests/integration/connection_handler_real_test.cpp` - 引用形式: ``tests/integration/connection_handler_real_test.cpp``

### docs/项目进展/v1.3.1/层次3_索引系统测试分析报告.md

- `tests/storage_engine/b_plus_tree_core_test.cpp` - 引用形式: ``tests/storage_engine/b_plus_tree_core_test.cpp``
- `tests/storage_engine/comprehensive_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/comprehensive_bplus_tree_test.cpp``
- `tests/storage_engine/final_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/final_bplus_tree_test.cpp``
- `tests/storage_engine/minimal_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/minimal_bplus_tree_test.cpp``
- `tests/storage_engine/simple_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/simple_bplus_tree_test.cpp``
- `tests/storage_engine/test_bplus_tree_fix.cpp` - 引用形式: ``tests/storage_engine/test_bplus_tree_fix.cpp``
- `tests/storage_engine/debug_bplus_tree.cpp` - 引用形式: ``tests/storage_engine/debug_bplus_tree.cpp``
- `tests/storage_engine/index_manager_test.cpp` - 引用形式: ``tests/storage_engine/index_manager_test.cpp``
- `tests/storage_engine/index_insert_test.cpp` - 引用形式: ``tests/storage_engine/index_insert_test.cpp``
- `tests/performance/index_constraint_benchmark.cc` - 引用形式: ``tests/performance/index_constraint_benchmark.cc``
- `tests/performance/large_scale_index_constraint_test.cc` - 引用形式: ``tests/performance/large_scale_index_constraint_test.cc``
- `tests/unit/storage/index_constraint_benchmark.cc` - 引用形式: ``tests/unit/storage/index_constraint_benchmark.cc``
- `tests/unit/storage/index_performance_test.h` - 引用形式: ``tests/unit/storage/index_performance_test.h``
- `tests/storage_engine/b_plus_tree_core_test.cpp` - 引用形式: ``tests/storage_engine/b_plus_tree_core_test.cpp``
- `tests/storage_engine/comprehensive_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/comprehensive_bplus_tree_test.cpp``
- `tests/storage_engine/final_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/final_bplus_tree_test.cpp``
- `tests/storage_engine/minimal_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/minimal_bplus_tree_test.cpp``
- `tests/storage_engine/simple_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/simple_bplus_tree_test.cpp``
- `tests/storage_engine/test_bplus_tree_fix.cpp` - 引用形式: ``tests/storage_engine/test_bplus_tree_fix.cpp``
- `tests/storage_engine/index_manager_test.cpp` - 引用形式: ``tests/storage_engine/index_manager_test.cpp``
- `tests/storage_engine/index_insert_test.cpp` - 引用形式: ``tests/storage_engine/index_insert_test.cpp``
- `tests/performance/index_constraint_benchmark.cc` - 引用形式: ``tests/performance/index_constraint_benchmark.cc``
- `tests/performance/large_scale_index_constraint_test.cc` - 引用形式: ``tests/performance/large_scale_index_constraint_test.cc``

### docs/项目进展/v1.3.8/v1.3.8_历史TODO.md

- `src/sql_executor/ddl_executor.cpp` - 引用形式: ``src/sql_executor/ddl_executor.cpp``
- `src/execution/ddl_executor.cpp` - 引用形式: ``src/execution/ddl_executor.cpp``

### docs/项目进展/v1.3.8/v1.3.8_变更记录报告.md

- `include/storage/buffer_pool_sharded.h` - 引用形式: ``include/storage/buffer_pool_sharded.h``
- `src/storage_engine/disk_manager/disk_manager.h` - 引用形式: ``src/storage_engine/disk_manager/disk_manager.h``

### docs/项目进展/v1.3.8/v1.3.8工作日记.md

- `sql_parser/ast_node.h` - 引用形式: ``sql_parser/ast_node.h``

### docs/项目进展/v1.3.1/v1.2.9/错误处理覆盖分析报告.md

- `index_manager/index_manager.cpp` - 引用形式: ``index_manager/index_manager.cpp``
- `index_manager/enhanced_index_manager.cpp` - 引用形式: ``index_manager/enhanced_index_manager.cpp``
- `index_manager/transactional_index_manager.cpp` - 引用形式: ``index_manager/transactional_index_manager.cpp``

### docs/项目进展/v1.3.1/v1.2.9/B+树索引实现算法分析报告.md

- `src/storage_engine/b_plus_tree_index.cpp` - 引用形式: ``src/storage_engine/b_plus_tree_index.cpp``

### docs/项目进展/v1.3.1/v1.2.9/下一步行动执行指南_20251225.md

- `tests/unit/logger_test.cpp` - 引用形式: ``tests/unit/logger_test.cpp``
- `tests/unit/logger_test.cpp` - 引用形式: ``tests/unit/logger_test.cpp``

### docs/项目进展/v1.3.1/v1.2.9/SQLCC测试覆盖率阶段1改进完成报告.md

- `tests/storage_engine/storage_engine_comprehensive_test.cpp` - 引用形式: ``tests/storage_engine/storage_engine_comprehensive_test.cpp``
- `tests/storage_engine/storage_engine_comprehensive_test.cpp` - 引用形式: ``tests/storage_engine/storage_engine_comprehensive_test.cpp``

### docs/项目进展/v1.3.1/v1.2.9/BUILD_配置修复报告_20251225.md

- `include/utils/logger.h` - 引用形式: ``include/utils/logger.h``

### docs/项目进展/v1.3.1/v1.2.9/demo测试修复与覆盖率收集报告.md

- `include/user_manager.h` - 引用形式: ``include/user_manager.h``
- `src/dcl_test.cpp` - 引用形式: ``src/dcl_test.cpp``
- `src/ddl_test.cpp` - 引用形式: ``src/ddl_test.cpp``

### docs/项目进展/v1.3.1/v1.2.9/SQLCC测试覆盖率改进项目完成报告_20251225.md

- `tests/unit/network/network_boundary_test.cpp` - 引用形式: ``tests/unit/network/network_boundary_test.cpp``
- `tests/unit/executor/set_operation_boundary_test.cpp` - 引用形式: ``tests/unit/executor/set_operation_boundary_test.cpp``
- `tests/unit/executor/window_function_boundary_test.cpp` - 引用形式: ``tests/unit/executor/window_function_boundary_test.cpp``
- `tests/unit/executor/join_executor_boundary_test.cpp` - 引用形式: ``tests/unit/executor/join_executor_boundary_test.cpp``
- `tests/unit/executor/load_data_boundary_test.cpp` - 引用形式: ``tests/unit/executor/load_data_boundary_test.cpp``

### docs/项目进展/v1.3.1/v1.2.9/编译错误修复与测试执行报告.md

- `include/sql_parser/set_operation.h` - 引用形式: ``include/sql_parser/set_operation.h``

### docs/项目进展/v1.3.1/v1.2.9/Logger测试覆盖率手动评估报告.md

- `include/utils/logger.h` - 引用形式: ``include/utils/logger.h``

### docs/项目进展/v1.3.1/v1.2.9/最新覆盖率测试报告_20251226.md

- `src/storage_engine/b_plus_tree.cpp` - 引用形式: ``src/storage_engine/b_plus_tree.cpp``
- `src/storage_engine/b_plus_tree_index.cpp` - 引用形式: ``src/storage_engine/b_plus_tree_index.cpp``
- `src/storage_engine/storage_engine.cpp` - 引用形式: ``src/storage_engine/storage_engine.cpp``
- `tests/storage_engine/storage_engine_comprehensive_test.cpp` - 引用形式: ``tests/storage_engine/storage_engine_comprehensive_test.cpp``
- `tests/storage_engine/comprehensive_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/comprehensive_bplus_tree_test.cpp``
- `tests/storage_engine/simple_bplus_tree_test.cpp` - 引用形式: ``tests/storage_engine/simple_bplus_tree_test.cpp``

### docs/项目进展/v1.3.1/v1.2.9/SQLCC测试覆盖率改进项目完成报告.md

- `tests/unit/logger_test.cpp` - 引用形式: ``tests/unit/logger_test.cpp``


## 修复建议

1. 删除或更正这些无效的文件引用
2. 使用正确的路径替换这些引用
3. 如果引用是示例或占位符，考虑使用删除线标记

## 有效引用统计

发现 1610 个有效引用，这些不需要修复。
