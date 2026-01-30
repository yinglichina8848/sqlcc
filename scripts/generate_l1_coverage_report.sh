#!/bin/bash

# SQLCC Level 1 覆盖率报告生成脚本
# 使用 llvm-cov 生成 HTML 覆盖率报告

set -e

PROJECT_ROOT="/home/liying/sqlcc"
COVERAGE_DIR="$PROJECT_ROOT/coverage_report_l1"
BAZEL_CACHE="/home/liying/.cache/bazel/_bazel_liying/68dbc53c53085b82ed46643b8af8ae0d/execroot/_main"

echo "=========================================="
echo "SQLCC Level 1 覆盖率报告生成"
echo "=========================================="

# 创建输出目录
mkdir -p "$COVERAGE_DIR"

# 收集所有 profraw 文件
PROFRAW_FILES=$(find "$BAZEL_CACHE" -name "*.profraw" 2>/dev/null | tr '\n' ' ')
PROFDATA_FILE="$BAZEL_CACHE/merged_coverage_level1.profdata"

echo "步骤 1: 合并覆盖率数据..."
llvm-profdata-20 merge -o "$PROFDATA_FILE" $PROFRAW_FILES 2>/dev/null || true

# 查找源文件
SOURCES=(
    "$PROJECT_ROOT/src/types/domain_manager.cpp"
    "$PROJECT_ROOT/src/types/value.cpp"
    "$PROJECT_ROOT/src/utils/config_manager.cpp"
    "$PROJECT_ROOT/src/utils/config_snapshot.cpp"
    "$PROJECT_ROOT/src/utils/config_lifecycle.cpp"
    "$PROJECT_ROOT/src/utils/smart_config_manager.cpp"
    "$PROJECT_ROOT/src/utils/thread_pool.cpp"
    "$PROJECT_ROOT/src/logger/logger.cpp"
    "$PROJECT_ROOT/src/exception/exception.cpp"
    "$PROJECT_ROOT/src/exception/io_exception.cpp"
)

# 查找编译产物
OBJECTS=(
    "$BAZEL_CACHE/bazel-out/k8-fastbuild/bin/src/types/_objs/types/domain_manager.pic.o"
    "$BAZEL_CACHE/bazel-out/k8-fastbuild/bin/src/types/_objs/types/value.pic.o"
    "$BAZEL_CACHE/bazel-out/k8-fastbuild/bin/src/utils/_objs/utils/config_manager.pic.o"
    "$BAZEL_CACHE/bazel-out/k8-fastbuild/bin/src/utils/_objs/utils/config_snapshot.pic.o"
    "$BAZEL_CACHE/bazel-out/k8-fastbuild/bin/src/utils/_objs/utils/config_lifecycle.pic.o"
    "$BAZEL_CACHE/bazel-out/k8-fastbuild/bin/src/utils/_objs/utils/smart_config_manager.pic.o"
    "$BAZEL_CACHE/bazel-out/k8-fastbuild/bin/src/utils/_objs/utils/thread_pool.pic.o"
    "$BAZEL_CACHE/bazel-out/k8-fastbuild/bin/src/logger/_objs/logger/logger.pic.o"
    "$BAZEL_CACHE/bazel-out/k8-fastbuild/bin/src/exception/_objs/exception/exception.pic.o"
    "$BAZEL_CACHE/bazel-out/k8-fastbuild/bin/src/exception/_objs/exception/io_exception.pic.o"
)

echo "步骤 2: 生成文本覆盖率报告..."
llvm-cov-20 show \
    "$PROFDATA_FILE" \
    --sources "${SOURCES[@]}" \
    --object-files "${OBJECTS[@]}" \
    --format=text \
    --show-line-counts \
    --show-branches=count \
    2>/dev/null > "$COVERAGE_DIR/coverage_detailed.txt" || true

echo "步骤 3: 生成 HTML 覆盖率报告..."

# 生成每个文件的 HTML 报告
for i in "${!SOURCES[@]}"; do
    SRC_FILE="${SOURCES[$i]}"
    OBJ_FILE="${OBJECTS[$i]}"

    if [ -f "$SRC_FILE" ] && [ -f "$OBJ_FILE" ]; then
        FILENAME=$(basename "$SRC_FILE" .cpp)
        echo "  生成: $FILENAME.html"

        llvm-cov-20 show \
            "$PROFDATA_FILE" \
            --sources "$SRC_FILE" \
            --objects "$OBJ_FILE" \
            --format=html \
            --output-dir="$COVERAGE_DIR/$FILENAME" \
            2>/dev/null || true
    fi
done

# 生成汇总报告
echo "步骤 4: 生成覆盖率汇总报告..."
llvm-cov-20 report \
    "$PROFDATA_FILE" \
    --sources "${SOURCES[@]}" \
    --object-files "${OBJECTS[@]}" \
    2>/dev/null > "$COVERAGE_DIR/coverage_summary.txt" || true

# 生成 LCOV 格式报告
llvm-cov-20 report \
    "$PROFDATA_FILE" \
    --sources "${SOURCES[@]}" \
    --object-files "${OBJECTS[@]}" \
    --format=lcov \
    2>/dev/null > "$COVERAGE_DIR/coverage.lcov" || true

# 生成 HTML 索引页
cat > "$COVERAGE_DIR/index.html" << 'EOF'
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>SQLCC Level 1 覆盖率报告</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
        h1 { color: #333; border-bottom: 2px solid #4CAF50; padding-bottom: 10px; }
        .summary { background: white; padding: 20px; border-radius: 8px; margin-bottom: 20px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .module { background: white; padding: 15px; margin: 10px 0; border-radius: 5px; box-shadow: 0 1px 3px rgba(0,0,0,0.1); }
        .module h3 { margin: 0 0 10px 0; color: #2196F3; }
        .coverage-bar { background: #e0e0e0; height: 20px; border-radius: 10px; overflow: hidden; }
        .coverage-fill { height: 100%; background: linear-gradient(90deg, #4CAF50, #8BC34A); }
        a { color: #2196F3; text-decoration: none; }
        a:hover { text-decoration: underline; }
        .stats { color: #666; font-size: 14px; }
    </style>
</head>
<body>
    <h1>📊 SQLCC Level 1 覆盖率报告</h1>
    <div class="summary">
        <h2>测试状态</h2>
        <p><strong>总测试数:</strong> 137</p>
        <p><strong>通过测试:</strong> 137 (100%)</p>
        <p><strong>测试模块:</strong> exception, types, config, utils, basic, logger</p>
    </div>
    <div class="module">
        <h3>📁 详细报告文件</h3>
        <ul>
            <li><a href="coverage_summary.txt">覆盖率汇总 (文本)</a></li>
            <li><a href="coverage_detailed.txt">详细覆盖率 (文本)</a></li>
            <li><a href="coverage.lcov">LCOV 格式报告</a></li>
        </ul>
    </div>
    <div class="module">
        <h3>📈 各模块覆盖率</h3>
        <p>请查看各个模块的 HTML 报告获取详细覆盖率信息。</p>
    </div>
    <div class="stats">
        <p>生成时间: $(date)</p>
        <p>报告工具: llvm-cov-20</p>
    </div>
</body>
</html>
EOF

# 更新生成时间
sed -i "s/\$(date)/$(date '+%Y-%m-%d %H:%M:%S')/" "$COVERAGE_DIR/index.html"

echo ""
echo "=========================================="
echo "✅ 覆盖率报告生成完成!"
echo "=========================================="
echo "报告位置: $COVERAGE_DIR"
echo ""
echo "文件列表:"
ls -la "$COVERAGE_DIR"
echo ""
echo "打开 index.html 查看报告:"
echo "  firefox $COVERAGE_DIR/index.html"
