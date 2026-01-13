#!/bin/bash
# SQLCC测试文件依赖关系分析脚本

echo "=== SQLCC测试文件依赖关系分析 ==="

# 输出文件
OUTPUT_FILE="test_dependencies_analysis.txt"
HIERARCHY_REPORT="test_hierarchy_report.txt"

# 清空输出文件
> "$OUTPUT_FILE"
> "$HIERARCHY_REPORT"

echo "分析时间: $(date)" >> "$OUTPUT_FILE"
echo "分析时间: $(date)" >> "$HIERARCHY_REPORT"
echo "" >> "$OUTPUT_FILE"
echo "" >> "$HIERARCHY_REPORT"

# 查找所有测试文件
echo "查找测试文件..."
find tests -name "*.cpp" -o -name "*.cc" | sort > test_files.txt

TOTAL_FILES=$(wc -l < test_files.txt)
echo "发现 $TOTAL_FILES 个测试文件"

echo "=== 测试文件依赖关系分析 ===" >> "$OUTPUT_FILE"
echo "总文件数: $TOTAL_FILES" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# 分析每个文件的include依赖
echo "分析include依赖关系..."
while IFS= read -r file; do
    echo "分析: $file"

    # 提取include语句
    includes=$(grep '^#include' "$file" 2>/dev/null | sed 's/#include//' | tr -d ' "<>' | sort)

    # 提取相对路径作为文件标识
    relative_path="${file#tests/}"

    echo "文件: $relative_path" >> "$OUTPUT_FILE"
    echo "路径: $file" >> "$OUTPUT_FILE"
    echo "Include依赖:" >> "$OUTPUT_FILE"

    if [ -n "$includes" ]; then
        echo "$includes" | while read -r inc; do
            echo "  - $inc" >> "$OUTPUT_FILE"
        done
    else
        echo "  (无include依赖)" >> "$OUTPUT_FILE"
    fi
    echo "" >> "$OUTPUT_FILE"

done < test_files.txt

# 分析层次分类问题
echo "=== 层次分类分析 ===" >> "$HIERARCHY_REPORT"

# 定义层次映射（基于include路径判断）
declare -A layer_mapping
layer_mapping["core/"]="5"  # 执行引擎
layer_mapping["execution/"]="5"  # 执行引擎
layer_mapping["storage_engine/"]="3"  # 存储引擎
layer_mapping["transaction/"]="5"  # 执行引擎
layer_mapping["network/"]="6"  # 网络通信
layer_mapping["sql_parser/"]="4"  # SQL解析器
layer_mapping["sql_executor/"]="5"  # 执行引擎
layer_mapping["procedure/"]="7"  # 企业级特性
layer_mapping["trigger/"]="7"  # 企业级特性
layer_mapping["security/"]="6"  # 网络通信(安全)
layer_mapping["config/"]="2"  # 核心组件
layer_mapping["utils/"]="1"  # 基础工具
layer_mapping["logger/"]="2"  # 核心组件

POTENTIAL_MISCLASSIFIED=()

while IFS= read -r file; do
    relative_path="${file#tests/}"
    dirname_path=$(dirname "$relative_path")

    # 提取include依赖
    includes=$(grep '^#include' "$file" 2>/dev/null | sed 's/#include//' | tr -d ' "<>' | grep -E '\.h$|\.hpp$' | sort)

    # 判断实际应该属于的层次
    actual_layer="1"  # 默认基础层
    max_layer="1"

    echo "$includes" | while read -r inc; do
        for prefix in "${!layer_mapping[@]}"; do
            if [[ "$inc" == *"$prefix"* ]]; then
                layer="${layer_mapping[$prefix]}"
                if [ "$layer" -gt "$max_layer" ]; then
                    max_layer="$layer"
                fi
            fi
        done
    done

    # 检查当前目录层次
    current_layer="1"
    case "$dirname_path" in
        "unit/basic") current_layer="1" ;;
        "unit/core") current_layer="2" ;;
        "unit/storage") current_layer="3" ;;
        "unit/parser") current_layer="4" ;;
        "unit/executor") current_layer="5" ;;
        "integration") current_layer="7" ;;
        "network") current_layer="6" ;;
        "performance") current_layer="8" ;;
        *) current_layer="1" ;;
    esac

    # 检查是否被错误分类
    if [ "$max_layer" != "$current_layer" ]; then
        echo "POTENTIAL_MISCLASSIFICATION: $relative_path" >> "$HIERARCHY_REPORT"
        echo "  当前层次: $current_layer ($dirname_path)" >> "$HIERARCHY_REPORT"
        echo "  实际层次: $max_layer (基于依赖分析)" >> "$HIERARCHY_REPORT"
        echo "  依赖组件: $includes" >> "$HIERARCHY_REPORT"
        echo "" >> "$HIERARCHY_REPORT"

        POTENTIAL_MISCLASSIFIED+=("$relative_path")
    fi

done < test_files.txt

echo "=== 总结 ===" >> "$HIERARCHY_REPORT"
echo "潜在错误分类文件数: ${#POTENTIAL_MISCLASSIFIED[@]}" >> "$HIERARCHY_REPORT"
echo "错误分类的文件列表:" >> "$HIERARCHY_REPORT"
for file in "${POTENTIAL_MISCLASSIFIED[@]}"; do
    echo "  - $file" >> "$HIERARCHY_REPORT"
done

# 清理临时文件
rm test_files.txt

echo "分析完成!"
echo "结果已保存到:"
echo "  - $OUTPUT_FILE"
echo "  - $HIERARCHY_REPORT"