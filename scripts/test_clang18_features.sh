#!/bin/bash

# Clang 18 功能验证脚本
# 测试 C++20 modules、GTest、覆盖率测试、内存安全性测试支持

set -e

echo "=== Clang 18 功能验证脚本 ==="
echo "验证时间: $(date)"
echo

# 检查 Clang 18 是否安装
check_clang18() {
    echo "1. 检查 Clang 18 安装状态..."
    if command -v clang++-18 &> /dev/null; then
        echo "✅ Clang 18 已安装"
        clang++-18 --version
        return 0
    else
        echo "❌ Clang 18 未安装"
        echo "请运行: sudo apt install clang-20 clang++-20 libc++-20-dev libc++abi-20-dev"
        return 1
    fi
}

# 测试 C++20 modules 支持
test_modules() {
    echo
    echo "2. 测试 C++20 Modules 支持..."

    # 创建测试模块
    cat > test_module.cppm << 'EOF'
// 测试模块
export module test.math;

import <iostream>;

export namespace test {
    export int add(int a, int b) {
        return a + b;
    }

    export void print_result(int result) {
        std::cout << "Result: " << result << std::endl;
    }
}
EOF

    # 创建使用模块的代码
    cat > test_module_user.cpp << 'EOF'
import test.math;

int main() {
    int result = test::add(5, 3);
    test::print_result(result);
    return 0;
}
EOF

    if command -v clang++-18 &> /dev/null; then
        echo "编译测试模块..."
        clang++-18 --std=c++20 -c test_module.cppm -o test_module.o

        echo "编译并链接测试程序..."
        clang++-18 --std=c++20 test_module_user.cpp test_module.o -o test_module_app

        echo "运行测试程序..."
        ./test_module_app

        echo "✅ C++20 Modules 支持正常"
        rm -f test_module.cppm test_module_user.cpp test_module.o test_module_app
    else
        echo "⚠️ 跳过实际编译测试（Clang 18未安装）"
        echo "预期输出格式:"
        echo "Result: 8"
    fi
}

# 测试 GTest 支持
test_gtest() {
    echo
    echo "3. 测试 Google Test 支持..."

    # 检查 GTest 是否安装
    if pkg-config --exists gtest; then
        echo "✅ GTest 已安装"
        pkg-config --modversion gtest
    else
        echo "❌ GTest 未安装"
        echo "请运行: sudo apt install libgtest-dev"
        return 1
    fi

    # 创建 GTest 测试
    cat > test_gtest.cpp << 'EOF'
#include <gtest/gtest.h>

TEST(MathTest, Addition) {
    EXPECT_EQ(2 + 2, 4);
    EXPECT_EQ(10 + 5, 15);
}

TEST(MathTest, Multiplication) {
    EXPECT_EQ(3 * 4, 12);
    EXPECT_EQ(0 * 5, 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
EOF

    if command -v clang++-18 &> /dev/null && pkg-config --exists gtest; then
        echo "编译 GTest 测试..."
        clang++-18 --std=c++20 test_gtest.cpp -o test_gtest_app $(pkg-config --cflags --libs gtest)

        echo "运行 GTest 测试..."
        ./test_gtest_app

        echo "✅ GTest 支持正常"
        rm -f test_gtest.cpp test_gtest_app
    else
        echo "⚠️ 跳过实际测试（Clang 18或GTest未安装）"
        echo "预期输出格式:"
        echo "[==========] Running 2 tests from 1 test suite."
        echo "[----------] Global test environment set-up."
        echo "[----------] 2 tests from MathTest"
        echo "[ RUN      ] MathTest.Addition"
        echo "[       OK ] MathTest.Addition (0 ms)"
        echo "[ RUN      ] MathTest.Multiplication"
        echo "[       OK ] MathTest.Multiplication (0 ms)"
        echo "[----------] 2 tests from MathTest (0 ms total)"
        echo "[----------] Global test environment tear-down (0 ms total)"
        echo "[==========] 2 tests from 1 test suite ran. (0 ms total)"
        echo "[  PASSED  ] 2 tests."
    fi
}

# 测试覆盖率支持
test_coverage() {
    echo
    echo "4. 测试覆盖率测试支持..."

    # 检查 LLVM coverage 工具
    if command -v llvm-cov &> /dev/null || command -v llvm-cov-18 &> /dev/null; then
        echo "✅ LLVM coverage 工具可用"
    else
        echo "❌ LLVM coverage 工具不可用"
        echo "请运行: sudo apt install llvm-18"
        return 1
    fi

    # 创建测试代码
    cat > test_coverage.cpp << 'EOF'
#include <iostream>

bool is_even(int n) {
    return n % 2 == 0;
}

int main() {
    std::cout << "Testing coverage..." << std::endl;

    for (int i = 0; i < 5; ++i) {
        if (is_even(i)) {
            std::cout << i << " is even" << std::endl;
        } else {
            std::cout << i << " is odd" << std::endl;
        }
    }

    return 0;
}
EOF

    if command -v clang++-18 &> /dev/null && (command -v llvm-cov &> /dev/null || command -v llvm-cov-18 &> /dev/null); then
        echo "编译带覆盖率信息的程序..."
        clang++-18 --std=c++20 --coverage -fprofile-instr-generate -fcoverage-mapping \
                 test_coverage.cpp -o test_coverage_app

        echo "运行程序生成覆盖率数据..."
        ./test_coverage_app

        echo "生成覆盖率报告..."
        llvm-profdata merge -sparse default.profraw -o default.profdata
        llvm-cov show ./test_coverage_app -instr-profile=default.profdata

        echo "✅ 覆盖率测试支持正常"
        rm -f test_coverage.cpp test_coverage_app *.profraw *.profdata
    else
        echo "⚠️ 跳过实际测试（Clang 18或llvm-cov未安装）"
        echo "预期会生成覆盖率数据文件和HTML报告"
    fi
}

# 测试内存安全性支持
test_memory_safety() {
    echo
    echo "5. 测试内存安全性测试支持..."

    # 创建测试代码
    cat > test_memory.cpp << 'EOF'
#include <iostream>
#include <vector>
#include <memory>

void test_vector_access() {
    std::vector<int> vec = {1, 2, 3, 4, 5};

    // 边界检查访问
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << "vec[" << i << "] = " << vec[i] << std::endl;
    }

    // 测试智能指针
    auto ptr = std::make_unique<int>(42);
    std::cout << "Smart pointer value: " << *ptr << std::endl;
}

int main() {
    test_vector_access();
    return 0;
}
EOF

    if command -v clang++-18 &> /dev/null; then
        echo "编译带内存安全性检查的程序..."
        clang++-18 --std=c++20 -fsanitize=address -fsanitize=undefined \
                 test_memory.cpp -o test_memory_app

        echo "运行内存安全性测试..."
        ./test_memory_app

        echo "✅ 内存安全性测试支持正常"
        rm -f test_memory.cpp test_memory_app
    else
        echo "⚠️ 跳过实际测试（Clang 18未安装）"
        echo "Sanitizers 支持包括:"
        echo "  - AddressSanitizer (-fsanitize=address)"
        echo "  - UndefinedBehaviorSanitizer (-fsanitize=undefined)"
        echo "  - ThreadSanitizer (-fsanitize=thread)"
        echo "  - MemorySanitizer (-fsanitize=memory)"
    fi
}

# 主函数
main() {
    echo "开始 Clang 18 功能验证..."
    echo

    check_clang18
    CLANG_INSTALLED=$?

    test_modules
    test_gtest
    test_coverage
    test_memory_safety

    echo
    echo "=== 验证总结 ==="

    if [ $CLANG_INSTALLED -eq 0 ]; then
        echo "✅ Clang 18 已安装，所有功能均支持"
        echo
        echo "推荐的编译选项:"
        echo "  C++20 Modules: clang++-18 --std=c++20 file.cppm"
        echo "  GTest: clang++-18 --std=c++20 test.cpp \$(pkg-config --cflags --libs gtest)"
        echo "  覆盖率: clang++-18 --std=c++20 --coverage -fprofile-instr-generate -fcoverage-mapping"
        echo "  内存检查: clang++-18 --std=c++20 -fsanitize=address -fsanitize=undefined"
    else
        echo "❌ Clang 18 未安装"
        echo
        echo "安装命令:"
        echo "  sudo apt update"
        echo "  sudo apt install clang-18 clang++-18 libc++-18-dev libc++abi-18-dev"
        echo "  sudo apt install libgtest-dev llvm-18"  # 可选，用于测试和覆盖率
    fi

    echo
    echo "验证完成时间: $(date)"
}

# 运行主函数
main "$@"
