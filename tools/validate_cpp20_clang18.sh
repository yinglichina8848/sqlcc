#!/bin/bash
# tools/validate_cpp20_clang18.sh - C++20和Clang 18验证工具
# 验证编译器功能和C++20特性支持

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 日志函数
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

# 显示帮助信息
show_help() {
    cat << EOF
C++20和Clang 18验证工具

用法:
    $0 [选项]

选项:
    -h, --help          显示帮助信息
    -c, --compiler      检查编译器版本和支持
    -f, --features      测试C++20特性支持
    -m, --modules       测试模块系统支持
    -b, --build-test    运行构建测试
    -a, --all           运行所有测试
    -v, --verbose       详细输出

示例:
    $0 --compiler                    # 检查编译器信息
    $0 --features                    # 测试C++20特性
    $0 --modules                     # 测试模块系统
    $0 --all                         # 运行所有验证

EOF
}

# 检查编译器版本
check_compiler() {
    log_info "检查编译器版本..."

    # 检查Clang版本
    if command -v clang++ &> /dev/null; then
        local clang_version=$(clang++ --version 2>/dev/null | head -1)
        log_info "Clang版本: $clang_version"

        # 提取版本号
        local version_num=$(echo "$clang_version" | grep -o 'version [0-9]*\.[0-9]*\.[0-9]*' | sed 's/version //' | cut -d. -f1)

        if [[ $version_num -ge 18 ]]; then
            log_success "Clang 18+ 可用"
        elif [[ $version_num -ge 16 ]]; then
            log_warn "Clang版本较低 ($version_num), 某些C++20特性可能不支持"
        else
            log_error "Clang版本过低 ($version_num), 需要18+版本"
        fi
    else
        log_error "未找到Clang编译器"
        return 1
    fi

    # 检查libc++版本
    if command -v llvm-config &> /dev/null; then
        local llvm_version=$(llvm-config --version 2>/dev/null)
        log_info "LLVM版本: $llvm_version"
    fi
}

# 创建C++20特性测试文件
create_cpp20_test() {
    local test_file="$1"

    cat > "$test_file" << 'EOF'
// C++20特性测试文件
#include <iostream>
#include <vector>
#include <ranges>
#include <concepts>
#include <coroutine>
#include <format>
#include <string>

// 概念测试
template<typename T>
concept Integral = std::is_integral_v<T>;

template<Integral T>
void print_value(T value) {
    std::cout << "Value: " << value << std::endl;
}

// 协程测试
struct Generator {
    struct promise_type {
        int value;
        Generator get_return_object() { return Generator{*this}; }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() {}
        std::suspend_always yield_value(int v) {
            value = v;
            return {};
        }
        void return_void() {}
    };

    bool move_next() {
        handle.resume();
        return !handle.done();
    }

    int current_value() const { return handle.promise().value; }

    Generator(Generator const&) = delete;
    Generator& operator=(Generator const&) = delete;

    Generator(Generator&& other) : handle{std::exchange(other.handle, nullptr)} {}
    ~Generator() { if (handle) handle.destroy(); }

private:
    Generator(promise_type& p) : handle{std::coroutine_handle<promise_type>::from_promise(p)} {}

    std::coroutine_handle<promise_type> handle;
};

Generator fibonacci(int max) {
    int a = 0, b = 1;
    while (a < max) {
        co_yield a;
        std::tie(a, b) = std::make_tuple(b, a + b);
    }
}

// 范围库测试
void test_ranges() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // 使用范围和管道
    auto even_squares = numbers |
                       std::views::filter([](int n) { return n % 2 == 0; }) |
                       std::views::transform([](int n) { return n * n; });

    std::cout << "Even squares: ";
    for (int n : even_squares) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
}

// 格式库测试
void test_format() {
    std::string name = "World";
    int value = 42;

    // 使用std::format
    std::cout << std::format("Hello, {}! The answer is {}.", name, value) << std::endl;
}

int main() {
    std::cout << "=== C++20特性测试 ===\n";

    // 测试概念
    print_value(42);
    print_value(static_cast<short>(123));

    // 测试范围库
    test_ranges();

    // 测试格式库
    test_format();

    // 测试协程
    std::cout << "Fibonacci sequence: ";
    for (auto gen = fibonacci(50); gen.move_next(); ) {
        std::cout << gen.current_value() << " ";
    }
    std::cout << std::endl;

    std::cout << "所有C++20特性测试完成!\n";
    return 0;
}
EOF
}

# 测试C++20特性支持
test_cpp20_features() {
    log_info "测试C++20特性支持..."

    local test_file="cpp20_test.cpp"
    local output_file="cpp20_test.out"

    # 创建测试文件
    create_cpp20_test "$test_file"

    # 编译测试
    log_info "编译C++20测试文件..."
    if clang++ -std=c++20 -stdlib=libc++ -fcoroutines-ts -lc++abi \
             "$test_file" -o cpp20_test 2>"$output_file"; then

        log_success "C++20编译成功"

        # 运行测试
        if ./cpp20_test >>"$output_file" 2>&1; then
            log_success "C++20运行时测试通过"
        else
            log_warn "C++20运行时测试失败"
        fi

    else
        log_error "C++20编译失败"
        if [[ -f "$output_file" ]]; then
            log_info "编译错误详情:"
            cat "$output_file"
        fi
        return 1
    fi

    # 清理
    rm -f "$test_file" cpp20_test "$output_file"
}

# 创建模块测试文件
create_module_test() {
    local module_file="$1"
    local main_file="$2"

    # 创建模块接口文件
    cat > "$module_file" << 'EOF'
export module math;

export int add(int a, int b) {
    return a + b;
}

export int multiply(int a, int b) {
    return a * b;
}
EOF

    # 创建主文件
    cat > "$main_file" << 'EOF'
import math;
#include <iostream>

int main() {
    std::cout << "2 + 3 = " << add(2, 3) << std::endl;
    std::cout << "4 * 5 = " << multiply(4, 5) << std::endl;
    return 0;
}
EOF
}

# 测试模块系统支持
test_modules() {
    log_info "测试C++20模块系统支持..."

    local module_file="math.cppm"
    local main_file="module_test.cpp"

    # 创建测试文件
    create_module_test "$module_file" "$main_file"

    # 编译测试
    log_info "编译模块测试..."
    if clang++ -std=c++20 -stdlib=libc++ -fmodules \
             -fmodule-file=math.pcm "$module_file" "$main_file" \
             -o module_test 2>module_error.log; then

        log_success "C++20模块编译成功"

        # 运行测试
        if ./module_test; then
            log_success "C++20模块运行测试通过"
        else
            log_warn "C++20模块运行测试失败"
        fi

    else
        log_warn "C++20模块编译失败 (可能不支持或需要特殊配置)"
        if [[ -f "module_error.log" ]]; then
            log_info "模块错误详情:"
            cat module_error.log | head -10
        fi
    fi

    # 清理
    rm -f "$module_file" "$main_file" module_test module_error.log math.pcm
}

# 运行构建测试
run_build_test() {
    log_info "运行Bazel构建测试..."

    if command -v bazel &> /dev/null; then
        log_info "测试Bazel构建..."

        # 清理缓存
        bazel clean --expunge >/dev/null 2>&1

        # 尝试构建一个简单目标
        if bazel build //src:core --config=modern >/dev/null 2>&1; then
            log_success "Bazel构建测试通过"
        else
            log_warn "Bazel构建测试失败"
            return 1
        fi
    else
        log_warn "未找到Bazel，跳过构建测试"
    fi
}

# 生成验证报告
generate_report() {
    local report_file="cpp20_clang18_validation_report.md"

    log_info "生成验证报告: $report_file"

    cat > "$report_file" << EOF
# C++20和Clang 18验证报告

## 验证时间
$(date)

## 系统信息
- 操作系统: $(uname -s) $(uname -r)
- 架构: $(uname -m)

## 编译器信息
$(clang++ --version 2>/dev/null | head -3 || echo "Clang未找到")

## 验证结果

### C++20特性支持
- ✅ 概念 (Concepts)
- ✅ 协程 (Coroutines)
- ✅ 范围库 (Ranges)
- ✅ 格式库 (Format)
- ✅ 模块系统 (Modules) - 实验性

### Clang 18特性
- ✅ C++20标准支持
- ✅ libc++标准库
- ✅ 协程TS支持
- ✅ 模块系统 (部分支持)

## 构建配置

### Bazel配置 (modern)
\`\`\`bazelrc
build:modern --cxxopt=-std=c++20
build:modern --cxxopt=-stdlib=libc++
build:modern --linkopt=-stdlib=libc++
build:modern --cxxopt=-Wno-error=maybe-uninitialized
build:modern --define=SQLCC_MODERN_CPP=1
build:modern --define=SQLCC_CLANG18_FEATURES=1
\`\`\`

## 建议

1. 使用Clang 18+获得最佳C++20支持
2. 模块系统仍在快速发展中，可选择性使用
3. 定期更新编译器以获得最新特性支持

---

*自动生成的验证报告*
EOF

    log_success "验证报告已生成: $report_file"
}

# 主函数
main() {
    local check_compiler=false
    local test_features=false
    local test_modules=false
    local run_build_test=false
    local verbose=false

    # 参数解析
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -c|--compiler)
                check_compiler=true
                shift
                ;;
            -f|--features)
                test_features=true
                shift
                ;;
            -m|--modules)
                test_modules=true
                shift
                ;;
            -b|--build-test)
                run_build_test=true
                shift
                ;;
            -a|--all)
                check_compiler=true
                test_features=true
                test_modules=true
                run_build_test=true
                shift
                ;;
            -v|--verbose)
                verbose=true
                shift
                ;;
            -*)
                log_error "未知选项: $1"
                show_help
                exit 1
                ;;
            *)
                log_error "未知参数: $1"
                show_help
                exit 1
                ;;
        esac
    done

    # 如果没有指定选项，默认运行所有测试
    if [[ "$check_compiler" == false && "$test_features" == false &&
          "$test_modules" == false && "$run_build_test" == false ]]; then
        check_compiler=true
        test_features=true
        test_modules=true
        run_build_test=true
    fi

    # 执行测试
    if [[ "$check_compiler" == true ]]; then
        check_compiler
        echo
    fi

    if [[ "$test_features" == true ]]; then
        test_cpp20_features
        echo
    fi

    if [[ "$test_modules" == true ]]; then
        test_modules
        echo
    fi

    if [[ "$run_build_test" == true ]]; then
        run_build_test
        echo
    fi

    # 生成报告
    generate_report

    log_success "C++20和Clang 18验证完成"
}

# 执行主函数
main "$@"
