分析时间: 2026年 01月 12日 星期一 19:59:40 CST

# Bazel测试层次重构迁移计划

## 概述
基于Bazel BUILD.bazel文件的依赖关系分析，重新确定测试层次分类。

## 当前分析结果
- 分析BUILD文件数: 24
- 发现测试目标数: 166

## 层次分布
- 层次1 (level1_foundation (基础工具测试)): 13 个测试
- 层次2 (level2_core (核心组件测试)): 4 个测试
- 层次3 (level3_storage (存储引擎测试)): 27 个测试
- 层次4 (level4_parser (SQL解析器测试)): 48 个测试
- 层次5 (level5_execution (执行引擎测试)): 51 个测试
- 层次6 (level6_network (网络通信测试)): 8 个测试
- 层次7 (level7_enterprise (企业级特性测试)): 15 个测试
- 层次8 (level8_integration (系统集成测试)): 0 个测试

## 关键发现
### execution_context_test 分析
- 当前层次: 1 (unit/basic)
- 建议层次: 5 (level5_*)
- 依赖分析: //src/core:core
//src/core:database_manager
//src/sql_executor:sql_executor
- 结论: 需要迁移到层次5 ✅

## 迁移策略
1. **基于依赖的自动分类**: 使用依赖关系分析确定层次
2. **层次隔离**: 确保低层次测试不依赖高层次组件
3. **渐进迁移**: 分批次迁移，避免大面积修改

## 实施步骤
1. 备份当前BUILD.bazel配置
2. 创建新的层次目录结构
3. 按层次迁移cc_test目标
4. 更新依赖路径
5. 验证编译和测试执行

