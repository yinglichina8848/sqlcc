# SQLCC 版本总览

## 📋 **版本列表**

| 版本号 | 发布日期 | 版本类型 | 简要描述 | ReleaseNote | ChangeLog |
|-------|---------|---------|---------|------------|----------|
| v1.0.9 | 2025-12-04 | SQL解析器架构重大更新 | 全新ParserNew架构，基于严格BNF语法设计，实现30%性能提升 | [查看](RELEASE_NOTES_v1.0.9.md) | [查看](CHANGELOG_v1.0.9.md) |
| v1.0.8 | 2025-12-03 | 复杂查询功能增强 | 完整的JOIN操作执行，支持INNER JOIN、LEFT JOIN、RIGHT JOIN、FULL JOIN、CROSS JOIN和NATURAL JOIN | [查看](RELEASE_NOTES_v1.0.8.md) | [查看](CHANGELOG_v1.0.8.md) |
| v1.0.7 | 2025-12-03 | 设计文档可视化升级 | 使用Mermaid diagrams替代源代码示例，提高文档可读性，实现分层架构流程图 | [查看](RELEASE_NOTES_v1.0.7.md) | [查看](CHANGELOG_v1.0.7.md) |
| v1.0.6 | 2025-12-03 | HAVING子句与索引优化 | HAVING子句完整实现，索引查询优化功能，支持等式查询和范围查询优化 | [查看](RELEASE_NOTES_v1.0.6.md) | [查看](CHANGELOG_v1.0.6.md) |
| v1.0.5 | 2025-12-02 | DDL/DML权限检查框架 | DDL/DML权限检查框架实现，DDL元数据同步框架，索引维护框架清晰化 | [查看](RELEASE_NOTES_v1.0.5.md) | [查看](CHANGELOG_v1.0.5.md) |
| v1.0.4 | 2025-12-02 | REVOKE权限功能与网络安全 | REVOKE权限撤销功能完整实现，UserManager与SystemDatabase集成，完整测试覆盖率报告 | [查看](RELEASE_NOTES_v1.0.4.md) | [查看](CHANGELOG_v1.0.4.md) |
| v1.0.3 | 2025-12-02 | 死锁修复与网络安全增强 | UserManager和TransactionManager死锁问题修复，HMAC-SHA256防篡改机制，PBKDF2密钥派生，TLS/SSL完整集成 | [查看](RELEASE_NOTES_v1.0.3.md) | [查看](CHANGELOG_v1.0.3.md) |
| v1.0.2 | 2025-11-30 | 项目编译与持久化验证 | 修复了StorageEngine和DatabaseManager类，成功编译了整个项目，验证了持久化功能的实现 | [查看](RELEASE_NOTES_v1.0.2.md) | [查看](CHANGELOG_v1.0.2.md) |
| v1.0.1 | 2025-11-28 | B+树大规模数据修复 | 修复了B+树大规模数据插入和删除时的无效页面ID问题，优化了B+树节点合并逻辑 | [查看](RELEASE_NOTES_v1.0.1.md) | [查看](CHANGELOG_v1.0.1.md) |
| v1.0.0 | 2025-11-25 | 首个正式版本发布 | 实现了完整的SQL数据库系统架构，支持基本的CRUD操作和事务管理，实现了索引系统和查询优化 | [查看](RELEASE_NOTES_v1.0.0.md) | [查看](CHANGELOG_v1.0.0.md) |
| v0.6.7 | 2025-11-29 | 测试覆盖率与SQL测试 | 更新覆盖率测试并添加dml_test，更新文档，版本号升至0.6.7 | [查看](RELEASE_NOTES_v0.6.7.md) | [查看](CHANGELOG_v0.6.7.md) |
| v0.6.6 | 2025-12-04 | 综合SQL测试脚本 | 实现了综合SQL测试脚本advanced_comprehensive_test.sql，开发了comprehensive_test.cpp测试程序 | [查看](RELEASE_NOTES_v0.6.6.md) | [查看](CHANGELOG_v0.6.6.md) |
| v0.6.5 | 2025-12-03 | 系统稳定性增强 | 增强系统稳定性和错误处理能力，完善网络通信模块的安全机制 | [查看](RELEASE_NOTES_v0.6.5.md) | [查看](CHANGELOG_v0.6.5.md) |
| v0.6.4 | 2025-12-02 | 测试框架与单元测试 | 实现了完整的测试框架和单元测试系统，开发了数据库连接测试和DCL操作测试用例 | [查看](RELEASE_NOTES_v0.6.4.md) | [查看](CHANGELOG_v0.6.4.md) |
| v0.6.3 | 2025-12-01 | 客户机-服务器融合架构 | 实现了客户机-服务器融合架构，支持本地和远程访问模式，设计了统一客户端接口 | [查看](RELEASE_NOTES_v0.6.3.md) | [查看](CHANGELOG_v0.6.3.md) |
| v0.6.2 | 2025-11-23 | 基本CRUD模拟功能 | 实现了基本的CRUD模拟功能，SQL执行器支持DML语句（INSERT、UPDATE、DELETE、SELECT） | [查看](RELEASE_NOTES_v0.6.2.md) | [查看](CHANGELOG_v0.6.2.md) |
| v0.6.1 | 2025-11-20 | 文档一致性优化 | 添加了更多测试用例，完善了约束检查机制，修复了若干bug | [查看](RELEASE_NOTES_v0.6.1.md) | [查看](CHANGELOG_v0.6.1.md) |
| v0.6.0 | 2025-11-15 | 约束检查和执行器 | 实现了约束检查和执行器，添加了基本的SQL执行框架 | [查看](RELEASE_NOTES_v0.6.0.md) | [查看](CHANGELOG_v0.6.0.md) |
| v0.5.8 | 2025-11-20 | 数据库原理教材体系构建 | 完成数据库原理教材体系全十章构建，文档系统优化 | [查看](RELEASE_NOTES_v0.5.8.md) | [查看](CHANGELOG_v0.5.8.md) |
| v0.5.7 | 2025-11-19 | 完整文档生态系统 | 完整文档生态系统和知识集成，添加了分布式协作开发指南 | [查看](RELEASE_NOTES_v0.5.7.md) | [查看](CHANGELOG_v0.5.7.md) |
| v0.5.6 | 2025-11-19 | 项目结构重构 | 综合项目结构重构，文档系统优化 | [查看](RELEASE_NOTES_v0.5.6.md) | [查看](CHANGELOG_v0.5.6.md) |
| v0.5.5 | 2025-11-19 | 性能优化与测试 | 索引与约束性能基准测试，大规模性能测试 | [查看](RELEASE_NOTES_v0.5.5.md) | [查看](CHANGELOG_v0.5.5.md) |
| v0.5.4 | 2025-11-19 | B+树索引系统 | 企业级B+树索引系统，B+树核心功能覆盖率达到90%+ | [查看](RELEASE_NOTES_v0.5.4.md) | [查看](CHANGELOG_v0.5.4.md) |
| v0.5.3 | 2025-11-19 | 企业性能测试套件 | 企业性能测试套件实现，性能基准测试 | [查看](RELEASE_NOTES_v0.5.3.md) | [查看](CHANGELOG_v0.5.3.md) |
| v0.5.2 | 2025-11-19 | 索引与约束性能测试 | 索引与约束性能基准测试，大规模性能测试 | [查看](RELEASE_NOTES_v0.5.2.md) | [查看](CHANGELOG_v0.5.2.md) |
| v0.5.1 | 2025-11-19 | 约束存储与性能测试 | 约束存储与大规模性能测试，企业数据完整性系统 | [查看](RELEASE_NOTES_v0.5.1.md) | [查看](CHANGELOG_v0.5.1.md) |
| v0.5.0 | 2025-11-10 | 事务管理器实现 | 实现了事务管理器，添加了并发控制机制，支持ACID事务属性 | [查看](RELEASE_NOTES_v0.5.0.md) | [查看](CHANGELOG_v0.5.0.md) |
| v0.4.8 | 2025-11-18 | 商业数据库设计 | 商业数据库设计思想完善与完整能力评估，WAL预写日志机制和事务管理完全实现 | [查看](RELEASE_NOTES_v0.4.8.md) | [查看](CHANGELOG_v0.4.8.md) |
| v0.4.7 | 2025-11-15 | BufferPool生产型重构 | BufferPool简化实现，从1200+行减少到500+行，分层锁架构，解决死锁问题 | [查看](RELEASE_NOTES_v0.4.7.md) | [查看](CHANGELOG_v0.4.7.md) |
| v0.4.5 | 2025-11-14 | CRUD功能增强 | CRUD功能完整实现，全面增强CRUD操作支持，包括INSERT、UPDATE、DELETE功能 | [查看](RELEASE_NOTES_v0.4.5.md) | [查看](CHANGELOG_v0.4.5.md) |
| v0.4.3 | 2025-11-13 | 测试结果与覆盖率分析 | 完善了测试结果报告，覆盖率分析增强，测试框架稳定性提升 | [查看](RELEASE_NOTES_v0.4.3.md) | [查看](CHANGELOG_v0.4.3.md) |
| v0.4.2 | 2025-11-13 | 配置管理器增强 | 为ConfigManager添加配置变更回调机制，支持注册和触发配置变更通知 | [查看](RELEASE_NOTES_v0.4.2.md) | [查看](CHANGELOG_v0.4.2.md) |
| v0.4.1 | 2025-11-12 | SQL解析器JOIN子句修复 | 在parseSelect方法中正确添加JOIN子句处理逻辑，移除导致段错误的无效代码 | [查看](RELEASE_NOTES_v0.4.1.md) | [查看](CHANGELOG_v0.4.1.md) |
| v0.4.0 | 2025-11-12 | BufferPool死锁修复 | 解决了在执行磁盘I/O操作时持有锁导致的死锁问题，并发性能优化 | [查看](RELEASE_NOTES_v0.4.0.md) | [查看](CHANGELOG_v0.4.0.md) |
| v0.3.9 | 2025-11-25 | BufferPool死锁修复 | 修复了BatchPrefetchPages和PrefetchPage方法中的死锁问题，在执行磁盘I/O前释放锁 | [查看](RELEASE_NOTES_v0.3.9.md) | [查看](CHANGELOG_v0.3.9.md) |
| v0.3.7 | 2025-11-12 | BufferPool页面ID分配修复 | 修复了DestructorFlushesPages测试失败问题，调整NewPage方法执行顺序 | [查看](RELEASE_NOTES_v0.3.7.md) | [查看](CHANGELOG_v0.3.7.md) |
| v0.3.6 | 2025-11-12 | 综合性能测试框架 | 达到400万ops/sec的高吞吐量性能，完成缓冲区池、磁盘I/O、存储引擎核心操作的性能基准测试 | [查看](RELEASE_NOTES_v0.3.6.md) | [查看](CHANGELOG_v0.3.6.md) |
| v0.3.5 | 2025-11-11 | 磁盘管理器测试修复 | 修复了DiskManager构造函数调用缺少ConfigManager参数的问题，统一使用page.GetData() | [查看](RELEASE_NOTES_v0.3.5.md) | [查看](CHANGELOG_v0.3.5.md) |
| v0.3.4 | 2025-11-11 | 版本记录管理规范化 | 建立完整的版本演进历史记录，版本号严格降序排列，版本描述格式统一 | [查看](RELEASE_NOTES_v0.3.4.md) | [查看](CHANGELOG_v0.3.4.md) |
| v0.3.3 | 2025-11-11 | 代码覆盖率深度分析 | 深度分析代码覆盖率，测试用例优化，覆盖率报告增强 | [查看](RELEASE_NOTES_v0.3.3.md) | [查看](CHANGELOG_v0.3.3.md) |
| v0.3.2 | 2025-11-11 | 性能测试框架完善 | 性能测试框架完善和优化，测试效率优化，性能指标收集增强 | [查看](RELEASE_NOTES_v0.3.2.md) | [查看](CHANGELOG_v0.3.2.md) |
| v0.3.1 | 2025-11-10 | 代码覆盖率分析 | 代码覆盖率深度分析和文档完善，测试报告生成 | [查看](RELEASE_NOTES_v0.3.1.md) | [查看](CHANGELOG_v0.3.1.md) |
| v0.3.0 | 2025-11-09 | 完整性能测试框架 | 完整性能测试框架初版，性能基准测试框架，吞吐量测试支持 | [查看](RELEASE_NOTES_v0.3.0.md) | [查看](CHANGELOG_v0.3.0.md) |
| v0.2.6 | 2025-11-08 | 自动化发布脚本 | 自动化发布脚本完善，发布流程标准化，文档自动生成和推送机制 | [查看](RELEASE_NOTES_v0.2.6.md) | [查看](CHANGELOG_v0.2.6.md) |
| v0.2.5 | 2025-11-07 | 代码覆盖率测试支持 | 代码覆盖率测试支持，覆盖率报告生成 (HTML格式)，CMake构建系统增强 | [查看](RELEASE_NOTES_v0.2.5.md) | [查看](CHANGELOG_v0.2.5.md) |
| v0.2.4 | 2025-11-07 | 文档版本统一 | 文档版本统一，更新所有文档的版本引用到v0.2.4，添加当前日期和时间标记 | [查看](RELEASE_NOTES_v0.2.4.md) | [查看](CHANGELOG_v0.2.4.md) |
| v0.2.3 | 2025-06-18 | 设计文档完善 | 设计文档完善，大幅增强storage_engine_design.md，添加详细的设计思想分析 | [查看](RELEASE_NOTES_v0.2.3.md) | [查看](CHANGELOG_v0.2.3.md) |
| v0.2.2 | 2025-11-05 | 文档分支管理系统 | 文档分支管理系统，创建了专门的docs分支用于管理Doxygen生成的API文档 | [查看](RELEASE_NOTES_v0.2.2.md) | [查看](CHANGELOG_v0.2.2.md) |
| v0.2.1 | 2025-11-05 | 文档分支管理系统 | 文档分支管理系统，完善的Doxygen API文档，详细的设计文档 | [查看](RELEASE_NOTES_v0.2.1.md) | [查看](CHANGELOG_v0.2.1.md) |
| v0.1.1 | 2025-11-05 | 基础存储引擎实现 | 基础存储引擎实现，页式存储管理，缓冲池和LRU替换策略，磁盘I/O管理 | [查看](RELEASE_NOTES_v0.1.1.md) | [查看](CHANGELOG_v0.1.1.md) |
| v0.0.1 | 2025-11-05 | 项目初始化 | 项目初始化，项目基础框架搭建，基本的配置管理功能，简单的命令行接口 | [查看](RELEASE_NOTES_v0.0.1.md) | [查看](CHANGELOG_v0.0.1.md) |

## 📖 **版本命名规则**

SQLCC采用语义化版本号（Semantic Versioning），格式为：MAJOR.MINOR.PATCH

- **MAJOR**: 当你做了不兼容的API修改
- **MINOR**: 当你添加了向下兼容的新功能
- **PATCH**: 当你做了向下兼容的bug修复

## 🚀 **版本升级指南**

- 对于MAJOR版本升级，请仔细阅读ReleaseNote，了解不兼容的API修改
- 对于MINOR版本升级，通常可以直接升级，不会影响现有功能
- 对于PATCH版本升级，建议及时升级，修复已知bug

## 📊 **版本统计**

- **总版本数**: 60+
- **第一个版本**: v0.0.1 (2025-11-05)
- **最新版本**: v1.0.9 (2025-12-04)
- **平均发布周期**: 约2-3天

## 📚 **版本管理文档**

- [版本摘要](VERSION_SUMMARY.md): 最简略的版本说明
- [版本详情](VERSION_DETAILS.md): 完整的版本信息，包括核心功能、改进和测试细节
- [发布流程](release_process.md): 自动化发布流程说明

## 🔗 **相关链接**

- [项目README](../../README.md): 项目概述和使用指南
- [技术文档索引](../guides/DOCUMENTATION_INDEX.md): 完整的技术文档索引
- [SQLCC主仓库](https://gitee.com/sqlcc/sqlcc): Gitee主仓库

---

**版本维护**: SQLCC团队  
**最后更新**: 2025-12-04