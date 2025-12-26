"""
SQLCC Include路径分析器配置加载器
负责加载和解析YAML配置文件
"""

import yaml
from pathlib import Path
from typing import Dict, Any, Optional
from .models import Config


class ConfigLoader:
    """配置加载器"""

    def __init__(self, config_path: str = "tools/include_path_analyzer/config.yaml"):
        self.config_path = Path(config_path)
        self._config = None

    def load_config(self) -> Config:
        """加载配置"""
        if self._config is not None:
            return self._config

        if not self.config_path.exists():
            raise FileNotFoundError(f"配置文件不存在: {self.config_path}")

        try:
            with open(self.config_path, 'r', encoding='utf-8') as f:
                data = yaml.safe_load(f)

            self._config = self._parse_config(data)
            return self._config

        except yaml.YAMLError as e:
            raise ValueError(f"YAML配置文件解析错误: {e}")
        except Exception as e:
            raise RuntimeError(f"加载配置文件失败: {e}")

    def _parse_config(self, data: Dict[str, Any]) -> Config:
        """解析配置数据"""
        # 项目基本信息
        project = data.get('project', {})
        config = Config(
            project_name=project.get('name', 'SQLCC'),
            project_root=project.get('root', '.'),
            include_dirs=project.get('include_dirs', ['include']),
            src_dirs=project.get('src_dirs', ['src', 'tests'])
        )

        # 分析选项
        analysis = data.get('analysis', {})
        config.max_include_depth = analysis.get('max_include_depth', 10)
        config.enable_circular_detection = analysis.get('enable_circular_detection', True)
        config.check_bazel_compatibility = analysis.get('check_bazel_compatibility', True)
        config.enable_auto_fix = analysis.get('enable_auto_fix', False)

        # 输出选项
        output = data.get('output', {})
        config.output_formats = output.get('formats', ['json', 'html', 'cli'])
        config.report_dir = output.get('report_dir', 'reports/include_analysis')
        config.enable_summary = output.get('enable_summary', True)
        config.enable_details = output.get('enable_details', True)

        # 修复选项
        fixing = data.get('fixing', {})
        config.backup_files = fixing.get('backup_files', True)
        config.dry_run = fixing.get('dry_run', False)
        config.max_fixes_per_file = fixing.get('max_fixes_per_file', 10)
        config.require_confirmation = fixing.get('require_confirmation', True)

        # 模块映射
        module_mappings = data.get('module_mappings', {})
        config.module_mappings = dict(module_mappings)

        # 标准库
        config.standard_libraries = data.get('standard_libraries', [])

        # 已弃用头文件
        issue_types = data.get('issue_types', [])
        for issue_type in issue_types:
            if issue_type.get('name') == 'deprecated_header':
                config.deprecated_headers = issue_type.get('deprecated_headers', [])
                break

        return config

    def save_config(self, config: Config, output_path: Optional[str] = None) -> None:
        """保存配置"""
        if output_path is None:
            output_path = self.config_path

        data = {
            'project': {
                'name': config.project_name,
                'root': config.project_root,
                'include_dirs': config.include_dirs,
                'src_dirs': config.src_dirs
            },
            'analysis': {
                'max_include_depth': config.max_include_depth,
                'enable_circular_detection': config.enable_circular_detection,
                'check_bazel_compatibility': config.check_bazel_compatibility,
                'enable_auto_fix': config.enable_auto_fix
            },
            'output': {
                'formats': config.output_formats,
                'report_dir': config.report_dir,
                'enable_summary': config.enable_summary,
                'enable_details': config.enable_details
            },
            'fixing': {
                'backup_files': config.backup_files,
                'dry_run': config.dry_run,
                'max_fixes_per_file': config.max_fixes_per_file,
                'require_confirmation': config.require_confirmation
            },
            'module_mappings': config.module_mappings,
            'standard_libraries': config.standard_libraries,
            'issue_types': [
                {
                    'name': 'deprecated_header',
                    'description': '使用已弃用的头文件',
                    'severity': 'medium',
                    'deprecated_headers': config.deprecated_headers,
                    'suggestion': '使用正确的头文件'
                }
            ]
        }

        output_path = Path(output_path)
        output_path.parent.mkdir(parents=True, exist_ok=True)

        with open(output_path, 'w', encoding='utf-8') as f:
            yaml.dump(data, f, indent=2, allow_unicode=True, sort_keys=False)

    def create_default_config(self, output_path: Optional[str] = None) -> Config:
        """创建默认配置"""
        config = Config()
        self.save_config(config, output_path)
        return config

    def validate_config(self, config: Config) -> list[str]:
        """验证配置有效性"""
        errors = []

        # 检查项目根目录
        if not Path(config.project_root).exists():
            errors.append(f"项目根目录不存在: {config.project_root}")

        # 检查include目录
        for include_dir in config.include_dirs:
            full_path = Path(config.project_root) / include_dir
            if not full_path.exists():
                errors.append(f"Include目录不存在: {full_path}")

        # 检查src目录
        for src_dir in config.src_dirs:
            full_path = Path(config.project_root) / src_dir
            if not full_path.exists():
                errors.append(f"源代码目录不存在: {full_path}")

        # 检查最大深度
        if config.max_include_depth <= 0:
            errors.append("max_include_depth必须大于0")

        # 检查每文件最大修复数
        if config.max_fixes_per_file <= 0:
            errors.append("max_fixes_per_file必须大于0")

        return errors
