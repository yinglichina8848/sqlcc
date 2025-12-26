"""
SQLCC Include路径分析器包
"""

__version__ = "1.0.0"
__author__ = "SQLCC AI Assistant"

from .include_path_analyzer import IncludePathAnalyzerApp
from .utils.config_loader import ConfigLoader
from .utils.models import Config, AnalysisResult, IncludeIssue

__all__ = [
    'IncludePathAnalyzerApp',
    'ConfigLoader',
    'Config',
    'AnalysisResult',
    'IncludeIssue',
    '__version__',
    '__author__'
]
