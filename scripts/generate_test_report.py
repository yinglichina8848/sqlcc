#!/usr/bin/env python3
"""
SQLCC测试报告生成脚本

该脚本用于生成SQLCC项目的综合测试报告，包括：
- 单元测试结果
- 集成测试结果
- 性能测试结果
- 代码覆盖率数据

使用方法：
    python generate_test_report.py --results-dir /path/to/results --output /path/to/output.html
"""

import os
import sys
import argparse
import json
import re
from datetime import datetime


def parse_args():
    """解析命令行参数"""
    parser = argparse.ArgumentParser(description='生成SQLCC测试报告')
    parser.add_argument('--results-dir', required=True, help='测试结果目录')
    parser.add_argument('--output', required=True, help='输出HTML报告路径')
    return parser.parse_args()


def collect_test_results(results_dir):
    """收集测试结果"""
    test_results = {
        'unit_tests': [],
        'integration_tests': [],
        'performance_tests': [],
        'client_server_tests': []
    }
    
    # 收集单元测试结果（从CTest输出）
    ctest_output = os.path.join(results_dir, 'Testing', 'Temporary', 'LastTest.log')
    if os.path.exists(ctest_output):
        with open(ctest_output, 'r') as f:
            content = f.read()
        
        # 解析CTest输出
        test_cases = re.findall(r'(Test case "([^"]+)"[^\n]+\n[^\n]+\n  Result: ([^\n]+))', content)
        for test_case in test_cases:
            full_match, test_name, result = test_case
            
            # 根据测试名称分类
            if 'client_server' in test_name.lower():
                test_results['client_server_tests'].append({
                    'name': test_name,
                    'status': 'PASSED' if 'Passed' in result else 'FAILED',
                    'output': full_match
                })
            elif 'integration' in test_name.lower():
                test_results['integration_tests'].append({
                    'name': test_name,
                    'status': 'PASSED' if 'Passed' in result else 'FAILED',
                    'output': full_match
                })
            elif 'performance' in test_name.lower():
                test_results['performance_tests'].append({
                    'name': test_name,
                    'status': 'PASSED' if 'Passed' in result else 'FAILED',
                    'output': full_match
                })
            else:
                test_results['unit_tests'].append({
                    'name': test_name,
                    'status': 'PASSED' if 'Passed' in result else 'FAILED',
                    'output': full_match
                })
    
    # 收集集成测试结果
    comprehensive_test_log = os.path.join(results_dir, 'test_reports', 'comprehensive_test.log')
    if os.path.exists(comprehensive_test_log):
        with open(comprehensive_test_log, 'r') as f:
            content = f.read()
        test_results['integration_tests'].append({
            'name': 'comprehensive_test',
            'status': 'PASSED' if 'FAILED' not in content else 'FAILED',
            'output': content
        })
    
    # 收集性能测试结果
    performance_logs_dir = os.path.join(results_dir, 'test_reports', 'performance')
    if os.path.exists(performance_logs_dir):
        for log_file in os.listdir(performance_logs_dir):
            if log_file.endswith('.log'):
                log_path = os.path.join(performance_logs_dir, log_file)
                with open(log_path, 'r') as f:
                    content = f.read()
                test_results['performance_tests'].append({
                    'name': os.path.splitext(log_file)[0],
                    'status': 'PASSED' if 'FAILED' not in content else 'FAILED',
                    'output': content
                })
    
    return test_results


def collect_coverage_data(results_dir):
    """收集覆盖率数据"""
    coverage_data = {
        'total_coverage': '0.00%',
        'files': []
    }
    
    # 从lcov输出中提取覆盖率数据
    coverage_info = os.path.join(results_dir, 'coverage', 'coverage_clean.info')
    if os.path.exists(coverage_info):
        with open(coverage_info, 'r') as f:
            content = f.read()
        
        # 提取总覆盖率
        total_match = re.search(r'Total:\s+(\d+\.\d+)%', content)
        if total_match:
            coverage_data['total_coverage'] = total_match.group(1) + '%'
        
        # 提取文件覆盖率
        file_matches = re.findall(r'SF:(.+?)\n.*?LH:(\d+).*?LF:(\d+)', content, re.DOTALL)
        for file_match in file_matches:
            file_path, covered_lines, total_lines = file_match
            if total_lines != '0':
                coverage = (int(covered_lines) / int(total_lines)) * 100
                coverage_data['files'].append({
                    'path': file_path,
                    'coverage': f'{coverage:.2f}%',
                    'covered_lines': covered_lines,
                    'total_lines': total_lines
                })
    
    return coverage_data


def generate_html_report(test_results, coverage_data, output_path):
    """生成HTML报告"""
    # 计算测试统计信息
    total_unit_tests = len(test_results['unit_tests'])
    passed_unit_tests = sum(1 for test in test_results['unit_tests'] if test['status'] == 'PASSED')
    
    total_integration_tests = len(test_results['integration_tests'])
    passed_integration_tests = sum(1 for test in test_results['integration_tests'] if test['status'] == 'PASSED')
    
    total_performance_tests = len(test_results['performance_tests'])
    passed_performance_tests = sum(1 for test in test_results['performance_tests'] if test['status'] == 'PASSED')
    
    total_client_server_tests = len(test_results['client_server_tests'])
    passed_client_server_tests = sum(1 for test in test_results['client_server_tests'] if test['status'] == 'PASSED')
    
    total_tests = total_unit_tests + total_integration_tests + total_performance_tests + total_client_server_tests
    passed_tests = passed_unit_tests + passed_integration_tests + passed_performance_tests + passed_client_server_tests
    
    if total_tests > 0:
        pass_rate = (passed_tests / total_tests) * 100
    else:
        pass_rate = 0.0
    
    # HTML模板
    html_template = """
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SQLCC测试报告</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: Arial, sans-serif;
            background-color: #f5f5f5;
            color: #333;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
        
        h1 {
            color: #2c3e50;
            margin-bottom: 20px;
            text-align: center;
        }
        
        .summary {
            background-color: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
            margin-bottom: 30px;
        }
        
        .summary h2 {
            color: #3498db;
            margin-bottom: 15px;
        }
        
        .summary-stats {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }
        
        .stat-card {
            background-color: #f8f9fa;
            padding: 15px;
            border-radius: 6px;
            text-align: center;
        }
        
        .stat-card h3 {
            color: #6c757d;
            font-size: 14px;
            margin-bottom: 10px;
        }
        
        .stat-card .value {
            font-size: 24px;
            font-weight: bold;
        }
        
        .stat-card.passed .value {
            color: #28a745;
        }
        
        .stat-card.failed .value {
            color: #dc3545;
        }
        
        .stat-card.total .value {
            color: #007bff;
        }
        
        .stat-card.rate .value {
            color: #ffc107;
        }
        
        .section {
            background-color: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
            margin-bottom: 30px;
        }
        
        .section h2 {
            color: #3498db;
            margin-bottom: 15px;
            border-bottom: 2px solid #e9ecef;
            padding-bottom: 10px;
        }
        
        .test-results {
            margin-top: 20px;
        }
        
        .test-category {
            margin-bottom: 30px;
        }
        
        .test-category h3 {
            color: #6c757d;
            margin-bottom: 15px;
        }
        
        .test-table {
            width: 100%;
            border-collapse: collapse;
        }
        
        .test-table th,
        .test-table td {
            padding: 12px;
            text-align: left;
            border-bottom: 1px solid #e9ecef;
        }
        
        .test-table th {
            background-color: #f8f9fa;
            font-weight: bold;
            color: #495057;
        }
        
        .test-table tr:hover {
            background-color: #f8f9fa;
        }
        
        .status {
            padding: 4px 8px;
            border-radius: 4px;
            font-size: 12px;
            font-weight: bold;
        }
        
        .status.passed {
            background-color: #d4edda;
            color: #155724;
        }
        
        .status.failed {
            background-color: #f8d7da;
            color: #721c24;
        }
        
        .coverage-info {
            margin-top: 20px;
        }
        
        .coverage-summary {
            background-color: #e3f2fd;
            padding: 15px;
            border-radius: 6px;
            margin-bottom: 20px;
        }
        
        .coverage-summary h3 {
            color: #1565c0;
            margin-bottom: 10px;
        }
        
        .coverage-files {
            margin-top: 20px;
        }
        
        .coverage-files table {
            width: 100%;
            border-collapse: collapse;
        }
        
        .coverage-files th,
        .coverage-files td {
            padding: 10px;
            text-align: left;
            border-bottom: 1px solid #e9ecef;
        }
        
        .coverage-files th {
            background-color: #f8f9fa;
            font-weight: bold;
        }
        
        .footer {
            text-align: center;
            color: #6c757d;
            font-size: 14px;
            margin-top: 50px;
            padding-top: 20px;
            border-top: 1px solid #e9ecef;
        }
        
        .timestamp {
            color: #6c757d;
            font-size: 14px;
            text-align: center;
            margin-bottom: 20px;
        }
        
        .toggle-button {
            background-color: #007bff;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            cursor: pointer;
            margin-bottom: 10px;
        }
        
        .toggle-button:hover {
            background-color: #0056b3;
        }
        
        .test-output {
            max-height: 200px;
            overflow-y: auto;
            background-color: #f8f9fa;
            padding: 10px;
            border-radius: 4px;
            font-family: monospace;
            font-size: 12px;
            margin-top: 5px;
            display: none;
        }
        
        .test-output.visible {
            display: block;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>SQLCC测试报告</h1>
        <div class="timestamp">生成时间：{{ timestamp }}</div>
        
        <div class="summary">
            <h2>测试摘要</h2>
            <div class="summary-stats">
                <div class="stat-card total">
                    <h3>总测试数</h3>
                    <div class="value">{{ total_tests }}</div>
                </div>
                <div class="stat-card passed">
                    <h3>通过测试数</h3>
                    <div class="value">{{ passed_tests }}</div>
                </div>
                <div class="stat-card failed">
                    <h3>失败测试数</h3>
                    <div class="value">{{ failed_tests }}</div>
                </div>
                <div class="stat-card rate">
                    <h3>通过率</h3>
                    <div class="value">{{ pass_rate }}%</div>
                </div>
                <div class="stat-card">
                    <h3>代码覆盖率</h3>
                    <div class="value">{{ coverage_rate }}</div>
                </div>
            </div>
        </div>
        
        <div class="section">
            <h2>测试结果详情</h2>
            
            <div class="test-results">
                {% if unit_tests %}
                <div class="test-category">
                    <h3>单元测试</h3>
                    <table class="test-table">
                        <thead>
                            <tr>
                                <th>测试名称</th>
                                <th>状态</th>
                                <th>操作</th>
                            </tr>
                        </thead>
                        <tbody>
                            {% for test in unit_tests %}
                            <tr>
                                <td>{{ test.name }}</td>
                                <td><span class="status {{ 'passed' if test.status == 'PASSED' else 'failed' }}">{{ test.status }}</span></td>
                                <td>
                                    <button class="toggle-button" onclick="toggleOutput(this)">查看输出</button>
                                    <div class="test-output">{{ test.output }}</div>
                                </td>
                            </tr>
                            {% endfor %}
                        </tbody>
                    </table>
                </div>
                {% endif %}
                
                {% if integration_tests %}
                <div class="test-category">
                    <h3>集成测试</h3>
                    <table class="test-table">
                        <thead>
                            <tr>
                                <th>测试名称</th>
                                <th>状态</th>
                                <th>操作</th>
                            </tr>
                        </thead>
                        <tbody>
                            {% for test in integration_tests %}
                            <tr>
                                <td>{{ test.name }}</td>
                                <td><span class="status {{ 'passed' if test.status == 'PASSED' else 'failed' }}">{{ test.status }}</span></td>
                                <td>
                                    <button class="toggle-button" onclick="toggleOutput(this)">查看输出</button>
                                    <div class="test-output">{{ test.output }}</div>
                                </td>
                            </tr>
                            {% endfor %}
                        </tbody>
                    </table>
                </div>
                {% endif %}
                
                {% if performance_tests %}
                <div class="test-category">
                    <h3>性能测试</h3>
                    <table class="test-table">
                        <thead>
                            <tr>
                                <th>测试名称</th>
                                <th>状态</th>
                                <th>操作</th>
                            </tr>
                        </thead>
                        <tbody>
                            {% for test in performance_tests %}
                            <tr>
                                <td>{{ test.name }}</td>
                                <td><span class="status {{ 'passed' if test.status == 'PASSED' else 'failed' }}">{{ test.status }}</span></td>
                                <td>
                                    <button class="toggle-button" onclick="toggleOutput(this)">查看输出</button>
                                    <div class="test-output">{{ test.output }}</div>
                                </td>
                            </tr>
                            {% endfor %}
                        </tbody>
                    </table>
                </div>
                {% endif %}
            </div>
        </div>
        
        <div class="section">
            <h2>代码覆盖率报告</h2>
            <div class="coverage-info">
                <div class="coverage-summary">
                    <h3>覆盖率摘要</h3>
                    <p>总代码覆盖率：{{ coverage_rate }}</p>
                </div>
                
                {% if coverage_files %}
                <div class="coverage-files">
                    <h3>文件覆盖率详情</h3>
                    <table>
                        <thead>
                            <tr>
                                <th>文件路径</th>
                                <th>覆盖率</th>
                                <th>覆盖行数</th>
                                <th>总行数</th>
                            </tr>
                        </thead>
                        <tbody>
                            {% for file in coverage_files %}
                            <tr>
                                <td>{{ file.path }}</td>
                                <td>{{ file.coverage }}</td>
                                <td>{{ file.covered_lines }}</td>
                                <td>{{ file.total_lines }}</td>
                            </tr>
                            {% endfor %}
                        </tbody>
                    </table>
                </div>
                {% endif %}
                
                <p style="margin-top: 20px;">
                    <a href="../coverage/index.html" target="_blank">查看详细覆盖率报告</a>
                </p>
            </div>
        </div>
        
        <div class="footer">
            <p>SQLCC测试报告 - 自动生成</p>
        </div>
    </div>
    
    <script>
        function toggleOutput(button) {
            const outputDiv = button.nextElementSibling;
            outputDiv.classList.toggle('visible');
            button.textContent = outputDiv.classList.contains('visible') ? '隐藏输出' : '查看输出';
        }
    </script>
</body>
</html>
    """
    
    # 渲染模板
    rendered_html = html_template
    rendered_html = rendered_html.replace('{{ timestamp }}', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
    rendered_html = rendered_html.replace('{{ total_tests }}', str(total_tests))
    rendered_html = rendered_html.replace('{{ passed_tests }}', str(passed_tests))
    rendered_html = rendered_html.replace('{{ failed_tests }}', str(total_tests - passed_tests))
    rendered_html = rendered_html.replace('{{ pass_rate }}', f'{pass_rate:.2f}')
    rendered_html = rendered_html.replace('{{ coverage_rate }}', coverage_data['total_coverage'])
    
    # 渲染单元测试结果
    unit_tests_html = ''
    for test in test_results['unit_tests']:
        unit_tests_html += f'''
            <tr>
                <td>{test['name']}</td>
                <td><span class="status {'passed' if test['status'] == 'PASSED' else 'failed'}">{test['status']}</span></td>
                <td>
                    <button class="toggle-button" onclick="toggleOutput(this)">查看输出</button>
                    <div class="test-output">{test['output'].replace('\n', '<br>').replace(' ', '&nbsp;')}</div>
                </td>
            </tr>
        '''
    rendered_html = rendered_html.replace('{% for test in unit_tests %}{% endfor %}', unit_tests_html)
    
    # 渲染集成测试结果
    integration_tests_html = ''
    for test in test_results['integration_tests']:
        integration_tests_html += f'''
            <tr>
                <td>{test['name']}</td>
                <td><span class="status {'passed' if test['status'] == 'PASSED' else 'failed'}">{test['status']}</span></td>
                <td>
                    <button class="toggle-button" onclick="toggleOutput(this)">查看输出</button>
                    <div class="test-output">{test['output'].replace('\n', '<br>').replace(' ', '&nbsp;')}</div>
                </td>
            </tr>
        '''
    rendered_html = rendered_html.replace('{% for test in integration_tests %}{% endfor %}', integration_tests_html)
    
    # 渲染性能测试结果
    performance_tests_html = ''
    for test in test_results['performance_tests']:
        performance_tests_html += f'''
            <tr>
                <td>{test['name']}</td>
                <td><span class="status {'passed' if test['status'] == 'PASSED' else 'failed'}">{test['status']}</span></td>
                <td>
                    <button class="toggle-button" onclick="toggleOutput(this)">查看输出</button>
                    <div class="test-output">{test['output'].replace('\n', '<br>').replace(' ', '&nbsp;')}</div>
                </td>
            </tr>
        '''
    rendered_html = rendered_html.replace('{% for test in performance_tests %}{% endfor %}', performance_tests_html)
    
    # 渲染覆盖率文件
    coverage_files_html = ''
    for file in coverage_data['files'][:20]:  # 只显示前20个文件
        coverage_files_html += f'''
            <tr>
                <td>{file['path']}</td>
                <td>{file['coverage']}</td>
                <td>{file['covered_lines']}</td>
                <td>{file['total_lines']}</td>
            </tr>
        '''
    rendered_html = rendered_html.replace('{% for file in coverage_files %}{% endfor %}', coverage_files_html)
    
    # 写入输出文件
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(rendered_html)
    
    print(f"测试报告已生成：{output_path}")


def main():
    """主函数"""
    args = parse_args()
    
    # 确保结果目录存在
    if not os.path.exists(args.results_dir):
        print(f"错误：结果目录不存在：{args.results_dir}")
        sys.exit(1)
    
    # 收集测试结果
    print("收集测试结果...")
    test_results = collect_test_results(args.results_dir)
    
    # 收集覆盖率数据
    print("收集覆盖率数据...")
    coverage_data = collect_coverage_data(args.results_dir)
    
    # 生成HTML报告
    print("生成HTML报告...")
    generate_html_report(test_results, coverage_data, args.output)
    
    print("测试报告生成完成！")


if __name__ == '__main__':
    main()
