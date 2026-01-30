# Temporary Files Directory

此目录用于存放临时文件和中间产物，保持主目录的整洁。

## 目录结构

```
temporary/
├── coverage_reports/       # 覆盖率报告和数据
├── html_reports/          # HTML报告文件
├── logs/                  # 日志文件
├── test_reports/          # 测试报告MD文件
├── test_results/          # 测试结果
└── coverage_report_l1_complete/  # Level 1完整覆盖率报告
```

## 说明

- **coverage_reports/**: 存放各种覆盖率测试生成的报告和数据
- **html_reports/**: 存放HTML格式的报告文件（包括覆盖率报告和可视化演示）
- **logs/**: 存放构建、测试和覆盖率相关的日志文件
- **test_reports/**: 存放测试相关的Markdown报告文件
- **test_results/**: 存放测试执行结果
- **coverage_report_l1_complete/**: Level 1 Foundation完整覆盖率报告

## 清理策略

这些文件都是临时生成的，可以定期清理：
- 定期清理旧的覆盖率报告
- 删除过期的日志文件
- 清理临时测试结果

**注意**: 请勿删除当前正在使用的文件。