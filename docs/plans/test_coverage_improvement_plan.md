# SQLCC测试覆盖率提升计划

## 1. 现有测试覆盖率分析

### 1.1 存储引擎模块覆盖率

| 组件 | 覆盖率 | 状态 | 优先级 |
|------|--------|------|--------|
| b_plus_tree.cpp | 68.87% | 良好 | 中 |
| buffer_pool.cpp | 27.40% | 较差 | 高 |
| disk_manager.cpp | 31.82% | 较差 | 高 |
| storage_engine.cpp | 53.41% | 中等 | 中 |
| page.cpp | 未统计 | 未知 | 中 |

### 1.2 其他模块覆盖率

| 模块 | 状态 | 优先级 |
|------|------|--------|
| sql_parser | 未统计 | 中 |
| sql_executor | 未统计 | 高 |
| core | 未统计 | 高 |
| transaction_manager | 未统计 | 中 |
| network | 未统计 | 低 |

## 2. 测试目标

- **整体覆盖率目标**：80%+ 代码覆盖率
- **存储引擎模块**：90%+ 代码覆盖率
- **核心功能模块**：85%+ 代码覆盖率
- **边缘功能模块**：70%+ 代码覆盖率

## 3. 测试策略

### 3.1 分层测试策略

1. **单元测试**：针对单个类或函数的测试，重点测试边界条件和错误处理
2. **集成测试**：测试模块间的交互，确保模块协同工作正常
3. **端到端测试**：测试完整的SQL执行流程，验证系统整体功能

### 3.2 测试重点

1. **高优先级模块**：
   - buffer_pool：重点测试LRU替换、脏页管理、并发访问
   - disk_manager：重点测试页面读写、文件管理、错误处理
   - sql_executor：重点测试SQL执行逻辑、索引使用、事务处理
   - core：重点测试数据库管理、配置管理

2. **低覆盖率函数**：
   - buffer_pool：FindVictimPage、ReplacePage、AdjustBufferPoolSize
   - disk_manager：BatchReadPages、PrefetchPage、DeallocatePage
   - b_plus_tree：MergeNodes、SplitNode、DeleteKey

## 4. 具体测试计划

### 4.1 存储引擎模块

#### 4.1.1 BufferPool测试用例

| 测试用例 | 测试目标 | 优先级 | 状态 |
|----------|----------|--------|------|
| BufferPoolTest.NewPage | 测试新页面分配 | 高 | 已完成 |
| BufferPoolTest.FetchPage | 测试页面获取 | 高 | 已完成 |
| BufferPoolTest.LRUReplacement | 测试LRU页面替换 | 高 | 失败（锁超时） |
| BufferPoolTest.DirtyFlag | 测试脏页标记 | 高 | 已完成 |
| BufferPoolTest.FlushPage | 测试页面刷新 | 高 | 已完成 |
| BufferPoolTest.FlushAllPages | 测试所有页面刷新 | 高 | 已完成 |
| BufferPoolTest.DeletePage | 测试页面删除 | 中 | 待添加 |
| BufferPoolTest.PrefetchPage | 测试页面预取 | 中 | 待添加 |
| BufferPoolTest.BatchFetchPages | 测试批量页面获取 | 中 | 待添加 |
| BufferPoolTest.ConcurrentAccess | 测试并发访问 | 高 | 待添加 |
| BufferPoolTest.AdjustPoolSize | 测试动态调整池大小 | 中 | 待添加 |
| BufferPoolTest.Statistics | 测试统计信息收集 | 低 | 待添加 |

#### 4.1.2 DiskManager测试用例

| 测试用例 | 测试目标 | 优先级 | 状态 |
|----------|----------|--------|------|
| DiskManagerTest.AllocatePage | 测试页面分配 | 高 | 已完成 |
| DiskManagerTest.ReadPage | 测试页面读取 | 高 | 失败 |
| DiskManagerTest.WritePage | 测试页面写入 | 高 | 已完成 |
| DiskManagerTest.DeallocatePage | 测试页面释放 | 高 | 失败 |
| DiskManagerTest.FileSizeManagement | 测试文件大小管理 | 中 | 失败 |
| DiskManagerTest.PageContentConsistency | 测试页面内容一致性 | 高 | 已完成 |
| DiskManagerTest.BatchReadPages | 测试批量页面读取 | 中 | 待添加 |
| DiskManagerTest.PrefetchPage | 测试页面预取 | 中 | 待添加 |
| DiskManagerTest.Sync | 测试文件同步 | 中 | 待添加 |
| DiskManagerTest.IOStats | 测试I/O统计 | 低 | 待添加 |
| DiskManagerTest.ErrorHandling | 测试错误处理 | 高 | 待添加 |

#### 4.1.3 BPlusTree测试用例

| 测试用例 | 测试目标 | 优先级 | 状态 |
|----------|----------|--------|------|
| BPlusTreeSimpleTest.SingleInsertAndSearch | 测试单条插入和查询 | 高 | 已完成 |
| BPlusTreeSimpleTest.MultipleInsertsAndSearches | 测试多条插入和查询 | 高 | 已完成 |
| BPlusTreeSimpleTest.SearchRange | 测试范围查询 | 高 | 已完成 |
| BPlusTreeSimpleTest.Delete | 测试删除操作 | 高 | 已完成 |
| BPlusTreeSimpleTest.InsertUpdateDeletePattern | 测试插入更新删除模式 | 高 | 已完成 |
| BPlusTreeSimpleTest.EdgeCaseEmptyString | 测试空字符串处理 | 中 | 已完成 |
| BPlusTreeSimpleTest.EdgeCaseSpecialCharacters | 测试特殊字符处理 | 中 | 已完成 |
| BPlusTreeSimpleTest.BasicAPICompatibility | 测试API兼容性 | 中 | 已完成 |
| BPlusTreeSimpleTest.LargeScaleInsert | 测试大规模插入 | 中 | 已完成 |
| BPlusTreeSimpleTest.LargeScaleDelete | 测试大规模删除 | 中 | 已完成 |
| BPlusTreeSimpleTest.InsertDeleteBalance | 测试插入删除平衡 | 中 | 已完成 |
| BPlusTreeSimpleTest.EdgeCaseTreeHeight | 测试树高边界情况 | 中 | 已完成 |
| BPlusTreeMergeTest.InternalNodeMerge | 测试内部节点合并 | 高 | 失败 |
| BPlusTreeMergeTest.RootNodeSplit | 测试根节点分裂 | 高 | 失败 |
| BPlusTreeTest.MergeNodes | 测试节点合并 | 高 | 待添加 |
| BPlusTreeTest.SplitNode | 测试节点分裂 | 高 | 待添加 |
| BPlusTreeTest.DeleteKey | 测试键删除 | 高 | 待添加 |
| BPlusTreeTest.ConcurrentOperations | 测试并发操作 | 中 | 待添加 |

### 4.2 SQL执行器模块

| 测试用例 | 测试目标 | 优先级 | 状态 |
|----------|----------|--------|------|
| SQLExecutorTest.SelectStatement | 测试SELECT语句执行 | 高 | 待添加 |
| SQLExecutorTest.InsertStatement | 测试INSERT语句执行 | 高 | 待添加 |
| SQLExecutorTest.UpdateStatement | 测试UPDATE语句执行 | 高 | 待添加 |
| SQLExecutorTest.DeleteStatement | 测试DELETE语句执行 | 高 | 待添加 |
| SQLExecutorTest.JoinOperation | 测试JOIN操作 | 高 | 待添加 |
| SQLExecutorTest.IndexUsage | 测试索引使用 | 高 | 待添加 |
| SQLExecutorTest.AggregateFunctions | 测试聚合函数 | 中 | 待添加 |
| SQLExecutorTest.Subquery | 测试子查询 | 中 | 待添加 |
| SQLExecutorTest.ErrorHandling | 测试错误处理 | 高 | 待添加 |

### 4.3 核心模块

| 测试用例 | 测试目标 | 优先级 | 状态 |
|----------|----------|--------|------|
| CoreTest.DatabaseManager | 测试数据库管理 | 高 | 待添加 |
| CoreTest.ConfigManager | 测试配置管理 | 高 | 待添加 |
| CoreTest.Logger | 测试日志功能 | 中 | 待添加 |
| CoreTest.ExceptionHandling | 测试异常处理 | 高 | 待添加 |

## 5. 测试执行计划

### 5.1 阶段计划

| 阶段 | 时间 | 目标 |
|------|------|------|
| 阶段1 | 1-2天 | 修复现有测试用例，确保所有测试通过 |
| 阶段2 | 3-5天 | 补充存储引擎模块测试用例，达到90%+覆盖率 |
| 阶段3 | 5-7天 | 补充SQL执行器和核心模块测试用例 |
| 阶段4 | 1-2天 | 运行所有测试，生成覆盖率报告 |
| 阶段5 | 1天 | 分析覆盖率报告，补充遗漏的测试用例 |

### 5.2 覆盖率验证

1. 使用gcov/gcovr生成覆盖率报告
2. 分析覆盖率报告，识别低覆盖率区域
3. 针对低覆盖率区域补充测试用例
4. 重复步骤1-3，直到达到目标覆盖率

## 6. 测试工具和环境

- **测试框架**：Google Test
- **覆盖率工具**：gcov/gcovr
- **构建系统**：CMake
- **编译选项**：-fprofile-arcs -ftest-coverage

## 7. 预期成果

1. 所有现有测试用例通过
2. 存储引擎模块覆盖率达到90%+
3. 整体代码覆盖率达到80%+
4. 生成详细的覆盖率报告
5. 建立持续集成测试流程

## 8. 风险和挑战

1. **并发测试**：需要设计合理的并发测试用例，避免死锁和竞态条件
2. **边界条件**：需要测试各种边界情况，如空输入、大数据量、错误输入
3. **测试执行时间**：大规模测试可能需要较长时间，需要优化测试执行效率
4. **测试维护**：需要定期更新测试用例，确保与代码同步

## 9. 后续计划

1. 建立持续集成测试流程，自动运行测试和生成覆盖率报告
2. 为新功能添加测试用例，确保测试覆盖率持续保持在80%以上
3. 定期分析测试结果，优化测试用例
4. 探索性能测试和压力测试，确保系统在高负载下正常运行

---

**计划制定日期**：2025-11-28
**计划执行负责人**：开发团队
**预期完成日期**：2025-12-15