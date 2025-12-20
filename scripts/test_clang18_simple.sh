#!/bin/bash

# Clang 18 简化功能验证脚本
# 验证基本的C++20功能和编译器可用性

set -e

echo "=== Clang 18 简化功能验证脚本 ==="
echo "验证时间: $(date)"
echo

# 检查 Clang 18 是否安装
check_clang18() {
    echo "1. 检查 Clang 18 安装状态..."
    if command -v clang++-18 &> /dev/null; then
        echo "✅ Clang 18 已安装"
        clang++-18 --version | head -2
        return 0
    else
        echo "❌ Clang 18 未安装"
        echo "请运行: sudo apt install clang-18 clang++-18"
        return 1
    fi
}

# 测试基本 C++20 编译
test_basic_cpp20() {
    echo
    echo "2. 测试基本 C++20 编译..."

    cat > test_basic.cpp << 'EOF'
// 基本 C++20 功能测试
#include <iostream>
#include <vector>
#include <string>

int main() {
    std::vector<std::string> args = {"Hello", "C++20", "World"};

    for (const auto& arg : args) {
        std::cout << arg << " ";
    }
    std::cout << std::endl;

    // 测试 auto 类型推导
    auto message = "Compilation successful!";
    std::cout << message << std::endl;

    return 0;
}
EOF

    if command -v clang++-18 &> /dev/null; then
        echo "编译测试程序..."
        clang++-18 --std=c++20 test_basic.cpp -o test_basic

        echo "运行测试程序..."
        ./test_basic

        echo "✅ 基本 C++20 编译正常"
        rm -f test_basic.cpp test_basic
    else
        echo "⚠️ 跳过实际测试（Clang 18未安装）"
    fi
}

# 测试 GTest 编译
test_gtest_compile() {
    echo
    echo "3. 测试 Google Test 编译..."

    # 检查 GTest
    if pkg-config --exists gtest 2>/dev/null; then
        echo "✅ GTest 已安装"
        pkg-config --modversion gtest 2>/dev/null || echo "版本信息不可用"
    else
        echo "❌ GTest 未安装"
        echo "可安装命令: sudo apt install libgtest-dev"
        return 1
    fi

    cat > test_gtest.cpp << 'EOF'
#include <gtest/gtest.h>

TEST(BasicTest, AlwaysTrue) {
    EXPECT_TRUE(true);
    EXPECT_EQ(2 + 2, 4);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
EOF

    if command -v clang++-18 &> /dev/null && pkg-config --exists gtest 2>/dev/null; then
        echo "编译 GTest 程序..."
        clang++-18 --std=c++20 test_gtest.cpp -o test_gtest $(pkg-config --cflags --libs gtest 2>/dev/null)

        echo "运行 GTest 程序..."
        ./test_gtest

        echo "✅ GTest 编译和运行正常"
        rm -f test_gtest.cpp test_gtest
    else
        echo "⚠️ 跳过实际测试（依赖未满足）"
    fi
}

# 测试覆盖率编译
test_coverage_compile() {
    echo
    echo "4. 测试覆盖率编译支持..."

    cat > test_coverage.cpp << 'EOF'
#include <iostream>

void test_function() {
    std::cout << "Testing coverage instrumentation" << std::endl;
}

int main() {
    test_function();
    return 0;
}
EOF

    if command -v clang++-18 &> /dev/null; then
        echo "编译带覆盖率信息的程序..."
        clang++-18 --std=c++20 --coverage -fprofile-instr-generate \
                 -fcoverage-mapping test_coverage.cpp -o test_coverage

        echo "运行程序..."
        ./test_coverage

        echo "✅ 覆盖率编译支持正常"
        rm -f test_coverage.cpp test_coverage *.profraw *.profdata
    else
        echo "⚠️ 跳过实际测试（Clang 18未安装）"
    fi
}

# 测试内存安全性编译
test_sanitizer_compile() {
    echo
    echo "5. 测试内存安全性编译支持..."

    cat > test_sanitizer.cpp << 'EOF'
#include <iostream>
#include <vector>

int main() {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::cout << "Vector size: " << vec.size() << std::endl;

    // 安全的访问
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << "vec[" << i << "] = " << vec[i] << std::endl;
    }

    return 0;
}
EOF

    if command -v clang++-18 &> /dev/null; then
        echo "编译带 AddressSanitizer 的程序..."
        clang++-18 --std=c++20 -fsanitize=address -fsanitize=undefined \
                 test_sanitizer.cpp -o test_sanitizer

        echo "运行程序..."
        ./test_sanitizer

        echo "✅ 内存安全性编译支持正常"
        rm -f test_sanitizer.cpp test_sanitizer
    else
        echo "⚠️ 跳过实际测试（Clang 18未安装）"
    fi
}

# 总结报告
print_summary() {
    echo
    echo "=== 验证总结 ==="

    if command -v clang++-18 &> /dev/null; then
        echo "✅ Clang 18 编译器: 已安装"
        echo "✅ C++20 标准支持: 可用"
        echo "✅ 编译器功能: 基本验证通过"

        if pkg-config --exists gtest 2>/dev/null; then
            echo "✅ Google Test: 已安装"
        else
            echo "⚠️ Google Test: 未安装 (可选)"
        fi

        echo
        echo "推荐的 C++20 开发工作流:"
        echo "  基本编译: clang++-18 --std=c++20 file.cpp -o program"
        echo "  GTest编译: clang++-18 --std=c++20 test.cpp \$(pkg-config --cflags --libs gtest)"
        echo "  覆盖率: clang++-18 --std=c++20 --coverage -fprofile-instr-generate -fcoverage-mapping"
        echo "  内存检查: clang++-18 --std=c++20 -fsanitize=address -fsanitize=undefined"

    else
        echo "❌ Clang 18 编译器: 未安装"
        echo
        echo "安装命令:"
        echo "  sudo apt update"
        echo "  sudo apt install clang-18 clang++-18"
        echo "  sudo apt install libgtest-dev llvm-18  # 可选，用于测试和覆盖率"
    fi

    echo
    echo "注意事项:"
    echo "  - C++20 modules 在 Clang 18 中仍处于发展阶段"
    echo "  - 建议从传统头文件开始，逐步迁移到 modules"
    echo "  - 生产环境使用时请进行充分测试"

    echo
    echo "验证完成时间: $(date)"
}

# 主函数
main() {
    echo "开始 Clang 18 基础功能验证..."
    echo

    check_clang18
    CLANG_INSTALLED=$?

    test_basic_cpp20
    test_gtest_compile
    test_coverage_compile
    test_sanitizer_compile

    print_summary
}

# 运行主函数
main "$@"
