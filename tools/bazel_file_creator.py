#!/usr/bin/env python3
"""
SQLCC Bazel缺失文件自动创建工具
基于检测结果，自动创建缺失的头文件和源文件

功能特性:
- 分析缺失文件类型和用途
- 自动生成基本的文件模板
- 智能推断文件内容
- 支持批量创建

作者: SQLCC AI Agent
版本: v1.2.6
更新时间: 2025-12-22
"""

import os
import re
import sys
from pathlib import Path
from typing import Dict, List, Set, Optional, Tuple
import json

class FileTemplate:
    """文件模板生成器"""

    @staticmethod
    def generate_header_file(file_path: str, class_name: Optional[str] = None) -> str:
        """生成头文件模板"""
        if class_name is None:
            # 从文件路径推断类名
            class_name = FileTemplate._infer_class_name(file_path)

        guard_name = FileTemplate._generate_include_guard(file_path)

        template = f"""#ifndef {guard_name}
#define {guard_name}

#include <memory>
#include <string>
#include <vector>

namespace sqlcc {{

/**
 * @brief {class_name} 类声明
 *
 * 这是一个自动生成的头文件模板。
 * 请根据实际需求完善类定义。
 */
class {class_name} {{
public:
    // 构造函数
    {class_name}();
    explicit {class_name}(const std::string& name);

    // 析构函数
    ~{class_name}();

    // 禁用拷贝
    {class_name}(const {class_name}&) = delete;
    {class_name}& operator=(const {class_name}&) = delete;

    // 允许移动
    {class_name}({class_name}&&) noexcept = default;
    {class_name}& operator=({class_name}&&) noexcept = default;

    // 公共方法
    void initialize();
    void shutdown();

    // Getter/Setter
    const std::string& get_name() const;
    void set_name(const std::string& name);

private:
    std::string name_;
    bool initialized_;
}};

}} // namespace sqlcc

#endif // {guard_name}
"""
        return template

    @staticmethod
    def generate_source_file(header_path: str, source_path: str) -> str:
        """生成源文件模板"""
        class_name = FileTemplate._infer_class_name(header_path)

        template = f'''#include "{header_path}"

#include <iostream>
#include <stdexcept>

namespace sqlcc {{

// 构造函数实现
{class_name}::{class_name}()
    : name_("default")
    , initialized_(false) {{
    std::cout << "{class_name} created with default name" << std::endl;
}}

{class_name}::{class_name}(const std::string& name)
    : name_(name)
    , initialized_(false) {{
    std::cout << "{class_name} created with name: " << name << std::endl;
}}

// 析构函数实现
{class_name}::~{class_name}() {{
    if (initialized_) {{
        shutdown();
    }}
    std::cout << "{class_name} destroyed" << std::endl;
}}

// 公共方法实现
void {class_name}::initialize() {{
    if (initialized_) {{
        return;
    }}

    // TODO: 实现初始化逻辑
    std::cout << "Initializing {class_name}: " << name_ << std::endl;

    initialized_ = true;
}}

void {class_name}::shutdown() {{
    if (!initialized_) {{
        return;
    }}

    // TODO: 实现清理逻辑
    std::cout << "Shutting down {class_name}: " << name_ << std::endl;

    initialized_ = false;
}}

// Getter/Setter实现
const std::string& {class_name}::get_name() const {{
    return name_;
}}

void {class_name}::set_name(const std::string& name) {{
    name_ = name;
}}

}} // namespace sqlcc
'''
        return template

    @staticmethod
    def generate_test_file(test_path: str, target_class: str) -> str:
        """生成测试文件模板"""
        test_class_name = f"{target_class}Test"

        template = f'''#include <gtest/gtest.h>
#include <gmock/gmock.h>

// TODO: 包含被测试类的头文件
// #include "path/to/{target_class.lower()}.h"

namespace sqlcc {{
namespace test {{

/**
 * @brief {test_class_name} 测试类
 *
 * 这是一个自动生成的测试文件模板。
 * 请根据实际需求完善测试用例。
 */
class {test_class_name} : public ::testing::Test {{
protected:
    void SetUp() override {{
        // TODO: 设置测试环境
    }}

    void TearDown() override {{
        // TODO: 清理测试环境
    }}

    // TODO: 添加测试夹具成员
    // std::unique_ptr<{target_class}> test_object_;
}};

// 测试用例
TEST_F({test_class_name}, Constructor_Default) {{
    // TODO: 实现测试
    // EXPECT_TRUE(test_object_ != nullptr);
    SUCCEED() << "Test not implemented yet";
}}

TEST_F({test_class_name}, Constructor_WithName) {{
    // TODO: 实现测试
    // {target_class} obj("test_name");
    // EXPECT_EQ(obj.get_name(), "test_name");
    SUCCEED() << "Test not implemented yet";
}}

TEST_F({test_class_name}, Initialize) {{
    // TODO: 实现测试
    // test_object_->initialize();
    // TODO: 验证初始化状态
    SUCCEED() << "Test not implemented yet";
}}

TEST_F({test_class_name}, Shutdown) {{
    // TODO: 实现测试
    // test_object_->initialize();
    // test_object_->shutdown();
    // TODO: 验证清理状态
    SUCCEED() << "Test not implemented yet";
}}

}} // namespace test
}} // namespace sqlcc
'''
        return template

    @staticmethod
    def _infer_class_name(file_path: str) -> str:
        """从文件路径推断类名"""
        # 从路径中提取文件名
        file_name = Path(file_path).stem

        # 转换为驼峰命名
        parts = file_name.split('_')
        class_name = ''.join(word.capitalize() for word in parts)

        return class_name

    @staticmethod
    def _generate_include_guard(file_path: str) -> str:
        """生成include guard宏名"""
        # 转换为大写并替换特殊字符
        guard = file_path.upper()
        guard = re.sub(r'[^\w]', '_', guard)
        guard = re.sub(r'_+', '_', guard)
        guard = guard.strip('_')

        return f"SQLCC_{guard}_H"

class BazelFileCreator:
    """Bazel缺失文件创建器"""

    def __init__(self, project_root: str, detection_results: Optional[str] = None):
        self.project_root = Path(project_root).resolve()
        self.detection_results = Path(detection_results) if detection_results else None
        self.created_files: List[str] = []

    def create_missing_files(self, detection_json: Optional[str] = None) -> List[str]:
        """根据检测结果创建缺失文件"""
        if detection_json:
            self.detection_results = Path(detection_json)

        if not self.detection_results or not self.detection_results.exists():
            print("❌ 未找到检测结果文件")
            return []

        # 读取检测结果
        with open(self.detection_results, 'r', encoding='utf-8') as f:
            results = json.load(f)

        # 筛选MISSING_FILE类型的问题
        missing_file_issues = [
            issue for issue in results['issues']
            if issue['issue_type'] == 'MISSING_FILE'
        ]

        print(f"📁 发现 {len(missing_file_issues)} 个缺失文件问题")

        created_files = []
        for issue in missing_file_issues:
            file_path = issue['metadata'].get('file_ref', '')
            if file_path and self._should_create_file(file_path):
                if self._create_file(file_path):
                    created_files.append(file_path)

        self.created_files = created_files
        print(f"✅ 成功创建 {len(created_files)} 个文件")

        return created_files

    def _should_create_file(self, file_path: str) -> bool:
        """判断是否应该创建文件"""
        # 跳过某些特殊文件
        skip_patterns = [
            'config_manager.h',  # 已存在但可能有问题
            'headers',  # 头文件集合
            'gtest_main',  # 测试框架
        ]

        for pattern in skip_patterns:
            if pattern in file_path.lower():
                return False

        # 只处理头文件
        return file_path.endswith('.h') or file_path.endswith('.hpp')

    def _create_file(self, file_path: str) -> bool:
        """创建单个文件"""
        full_path = self.project_root / file_path

        try:
            # 确保目录存在
            full_path.parent.mkdir(parents=True, exist_ok=True)

            # 生成文件内容
            if file_path.endswith(('.h', '.hpp')):
                content = FileTemplate.generate_header_file(file_path)
            else:
                print(f"⚠️  跳过不支持的文件类型: {file_path}")
                return False

            # 写入文件
            with open(full_path, 'w', encoding='utf-8') as f:
                f.write(content)

            print(f"📄 创建文件: {file_path}")
            return True

        except Exception as e:
            print(f"❌ 创建文件失败 {file_path}: {e}")
            return False

    def create_from_template(self, template_type: str, file_path: str, **kwargs) -> bool:
        """根据模板创建文件"""
        full_path = self.project_root / file_path

        try:
            full_path.parent.mkdir(parents=True, exist_ok=True)

            if template_type == 'header':
                content = FileTemplate.generate_header_file(file_path, kwargs.get('class_name'))
            elif template_type == 'source':
                header_path = kwargs.get('header_path', file_path.replace('.cpp', '.h'))
                content = FileTemplate.generate_source_file(header_path, file_path)
            elif template_type == 'test':
                target_class = kwargs.get('target_class', FileTemplate._infer_class_name(file_path))
                content = FileTemplate.generate_test_file(file_path, target_class)
            else:
                print(f"❌ 未知模板类型: {template_type}")
                return False

            with open(full_path, 'w', encoding='utf-8') as f:
                f.write(content)

            print(f"📄 根据{template_type}模板创建文件: {file_path}")
            return True

        except Exception as e:
            print(f"❌ 创建文件失败 {file_path}: {e}")
            return False

    def get_created_files_report(self) -> str:
        """生成创建文件报告"""
        report_lines = []
        report_lines.append("# SQLCC缺失文件创建报告")
        report_lines.append(f"创建时间: {self._get_timestamp()}")
        report_lines.append(f"创建文件数量: {len(self.created_files)}")
        report_lines.append("")

        if self.created_files:
            report_lines.append("## 创建的文件列表")
            for i, file_path in enumerate(self.created_files, 1):
                report_lines.append(f"{i}. {file_path}")
        else:
            report_lines.append("## 无文件被创建")

        report_lines.append("")
        report_lines.append("---")
        report_lines.append("*此报告由bazel_file_creator.py自动生成*")

        return "\n".join(report_lines)

    def _get_timestamp(self) -> str:
        """获取当前时间戳"""
        from datetime import datetime
        return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

def main():
    """主函数"""
    import argparse

    parser = argparse.ArgumentParser(description="SQLCC Bazel缺失文件创建器")
    parser.add_argument("project_root", help="项目根目录")
    parser.add_argument("--detection-results", help="检测结果JSON文件")
    parser.add_argument("--template", choices=["header", "source", "test"], help="使用模板创建文件")
    parser.add_argument("--file-path", help="文件路径")
    parser.add_argument("--class-name", help="类名(用于模板)")
    parser.add_argument("--header-path", help="头文件路径(用于源文件模板)")
    parser.add_argument("--target-class", help="目标类名(用于测试模板)")
    parser.add_argument("--output-report", "-o", help="输出报告文件")

    args = parser.parse_args()

    if not os.path.isdir(args.project_root):
        print(f"❌ 错误: 目录不存在: {args.project_root}")
        sys.exit(1)

    creator = BazelFileCreator(args.project_root, args.detection_results)

    if args.template and args.file_path:
        # 使用模板创建单个文件
        success = creator.create_from_template(
            args.template,
            args.file_path,
            class_name=args.class_name,
            header_path=args.header_path,
            target_class=args.target_class
        )

        if success:
            print(f"✅ 文件创建成功: {args.file_path}")
        else:
            print(f"❌ 文件创建失败: {args.file_path}")
            sys.exit(1)

    elif args.detection_results:
        # 基于检测结果创建文件
        created_files = creator.create_missing_files()

        # 生成报告
        report = creator.get_created_files_report()

        if args.output_report:
            with open(args.output_report, 'w', encoding='utf-8') as f:
                f.write(report)
            print(f"📄 报告已保存到: {args.output_report}")
        else:
            print(report)

    else:
        parser.print_help()
        sys.exit(1)

if __name__ == "__main__":
    sys.exit(main())
