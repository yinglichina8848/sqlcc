# SQLCC v1.1.2 版本综合评估报告

## 1. 项目概述

### 1.1 版本信息
- **版本号**: 1.1.2
- **评估日期**: 2025-12-10
- **语言/平台**: C++（主代码采用 C++17 风格）；构建系统：Bazel（同时保留 CMake 配置）
- **评估目标**: 全面评估当前代码库在功能实现、稳定性、内存/资源安全、并发/线程安全、测试与 CI、以及可维护性方面的现状，并给出切实可行的改进计划。

### 1.2 本次评估的依据与方法
- 阅读仓库结构与关键源文件（`src/`、`include/`、`tests/` 等）
- 分析已有评估文档与历史报告（`docs/evaluation/v1.0.*`、`v1.1.0`、`v1.1.1` 等）以保持格式与比较基线
- 在本地尝试使用 Bazel + Sanitizer 构建并运行示例测试（记录遇到的构建问题）
- 静态扫描关键路径寻找潜在内存/资源/并发风险（含 raw pointers、全局单例、未 join 的线程等）
- 实际运行测试套件并分析测试结果

## 2. 主要改进/变更点（相较 v1.1.1）

### 2.1 构建系统改进
- 在仓库根 `.bazelrc` 中新增了可选的 Sanitizer 配置（`--config=asan/tsan/ubsan`），便于开发者和 CI 快速启用 AddressSanitizer / ThreadSanitizer / UBSan。
- 修复了一个阻止 Bazel 包加载的异常文档文件名问题（`docs/progress` 下包含非法字符的文件），在根 `BUILD.bazel` 中对 `generate_docs` 的 glob 做了排除处理以避免包加载失败。
- 修复了多个 BUILD 文件中的依赖关系问题，使得测试套件能够正确编译和运行。

### 2.2 测试基础设施完善
- 修复了核心组件、解析器、执行器、存储、网络、安全和事务组件的测试套件，使其能够正确运行。
- 修复了 system_database_test 的链接错误，成功运行了所有 22 个测试用例。
- 完善了 tests/components 目录下的测试配置，使测试能够实际运行。

### 2.3 代码质量提升
- 修复了网络组件测试的链接错误，通过在网络库的 BUILD 文件中添加对解析器库和执行器库的依赖。
- 修复了测试组件中 gtest 引用不正确的问题。
- 修复了测试组件中库引用不正确的问题。
- 修复了头文件包含路径的问题。

## 3. 功能评估（功能完成度）

### 3.1 核心功能（已实现或部分实现）
- 存储引擎（页式存储、缓冲池、磁盘 I/O）
- SQL 解析器与 AST（基础 SQL 语句支持：DDL、DML、简单 JOIN、子查询基础）
- 索引（B+树）实现，支持点查与范围查
- 事务管理（基本事务生命周期、保存点、WAL）
- 网络服务器与客户端基础通信能力
- 单元测试与组件测试框架（Google Test）和性能测试脚本
- 系统数据库管理功能（创建、管理元数据表）

### 3.2 高级功能（缺失或不完善）
- 窗口函数、CTE、复杂集合操作（UNION/INTERSECT/EXCEPT）
- 完整隔离级别（仅有 READ COMMITTED，缺乏 REPEATABLE READ / SERIALIZABLE）
- MVCC 多版本并发控制（目前以两阶段锁为主）
- 检查点/更健壮的崩溃恢复机制
- 复杂的 SQL 语句解析（如 INSERT 语句解析存在问题）

## 4. 关键缺陷与风险点（按类别）

### 4.1 构建与工程（P0/P1）
- 若干 BUILD 规则与 src/include 的关系不一致（`includes`、`hdrs`、`srcs` 配置存在遗漏），导致在不同构建配置下（Sanitizer/Debug）出现编译失败。
- 某些文档/资源文件名包含非法字符，导致 Bazel 包加载失败（已临时排除 `docs/progress/**`）。

风险/影响：阻碍 CI 与本地启用 sanitizer 验证，影响开发效率。

建议：系统性清理 BUILD 文件，统一头文件导出（使用 `hdrs`/`includes` 明确声明），并规范 docs 文件命名。

### 4.2 内存/资源泄漏风险（P0/P1）
- 代码中存在裸 `new`/`delete`、C 风格资源（`FILE*`/fd/mmap`）以及容器保存裸指针的模式（需进一步 grep 确认）。
- 长期运行组件（WAL、缓冲池、事务管理器）如果在错误路径或异常路径未正确释放资源，会导致内存持续增长。

建议：启用 ASan/LSan、Valgrind 并审计 `new`/`delete`、`malloc`/`free`、容器 T* 的用法，优先将明显的所有权改为 `std::unique_ptr` 或 RAII 封装。

### 4.3 并发/线程安全（P0/P1）
- 使用 Abseil 线程安全注解，但存在 `NO_THREAD_SAFETY_ANALYSIS` 的绕过点，可能掩盖数据竞争。
- 线程生命周期管理不严谨（存在 detached 线程或线程池未显式 join 的风险）。

建议：启用 TSan 在 CI/本地跑关键并发组件；移除或审查 `NO_THREAD_SAFETY_ANALYSIS` 使用处，确保锁注解与实际实现一致。

### 4.4 功能稳定性与测试覆盖（P1/P2）
- 部分核心模块（SQL执行器、事务管理、解析器）的测试覆盖率偏低；错误处理路径未被充分测试。
- 压力/崩溃恢复测试不足，缺少长期运行内存/资源回收验证。
- SQL 解析器存在严重问题，无法正确解析 INSERT 语句，导致 constraint_validation_test 测试失败。

建议：补充单元/集成测试，增加连续创建/销毁管理器的回归测试与长时压力测试。

### 4.5 SQL 解析器问题（P0）
- SQL 解析器无法正确解析 INSERT 语句，这是导致 constraint_validation_test 测试失败的根本原因。
- 解析器在处理 INSERT INTO ... VALUES ...语句时出现状态机错误，最终解析出 0 个语句。

建议：深入分析 SQL 解析器的状态机实现，修复 INSERT 语句解析失败的问题。

## 5. 本次本地验证中发现的具体问题（摘录）
- Bazel 包加载失败：`genrule` 的 glob 覆盖到非法文件名（已通过 BUILD 中排除修复）。
- 编译失败：`src/execution_engine.cpp` 报错找不到 `execution_context.h`（因部分包含路径使用相对 include，BUILD 中 `includes` 需补齐或源码 include 改为 `core/execution_context.h`）。
- 多处 Bazel 警告：`includes` 指向仓库根 `include` 的相对路径，会在未来 Bazel 版本报错；建议改为显式 hdrs/filegroup 引用。
- SQL 解析器无法正确解析 INSERT 语句，导致 constraint_validation_test 测试失败。

## 6. 改进计划（优先级、负责人建议与时间线）
注：优先级分为 P0（必须尽快修复），P1（高优先级），P2（中低优先级）。

### 阶段 0 — 立刻可执行（24–72 小时）
- P0-1: 修复 Bazel 构建阻塞问题
  - 任务：清理 `BUILD.bazel` 中导致包加载失败或头文件查找失败的问题（规范 `hdrs`/`includes`/`filegroup` 的使用）。
  - 预期产出：能够在本地成功运行 `bazel build` 与 `bazel test --config=asan` 单个测试目标。
- P0-2: 在 CI 或本地启用 Sanitizer 快速检测
  - 任务：使用已添加的 `--config=asan/tsan/ubsan` 运行关键单元测试（先小范围，例如 `//src:parser_unit_tests` 或 `//src:storage_component_tests`）。
  - 预期产出：得到一轮 ASan/TSan 错误报告，定位显著内存/并发缺陷。
- P0-3: 修复 SQL 解析器 INSERT 语句解析问题
  - 任务：深入分析 SQL 解析器的状态机实现，修复 INSERT 语句解析失败的问题。
  - 预期产出：SQL 解析器能够正确解析 INSERT 语句，constraint_validation_test 测试通过。

### 阶段 1 — 代码安全与稳定性（1–3 周）
- P0-4: 内存/资源泄漏修复
  - 任务：对 Sanitizer/Valgrind 报告的高频错误点进行修复；把裸指针替换为 `std::unique_ptr` / RAII，封装文件描述符等。
  - 预期产出：通过 LSan/ASan 验证的内存问题显著减少。
- P1-1: 并发/数据竞争修复
  - 任务：修正 `NO_THREAD_SAFETY_ANALYSIS` 的滥用，使用 TSan 查到的竞争点修复同步策略。
  - 预期产出：关键并发场景下无 TSan 报错。
- P1-2: 添加长期稳定性测试
  - 任务：增加一个循环创建/销毁数据库实例、并发事务压力测试，运行 1–4 小时，监测内存/FD 使用。
  - 预期产出：发现并修复长期运行下的资源泄漏或性能退化。

### 阶段 2 — 功能增强与重构（4–12 周）
- P1-3: 增强 SQL 功能
  - 任务：优先实现 HAVING、窗口函数（基础实现）、CTE 支持，并扩展执行器的统一查询规划接口。
  - 预期产出：SQL 表达能力提升，复杂查询支持改善。
- P1-4: 完善测试覆盖
  - 任务：修复 AST 节点测试中的 SelectStatementTest 失败和 PermissionValidatorTest 中的 DropPermissionTest 失败。
  - 预期产出：核心测试套件全部通过。
- P2-1: 事务与并发模型重构（中长期）
  - 任务：评估并逐步引入 MVCC 或混合并发控制策略，优化锁粒度与死锁检测策略。
  - 预期产出：并发吞吐量提升，锁竞争减轻。
- P2-2: 覆盖率与测试完善
  - 任务：补充测试用例至低覆盖模块（执行器、解析器、事务管理）、引入覆盖率门禁（CI 阶段）。
  - 预期产出：整体覆盖率显著提高（目标：行覆盖率 ≥ 70%）。

## 7. 可交付的短期工作项（可直接创建 issue/PR）
- Issue/PR #1: 规范 Bazel `includes`/`hdrs` 用法并修复 `generate_docs` 的 glob（已提交临时修复）。
- Issue/PR #2: 添加 `--config=asan/tsan/ubsan` 使用说明至 README，并在 CI 中新增可选 job（手动开启）。
- Issue/PR #3: 在 `src/` 中 grep 并列出所有 `new`/`delete`/`malloc`/`free` 的位置，产出候选修复清单。
- Issue/PR #4: 为 `transaction_manager`、`wal_manager`、`disk_manager` 添加短期内存/资源回收单元测试（创建/销毁循环 10k 次）。
- Issue/PR #5: 增加一个 sanitizer 验证脚本 `scripts/run_sanitizers.sh`（封装 bazel test --config=asan/--config=tsan 的常用参数）。
- Issue/PR #6: 修复 SQL 解析器 INSERT 语句解析问题。

## 8. 验证步骤与命令（复制粘贴即可运行）
- 列出测试目标（供选择）:
```bash
bazel query 'tests(//...)' | sed -n '1,200p'
```
- 以 ASan 运行特定测试目标（示例）:
```bash
bazel test --config=asan //src:parser_unit_tests --test_env=ASAN_OPTIONS=detect_leaks=1:abort_on_error=1
```
- 以 TSan 运行（示例）:
```bash
bazel test --config=tsan //src:storage_component_tests --test_env=TSAN_OPTIONS=report_thread_leaks=1
```
- 使用 Valgrind（针对单个可执行文件）:
```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bazel-bin/<target>
```

## 9. 风险评估
- 若短期内不修复 BUILD/包含路径问题，将无法稳定在 CI 中启用 Sanitizer，从而使内存/并发问题难以被持续发现。
- 大量使用裸资源（裸指针、未封装 FD、未 join 的线程）会在长期运行的生产/测试环境中导致不可预测的资源枯竭。
- SQL 解析器的核心问题若不及时修复，将严重影响数据库的基本功能，阻碍后续测试和开发工作。

## 10. 结论与下一步建议
- 总体结论: SQLCC 在教学场景下已实现核心模块且设计较为清晰，但在工程化（构建一致性、自动化检测）、内存/资源安全与并发健壮性方面存在不足，需要以 Sanitizer 为中心的短期修复以及中长期的并发&事务模型改进。此外，SQL 解析器的核心问题需要优先解决。

- 建议的第一步（即刻执行）:
  1. 修复 Bazel 构建的阻塞项（HEAD：`BUILD.bazel` 与 `includes`、有问题的 docs 文件名）
  2. 使用 `--config=asan` 对关键单元测试进行一轮扫描并处理高优先级错误
  3. 在 CI 中新增可选 sanitizer job，作为质量门禁的一部分
  4. 优先修复 SQL 解析器 INSERT 语句解析问题

---
**评估报告编写人**: AI 助手
**评估版本**: SQLCC v1.1.2
**编写日期**: 2025-12-10