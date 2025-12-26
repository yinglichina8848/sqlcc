# SQLCC Bazel覆盖率测试BUILD配置文件模板
# 用于在保证测试运行的同时收集覆盖率数据

def sqlcc_coverage_test(name, srcs, deps, **kwargs):
    """
    封装的覆盖率测试规则，确保测试运行时收集覆盖率数据

    Args:
        name: 测试目标名称
        srcs: 源文件列表
        deps: 依赖列表
        **kwargs: 其他Bazel测试参数
    """

    # 基础测试配置
    native.cc_test(
        name = name,
        srcs = srcs,
        deps = deps + [
            # 确保必要的覆盖率运行时依赖
            "@com_google_googletest//:gtest_main",
        ],
        copts = [
            "-std=c++20",
            "-stdlib=libc++",
            # 覆盖率编译选项
            "-fprofile-instr-generate",
            "-fcoverage-mapping",
            "-O0",  # 调试模式优化覆盖率收集
            "-g",
        ],
        linkopts = [
            "-stdlib=libc++",
            "-lc++abi",
            # 覆盖率链接选项
            "-fprofile-instr-generate",
            "-fcoverage-mapping",
        ],
        # 测试环境变量设置
        env = {
            "LLVM_PROFILE_FILE": "/tmp/coverage/%p.profraw",
            "GCOV_PREFIX": "/tmp/coverage",
            "GCOV_PREFIX_STRIP": "1",
        },
        # 测试标签，标记为覆盖率测试
        tags = ["coverage", "manual"] + kwargs.get("tags", []),
        # 测试超时设置
        timeout = "moderate",
        **kwargs
    )

def sqlcc_coverage_test_suite(name, tests):
    """
    创建覆盖率测试套件

    Args:
        name: 测试套件名称
        tests: 测试目标列表
    """
    native.test_suite(
        name = name,
        tests = tests,
        tags = ["coverage", "manual"],
    )

def configure_coverage_collection():
    """
    配置覆盖率收集设置

    返回覆盖率相关的编译和链接选项
    """
    coverage_copts = [
        "-fprofile-instr-generate",
        "-fcoverage-mapping",
        "-O0",  # 调试模式
        "-g",
        "--coverage",  # GCC兼容选项
    ]

    coverage_linkopts = [
        "-fprofile-instr-generate",
        "-fcoverage-mapping",
        "--coverage",
    ]

    return coverage_copts, coverage_linkopts

def sqlcc_coverage_library(name, srcs, hdrs, deps, **kwargs):
    """
    带覆盖率支持的库定义

    Args:
        name: 库名称
        srcs: 源文件
        hdrs: 头文件
        deps: 依赖
        **kwargs: 其他参数
    """
    coverage_copts, coverage_linkopts = configure_coverage_collection()

    native.cc_library(
        name = name,
        srcs = srcs,
        hdrs = hdrs,
        deps = deps,
        copts = coverage_copts + kwargs.get("copts", []),
        linkopts = coverage_linkopts + kwargs.get("linkopts", []),
        **kwargs
    )

# 覆盖率报告生成配置
def generate_coverage_report(name, test_targets, output_dir = "coverage_report"):
    """
    生成覆盖率报告的规则

    Args:
        name: 报告生成目标名称
        test_targets: 要收集覆盖率的测试目标列表
        output_dir: 输出目录
    """

    # 合并覆盖率数据的脚本
    merge_script = """
#!/bin/bash
set -e

echo "合并覆盖率数据..."
mkdir -p {output_dir}

# 查找所有profraw文件
find /tmp/coverage -name "*.profraw" -exec echo "Found: {{}}" \\;

# 使用llvm-profdata合并数据
llvm-profdata merge -o {output_dir}/coverage.profdata /tmp/coverage/*.profraw 2>/dev/null || echo "No profraw files found"

# 生成覆盖率报告
llvm-cov report \\
    --instr-profile={output_dir}/coverage.profdata \\
    --object="$1" \\
    --format=html \\
    --output-dir={output_dir}/html \\
    --show-functions \\
    --show-line-counts-or-regions \\
    --ignore-filename-regex=".*test.*" \\
    --ignore-filename-regex=".*Test.*" \\
    --ignore-filename-regex=".*gtest.*" \\
    --ignore-filename-regex=".*gmock.*" \\
    --ignore-filename-regex="third_party/.*" \\
    --ignore-filename-regex="external/.*" \\
    > {output_dir}/coverage.txt

echo "覆盖率报告生成完成: {output_dir}/coverage.txt"
echo "HTML报告: {output_dir}/html/index.html"
""".format(output_dir = output_dir)

    native.genrule(
        name = name + "_merge_coverage",
        srcs = [],  # 依赖测试目标
        outs = [output_dir + "/coverage.txt"],
        cmd = merge_script,
        executable = True,
        tags = ["coverage", "manual"],
    )

# 使用示例：
#
# load("//tools:bazel_coverage_build_template.bzl", "sqlcc_coverage_test", "sqlcc_coverage_test_suite", "generate_coverage_report")
#
# sqlcc_coverage_test(
#     name = "my_unit_test",
#     srcs = ["my_unit_test.cpp"],
#     deps = [
#         "//src/my_library",
#         "@com_google_googletest//:gtest_main",
#     ],
# )
#
# sqlcc_coverage_test_suite(
#     name = "unit_tests_with_coverage",
#     tests = [
#         ":my_unit_test",
#         # 其他测试...
#     ],
# )
#
# generate_coverage_report(
#     name = "coverage_report",
#     test_targets = [":unit_tests_with_coverage"],
#     output_dir = "test_coverage",
# )
