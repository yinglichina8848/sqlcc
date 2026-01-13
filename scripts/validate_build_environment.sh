#!/bin/bash
# 编译环境验证脚本

echo "=== 编译环境验证 ==="

# 加载统一配置
source tests/build_config.unified.sh

# 验证各层次的环境
LEVELS=(
    "level1_foundation"
    "level2_core"
    "level3_storage"
    "level4_parser"
    "level5_execution"
    "level6_network"
    "level7_enterprise"
    "level8_integration"
)

for level in "${LEVELS[@]}"; do
    echo "验证 $level 环境..."
    if init_build_environment "$level" 2>/dev/null; then
        echo "  ✓ 环境初始化成功"
    else
        echo "  ✗ 环境初始化失败"
    fi
done

echo "环境验证完成"

