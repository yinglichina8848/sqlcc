# 《SQLCC 标准化 BUILD 目录与规则规范（强制版）》

**版本**: v1.0
**制定时间**: 2026年1月21日
**制定者**: AI Assistant（基于架构师审查意见）
**适用范围**: SQLCC 项目所有 BUILD.bazel 文件

## 📋 目标

消除 include/ 与 src/ 双头文件体系

彻底杜绝 Bazel includes / 相对路径 / 隐式依赖

让 bazel query deps(...) 图天然可读、可维护

## 一、目录结构总规范（最终态）

### ✅ 唯一允许的工程结构
```
sqlcc/
├── include/                 # 仅存放"全局公共接口"
│   ├── common_includes.h    # 降级后的全局头文件（原sqlcc_pch.h）
│   ├── storage_common_includes.h # 降级后的存储头文件（原storage_engine_pch.h）
│   └── BUILD.bazel          # 仅包含全局头文件target
│
├── src/
│   ├── core/                # 核心服务
│   │   ├── *.h              # 头文件与实现同目录
│   │   ├── *.cpp
│   │   └── BUILD.bazel      # 严格规范化
│   ├── utils/               # 基础工具
│   ├── exception/           # 异常处理
│   ├── execution/           # 执行引擎
│   ├── sql_parser/          # SQL解析器
│   ├── sql_executor/        # SQL执行器
│   ├── transaction/         # 事务管理
│   ├── storage_engine/      # 存储引擎
│   │   ├── common/          # 公共类型定义
│   │   ├── buffer_pool/     # 缓冲池
│   │   ├── disk_manager/    # 磁盘管理
│   │   ├── wal/             # WAL日志
│   │   └── access/          # 存储访问
│   ├── network/             # 网络通信
│   └── BUILD.bazel          # 根级构建配置
│
├── tests/                   # 镜像src结构
└── WORKSPACE
```

## 二、头文件放置铁律（零例外）

### 🔒 规则 1：头文件"就地原则"
一个模块的 .h 必须与其 .cpp 在同一目录

```
src/utils/
├── logger.h
├── logger.cpp
└── BUILD.bazel
```

**禁止**:
- ❌ `include/utils/logger.h`
- ❌ `src/utils/include/logger.h`

### 🔒 规则 2：include/ 目录只允许 3 类文件
| 类型 | 示例 | 说明 |
|------|------|------|
| 全局公共头文件 | `common_includes.h` | 编译优化用普通头文件 |
| 稳定公共API | （极少） | 对外部接口 |
| 过渡文件 | `TEMP_*` | 必须有删除期限 |

**其余一律迁走或删除**

## 三、BUILD.bazel 书写规范（核心）

### 1️⃣ 每个模块只允许一个 cc_library
```
src/<module>/BUILD.bazel
```

### ✅ 标准模板（强制）
```bazel
cc_library(
    name = "<module>",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    strip_include_prefix = "",
    include_prefix = "",
    deps = [
        "//src/utils",
        "//src/exception",
        # 严格按照依赖层级声明，无循环依赖
    ],
    visibility = ["//src:__subpackages__"],
)
```

### ❌ 明令禁止的 BUILD 写法
| 禁止项 | 原因 |
|--------|------|
| `includes = ["."]` | Bazel 反模式，破坏依赖图 |
| `includes = [".."]` | 路径泄漏 |
| `strip_include_prefix = "src"` | 错误配置 |
| `include_prefix = "src/<module>"` | 冗余路径 |
| 一个目录多个 cc_library | 依赖爆炸 |
| hdrs 引用外部目录 | 结构污染 |

## 四、#include 规范（最关键）

### 🔑 唯一合法写法：基于 Bazel target 的虚拟根
```cpp
#include "utils/logger.h"
#include "core/execution_context.h"
#include "transaction/transaction_manager.h"
```

**对应关系**:
- `"utils/logger.h"` ↳ `//src/utils:utils`
- `"core/execution_context.h"` ↳ `//src/core:core`

### ❌ 严禁的 include 形式
```cpp
#include "../utils/logger.h"           // 相对路径
#include "../../include/utils/logger.h" // 绝对路径
#include <utils/logger.h>              // 系统头文件风格
#include "src/utils/logger.h"          // Bazel 反模式
```

**发现一次，必须重构，不允许"先过编译"**

## 五、依赖方向强约束（防循环）

### 🔺 模块层级（从下到上）
```
utils (基础工具)
  ↓
exception (异常处理)
  ↓
types (类型定义)
  ↓
core (核心服务)
  ↓
execution (执行引擎) ← sql_parser (SQL解析)
  ↓
sql_executor (SQL执行)
  ↓
transaction (事务管理)
  ↓
storage_engine (存储引擎)
  ↓
network (网络通信)
  ↓
bin/app (应用层)
```

### ❌ 禁止规则
- ❌ `utils → core`（反向依赖）
- ❌ `exception → sql_executor`（跨层依赖）
- ❌ `storage_engine → sql_executor`（反向依赖）
- ❌ `transaction → execution`（反向依赖）

**验证方式**:
```bash
bazel query 'allpaths(//src/utils, //src/core)'  # 应为空
```

## 六、include/BUILD.bazel 最终形态

```bazel
cc_library(
    name = "global_headers",
    hdrs = [
        "common_includes.h",
        "storage_common_includes.h",
    ],
    visibility = ["//visibility:public"],
)
# ❌ 禁止 deps、srcs 等
```

## 七、强制校验清单（Code Review 用）

### 每个 PR 必须满足：
- ✅ 新增 .h 与 .cpp 同目录
- ✅ BUILD 中 无 `includes = [...]`
- ✅ include 语句不含 `..` 或 `src/`
- ✅ deps 只指向 `//src/*`
- ✅ bazel query deps(...) 无环

### CI Gate 验证：
```bash
# 依赖层级检查
bazel query 'allpaths(//src/utils, //src/core)'

# 构建规范检查
bazel query 'deps(//src/...)' | grep -E 'includes.*\["\."\]'

# include 路径检查
find src -name "*.cpp" -o -name "*.h" | xargs grep -n '#include "src/'
```

## 八、实施路径建议

### ✅ Step 1（今天就能做）：选 utils/ 作为样板
1. 完成头文件迁移
2. 重写 BUILD.bazel 为最严格规范
3. 验证依赖层级约束
4. 成功后复制模式到其他模块

### ✅ Step 2：core/、exception/ 跟随
- 无状态、最安全
- 建立标准化模式

### ❌ 暂缓：storage_engine/、transaction/
- 依赖关系最复杂
- 需要专门的子阶段拆分

---

## 九、工具支持

### 推荐开发工具：
1. **依赖分析器**: `tools/bazel_dep_analyzer.py`
2. **include 检查器**: `tools/include_path_checker.py`
3. **BUILD 规范化器**: `tools/build_standardizer.py`

### CI 集成：
```yaml
# .github/workflows/pr-check.yml
- name: Bazel Dependency Check
  run: bazel query 'allpaths(//src/..., //src/...)'

- name: Include Path Check
  run: find src -name "*.cpp" -exec grep -l '#include "src/' {} \;
```

---

## 十、问答

### Q: 为什么禁止 includes = ["."]？
**A**: 这会让 Bazel 无法验证头文件边界，导致隐式依赖和循环依赖风险。

### Q: strip_include_prefix = "" 是什么意思？
**A**: 告诉 Bazel 从当前目录开始生成 include 路径，不需要前缀修剪。

### Q: 如何处理全局头文件？
**A**: 降级为普通头文件，在需要的地方显式 include，不要依赖 pch 机制。

---

*本规范为强制性要求，任何违反将导致 PR 驳回*

**维护者**: 架构师 + 技术负责人
**最后更新**: 2026年1月21日
