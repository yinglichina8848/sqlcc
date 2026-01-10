"""内存安全测试框架构建配置"""

load("@rules_cc//cc:defs.bzl", "cc_test")


cc_test(
    name = "memory_safety_framework",
    srcs = ["memory_safety_framework.cpp"],
    deps = [
        "@com_google_googletest//:gtest_main",
        "//src/core:sqlcc_core_lib",
        "//src/security",
    ],
    copts = [
        "-std=c++17",
        "-fsanitize=address",
        "-fsanitize=undefined",
        "-fno-omit-frame-pointer",
        "-g",
    ],
    linkopts = [
        "-fsanitize=address",
        "-fsanitize=undefined",
    ],
    tags = [
        "memory_safety",
        "security",
        "requires_memory",
    ],
)

# 内存安全专项测试套件
cc_test(
    name = "memory_leak_detection_test",
    srcs = ["memory_safety_framework.cpp"],
    deps = [
        "@com_google_googletest//:gtest_main",
        "//src/core:sqlcc_core_lib",
    ],
    copts = [
        "-std=c++17",
        "-fsanitize=address",
        "-g",
    ],
    linkopts = [
        "-fsanitize=address",
    ],
    args = [
        "--gtest_filter=MemorySafetyFrameworkTest.MemoryLeakDetection",
    ],
    tags = ["memory_leak"],
)

cc_test(
    name = "smart_pointer_coverage_test",
    srcs = ["memory_safety_framework.cpp"],
    deps = [
        "@com_google_googletest//:gtest_main",
    ],
    copts = ["-std=c++17", "-g"],
    args = [
        "--gtest_filter=MemorySafetyFrameworkTest.SmartPointerCoverage",
    ],
    tags = ["smart_pointer"],
)

cc_test(
    name = "concurrent_memory_safety_test",
    srcs = ["memory_safety_framework.cpp"],
    deps = [
        "@com_google_googletest//:gtest_main",
    ],
    copts = ["-std=c++17", "-g"],
    args = [
        "--gtest_filter=MemorySafetyFrameworkTest.ConcurrentMemorySafety",
    ],
    tags = ["concurrent", "thread_safety"],
)

# 内存安全测试套件
cc_test(
    name = "memory_safety_test_suite",
    srcs = ["memory_safety_framework.cpp"],
    deps = [
        "@com_google_googletest//:gtest_main",
        "//src/core:sqlcc_core_lib",
        "//src/security",
    ],
    copts = ["-std=c++17", "-g"],
    args = [
        "--gtest_filter=MemorySafetyFrameworkTest.*",
    ],
    tags = ["memory_safety_suite"],
)
