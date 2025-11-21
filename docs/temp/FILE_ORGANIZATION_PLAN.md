# SQLCC项目文件整理计划

## 整理目标
对项目根目录、docs/、scripts/目录进行整理，建立清晰的文件分类结构，提高项目维护性。

## 当前问题
1. docs/目录下文件众多，缺乏分类
2. SQL脚本散落在根目录
3. 测试报告和性能分析文档分布混乱

## 整理方案

### 1. docs/目录结构改进
```
docs/
├── design/           # 设计文档
│   ├── SQL_PARSER_DESIGN.md
│   ├── storage_engine_design.md
│   ├── config_manager_design.md
│   ├── DESIGN_IMPROVEMENT_PLAN.md
│   ├── lock_mechanism_update.md
│   ├── single_node_enhancement_plan.md
│   └── distributed/
│       ├── api/
│       └── coordination/
├── testing/          # 测试相关
│   ├── TESTING_IMPROVEMENTS_SUMMARY.md
│   ├── TESTING_ENHANCEMENT_REPORT_v0.5.3.md
│   ├── TESTING_ENHANCEMENT_REPORT_v0.5.4.md
│   ├── TESTING_SUMMARY_REPORT.md
│   ├── unit_testing.md
│   ├── TEMPORARY_TEST_FILES.md
│   ├── SQLCC_REAL_WORLD_TESTING_PLAN.md
│   └── TODO/
│       ├── 单机功能增强计划.md
│       └── 网络功能增强.md
├── performance/      # 性能文档
│   ├── performance_optimization_recommendations.md
│   ├── performance_testing_guide.md
│   ├── performance_testing_plan.md
│   ├── performance_test_content_details.md
│   ├── performance_test_framework_design.md
│   ├── performance_test_report.md
│   ├── shard_vs_finegrained_lock_analysis.md
│   └── million_insert_performance_report.md
├── reports/          # 状态报告
│   ├── SQLCC_CAPABILITY_ASSESSMENT.md
│   ├── SQLCC_STATUS_REPORT_v0.5.5.md
│   ├── ACCURATE_COVERAGE_REPORT_v0.5.5.md
│   ├── COVERAGE_ASSESSMENT_AND_IMPROVEMENT_PLAN.md
│   ├── CONSTRAINT_IMPLEMENTATION_STATUS.md
│   ├── DOCUMENTATION_COMPLETENESS_REPORT.md
│   ├── SQLCC真实测试覆盖率分析报告.md
│   └── PHASE_II_SQL92_COMPLIANCE_TEST_REPORT.md
├── guides/           # 用户指南
│   ├── Guide.md
│   ├── DEVELOPMENT_GUIDE.md
│   ├── index_functionality_guide.md
│   ├── release_process.md
│   ├── DOCUMENTATION_INDEX.md
│   ├── GENERATED_DOCUMENTATION_GUIDE.md
│   ├── PROJECT_STRUCTURE.md
│   └── index.md
└── TODO.md          # 根级别的TODO仍保留在docs/
```

### 2. scripts/目录结构改进
```
scripts/
├── sql/              # SQL测试脚本
│   ├── test_script.sql
│   ├── crud_test_script.sql
│   ├── large_test_script.sql
│   ├── performance_test.sql
│   └── crud_performance_benchmark.sql
├── python/           # Python脚本
│   ├── analyze_performance_results.py
│   └── generate_performance_test_data.py
└── shell/            # Shell脚本
    ├── generate_docs.sh
    ├── plot_batch_prefetch_results.py  # 移到python目录
    ├── quick_release.sh
    ├── release_automation.sh
    ├── release.sh
    ├── run_performance_example.sh
    └── view_performance_results.sh
```

### 3. 根目录清理 (扩展规划)
将根目录的分散文件整理到相应位置：

#### 版本文档 (新分类)
- `RELEASE_NOTES_v0.*.md` -> `docs/releases/`
- `CHANGELOG.md` -> `docs/releases/`
- `VERSION_SUMMARY.md` -> `docs/releases/`
- `RELEASE.md` -> `docs/releases/`

#### 架构文档
- `Architecture.md` -> `docs/design/`
- `LOCKING_MECHANISM_ANALYSIS.md` -> `docs/design/`
- `TRANSACTION_IMPLEMENTATION_SUMMARY.md` -> `docs/design/`
- `Guide.md` -> `docs/guides/`

#### 临时/工作文档
- `TRAE-Chat.md` -> `docs/temp/` 或考虑删除
- `FILE_ORGANIZATION_PLAN.md` -> `docs/temp/` 或删除
- `REORGANIZATION_SUMMARY.md` -> `docs/temp/`
- `TASK_PROGRESS.md` -> 删除 (已完成任务)
- `test_report.md` -> `docs/reports/`

#### 项目文档
- `compilation_verification.md` -> `docs/development/`
- `SQLCC_COMPREHENSIVE_DESIGN_PLAN.md` -> `docs/design/`

## 执行计划

### 阶段1: 创建目录结构 (已完成)
1. 创建 docs 子目录
2. 创建 scripts 子目录

### 阶段2: 移动文档文件 (进行中)
1. 移动 docs/ 下文件到各级子目录 (已完成)
2. 创建新增的分类目录: releases/, development/, temp/
3. 移动根目录版本文档到 docs/releases/
4. 移动架构文档到 docs/design/
5. 清理临时文档

### 阶段3: 整理脚本和SQL文件 (已完成)
1. 移动 SQL 脚本到 scripts/sql/
2. 移动 Python 脚本到 scripts/python/
3. 整理 Shell 脚本到 scripts/shell/

### 阶段4: 根目录深度清理
1. 移动剩余相关文件到相应分类
2. 清理临时文件
3. 更新文档和脚本中的路径引用

### 阶段5: 最终验证
1. 检查文件是否正确移动
2. 更新所有受影响的文档链接
3. 运行测试确保功能正常
4. 生成最终整理报告

## 注意事项
- 需要检查文档中的相对链接是否需要更新
- 脚本中的文件路径需要相应调整
- 需要更新README等根文件中的文档引用
