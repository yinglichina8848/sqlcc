# SQLCC依赖关系修复报告
修复时间: 2025-12-22 03:37:05
创建目标数量: 30
修复条目数量: 30

## 修复详情
### 修复 1
- **动作**: Created Target
- **目标**: //src/network:network
- **文件**: /home/liying/sqlcc/src/network/BUILD.bazel
- **源文件**: network.cpp
- **原因**: Missing dependency referenced in /home/liying/sqlcc/BUILD.bazel

### 修复 2
- **动作**: Created Placeholder
- **目标**: //src/trigger:trigger
- **文件**: /home/liying/sqlcc/src/trigger/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/tests/BUILD.bazel

### 修复 3
- **动作**: Created Target
- **目标**: //src/network:network
- **文件**: /home/liying/sqlcc/src/network/BUILD.bazel
- **源文件**: network.cpp
- **原因**: Missing dependency referenced in /home/liying/sqlcc/tests/unit/network/BUILD.bazel

### 修复 4
- **动作**: Created Target
- **目标**: //src/storage_engine:lazy_writer
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **源文件**: lazy_writer.cpp
- **原因**: Missing dependency referenced in /home/liying/sqlcc/tests/unit/storage/BUILD.bazel

### 修复 5
- **动作**: Created Placeholder
- **目标**: //src/core:sqlcc_core_lib
- **文件**: /home/liying/sqlcc/src/core/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/tests/unit/basic/BUILD.bazel

### 修复 6
- **动作**: Created Target
- **目标**: //src:execution_context
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **源文件**: execution_context.cpp
- **原因**: Missing dependency referenced in /home/liying/sqlcc/tests/unit/basic/BUILD.bazel

### 修复 7
- **动作**: Created Placeholder
- **目标**: //src/core:sqlcc_core_lib
- **文件**: /home/liying/sqlcc/src/core/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/tests/integration/BUILD.bazel

### 修复 8
- **动作**: Created Placeholder
- **目标**: //src/core:sqlcc_database_core
- **文件**: /home/liying/sqlcc/src/core/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/tests/integration/BUILD.bazel

### 修复 9
- **动作**: Created Target
- **目标**: //src/network:network
- **文件**: /home/liying/sqlcc/src/network/BUILD.bazel
- **源文件**: network.cpp
- **原因**: Missing dependency referenced in /home/liying/sqlcc/tests/integration/BUILD.bazel

### 修复 10
- **动作**: Created Placeholder
- **目标**: //src/trigger:trigger
- **文件**: /home/liying/sqlcc/src/trigger/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/tests/integration/BUILD.bazel

### 修复 11
- **动作**: Created Placeholder
- **目标**: //src/sql_executor:sqlcc_executor
- **文件**: /home/liying/sqlcc/src/sql_executor/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/tests/integration/BUILD.bazel

### 修复 12
- **动作**: Created Target
- **目标**: //src:unified_executor
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **源文件**: unified_executor.cpp
- **原因**: Missing dependency referenced in /home/liying/sqlcc/tests/integration/BUILD.bazel

### 修复 13
- **动作**: Created Target
- **目标**: //src/network:network
- **文件**: /home/liying/sqlcc/src/network/BUILD.bazel
- **源文件**: network.cpp
- **原因**: Missing dependency referenced in /home/liying/sqlcc/tests/components/network/BUILD.bazel

### 修复 14
- **动作**: Created Placeholder
- **目标**: //src/network:sqlcc_network
- **文件**: /home/liying/sqlcc/src/network/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/tests/components/network/BUILD.bazel

### 修复 15
- **动作**: Created Placeholder
- **目标**: //src/core:sqlcc_core_lib
- **文件**: /home/liying/sqlcc/src/core/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/tests/components/core/BUILD.bazel

### 修复 16
- **动作**: Created Placeholder
- **目标**: //src/sql_executor:sqlcc_executor
- **文件**: /home/liying/sqlcc/src/sql_executor/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/tests/components/core/BUILD.bazel

### 修复 17
- **动作**: Created Target
- **目标**: //src:execution_engine
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **源文件**: execution_engine.cpp
- **原因**: Missing dependency referenced in /home/liying/sqlcc/tests/components/core/BUILD.bazel

### 修复 18
- **动作**: Created Target
- **目标**: //src:unified_executor
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **源文件**: unified_executor.cpp
- **原因**: Missing dependency referenced in /home/liying/sqlcc/tests/components/core/BUILD.bazel

### 修复 19
- **动作**: Created Placeholder
- **目标**: //src/core:sqlcc_core_lib
- **文件**: /home/liying/sqlcc/src/core/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/tests/performance/BUILD.bazel

### 修复 20
- **动作**: Created Placeholder
- **目标**: //src/sql_executor:sqlcc_executor
- **文件**: /home/liying/sqlcc/src/sql_executor/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/tests/performance/BUILD.bazel

### 修复 21
- **动作**: Created Placeholder
- **目标**: //src:sqlcc_core
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/tests/performance/BUILD.bazel

### 修复 22
- **动作**: Created Placeholder
- **目标**: //src/sql_executor:sqlcc_executor
- **文件**: /home/liying/sqlcc/src/sql_executor/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/tests/sql_executor/BUILD.bazel

### 修复 23
- **动作**: Created Target
- **目标**: //src/network:network
- **文件**: /home/liying/sqlcc/src/network/BUILD.bazel
- **源文件**: network.cpp
- **原因**: Missing dependency referenced in /home/liying/sqlcc/src/isql_network/BUILD.bazel

### 修复 24
- **动作**: Created Target
- **目标**: //src/network:network
- **文件**: /home/liying/sqlcc/src/network/BUILD.bazel
- **源文件**: network.cpp
- **原因**: Missing dependency referenced in /home/liying/sqlcc/src/sqlcc_server/BUILD.bazel

### 修复 25
- **动作**: Created Placeholder
- **目标**: //src/network:sqlcc_network
- **文件**: /home/liying/sqlcc/src/network/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/src/sqlcc_server/BUILD.bazel

### 修复 26
- **动作**: Created Placeholder
- **目标**: //src/storage_engine:index_manager
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/src/sqlcc_server/BUILD.bazel

### 修复 27
- **动作**: Created Placeholder
- **目标**: //src/sql_executor:sqlcc_executor
- **文件**: /home/liying/sqlcc/src/sql_executor/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/src/sqlcc_server/BUILD.bazel

### 修复 28
- **动作**: Created Target
- **目标**: //src:unified_executor
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **源文件**: unified_executor.cpp
- **原因**: Missing dependency referenced in /home/liying/sqlcc/src/sqlcc_server/BUILD.bazel

### 修复 29
- **动作**: Created Placeholder
- **目标**: //src/trigger:trigger
- **文件**: /home/liying/sqlcc/src/trigger/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/src/execution/BUILD.bazel

### 修复 30
- **动作**: Created Placeholder
- **目标**: //src/trigger:trigger
- **文件**: /home/liying/sqlcc/src/trigger/BUILD.bazel
- **原因**: Created placeholder for missing dependency referenced in /home/liying/sqlcc/src/trigger/BUILD.bazel


---
*此报告由bazel_dependency_fixer.py自动生成*