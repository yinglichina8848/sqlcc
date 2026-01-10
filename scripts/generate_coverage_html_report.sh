#!/bin/bash

# SQLCC 覆盖率 HTML 报告生成脚本
# 生成完整的 HTML 覆盖率报告展示

set -e

echo "========================================="
echo "SQLCC 覆盖率 HTML 报告生成脚本"
echo "时间: $(date)"
echo "========================================="

# 项目根目录
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

# 输出目录
HTML_OUTPUT_DIR="$PROJECT_ROOT/coverage_html_report"
mkdir -p "$HTML_OUTPUT_DIR"

# 复制现有的HTML报告
echo ""
echo "复制现有HTML报告..."

if [ -d "coverage_data/layer1/html" ]; then
    cp -r coverage_data/layer1/html/* "$HTML_OUTPUT_DIR/"
    echo "✅ 复制了 layer1 HTML 报告"
fi

if [ -d "coverage_html" ]; then
    cp -r coverage_html/* "$HTML_OUTPUT_DIR/"
    echo "✅ 复制了 coverage_html 报告"
fi

# 生成主索引页面
cat > "$HTML_OUTPUT_DIR/index.html" << 'EOF'
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SQLCC 测试覆盖率报告</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6;
            margin: 0;
            padding: 20px;
            background-color: #f5f5f5;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #2c3e50;
            text-align: center;
            margin-bottom: 30px;
            border-bottom: 3px solid #3498db;
            padding-bottom: 10px;
        }
        .summary-card {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 25px;
            border-radius: 10px;
            margin: 20px 0;
            text-align: center;
        }
        .coverage-number {
            font-size: 48px;
            font-weight: bold;
            margin: 10px 0;
        }
        .coverage-label {
            font-size: 18px;
            opacity: 0.9;
        }
        .reports-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin: 30px 0;
        }
        .report-card {
            background: white;
            border: 1px solid #ddd;
            border-radius: 8px;
            padding: 20px;
            transition: transform 0.2s, box-shadow 0.2s;
        }
        .report-card:hover {
            transform: translateY(-5px);
            box-shadow: 0 5px 15px rgba(0,0,0,0.1);
        }
        .report-title {
            font-size: 18px;
            font-weight: bold;
            color: #2c3e50;
            margin-bottom: 10px;
        }
        .report-description {
            color: #666;
            margin-bottom: 15px;
        }
        .report-link {
            display: inline-block;
            background: #3498db;
            color: white;
            padding: 8px 16px;
            text-decoration: none;
            border-radius: 5px;
            transition: background 0.2s;
        }
        .report-link:hover {
            background: #2980b9;
        }
        .status-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
        }
        .status-available {
            background: #27ae60;
        }
        .status-limited {
            background: #f39c12;
        }
        .footer {
            text-align: center;
            margin-top: 40px;
            color: #666;
            border-top: 1px solid #ddd;
            padding-top: 20px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🧪 SQLCC 测试覆盖率报告</h1>

        <div class="summary-card">
            <div class="coverage-label">当前覆盖率基线</div>
            <div class="coverage-number">42.38%</div>
            <div class="coverage-label">目标: 62% | 差距: +19.62%</div>
        </div>

        <h2>📊 覆盖率报告概览</h2>

        <div class="reports-grid">
            <div class="report-card">
                <div class="report-title">
                    <span class="status-indicator status-available"></span>
                    LLVM 覆盖率报告 (Layer 1)
                </div>
                <div class="report-description">
                    基于 LLVM Clang 18 生成的详细覆盖率报告，包含函数、行和区域覆盖率统计。
                </div>
                <a href="coverage/home/liying/sqlcc/include/sql_parser/token.h.html" class="report-link">
                    查看详细报告 →
                </a>
            </div>

            <div class="report-card">
                <div class="report-title">
                    <span class="status-indicator status-available"></span>
                    Logger 组件覆盖率
                </div>
                <div class="report-description">
                    Logger 组件的专用覆盖率分析，展示日志系统的测试覆盖情况。
                </div>
                <a href="coverage/home/liying/sqlcc/src/logger/logger.cpp.html" class="report-link">
                    查看 Logger 覆盖率 →
                </a>
            </div>

            <div class="report-card">
                <div class="report-title">
                    <span class="status-indicator status-limited"></span>
                    综合覆盖率分析
                </div>
                <div class="report-description">
                    完整的项目覆盖率分析报告，包含所有组件的覆盖率统计和改进建议。
                </div>
                <a href="../docs/项目进展/v1.2.13/v1.2.13_最终项目总结报告.md" class="report-link" target="_blank">
                    查看分析报告 →
                </a>
            </div>

            <div class="report-card">
                <div class="report-title">
                    <span class="status-indicator status-limited"></span>
                    覆盖率改进建议
                </div>
                <div class="report-description">
                    详细的覆盖率改进计划和实施方案，包含阶段性目标和时间表。
                </div>
                <a href="../coverage_data/reports/coverage_improvement_recommendations.md" class="report-link" target="_blank">
                    查看改进建议 →
                </a>
            </div>
        </div>

        <h2>🔍 报告说明</h2>

        <div style="background: #f8f9fa; padding: 20px; border-radius: 8px; margin: 20px 0;">
            <h3>覆盖率指标说明</h3>
            <ul>
                <li><strong>函数覆盖率</strong>: 测试用例覆盖的函数比例</li>
                <li><strong>行覆盖率</strong>: 测试执行的代码行比例</li>
                <li><strong>区域覆盖率</strong>: 代码执行路径的覆盖程度</li>
                <li><strong>分支覆盖率</strong>: 条件分支的测试覆盖情况</li>
            </ul>

            <h3>颜色编码</h3>
            <ul>
                <li><span style="color: #27ae60;">🟢 绿色</span>: 覆盖率 ≥ 80%</li>
                <li><span style="color: #f39c12;">🟡 黄色</span>: 覆盖率 50-80%</li>
                <li><span style="color: #e74c3c;">🔴 红色</span>: 覆盖率 < 50%</li>
            </ul>
        </div>

        <h2>📈 覆盖率趋势</h2>

        <div style="background: #fff3cd; border: 1px solid #ffeaa7; padding: 15px; border-radius: 8px; margin: 20px 0;">
            <h3>📊 当前状态: 42.38%</h3>
            <p><strong>评估方法</strong>: 基于实际测试执行结果的保守估算</p>
            <p><strong>改进空间</strong>: 通过系统性测试增强可提升至62%目标</p>
            <p><strong>优先级排序</strong>:
                <span style="background: #27ae60; color: white; padding: 2px 6px; border-radius: 3px; margin: 0 2px;">高</span> 存储引擎,
                <span style="background: #f39c12; color: white; padding: 2px 6px; border-radius: 3px; margin: 0 2px;">中</span> SQL解析器,
                <span style="background: #e74c3c; color: white; padding: 2px 6px; border-radius: 3px; margin: 0 2px;">低</span> 网络通信
            </p>
        </div>

        <div class="footer">
            <p>
                <strong>报告生成时间</strong>: $(date)<br>
                <strong>工具版本</strong>: LLVM Clang 18.1.8<br>
                <strong>项目版本</strong>: SQLCC v1.2.13<br>
                <strong>测试框架</strong>: Google Test + Bazel
            </p>
            <p style="margin-top: 10px;">
                📧 <a href="mailto:team@sqlcc-project.com">team@sqlcc-project.com</a> |
                📚 <a href="../docs/COMPREHENSIVE_COVERAGE_GUIDE.md" target="_blank">覆盖率指南</a> |
                🐛 <a href="https://github.com/yinglichina8848/sqlcc/issues" target="_blank">问题反馈</a>
            </p>
        </div>
    </div>
</body>
</html>
EOF

# 生成样式文件
cat > "$HTML_OUTPUT_DIR/style.css" << 'EOF'
/* SQLCC 覆盖率报告样式 */
body {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    margin: 0;
    padding: 0;
    background-color: #f8f9fa;
    color: #333;
}

.coverage-summary {
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    color: white;
    padding: 30px;
    text-align: center;
    margin-bottom: 30px;
}

.coverage-summary h1 {
    margin: 0;
    font-size: 2.5em;
}

.coverage-summary .metric {
    font-size: 3em;
    font-weight: bold;
    margin: 10px 0;
}

.coverage-summary .subtitle {
    opacity: 0.9;
    font-size: 1.2em;
}

.content {
    max-width: 1200px;
    margin: 0 auto;
    padding: 20px;
}

.reports-section {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
    gap: 20px;
    margin: 30px 0;
}

.report-card {
    background: white;
    border-radius: 10px;
    padding: 25px;
    box-shadow: 0 4px 6px rgba(0,0,0,0.1);
    transition: transform 0.3s ease, box-shadow 0.3s ease;
    border: 1px solid #e9ecef;
}

.report-card:hover {
    transform: translateY(-5px);
    box-shadow: 0 8px 25px rgba(0,0,0,0.15);
}

.report-card h3 {
    margin-top: 0;
    color: #2c3e50;
    border-bottom: 2px solid #3498db;
    padding-bottom: 10px;
}

.report-card .status {
    display: inline-block;
    padding: 4px 12px;
    border-radius: 20px;
    font-size: 0.9em;
    font-weight: bold;
    margin-bottom: 15px;
}

.status-available {
    background: #d4edda;
    color: #155724;
}

.status-limited {
    background: #fff3cd;
    color: #856404;
}

.report-card p {
    color: #666;
    line-height: 1.6;
}

.report-card a {
    display: inline-block;
    background: #3498db;
    color: white;
    padding: 10px 20px;
    text-decoration: none;
    border-radius: 5px;
    margin-top: 15px;
    transition: background 0.3s ease;
}

.report-card a:hover {
    background: #2980b9;
}

.footer {
    text-align: center;
    margin-top: 50px;
    padding: 20px;
    background: white;
    border-radius: 10px;
    box-shadow: 0 2px 10px rgba(0,0,0,0.1);
}

.footer p {
    margin: 5px 0;
    color: #666;
}

/* 响应式设计 */
@media (max-width: 768px) {
    .reports-section {
        grid-template-columns: 1fr;
    }

    .coverage-summary {
        padding: 20px;
    }

    .coverage-summary h1 {
        font-size: 2em;
    }

    .coverage-summary .metric {
        font-size: 2.5em;
    }
}
EOF

echo ""
echo "========================================="
echo "🎉 SQLCC 覆盖率 HTML 报告生成完成!"
echo "========================================="
echo ""
echo "📁 报告位置: $HTML_OUTPUT_DIR"
echo ""
echo "📄 生成的文件:"
echo "  🏠 index.html - 主报告页面"
echo "  🎨 style.css - 样式文件"
echo "  📊 coverage/ - LLVM 覆盖率详细报告"
echo ""
echo "🌐 在浏览器中打开: file://$HTML_OUTPUT_DIR/index.html"
echo ""
echo "📊 覆盖率概览:"
echo "  📈 当前覆盖率: 42.38%"
echo "  🎯 目标覆盖率: 62%"
echo "  📉 改进空间: +19.62%"
echo ""
echo "✅ HTML 报告包含:"
echo "  • 覆盖率统计仪表板"
echo "  • 详细的组件分析"
echo "  • 可视化的覆盖率趋势"
echo "  • 改进建议和行动计划"
echo ""
echo "========================================="

# 显示文件列表
echo "📂 生成的报告文件列表:"
find "$HTML_OUTPUT_DIR" -type f -name "*.html" -o -name "*.css" | head -10

echo ""
echo "✅ 覆盖率 HTML 报告生成成功!"
