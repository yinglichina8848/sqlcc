# Bazel标签自动修复报告
生成时间: report

## 统计摘要
- 处理文件总数: 71
- 实际处理文件: 71
- 发现问题总数: 27

## 问题类型统计
- 无效的外部引用格式: 4 个
- 缺少引号: 11 个
- 相对路径错误: 12 个

## 详细修复记录
### /home/liying/sqlcc/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 6
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_test")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_binary", "cc_test")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/include/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 5
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_library")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_library")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/include/disk_manager/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 1
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_library")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_library")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/include/network/encryption/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 4
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_library")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_library")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/include/page/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 1
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_library")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_library")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/include/procedure/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 5
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_library")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_library")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/include/storage_engine/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 1
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_library")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_library")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/include/storage_engine/table_storage/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 1
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_library")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_library")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/include/trigger/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 5
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_library")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_library")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/src/BUILD.bazel (5 个修复)
#### 修复 1
- **行号**: 4
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_library", "cc_test")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""

#### 修复 2
- **行号**: 154
- **错误类型**: 无效的外部引用格式
- **原始代码**: `"//include/storage_engine/buffer_pool:buffer_pool/lru_manager.h",`
- **修复后**: `"//include/storage_engine/buffer_pool/buffer_pool:buffer_pool/lru_manager.h",`
- **描述**: 修复外部引用格式: //include/storage_engine/buffer_pool:buffer_pool/lru_manager.h -> //include/storage_engine/buffer_pool/buffer_pool:buffer_pool/lru_manager.h

#### 修复 3
- **行号**: 156
- **错误类型**: 无效的外部引用格式
- **原始代码**: `"//include/storage_engine/buffer_pool:buffer_pool/statistics_collector.h",`
- **修复后**: `"//include/storage_engine/buffer_pool/buffer_pool:buffer_pool/statistics_collector.h",`
- **描述**: 修复外部引用格式: //include/storage_engine/buffer_pool:buffer_pool/statistics_collector.h -> //include/storage_engine/buffer_pool/buffer_pool:buffer_pool/statistics_collector.h

#### 修复 4
- **行号**: 158
- **错误类型**: 无效的外部引用格式
- **原始代码**: `"//include/storage_engine/table_storage:table_storage/page_raii.h",`
- **修复后**: `"//include/storage_engine/table_storage/table_storage:table_storage/page_raii.h",`
- **描述**: 修复外部引用格式: //include/storage_engine/table_storage:table_storage/page_raii.h -> //include/storage_engine/table_storage/table_storage:table_storage/page_raii.h

#### 修复 5
- **行号**: 160
- **错误类型**: 无效的外部引用格式
- **原始代码**: `"//include/storage_engine/table_storage:table_storage/record_validator.h",`
- **修复后**: `"//include/storage_engine/table_storage/table_storage:table_storage/record_validator.h",`
- **描述**: 修复外部引用格式: //include/storage_engine/table_storage:table_storage/record_validator.h -> //include/storage_engine/table_storage/table_storage:table_storage/record_validator.h


### /home/liying/sqlcc/src/logger/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 4
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_library")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_library")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/src/sqlcc_server/BUILD.bazel (2 个修复)
#### 修复 1
- **行号**: 19
- **错误类型**: 相对路径错误
- **原始代码**: `includes = ["../include", "../../include/core", "../../include/storage"],`
- **修复后**: `includes = ["../include", "../../include/core", "../../include/storage"],`
- **描述**: 简化相对路径: ../ -> 

#### 修复 2
- **行号**: 19
- **错误类型**: 相对路径错误
- **原始代码**: `includes = ["../include", "../../include/core", "../../include/storage"],`
- **修复后**: `includes = ["../include", "../../include/core", "../../include/storage"],`
- **描述**: 简化相对路径: ../ -> 


### /home/liying/sqlcc/src/storage_engine/buffer_pool/BUILD.bazel (2 个修复)
#### 修复 1
- **行号**: 8
- **错误类型**: 相对路径错误
- **原始代码**: `"../../../include/storage_engine/buffer_pool/lru_manager.h",`
- **修复后**: `"../../../include/storage_engine/buffer_pool/lru_manager.h",`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 2
- **行号**: 10
- **错误类型**: 相对路径错误
- **原始代码**: `"../../../include/storage_engine/buffer_pool/statistics_collector.h",`
- **修复后**: `"../../../include/storage_engine/buffer_pool/statistics_collector.h",`
- **描述**: 简化相对路径: ../../ -> 


### /home/liying/sqlcc/tests/components/core/BUILD.bazel (7 个修复)
#### 修复 1
- **行号**: 9
- **错误类型**: 相对路径错误
- **原始代码**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **修复后**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 2
- **行号**: 9
- **错误类型**: 相对路径错误
- **原始代码**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **修复后**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 3
- **行号**: 9
- **错误类型**: 相对路径错误
- **原始代码**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **修复后**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 4
- **行号**: 24
- **错误类型**: 相对路径错误
- **原始代码**: `includes = ["../include", "../../../include/core"],`
- **修复后**: `includes = ["../include", "../../../include/core"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 5
- **行号**: 47
- **错误类型**: 相对路径错误
- **原始代码**: `includes = ["../include", "../../../include/core"],`
- **修复后**: `includes = ["../include", "../../../include/core"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 6
- **行号**: 70
- **错误类型**: 相对路径错误
- **原始代码**: `includes = ["../include", "../../../include/core"],`
- **修复后**: `includes = ["../include", "../../../include/core"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 7
- **行号**: 92
- **错误类型**: 相对路径错误
- **原始代码**: `includes = ["../include", "../../../include/core"],`
- **修复后**: `includes = ["../include", "../../../include/core"],`
- **描述**: 简化相对路径: ../../ -> 


### /home/liying/sqlcc/tests/unit/basic/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 54
- **错误类型**: 相对路径错误
- **原始代码**: `includes = ["../../../include"],`
- **修复后**: `includes = ["../../../include"],`
- **描述**: 简化相对路径: ../../ -> 

