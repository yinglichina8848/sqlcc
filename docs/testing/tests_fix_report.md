# Bazel标签自动修复报告
生成时间: report

## 统计摘要
- 处理文件总数: 71
- 实际处理文件: 25
- 发现问题总数: 20

## 问题类型统计
- 缺少引号: 12 个
- 相对路径错误: 8 个

## 详细修复记录
### /home/liying/sqlcc/tests/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 4
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_test")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_test")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/tests/components/core/BUILD.bazel (8 个修复)
#### 修复 1
- **行号**: 3
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_test")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_test")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""

#### 修复 2
- **行号**: 8
- **错误类型**: 相对路径错误
- **原始代码**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **修复后**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 3
- **行号**: 8
- **错误类型**: 相对路径错误
- **原始代码**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **修复后**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 4
- **行号**: 8
- **错误类型**: 相对路径错误
- **原始代码**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **修复后**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 5
- **行号**: 21
- **错误类型**: 相对路径错误
- **原始代码**: `includes = ["../include", "../../../include/core"],`
- **修复后**: `includes = ["../include", "../../../include/core"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 6
- **行号**: 42
- **错误类型**: 相对路径错误
- **原始代码**: `includes = ["../include", "../../../include/core"],`
- **修复后**: `includes = ["../include", "../../../include/core"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 7
- **行号**: 63
- **错误类型**: 相对路径错误
- **原始代码**: `includes = ["../include", "../../../include/core"],`
- **修复后**: `includes = ["../include", "../../../include/core"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 8
- **行号**: 83
- **错误类型**: 相对路径错误
- **原始代码**: `includes = ["../include", "../../../include/core"],`
- **修复后**: `includes = ["../include", "../../../include/core"],`
- **描述**: 简化相对路径: ../../ -> 


### /home/liying/sqlcc/tests/components/network/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 4
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_test")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_test")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/tests/components/parser/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 4
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_test", "cc_binary")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_test", "cc_binary")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/tests/components/transaction/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 3
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_test")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_test")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/tests/integration/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 3
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_test")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_test")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/tests/security/BUILD (1 个修复)
#### 修复 1
- **行号**: 3
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_test")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_test")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/tests/sql/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 5
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_test")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_test")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/tests/sql_executor/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 1
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_test")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_test")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/tests/sql_parser/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 3
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_test")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_test")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/tests/unit/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 4
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_test")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_test")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""


### /home/liying/sqlcc/tests/unit/basic/BUILD.bazel (2 个修复)
#### 修复 1
- **行号**: 1
- **错误类型**: 缺少引号
- **原始代码**: `load("@rules_cc//cc:defs.bzl", "cc_test")`
- **修复后**: `load("@rules_cc"//cc:defs.bzl"", "cc_test")`
- **描述**: 添加缺失的引号: //cc:defs.bzl" -> "//cc:defs.bzl""

#### 修复 2
- **行号**: 53
- **错误类型**: 相对路径错误
- **原始代码**: `includes = ["../../../include"],`
- **修复后**: `includes = ["../../../include"],`
- **描述**: 简化相对路径: ../../ -> 

