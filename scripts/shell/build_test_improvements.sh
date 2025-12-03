#!/bin/bash

# SQLCC v1.0.8 测试改进计划构建脚本
# 此脚本用于构建新的测试结构并运行测试

set -e

# 默认值
BUILD_DIR="build_test_improvements"
BUILD_TYPE="Release"
ENABLE_COVERAGE=false
RUN_TESTS=true
CLEAN_BUILD=false
VERBOSE=false

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -b|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -t|--build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -c|--coverage)
            ENABLE_COVERAGE=true
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --no-test)
            RUN_TESTS=false
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [options]"
            echo "  -b, --build-dir DIR     Build directory (default: build_test_improvements)"
            echo "  -t, --build-type TYPE   Build type: Debug, Release, RelWithDebInfo (default: Release)"
            echo "  -c, --coverage          Enable coverage (sets build type to Debug)"
            echo "  --clean                 Clean build directory before building"
            echo "  -v, --verbose           Verbose output"
            echo "  --no-test               Don't run tests after building"
            echo "  -h, --help              Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# 脚本开始
echo "SQLCC v1.0.8 测试改进计划构建脚本"
echo "=================================="
echo "Build directory: $BUILD_DIR"
echo "Build type: $BUILD_TYPE"
echo "Coverage: $ENABLE_COVERAGE"
echo "Clean build: $CLEAN_BUILD"
echo "Run tests: $RUN_TESTS"
echo "Verbose: $VERBOSE"
echo "----------------------------------------"

# 清理构建目录
if [ "$CLEAN_BUILD" = true ] && [ -d "$BUILD_DIR" ]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# 创建构建目录
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory: $BUILD_DIR"
    mkdir -p "$BUILD_DIR"
fi

# 设置CMake参数
CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"

if [ "$ENABLE_COVERAGE" = true ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DENABLE_COVERAGE=ON"
    echo "Coverage enabled"
fi

# 设置构建参数
BUILD_ARGS=""
if [ "$VERBOSE" = true ]; then
    BUILD_ARGS="$BUILD_ARGS VERBOSE=1"
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_VERBOSE_MAKEFILE=ON"
fi

# 配置项目
echo "Configuring project..."
cd "$BUILD_DIR"
cmake .. $CMAKE_ARGS

# 构建项目
echo "Building project..."
make -j$(nproc) $BUILD_ARGS

# 运行测试
if [ "$RUN_TESTS" = true ]; then
    echo "Running tests..."
    if [ "$VERBOSE" = true ]; then
        ctest --output-on-failure --verbose
    else
        ctest --output-on-failure
    fi
    
    # 生成覆盖率报告
    if [ "$ENABLE_COVERAGE" = true ]; then
        echo "Generating coverage report..."
        cd ..
        ./tests/generate_coverage.sh -b "$BUILD_DIR" -o coverage_reports
        
        echo "Coverage report generated in: coverage_reports"
        echo "HTML report: coverage_reports/coverage.html"
    fi
fi

echo "----------------------------------------"
echo "Build completed successfully!"
echo "Build directory: $BUILD_DIR"

# 生成测试报告
if [ "$RUN_TESTS" = true ]; then
    echo "Running unified test script..."
    cd ..
    ./tests/run_all_tests.sh -b "$BUILD_DIR" -o test_reports
    
    echo "Test reports generated in: test_reports"
    echo "Test summary: test_reports/test_summary.txt"
fi

exit 0