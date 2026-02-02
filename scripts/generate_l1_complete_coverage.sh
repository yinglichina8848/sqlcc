#!/bin/bash

# SQLCC Level 1 完整覆盖率报告生成脚本 (v1.3.9 更新版)
# 收集所有核心源码的覆盖率数据并生成专业报告

set -e

PROJECT_ROOT="/home/liying/sqlcc"
COVERAGE_DIR="$PROJECT_ROOT/coverage_report_l1_complete"
BAZEL_CACHE="/home/liying/.cache/bazel/_bazel_liying/68dbc53c53085b82ed46643b8af8ae0d/execroot/_main"

echo "=========================================="
echo "SQLCC Level 1 完整覆盖率报告生成 (v1.3.9)"
echo "=========================================="

mkdir -p "$COVERAGE_DIR"

# 核心模块配置 - 源码目录映射
declare -A CORE_MODULES=(
    ["exception"]="src/exception"
    ["logger"]="src/logger"
    ["types"]="src/types"
    ["utils"]="src/utils"
)

# 所有 Level 1 测试目标 (包含新的测试)
declare -A TEST_TARGETS=(
    # exception
    ["exception"]="//tests/level1_foundation/exception:exception_test"
    ["basic"]="//tests/level1_foundation/basic:basic_test"
    
    # logger
    ["logger"]="//tests/level1_foundation/logger:logger_test"
    
    # types
    ["types"]="//tests/level1_foundation/types:types_test"
    
    # utils - 多个测试目标
    ["utils"]="//tests/level1_foundation/utils:utils_test"
    ["file_descriptor_version"]="//tests/level1_foundation/utils:file_descriptor_version_test"
    ["smart_config"]="//tests/level1_foundation/utils:smart_config_test"
    ["ssl_connection_pool"]="//tests/level1_foundation/utils:ssl_connection_pool_test"
    ["thread_pool"]="//tests/level1_foundation/utils:thread_pool_test"
    
    # config
    ["config"]="//tests/level1_foundation/config:config_test"
)

echo "步骤 1: 运行所有 Level 1 覆盖率测试..."
cd "$PROJECT_ROOT"

# 清理旧的覆盖率数据
rm -f "$COVERAGE_DIR"/*.profdata 2>/dev/null || true
rm -f "$COVERAGE_DIR"/*.profraw 2>/dev/null || true

# 运行所有测试并收集覆盖率数据
bazel test \
    //tests/level1_foundation/exception:exception_test \
    //tests/level1_foundation/basic:basic_test \
    //tests/level1_foundation/logger:logger_test \
    //tests/level1_foundation/types:types_test \
    //tests/level1_foundation/utils:utils_test \
    //tests/level1_foundation/utils:file_descriptor_version_test \
    //tests/level1_foundation/utils:smart_config_test \
    //tests/level1_foundation/utils:ssl_connection_pool_test \
    //tests/level1_foundation/utils:thread_pool_test \
    //tests/level1_foundation/config:config_test \
    --test_output=errors 2>&1 | tail -20

echo ""
echo "步骤 2: 收集各模块覆盖率数据..."

# 收集每个测试模块的 profraw 文件
for TEST_NAME in "${!TEST_TARGETS[@]}"; do
    TEST_TARGET="${TEST_TARGETS[$TEST_NAME]}"
    
    echo "  📊 收集: $TEST_NAME"
    
    # 清理并创建目录
    mkdir -p "$COVERAGE_DIR/$TEST_NAME"
    
    # 查找覆盖率测试输出目录
    COV_DIR=$(find "$BAZEL_CACHE" -path "*_coverage*" -path "*/${TEST_NAME}_test/test" -type d 2>/dev/null | head -1)
    
    # 如果找不到，尝试其他模式
    if [ -z "$COV_DIR" ]; then
        COV_DIR=$(find "$BAZEL_CACHE" -path "*_coverage*" -name "${TEST_NAME}_test" -type d 2>/dev/null | head -1)
    fi
    
    # 如果还是找不到，尝试找包含测试名的目录
    if [ -z "$COV_DIR" ]; then
        COV_DIR=$(find "$BAZEL_CACHE" -path "*_coverage*" -exec basename {} \; 2>/dev/null | grep -i "$TEST_NAME" | head -1)
        if [ -n "$COV_DIR" ]; then
            COV_DIR=$(find "$BAZEL_CACHE" -path "*_coverage*" -type d -name "*$COV_DIR*" 2>/dev/null | head -1)
        fi
    fi
    
    if [ -n "$COV_DIR" ] && [ -d "$COV_DIR" ]; then
        # 复制 profraw 文件
        cp "$COV_DIR"/*.profraw "$COVERAGE_DIR/$TEST_NAME/" 2>/dev/null || true
        
        # 合并 profraw 文件
        if ls "$COVERAGE_DIR/$TEST_NAME"/*.profraw 1> /dev/null 2>&1; then
            llvm-profdata-20 merge -o "$COVERAGE_DIR/$TEST_NAME/$TEST_NAME.profdata" "$COVERAGE_DIR/$TEST_NAME"/*.profraw 2>/dev/null || true
            if [ -f "$COVERAGE_DIR/$TEST_NAME/$TEST_NAME.profdata" ]; then
                echo "    ✓ 覆盖率数据: $COVERAGE_DIR/$TEST_NAME/$TEST_NAME.profdata"
            fi
        fi
    else
        echo "    ⚠️  未找到覆盖率数据目录: $TEST_NAME"
    fi
done

echo ""
echo "步骤 3: 合并所有覆盖率数据..."

# 合并所有 profdata 文件到合并的 profdata
ALL_PROFDATA=$(find "$COVERAGE_DIR" -name "*.profdata" 2>/dev/null | tr '\n' ' ')
if [ -n "$ALL_PROFDATA" ]; then
    llvm-profdata-20 merge -o "$COVERAGE_DIR/merged_all.profdata" $ALL_PROFDATA 2>/dev/null || true
    echo "  ✓ 合并覆盖率数据: $COVERAGE_DIR/merged_all.profdata"
fi

echo ""
echo "步骤 4: 生成各模块详细 HTML 覆盖率报告..."

# 为每个模块生成报告
for MODULE_DIR in "$COVERAGE_DIR"/*/; do
    if [ -d "$MODULE_DIR" ]; then
        MODULE_NAME=$(basename "$MODULE_DIR")
        PROFDATA="$MODULE_DIR$MODULE_NAME.profdata"
        
        if [ -f "$PROFDATA" ]; then
            echo "  📊 生成报告: $MODULE_NAME"
            
            # 确定源码目录
            case "$MODULE_NAME" in
                exception|basic)
                    SOURCE_DIR="src/exception"
                    ;;
                logger)
                    SOURCE_DIR="src/logger"
                    ;;
                types)
                    SOURCE_DIR="src/types"
                    ;;
                utils|file_descriptor_version|smart_config|ssl_connection_pool)
                    SOURCE_DIR="src/utils"
                    ;;
                config)
                    SOURCE_DIR="src/utils"
                    ;;
                *)
                    SOURCE_DIR="src"
                    ;;
            esac
            
            # 查找 object 文件
            OBJ_FILES=$(find "$BAZEL_CACHE" -path "*/$SOURCE_DIR/*_coverage/*.pic.o" 2>/dev/null | tr '\n' ' ')
            
            if [ -n "$OBJ_FILES" ]; then
                # 生成 HTML 报告
                llvm-cov-20 show \
                    --instr-profile="$PROFDATA" \
                    --format=html \
                    --output-dir="$MODULE_DIR" \
                    $OBJ_FILES \
                    2>/dev/null || echo "    ⚠️  HTML 报告生成时出现警告"
                
                # 生成详细文本报告
                llvm-cov-20 report \
                    --instr-profile="$PROFDATA" \
                    --show-line-counts \
                    --show-branches=count \
                    $OBJ_FILES \
                    2>/dev/null > "$MODULE_DIR/coverage_report.txt" || echo "    ⚠️  报告生成失败"
                
                if [ -f "$MODULE_DIR/index.html" ]; then
                    echo "    ✓ HTML报告: $MODULE_DIR/index.html"
                fi
            else
                echo "    ⚠️  未找到目标文件: $SOURCE_DIR"
            fi
        fi
    fi
done

echo ""
echo "步骤 5: 生成全局覆盖率汇总报告..."

# 生成全局汇总报告
if [ -f "$COVERAGE_DIR/merged_all.profdata" ]; then
    # 收集所有源文件
    ALL_OBJ_FILES=$(find "$BAZEL_CACHE" -path "*/src/*_coverage/*.pic.o" 2>/dev/null | tr '\n' ' ')
    
    if [ -n "$ALL_OBJ_FILES" ]; then
        # 生成全局 HTML 报告
        llvm-cov-20 show \
            --instr-profile="$COVERAGE_DIR/merged_all.profdata" \
            --format=html \
            --output-dir="$COVERAGE_DIR/all_tests" \
            $ALL_OBJ_FILES \
            2>/dev/null || echo "  ⚠️  全局HTML报告生成时出现警告"
        
        # 生成汇总报告
        llvm-cov-20 report \
            --instr-profile="$COVERAGE_DIR/merged_all.profdata" \
            --summary-only \
            $ALL_OBJ_FILES \
            2>/dev/null > "$COVERAGE_DIR/TOTAL_COVERAGE_SUMMARY.txt" || echo "  ⚠️  汇总报告生成失败"
        
        if [ -f "$COVERAGE_DIR/all_tests/index.html" ]; then
            echo "  ✓ 全局HTML报告: $COVERAGE_DIR/all_tests/index.html"
        fi
    fi
fi

echo ""
echo "=========================================="
echo "✅ 覆盖率报告生成完成!"
echo "=========================================="
echo ""
echo "📂 报告位置: $COVERAGE_DIR"
echo ""
echo "📊 测试覆盖情况:"
echo "  - exception: exception_test, basic_test"
echo "  - logger: logger_test"
echo "  - types: types_test"
echo "  - utils: utils_test, file_descriptor_version_test, smart_config_test, ssl_connection_pool_test"
echo "  - config: config_test"
echo ""
echo "📁 生成的文件:"
find "$COVERAGE_DIR" -type f \( -name "*.html" -o -name "*.profdata" -o -name "*.txt" \) 2>/dev/null | head -30
echo ""
echo "🌐 查看报告:"
echo "  firefox $COVERAGE_DIR/all_tests/index.html"