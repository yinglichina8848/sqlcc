# SQLCC Include路径修复报告
修复时间: 2025-12-22 03:34:27
修复文件数量: 10
修复条目数量: 145

## 修复详情
### 修复 1
- **文件**: /home/liying/sqlcc/BUILD.bazel
- **旧路径**: //src/sqlcc_server:server_main.cpp
- **新路径**: "src/sqlcc_server/server_main.cpp"
- **原因**: Header file location changed during refactoring

### 修复 2
- **文件**: /home/liying/sqlcc/BUILD.bazel
- **旧路径**: //src/isql_network:client_main.cpp
- **新路径**: "src/isql_network/client_main.cpp"
- **原因**: Header file location changed during refactoring

### 修复 3
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/logger:logger.cpp
- **新路径**: "logger/logger.cpp"
- **原因**: Header file location changed during refactoring

### 修复 4
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/logger:logger_module_impl.cpp
- **新路径**: "logger/logger_module_impl.cpp"
- **原因**: Header file location changed during refactoring

### 修复 5
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/utils:logger.h
- **新路径**: "../include/utils/logger.h"
- **原因**: Header file location changed during refactoring

### 修复 6
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/utils:config_snapshot.cpp
- **新路径**: "utils/config_snapshot.cpp"
- **原因**: Header file location changed during refactoring

### 修复 7
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/utils:config_lifecycle.cpp
- **新路径**: "utils/config_lifecycle.cpp"
- **原因**: Header file location changed during refactoring

### 修复 8
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/utils:smart_config_manager.cpp
- **新路径**: "utils/smart_config_manager.cpp"
- **原因**: Header file location changed during refactoring

### 修复 9
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/utils:config_snapshot.h
- **新路径**: "../include/utils/config_snapshot.h"
- **原因**: Header file location changed during refactoring

### 修复 10
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/utils:config_lifecycle.h
- **新路径**: "../include/utils/config_lifecycle.h"
- **原因**: Header file location changed during refactoring

### 修复 11
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/utils:smart_config_manager.h
- **新路径**: "../include/utils/smart_config_manager.h"
- **原因**: Header file location changed during refactoring

### 修复 12
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/utils:ssl_wrapper.h
- **新路径**: "../include/utils/ssl_wrapper.h"
- **原因**: Header file location changed during refactoring

### 修复 13
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/core:user_manager.cpp
- **新路径**: "core/user_manager.cpp"
- **原因**: Header file location changed during refactoring

### 修复 14
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/core:user_manager.cpp
- **新路径**: "core/user_manager.cpp"
- **原因**: Header file location changed during refactoring

### 修复 15
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/core:user_manager.h
- **新路径**: "../include/core/user_manager.h"
- **原因**: Header file location changed during refactoring

### 修复 16
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/core:user_manager.h
- **新路径**: "../include/core/user_manager.h"
- **原因**: Header file location changed during refactoring

### 修复 17
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/core:execution_context.h
- **新路径**: "../include/core/execution_context.h"
- **原因**: Header file location changed during refactoring

### 修复 18
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/core:unified_executor.h
- **新路径**: "../include/core/unified_executor.h"
- **原因**: Header file location changed during refactoring

### 修复 19
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:storage_engine.cpp
- **新路径**: "storage_engine/storage_engine.cpp"
- **原因**: Header file location changed during refactoring

### 修复 20
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:b_plus_tree.cpp
- **新路径**: "storage_engine/b_plus_tree.cpp"
- **原因**: Header file location changed during refactoring

### 修复 21
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:buffer_pool.cpp
- **新路径**: "storage_engine/buffer_pool.cpp"
- **原因**: Header file location changed during refactoring

### 修复 22
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:wal_buffer.cpp
- **新路径**: "storage_engine/wal_buffer.cpp"
- **原因**: Header file location changed during refactoring

### 修复 23
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:wal_writer.cpp
- **新路径**: "storage_engine/wal_writer.cpp"
- **原因**: Header file location changed during refactoring

### 修复 24
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:checkpoint.cpp
- **新路径**: "storage_engine/checkpoint.cpp"
- **原因**: Header file location changed during refactoring

### 修复 25
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:page_allocator.cpp
- **新路径**: "storage_engine/page_allocator.cpp"
- **原因**: Header file location changed during refactoring

### 修复 26
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:concurrent_access_validator.cpp
- **新路径**: "storage_engine/concurrent_access_validator.cpp"
- **原因**: Header file location changed during refactoring

### 修复 27
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:disk_error_handler.cpp
- **新路径**: "storage_engine/disk_error_handler.cpp"
- **原因**: Header file location changed during refactoring

### 修复 28
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:partition_manager.cpp
- **新路径**: "storage_engine/partition_manager.cpp"
- **原因**: Header file location changed during refactoring

### 修复 29
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:record_boundary_validator.cpp
- **新路径**: "storage_engine/record_boundary_validator.cpp"
- **原因**: Header file location changed during refactoring

### 修复 30
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:replace_strategy.cpp
- **新路径**: "storage_engine/replace_strategy.cpp"
- **原因**: Header file location changed during refactoring

### 修复 31
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:cache_consistency_manager.cpp
- **新路径**: "storage_engine/cache_consistency_manager.cpp"
- **原因**: Header file location changed during refactoring

### 修复 32
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:advanced_lock_manager.cpp
- **新路径**: "storage_engine/advanced_lock_manager.cpp"
- **原因**: Header file location changed during refactoring

### 修复 33
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:data_integrity_validator.cpp
- **新路径**: "storage_engine/data_integrity_validator.cpp"
- **原因**: Header file location changed during refactoring

### 修复 34
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:buffer_pool_sharded.cpp
- **新路径**: "storage_engine/buffer_pool_sharded.cpp"
- **原因**: Header file location changed during refactoring

### 修复 35
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:concurrency_control.cpp
- **新路径**: "storage_engine/concurrency_control.cpp"
- **原因**: Header file location changed during refactoring

### 修复 36
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:table_storage_complete.cpp
- **新路径**: "storage_engine/table_storage_complete.cpp"
- **原因**: Header file location changed during refactoring

### 修复 37
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include:storage_engine
- **新路径**: "../include/storage_engine"
- **原因**: Header file location changed during refactoring

### 修复 38
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:b_plus_tree.h
- **新路径**: "../include/storage/b_plus_tree.h"
- **原因**: Header file location changed during refactoring

### 修复 39
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:buffer_pool.h
- **新路径**: "../include/storage/buffer_pool.h"
- **原因**: Header file location changed during refactoring

### 修复 40
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:wal_buffer.h
- **新路径**: "../include/storage/wal_buffer.h"
- **原因**: Header file location changed during refactoring

### 修复 41
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:wal_writer.h
- **新路径**: "../include/storage/wal_writer.h"
- **原因**: Header file location changed during refactoring

### 修复 42
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:checkpoint.h
- **新路径**: "../include/storage/checkpoint.h"
- **原因**: Header file location changed during refactoring

### 修复 43
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:page_allocator.h
- **新路径**: "../include/storage/page_allocator.h"
- **原因**: Header file location changed during refactoring

### 修复 44
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:concurrent_access_validator.h
- **新路径**: "../include/storage/concurrent_access_validator.h"
- **原因**: Header file location changed during refactoring

### 修复 45
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:disk_error_handler.h
- **新路径**: "../include/storage/disk_error_handler.h"
- **原因**: Header file location changed during refactoring

### 修复 46
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:partition_manager.h
- **新路径**: "../include/storage/partition_manager.h"
- **原因**: Header file location changed during refactoring

### 修复 47
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:record_boundary_validator.h
- **新路径**: "../include/storage/record_boundary_validator.h"
- **原因**: Header file location changed during refactoring

### 修复 48
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:replace_strategy.h
- **新路径**: "../include/storage/replace_strategy.h"
- **原因**: Header file location changed during refactoring

### 修复 49
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:cache_consistency_manager.h
- **新路径**: "../include/storage/cache_consistency_manager.h"
- **原因**: Header file location changed during refactoring

### 修复 50
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:advanced_lock_manager.h
- **新路径**: "../include/storage/advanced_lock_manager.h"
- **原因**: Header file location changed during refactoring

### 修复 51
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:data_integrity_validator.h
- **新路径**: "../include/storage/data_integrity_validator.h"
- **原因**: Header file location changed during refactoring

### 修复 52
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:buffer_pool_sharded.h
- **新路径**: "../include/storage/buffer_pool_sharded.h"
- **原因**: Header file location changed during refactoring

### 修复 53
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:concurrency_control.h
- **新路径**: "../include/storage/concurrency_control.h"
- **原因**: Header file location changed during refactoring

### 修复 54
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:b_plus_tree_nodes.h
- **新路径**: "../include/storage/b_plus_tree_nodes.h"
- **原因**: Header file location changed during refactoring

### 修复 55
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:buffer_pool_fixed.h
- **新路径**: "../include/storage/buffer_pool_fixed.h"
- **原因**: Header file location changed during refactoring

### 修复 56
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:buffer_pool_v2.h
- **新路径**: "../include/storage/buffer_pool_v2.h"
- **原因**: Header file location changed during refactoring

### 修复 57
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:disk_manager.h
- **新路径**: "../include/storage/disk_manager.h"
- **原因**: Header file location changed during refactoring

### 修复 58
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:index_manager/index_manager.cpp
- **新路径**: "storage_engine/index_manager/index_manager.cpp"
- **原因**: Header file location changed during refactoring

### 修复 59
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/storage_engine:index_manager/index_manager_smart_ptr_enhancement.cpp
- **新路径**: "storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp"
- **原因**: Header file location changed during refactoring

### 修复 60
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/storage:index_manager.h
- **新路径**: "../include/storage/index_manager.h"
- **原因**: Header file location changed during refactoring

### 修复 61
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/sql_parser:parser.cpp
- **新路径**: "sql_parser/parser.cpp"
- **原因**: Header file location changed during refactoring

### 修复 62
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/sql_parser:lexer.cpp
- **新路径**: "sql_parser/lexer.cpp"
- **原因**: Header file location changed during refactoring

### 修复 63
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/sql_parser:token.cpp
- **新路径**: "sql_parser/token.cpp"
- **原因**: Header file location changed during refactoring

### 修复 64
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/sql_parser:ast_node.cpp
- **新路径**: "sql_parser/ast_node.cpp"
- **原因**: Header file location changed during refactoring

### 修复 65
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/sql_parser:ast_nodes.cpp
- **新路径**: "sql_parser/ast_nodes.cpp"
- **原因**: Header file location changed during refactoring

### 修复 66
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/sql_parser:constraint.cpp
- **新路径**: "sql_parser/constraint.cpp"
- **原因**: Header file location changed during refactoring

### 修复 67
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/sql_parser:set_operation.cpp
- **新路径**: "sql_parser/set_operation.cpp"
- **原因**: Header file location changed during refactoring

### 修复 68
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/sql_parser:window_function.cpp
- **新路径**: "sql_parser/window_function.cpp"
- **原因**: Header file location changed during refactoring

### 修复 69
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/sql_parser:recursive_query.cpp
- **新路径**: "sql_parser/recursive_query.cpp"
- **原因**: Header file location changed during refactoring

### 修复 70
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/sql_parser:decimal.cpp
- **新路径**: "sql_parser/decimal.cpp"
- **原因**: Header file location changed during refactoring

### 修复 71
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/sql_parser:json.cpp
- **新路径**: "sql_parser/json.cpp"
- **原因**: Header file location changed during refactoring

### 修复 72
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/sql_parser:datetime.cpp
- **新路径**: "sql_parser/datetime.cpp"
- **原因**: Header file location changed during refactoring

### 修复 73
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/sql_parser:function_ast.cpp
- **新路径**: "sql_parser/function_ast.cpp"
- **原因**: Header file location changed during refactoring

### 修复 74
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/sql_parser:parser.h
- **新路径**: "../include/sql_parser/parser.h"
- **原因**: Header file location changed during refactoring

### 修复 75
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/sql_parser:lexer.h
- **新路径**: "../include/sql_parser/lexer.h"
- **原因**: Header file location changed during refactoring

### 修复 76
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/sql_parser:token.h
- **新路径**: "../include/sql_parser/token.h"
- **原因**: Header file location changed during refactoring

### 修复 77
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/sql_parser:ast_node.h
- **新路径**: "../include/sql_parser/ast_node.h"
- **原因**: Header file location changed during refactoring

### 修复 78
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/sql_parser:ast_nodes.h
- **新路径**: "../include/sql_parser/ast_nodes.h"
- **原因**: Header file location changed during refactoring

### 修复 79
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/sql_parser:constraint.h
- **新路径**: "../include/sql_parser/constraint.h"
- **原因**: Header file location changed during refactoring

### 修复 80
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/sql_parser:set_operation.h
- **新路径**: "../include/sql_parser/set_operation.h"
- **原因**: Header file location changed during refactoring

### 修复 81
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/sql_parser:window_function.h
- **新路径**: "../include/sql_parser/window_function.h"
- **原因**: Header file location changed during refactoring

### 修复 82
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/sql_parser:recursive_query.h
- **新路径**: "../include/sql_parser/recursive_query.h"
- **原因**: Header file location changed during refactoring

### 修复 83
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/sql_parser:decimal.h
- **新路径**: "../include/sql_parser/decimal.h"
- **原因**: Header file location changed during refactoring

### 修复 84
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/sql_parser:json.h
- **新路径**: "../include/sql_parser/json.h"
- **原因**: Header file location changed during refactoring

### 修复 85
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/sql_parser:datetime.h
- **新路径**: "../include/sql_parser/datetime.h"
- **原因**: Header file location changed during refactoring

### 修复 86
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/sql_parser:function_ast.h
- **新路径**: "../include/sql_parser/function_ast.h"
- **原因**: Header file location changed during refactoring

### 修复 87
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/sql_parser:node_visitor.h
- **新路径**: "../include/sql_parser/node_visitor.h"
- **原因**: Header file location changed during refactoring

### 修复 88
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/sql_parser:load_data_ast.h
- **新路径**: "../include/sql_parser/load_data_ast.h"
- **原因**: Header file location changed during refactoring

### 修复 89
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/execution:function_executor.cpp
- **新路径**: "execution/function_executor.cpp"
- **原因**: Header file location changed during refactoring

### 修复 90
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/execution:set_operation_executor.cpp
- **新路径**: "execution/set_operation_executor.cpp"
- **原因**: Header file location changed during refactoring

### 修复 91
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/execution:window_function_executor.cpp
- **新路径**: "execution/window_function_executor.cpp"
- **原因**: Header file location changed during refactoring

### 修复 92
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/execution:recursive_query_executor.cpp
- **新路径**: "execution/recursive_query_executor.cpp"
- **原因**: Header file location changed during refactoring

### 修复 93
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/execution:load_data_executor.cpp
- **新路径**: "execution/load_data_executor.cpp"
- **原因**: Header file location changed during refactoring

### 修复 94
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/execution:function_executor.h
- **新路径**: "../include/execution/function_executor.h"
- **原因**: Header file location changed during refactoring

### 修复 95
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/execution:set_operation_executor.h
- **新路径**: "../include/execution/set_operation_executor.h"
- **原因**: Header file location changed during refactoring

### 修复 96
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/execution:window_function_executor.h
- **新路径**: "../include/execution/window_function_executor.h"
- **原因**: Header file location changed during refactoring

### 修复 97
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/execution:recursive_query_executor.h
- **新路径**: "../include/execution/recursive_query_executor.h"
- **原因**: Header file location changed during refactoring

### 修复 98
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/execution:load_data_executor.h
- **新路径**: "../include/execution/load_data_executor.h"
- **原因**: Header file location changed during refactoring

### 修复 99
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/execution:join_executor.h
- **新路径**: "../include/execution/join_executor.h"
- **原因**: Header file location changed during refactoring

### 修复 100
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/execution:procedure_trigger_task.h
- **新路径**: "../include/execution/procedure_trigger_task.h"
- **原因**: Header file location changed during refactoring

### 修复 101
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/sql_executor:sql_executor.cpp
- **新路径**: "sql_executor/sql_executor.cpp"
- **原因**: Header file location changed during refactoring

### 修复 102
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/network:network.cpp
- **新路径**: "network/network.cpp"
- **原因**: Header file location changed during refactoring

### 修复 103
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/network:connection_state_machine.cpp
- **新路径**: "network/connection_state_machine.cpp"
- **原因**: Header file location changed during refactoring

### 修复 104
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/network:encryption.cpp
- **新路径**: "network/encryption.cpp"
- **原因**: Header file location changed during refactoring

### 修复 105
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/network:data_transmission_validator.cpp
- **新路径**: "network/data_transmission_validator.cpp"
- **原因**: Header file location changed during refactoring

### 修复 106
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/network:mysql_protocol.cpp
- **新路径**: "network/mysql_protocol.cpp"
- **原因**: Header file location changed during refactoring

### 修复 107
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/network:network.h
- **新路径**: "../include/network/network.h"
- **原因**: Header file location changed during refactoring

### 修复 108
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/network:connection_state.h
- **新路径**: "../include/network/connection_state.h"
- **原因**: Header file location changed during refactoring

### 修复 109
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/network:connection_state_machine.h
- **新路径**: "../include/network/connection_state_machine.h"
- **原因**: Header file location changed during refactoring

### 修复 110
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/network:encryption.h
- **新路径**: "../include/network/encryption.h"
- **原因**: Header file location changed during refactoring

### 修复 111
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/network:data_transmission_validator.h
- **新路径**: "../include/network/data_transmission_validator.h"
- **原因**: Header file location changed during refactoring

### 修复 112
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/network:mysql_protocol.h
- **新路径**: "../include/network/mysql_protocol.h"
- **原因**: Header file location changed during refactoring

### 修复 113
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/config_manager:config_manager.cpp
- **新路径**: "config_manager/config_manager.cpp"
- **原因**: Header file location changed during refactoring

### 修复 114
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/procedure:procedure_parser.cpp
- **新路径**: "procedure/procedure_parser.cpp"
- **原因**: Header file location changed during refactoring

### 修复 115
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/procedure:procedure_vm.cpp
- **新路径**: "procedure/procedure_vm.cpp"
- **原因**: Header file location changed during refactoring

### 修复 116
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/procedure:procedure_trigger_executor.cpp
- **新路径**: "procedure/procedure_trigger_executor.cpp"
- **原因**: Header file location changed during refactoring

### 修复 117
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/procedure:procedure_parser.h
- **新路径**: "../include/procedure/procedure_parser.h"
- **原因**: Header file location changed during refactoring

### 修复 118
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/procedure:procedure_vm.h
- **新路径**: "../include/procedure/procedure_vm.h"
- **原因**: Header file location changed during refactoring

### 修复 119
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/procedure:procedure_trigger_executor.h
- **新路径**: "../include/procedure/procedure_trigger_executor.h"
- **原因**: Header file location changed during refactoring

### 修复 120
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/trigger:trigger_manager.cpp
- **新路径**: "trigger/trigger_manager.cpp"
- **原因**: Header file location changed during refactoring

### 修复 121
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/trigger:trigger_manager.h
- **新路径**: "../include/trigger/trigger_manager.h"
- **原因**: Header file location changed during refactoring

### 修复 122
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/transaction:savepoint_manager.cpp
- **新路径**: "transaction/savepoint_manager.cpp"
- **原因**: Header file location changed during refactoring

### 修复 123
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/transaction:savepoint_manager.h
- **新路径**: "../include/transaction/savepoint_manager.h"
- **原因**: Header file location changed during refactoring

### 修复 124
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/types:domain_manager.cpp
- **新路径**: "types/domain_manager.cpp"
- **原因**: Header file location changed during refactoring

### 修复 125
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/types:domain_manager.h
- **新路径**: "../include/types/domain_manager.h"
- **原因**: Header file location changed during refactoring

### 修复 126
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //src/security:memory_monitor.cpp
- **新路径**: "security/memory_monitor.cpp"
- **原因**: Header file location changed during refactoring

### 修复 127
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **旧路径**: //include/security:memory_monitor.h
- **新路径**: "../include/security/memory_monitor.h"
- **原因**: Header file location changed during refactoring

### 修复 128
- **文件**: /home/liying/sqlcc/src/storage_engine/buffer_pool/BUILD.bazel
- **旧路径**: //include/storage_engine/buffer_pool:lru_manager.h
- **新路径**: "../../../include/storage_engine/buffer_pool/lru_manager.h"
- **原因**: Header file location changed during refactoring

### 修复 129
- **文件**: /home/liying/sqlcc/src/storage_engine/buffer_pool/BUILD.bazel
- **旧路径**: //include/storage_engine/buffer_pool:statistics_collector.h
- **新路径**: "../../../include/storage_engine/buffer_pool/statistics_collector.h"
- **原因**: Header file location changed during refactoring

### 修复 130
- **文件**: /home/liying/sqlcc/src/network/BUILD.bazel
- **旧路径**: //include/network:session.h
- **新路径**: "../../include/network/session.h"
- **原因**: Header file location changed during refactoring

### 修复 131
- **文件**: /home/liying/sqlcc/src/network/BUILD.bazel
- **旧路径**: //include/network:session_manager.h
- **新路径**: "../../include/network/session_manager.h"
- **原因**: Header file location changed during refactoring

### 修复 132
- **文件**: /home/liying/sqlcc/src/logger/BUILD.bazel
- **旧路径**: //include/utils:logger.h
- **新路径**: "../../include/utils/logger.h"
- **原因**: Header file location changed during refactoring

### 修复 133
- **文件**: /home/liying/sqlcc/src/core/BUILD.bazel
- **旧路径**: //include/core:user_manager.h
- **新路径**: "../../include/core/user_manager.h"
- **原因**: Header file location changed during refactoring

### 修复 134
- **文件**: /home/liying/sqlcc/src/core/BUILD.bazel
- **旧路径**: //include/core:permission_validator.h
- **新路径**: "../../include/core/permission_validator.h"
- **原因**: Header file location changed during refactoring

### 修复 135
- **文件**: /home/liying/sqlcc/src/core/BUILD.bazel
- **旧路径**: //include/core:sql_executor_interface.h
- **新路径**: "../../include/core/sql_executor_interface.h"
- **原因**: Header file location changed during refactoring

### 修复 136
- **文件**: /home/liying/sqlcc/src/utils/BUILD.bazel
- **旧路径**: //include/utils:config_manager.h
- **新路径**: "../../include/utils/config_manager.h"
- **原因**: Header file location changed during refactoring

### 修复 137
- **文件**: /home/liying/sqlcc/src/utils/BUILD.bazel
- **旧路径**: //include/utils:config_snapshot.h
- **新路径**: "../../include/utils/config_snapshot.h"
- **原因**: Header file location changed during refactoring

### 修复 138
- **文件**: /home/liying/sqlcc/src/utils/BUILD.bazel
- **旧路径**: //include/utils:config_lifecycle.h
- **新路径**: "../../include/utils/config_lifecycle.h"
- **原因**: Header file location changed during refactoring

### 修复 139
- **文件**: /home/liying/sqlcc/src/utils/BUILD.bazel
- **旧路径**: //include/utils:smart_config_manager.h
- **新路径**: "../../include/utils/smart_config_manager.h"
- **原因**: Header file location changed during refactoring

### 修复 140
- **文件**: /home/liying/sqlcc/src/utils/BUILD.bazel
- **旧路径**: //include/utils:ssl_wrapper.h
- **新路径**: "../../include/utils/ssl_wrapper.h"
- **原因**: Header file location changed during refactoring

### 修复 141
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **旧路径**: //include/procedure:procedure_parser.h
- **新路径**: "../../include/procedure/procedure_parser.h"
- **原因**: Header file location changed during refactoring

### 修复 142
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **旧路径**: //include/procedure:procedure_vm.h
- **新路径**: "../../include/procedure/procedure_vm.h"
- **原因**: Header file location changed during refactoring

### 修复 143
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **旧路径**: //include/procedure:procedure_trigger_executor.h
- **新路径**: "../../include/procedure/procedure_trigger_executor.h"
- **原因**: Header file location changed during refactoring

### 修复 144
- **文件**: /home/liying/sqlcc/src/execution/BUILD.bazel
- **旧路径**: //include/execution:function_executor.h
- **新路径**: "../../include/execution/function_executor.h"
- **原因**: Header file location changed during refactoring

### 修复 145
- **文件**: /home/liying/sqlcc/src/sql_executor/BUILD.bazel
- **旧路径**: //include:sql_executor
- **新路径**: "../../include/sql_executor"
- **原因**: Header file location changed during refactoring


---
*此报告由bazel_include_fixer.py自动生成*