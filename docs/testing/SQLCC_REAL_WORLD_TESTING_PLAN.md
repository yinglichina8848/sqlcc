# SQLCC 真实场景测试实施计划文档

## 前言

本文档综合了SQLite、MySQL、PostgreSQL主流数据库项目的测试方法与环境分析，以及基于SQLCC现有架构的具体测试计划设计。计划旨在将SQLCC从实验室原型提升为可部署的企业级数据库管理系统，确保在接近生产环境的压力下具备可靠性、性能和数据一致性。

文档基于11/21/2025调查结果制定，以TESTING_IMPROVEMENTS_SUMMARY.md为重要参考，计划分阶段实施，总周期约3-6个月。

---

## 1. 主流数据库项目测试方法与环境背景调查

### 1.1 SQLite 数据库测试方法

#### 测试类型
- **回归测试：** TCL脚本套件（tclsqlite）覆盖SQL语法、边界条件、错误处理，使用`make test`或`tcltest`执行2000+个测试用例
- **模糊测试（Fuzzing）：** SQLSmith随机生成SQL查询，发现崩溃和内存泄漏，重点测试edge cases和NULL值处理
- **性能测试：** sqlite3 shell运行批量操作，评估事务提交延迟和并发性能，支持WAL模式下崩溃恢复
- **多平台兼容：** Windows/Linux/macOS全平台运行，确保跨平台一致性

#### 测试环境
- **基本工具：** sqlite3命令行工具 + TCL解释器
- **配置策略：** 多线程环境测试锁竞争；不同页面大小（1KB-64KB）测试I/O效率
- **关键指标：** 100%回归通过率，无内存泄漏，TPC-B风格吞吐量基准

### 1.2 MySQL 数据库测试方法

#### 测试类型
- **回归测试：** run-tests.pl脚本驱动.mtr格式测试套件，覆盖CRUD操作、事务ACID约束
- **压力测试：** mysqlslap模拟多连接负载，测试连接池和缓存效率
- **集成测试：** 备份恢复（mysqldump/mysqlpump）、主从复制、XA分布式事务
- **崩溃测试：** kill进程后重启验证InnoDB数据一致性，检测孤儿事务

#### 测试环境
- **工具生态：** mysqlslap、sysbench、mysqlbinlog日志分析
- **配置模式：** InnoDB引擎配置测试表空间、MVCC、多版本并发
- **规模测试：** TB级数据表，评估索引重建和分区性能
- **扩展支持：** Galera Cluster多节点复制测试，Kubernetes容器化部署验证

### 1.3 PostgreSQL 数据库测试方法

#### 测试类型
- **回归测试：** make check运行TAP格式测试框架，PostgreSQL自有测试套件覆盖SQL标准兼容
- **基准测试：** pgbench实施TPC-B OLTP工作负载，模拟银行交易场景
- **并发测试：** 多个psql客户端连接，测试MVCC和死锁检测
- **持久性测试：** WAL写前日志重放，PITR点-in-time恢复

#### 测试环境
- **工具链：** pgbench、pg_dump/pg_restore、psql客户端
- **配置优化：** auto-vacuum和autovacuum参数调优，共享缓冲区大小测试
- **扩展测试：** PostGIS地理空间查询，plpgsql存储过程
- **企业特性：** 流复制、逻辑复制、分区表并行查询

---

## 2. SQLCC 项目现有测试架构分析

### 2.1 当前测试结构
基于src/、tests/和CMakeLists.txt分析，SQLCC测试分层：

- **单位测试（tests/unit/）：**
  - 覆盖率90%+，包含b_plus_tree、buffer_pool、config_manager等核心模块
  - 使用Google Test框架，支持timeout和资源清理
  - 每个模块有enhanced_test.cc文件，重点测试边界条件和并发

- **性能测试（tests/performance/）：**
  - batch_prefetch、disk_io、index_constraint等专项测试
  - million_insert_test实现大规模数据导入测试
  - concurrency_test目录支持多线程压力验证

- **集成环境：**
  - isql命令端支持交互SQL执行，DBMS服务可用
  - scripts/analyze_performance_results.py数据分析工具
  - generate_performance_test_data.py自动生成测试数据

### 2.2 测试改进现状与问题
参考TESTING_IMPROVEMENTS_SUMMARY.md：

- **已获取成就：** +22个新测试用例，100%编译成功，类型安全修复
- **当前问题：** 缓冲池分段故障（页面替换算法），磁盘管理器并发同步不足
- **覆盖率：** 代码90%+行覆盖，函数100% - 但需扩展到崩溃recovery场景

### 2.3 与主流数据库对比差距
- **功能差距：** 缺乏pgbench-style基准、fuzzing自动化、集群测试
- **环境差距：** 无Docker集成、CI/CD自动化、跨平台验证
- **成熟度：** 需要从原型验证向生产就绪转变

---

## 3. SQLCC 真实场景测试分阶段实施计划

### 阶段划分原则
采用渐进式方法：先修复基础Bug，再扩展负载测试，最后达到企业级验证。每个阶段包含明确KPI、搭建内容、开发任务和具体步骤。

### 阶段1: 基础巩固与回归测试增强（1-2周）

#### 目标与KPI
- 修复所有分段故障，实现100%回归测试通过
- KPI: 0分段故障，磁盘并发测试通过率100%

#### 搭建内容
- 安装验证工具: lcov、gcovr、clang-format（基于Detected CLI Tools）
- 扩展CMake配置: build-coverage目录，支持debug模式编译
- 数据生成器增强: 基于generate_performance_test_data.py创建10-100万记录测试集

#### 开发任务
- 缓冲池算法修复: 分析page replacement逻辑，解决LRU实现缺陷
- 磁盘同步优化: 添加互斥锁和条件变量到disk_manager.cc
- 边界值扩展: 完善page_id边界处理和异常消息验证

#### 具体步骤
1. **环境验证（1天）**:
   - `cmake .. -DCMAKE_BUILD_TYPE=Debug && make`
   - `sudo apt install lcov gcovr` 若缺失

2. **核心Bug修复（3天）**:
   - gdb调试buffer_pool_enhanced_test.cc定位分段位置
   - 重构页面置换逻辑，添加边界检查
   - 运行存储引擎测试验证修复

3. **并发问题解决（3天）**:
   - 分析ConcurrentFileAccess测试失败原因
   - 实现读写锁到disk_manager.cc
   - 多线程（10并发）验证无race condition

4. **自动化脚本创建（3天）**:
   - `scripts/run_regression.sh`集成本地所有测试
   - 集成coverage报告生成（lcov + html输出）
   - 日志聚合分析errors和warnings

### 阶段2: 集成测试与性能基准建立（3-4周）

#### 目标与KPI
- isql客户端全功能集成，TPC-B风格基准运行成功
- KPI: 100TPS+并发负载，90%事务成功率

#### 搭建内容
- isql自动化框架: shell脚本序列执行SQL命令
- 基准环境配置: 适配banking场景到SQLCC（账户/转账表设计）
- 本地CI环境: Makefile扩展添加测试目标

#### 开发任务
- 混合负载测试: 扩展现有mixed_workload_test.cc支持60/40读写比例
- 事务一致性增强: 添加XA-style全球化事务测试
- 压力框架开发: 新建tests/stress/目录，包含内存/磁盘压力场景

#### 具体步骤
1. **isql脚本框架（1周）**:
   - 创建tests/integration/包含CRUD+JOIN用例
   - shell脚本: `./isql --batch < script.sql | grep ERROR`
   - 时间测量: 统计每个SQL响应时间

2. **基准测试实现（1周）**:
   - 复制performance_test_base.cc适配TPC-B逻辑
   - 10000账户初始数据，随机转账事务（100倍执行）
   - metrics收集: TPS、latency平均值

3. **高并发验证（1-2周）**:
   - 多进程isql客户端模拟生产负载
   - dead lock detector测试，WAL文件一致性检查
   - 资源监控: top命令记录CPU/Memory峰值

4. **结果交付（1天）**:
   - docs/INTEGRATION_TEST_REPORT.md记录负载结果
   - 快照备份测试环境到git分支

### 阶段3: 高级真实场景与持续优化（8-12周）

#### 目标与KPI
- 模糊测试无崩溃，TB级数据处理，崩溃恢复99.9%成功率
- KPI: 企业级准备报告，sysbench兼容基准

#### 搭建内容
- 模糊测试工具: 集成外部SQL生成器或自建
- 分布式模拟: Docker环境多容器SQLCC测试
- 监控体系: 扩展analyze_performance_results.py监控系统资源

#### 开发任务
- 模糊测试框架: tests/fuzz/目录运行随机查询检测漏洞
- 大规模数据测试: 扩展到1B+记录，索引性能优化
- 企业特性: 备份/恢复、只读副本、多租户隔离

#### 具体步骤
1. **模糊测试搭建（2周）**:
   - 下载开源SQL fuzzer修改适配SQLCC语法
   - tests/fuzz/sql_fuzz_test.cc: 运行1天随机查询
   - 解析器bug修复（目标: 0崩溃）

2. **崩溃恢复深入（3周）**:
   - WAL故障注入: 断电模拟、进程杀掉测试
   - PITR脚本: 任意时间点恢复功能
   - ACID验证: 原子性、一致性测试框架

3. **大规模性能调优（3周）**:
   - 100GB数据集生成和加载
   - index构建时间基准（对比优化前后）
   - sysbench适配: read-only、read-write、update-index工作负载

4. **企业就绪验证（4周）**:
   - Docker-compose.yml多节点集群
   - docs/ENTERPRISE_READINESS.md评估报告
   - 代码重构确保Linux/Windows/Mac兼容

---

## 4. 实施指南与注意事项

### 资源需求与时间线
- **人力配置:** 阶段1-1人，阶段2-1-2人，阶段3-开发+测试团队
- **硬件要求:** 测试服务器16GB RAM以上，SSD存储，网络稳定
- **时间管控:** 每周状态会议，每阶段末里程碑评审

### 风险管理策略
- **代码安全:** 每天备份，git分支隔离新功能
- **回滚计划:** 每次提交前运行全套回归测试
- **依赖管理:** 记录所有CLI工具版本，避免环境漂移

### 成功指标与监控
- **量化指标:** 分段故障数（阶段1:0），并发TPS（阶段2:100+），恢复率（阶段3:99.9%）
- **质量门禁:** 仅在所有回归通过后晋级阶段
- **持续改进:** 每阶段后更新此文档，反馈循环优化

### 技术栈一致性
- **构建系统:** CMake支持，保持与现有项目一致
- **脚本语言:** Bash/Python首选，方便跨平台
- **文档化:** Markdown格式，集成到README技术栈

此计划将SQLCC打造成具备企业级测试成熟度的数据库系统，为生产部署奠定坚实基础。后续迭代可根据实际反馈调整优先级。
