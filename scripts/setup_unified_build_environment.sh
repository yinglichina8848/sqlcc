#!/bin/bash
# SQLCC统一编译环境配置脚本

echo "=== SQLCC统一编译环境配置 ==="

# 创建统一的编译环境配置文件
UNIFIED_CONFIG="tests/build_config.unified.sh"

cat > "$UNIFIED_CONFIG" << 'EOF'
#!/bin/bash
# SQLCC统一编译环境配置
# 适用于所有测试层次的标准化编译环境

# =============================================================================
# 编译器配置
# =============================================================================

# 主编译器设置 (clang-20优先)
export CC="${CC:-clang-20}"
export CXX="${CXX:-clang++-20}"

# 备选编译器
if ! command -v "$CC" &> /dev/null; then
    echo "警告: $CC 未找到，尝试使用gcc"
    export CC="gcc"
    export CXX="g++"
fi

# 编译标准
export CXX_STANDARD="c++20"
export C_STANDARD="c11"

# =============================================================================
# 基础编译参数
# =============================================================================

# 通用编译标志
COMMON_FLAGS=(
    "-std=${CXX_STANDARD}"
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Wno-unused-parameter"
    "-Wno-unused-variable"
    "-O2"
    "-g"
    "-fPIC"
)

# 调试编译标志
DEBUG_FLAGS=(
    "-O0"
    "-g3"
    "-DDEBUG"
)

# 发布编译标志
RELEASE_FLAGS=(
    "-O3"
    "-DNDEBUG"
    "-flto"
)

# =============================================================================
# 包含路径配置
# =============================================================================

# 基础包含路径
INCLUDE_PATHS=(
    "-I$(pwd)/include"
    "-I$(pwd)/tests/framework"
    "-I$(pwd)/third_party"
)

# 层次特定包含路径函数
get_level_include_paths() {
    local level="$1"
    local additional_paths=()

    case "$level" in
        "level1_foundation")
            additional_paths=(
                "-I$(pwd)/include/utils"
                "-I$(pwd)/include/logger"
                "-I$(pwd)/include/config"
            )
            ;;
        "level2_core")
            additional_paths=(
                "-I$(pwd)/include/core"
                "-I$(pwd)/include/database_manager"
                "-I$(pwd)/include/user_manager"
            )
            ;;
        "level3_storage")
            additional_paths=(
                "-I$(pwd)/include/storage_engine"
                "-I$(pwd)/include/page"
                "-I$(pwd)/include/disk_manager"
            )
            ;;
        "level4_parser")
            additional_paths=(
                "-I$(pwd)/include/sql_parser"
                "-I$(pwd)/include/lexer"
            )
            ;;
        "level5_execution")
            additional_paths=(
                "-I$(pwd)/include/execution"
                "-I$(pwd)/include/transaction"
                "-I$(pwd)/include/sql_executor"
            )
            ;;
        "level6_network")
            additional_paths=(
                "-I$(pwd)/include/network"
                "-I$(pwd)/include/security"
            )
            ;;
        "level7_enterprise")
            additional_paths=(
                "-I$(pwd)/include/procedure"
                "-I$(pwd)/include/trigger"
            )
            ;;
        "level8_integration")
            additional_paths=(
                "-I$(pwd)/include"
                "-I$(pwd)/tests"
            )
            ;;
    esac

    echo "${additional_paths[@]}"
}

# =============================================================================
# 库链接配置
# =============================================================================

# GTest配置
GTEST_LIBS=(
    "-lgtest"
    "-lgtest_main"
    "-pthread"
)

# SQLCC核心库
SQLCC_CORE_LIBS=(
    "-lsqlcc_core_lib"
    "-lsqlcc_parser"
    "-lsqlcc_config_manager"
    "-lsqlcc_storage_engine"
    "-lsqlcc_transaction_manager"
    "-lsqlcc_executor"
)

# 层次特定库依赖
get_level_library_deps() {
    local level="$1"
    local additional_libs=()

    case "$level" in
        "level6_network")
            additional_libs=(
                "-lsqlcc_network"
                "-lssl"
                "-lcrypto"
            )
            ;;
        "level8_integration")
            additional_libs=(
                "-lsqlcc_network"
                "-lssl"
                "-lcrypto"
                "-lmysqlclient"  # MySQL客户端库
            )
            ;;
    esac

    echo "${additional_libs[@]}"
}

# =============================================================================
# 覆盖率配置
# =============================================================================

# LLVM覆盖率标志 (适用于clang)
COVERAGE_FLAGS=(
    "-fprofile-instr-generate"
    "-fcoverage-mapping"
)

# 覆盖率数据输出
export LLVM_PROFILE_FILE="coverage_%p.profraw"

# =============================================================================
# 构建类型配置
# =============================================================================

# 默认构建类型
export BUILD_TYPE="${BUILD_TYPE:-Debug}"

# 根据构建类型设置标志
get_build_flags() {
    case "$BUILD_TYPE" in
        "Debug")
            echo "${DEBUG_FLAGS[@]}"
            ;;
        "Release")
            echo "${RELEASE_FLAGS[@]}"
            ;;
        "Coverage")
            echo "${DEBUG_FLAGS[@]}" "${COVERAGE_FLAGS[@]}"
            ;;
        *)
            echo "${COMMON_FLAGS[@]}"
            ;;
    esac
}

# =============================================================================
# 工具链验证
# =============================================================================

# 验证编译器版本
verify_compiler() {
    local compiler="$1"
    local expected_version="$2"

    if ! command -v "$compiler" &> /dev/null; then
        echo "错误: $compiler 未找到"
        return 1
    fi

    local version
    version=$("$compiler" --version | head -1)
    echo "$compiler 版本: $version"

    if [ -n "$expected_version" ]; then
        if ! echo "$version" | grep -q "$expected_version"; then
            echo "警告: 期望版本 $expected_version，实际版本可能不匹配"
        fi
    fi

    return 0
}

# 验证GTest
verify_gtest() {
    if ! pkg-config --exists gtest; then
        echo "警告: GTest pkg-config未找到"
        return 1
    fi

    local version
    version=$(pkg-config --modversion gtest)
    echo "GTest版本: $version"
    return 0
}

# =============================================================================
# 环境初始化函数
# =============================================================================

# 初始化构建环境
init_build_environment() {
    local level="$1"

    echo "初始化 $level 构建环境..."

    # 验证编译器
    verify_compiler "$CC" "clang"
    verify_compiler "$CXX" "clang"

    # 验证GTest
    verify_gtest

    # 设置包含路径
    local level_includes
    level_includes=$(get_level_include_paths "$level")
    export INCLUDE_FLAGS="${INCLUDE_PATHS[*]} $level_includes"

    # 设置库依赖
    local level_libs
    level_libs=$(get_level_library_deps "$level")
    export LIBRARY_FLAGS="${GTEST_LIBS[*]} $level_libs"

    # 设置构建标志
    local build_flags
    build_flags=$(get_build_flags)
    export CXXFLAGS="${COMMON_FLAGS[*]} $build_flags"

    echo "环境初始化完成"
    echo "CC=$CC"
    echo "CXX=$CXX"
    echo "CXXFLAGS=$CXXFLAGS"
    echo "INCLUDE_FLAGS=$INCLUDE_FLAGS"
    echo "LIBRARY_FLAGS=$LIBRARY_FLAGS"
}

# =============================================================================
# 编译函数
# =============================================================================

# 编译单个测试文件
compile_test() {
    local source_file="$1"
    local output_file="$2"
    local level="$3"

    # 初始化环境
    init_build_environment "$level"

    echo "编译 $source_file -> $output_file"

    # 构建编译命令
    local compile_cmd=(
        "$CXX"
        "$CXXFLAGS"
        "$INCLUDE_FLAGS"
        "-o" "$output_file"
        "$source_file"
        "$LIBRARY_FLAGS"
        "${SQLCC_CORE_LIBS[@]}"
    )

    # 执行编译
    if "${compile_cmd[@]}"; then
        echo "编译成功: $output_file"
        return 0
    else
        echo "编译失败: $source_file"
        return 1
    fi
}

# =============================================================================
# 运行测试函数
# =============================================================================

# 运行单个测试
run_test() {
    local test_executable="$1"
    local output_dir="${2:-test_results}"

    mkdir -p "$output_dir"

    local test_name
    test_name=$(basename "$test_executable" .test)
    local output_file="$output_dir/${test_name}_result.xml"

    echo "运行测试: $test_executable"

    if [ -x "$test_executable" ]; then
        # 运行测试并捕获输出
        if "$test_executable" --gtest_output="xml:$output_file"; then
            echo "测试通过: $test_name"
            return 0
        else
            echo "测试失败: $test_name"
            return 1
        fi
    else
        echo "错误: 测试可执行文件不存在或不可执行: $test_executable"
        return 1
    fi
}

# =============================================================================
# 覆盖率处理函数
# =============================================================================

# 生成覆盖率报告
generate_coverage_report() {
    local profraw_files="$1"
    local output_dir="${2:-coverage_report}"

    mkdir -p "$output_dir"

    echo "生成覆盖率报告..."

    # 合并profraw文件
    llvm-profdata merge -o "$output_dir/coverage.profdata" $profraw_files

    # 生成覆盖率报告
    llvm-cov show \
        --instr-profile="$output_dir/coverage.profdata" \
        --object="$1" \
        --format=html \
        --output-dir="$output_dir/html"

    llvm-cov report \
        --instr-profile="$output_dir/coverage.profdata" \
        --object="$1" \
        > "$output_dir/coverage.txt"

    echo "覆盖率报告生成完成: $output_dir"
}

# =============================================================================
# 使用示例
# =============================================================================

# 示例: 编译level5的execution_context_test.cpp
example_compile_level5_test() {
    local source="tests/level5_execution/execution_context/execution_context_test.cpp"
    local output="build/level5_execution/execution_context_test"

    compile_test "$source" "$output" "level5_execution"
}

# 示例: 运行测试
example_run_test() {
    local test_exe="build/level5_execution/execution_context_test"
    run_test "$test_exe" "test_results/level5"
}

echo "统一编译环境配置加载完成"
echo "使用方法:"
echo "  source $0"
echo "  init_build_environment level5_execution"
echo "  compile_test source.cpp output level5_execution"

EOF

chmod +x "$UNIFIED_CONFIG"

echo "统一编译环境配置文件已创建: $UNIFIED_CONFIG"

# 测试配置文件的语法
echo "验证配置文件语法..."
if bash -n "$UNIFIED_CONFIG"; then
    echo "语法验证: 通过"
else
    echo "语法验证: 失败"
    exit 1
fi

# 创建环境验证脚本
VALIDATION_SCRIPT="scripts/validate_build_environment.sh"
cat > "$VALIDATION_SCRIPT" << 'EOF'
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

EOF

chmod +x "$VALIDATION_SCRIPT"
echo "环境验证脚本已创建: $VALIDATION_SCRIPT"

echo ""
echo "=== 统一编译环境配置完成 ==="
echo "📁 主配置文件: $UNIFIED_CONFIG"
echo "🔍 验证脚本: $VALIDATION_SCRIPT"
echo ""
echo "使用方法:"
echo "  source $UNIFIED_CONFIG"
echo "  init_build_environment level5_execution"
echo "  compile_test source.cpp output level5_execution"