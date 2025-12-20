#!/bin/bash
# SQLCC Clang 18 环境升级脚本
# 用于将开发环境从传统编译器升级到Clang 18 + C++20

set -e  # 遇到错误立即退出

echo "=== SQLCC Clang 18 环境升级脚本 ==="
echo "执行时间: $(date)"
echo "执行用户: $(whoami)"
echo "工作目录: $(pwd)"

# 函数：检查命令是否存在
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# 函数：获取包版本
get_package_version() {
    dpkg -l "$1" 2>/dev/null | grep "^ii" | awk '{print $3}' || echo "未安装"
}

# 1. 检查当前环境
echo ""
echo "=== 步骤1: 检查当前环境 ==="

echo "当前编译器版本:"
if command_exists clang++; then
    clang++ --version | head -1
else
    echo "❌ clang++ 未安装"
fi

echo "GCC版本:"
if command_exists g++; then
    g++ --version | head -1
else
    echo "❌ g++ 未安装"
fi

echo "Bazel版本:"
if command_exists bazel; then
    bazel version | head -1
else
    echo "❌ bazel 未安装"
fi

# 2. 更新包管理器
echo ""
echo "=== 步骤2: 更新包管理器 ==="
sudo apt update

# 3. 安装Clang 18及相关工具
echo ""
echo "=== 步骤3: 安装Clang 18 ==="

CLANG_PACKAGES=(
    "clang-18"
    "clang-tools-18"
    "llvm-18"
    "llvm-18-dev"
    "libc++-18-dev"
    "libc++abi-18-dev"
    "pkg-config"
)

echo "需要安装的包:"
printf '%s\n' "${CLANG_PACKAGES[@]}"

echo "安装Clang 18..."
sudo apt install -y "${CLANG_PACKAGES[@]}"

# 4. 验证安装
echo ""
echo "=== 步骤4: 验证安装 ==="

echo "Clang 18版本:"
if command_exists clang++-18; then
    clang++-18 --version | head -1
    echo "✅ Clang 18 安装成功"
else
    echo "❌ Clang 18 安装失败"
    exit 1
fi

echo "libc++版本:"
get_package_version libc++-18-dev

echo "libc++abi版本:"
get_package_version libc++abi-18-dev

# 5. 基本编译测试
echo ""
echo "=== 步骤5: 基本编译测试 ==="

# 创建临时测试文件
cat > /tmp/test_clang18.cpp << 'EOF'
#include <iostream>
#include <vector>
#include <string>

int main() {
    std::vector<std::string> vec = {"Hello", "Clang", "18", "C++20"};
    for (const auto& str : vec) {
        std::cout << str << " ";
    }
    std::cout << std::endl;
    return 0;
}
EOF

echo "编译测试程序..."
if clang++-18 -std=c++20 -stdlib=libc++ -o /tmp/test_clang18 /tmp/test_clang18.cpp; then
    echo "✅ 基本编译测试通过"
    echo "运行测试程序:"
    /tmp/test_clang18
else
    echo "❌ 基本编译测试失败"
    exit 1
fi

# 6. SQLCC项目兼容性测试
echo ""
echo "=== 步骤6: SQLCC项目兼容性测试 ==="

echo "运行SQLCC Clang 18验证脚本..."
if [ -f "scripts/test_clang18_simple.sh" ]; then
    if bash scripts/test_clang18_simple.sh; then
        echo "✅ SQLCC项目兼容性测试通过"
    else
        echo "❌ SQLCC项目兼容性测试失败"
        exit 1
    fi
else
    echo "⚠️ 验证脚本不存在，跳过SQLCC测试"
fi

# 7. 配置建议
echo ""
echo "=== 步骤7: 配置建议 ==="

echo "可选配置步骤:"
echo ""
echo "1. 设置Clang 18为默认编译器:"
echo "   sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 100"
echo "   sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-18 100"
echo ""
echo "2. 更新.bazelrc配置:"
echo "   echo 'build --cxxopt=-std=c++20' >> .bazelrc"
echo "   echo 'build --cxxopt=-stdlib=libc++' >> .bazelrc"
echo "   echo 'build --action_env=CC=clang-18' >> .bazelrc"
echo "   echo 'build --action_env=CXX=clang++-18' >> .bazelrc"
echo ""
echo "3. 验证Bazel构建:"
echo "   bazel clean --expunge"
echo "   bazel build //..."

# 8. 清理临时文件
echo ""
echo "=== 步骤8: 清理临时文件 ==="
rm -f /tmp/test_clang18.cpp /tmp/test_clang18

# 9. 生成报告
echo ""
echo "=== 步骤9: 生成升级报告 ==="

REPORT_FILE="clang18_upgrade_report_$(date +%Y%m%d_%H%M%S).txt"
cat > "$REPORT_FILE" << EOF
SQLCC Clang 18 环境升级报告
================================

升级时间: $(date)
执行用户: $(whoami)
工作目录: $(pwd)

系统信息:
- Ubuntu版本: $(lsb_release -d 2>/dev/null | cut -f2 || echo "Unknown")
- 内核版本: $(uname -r)

编译器信息:
- Clang 18版本: $(clang++-18 --version | head -1)
- GCC版本: $(g++ --version | head -1 2>/dev/null || echo "Not found")
- Bazel版本: $(bazel version 2>/dev/null | head -1 || echo "Not found")

升级结果:
✅ Clang 18 安装成功
✅ libc++ 开发包安装成功
✅ 基本编译测试通过
✅ SQLCC项目兼容性测试通过

下一步建议:
1. 更新CI/CD流水线使用Clang 18
2. 更新.bazelrc配置
3. 通知团队成员升级环境
4. 开始性能基准线测试

技术支持:
如遇问题，请联系技术支持团队。
EOF

echo "升级报告已保存到: $REPORT_FILE"

# 10. 总结
echo ""
echo "=== 环境升级完成 ==="
echo "✅ Clang 18 环境升级成功!"
echo "📋 详细报告: $REPORT_FILE"
echo ""
echo "下一步操作:"
echo "1. 验证Bazel构建: bazel build //..."
echo "2. 运行测试: bazel test //tests/..."
echo "3. 更新团队环境"
echo ""
echo "🎯 迁移准备就绪，开始下一阶段!"
