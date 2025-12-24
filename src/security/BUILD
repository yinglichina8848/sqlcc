"""内存安全监控模块构建配置"""

load("@rules_cc"//cc:defs.bzl"", "cc_library")


cc_library(
    name = "security",
    srcs = [
        "memory_monitor.cpp",
    ],
    deps = [
        "//include/security:memory_monitor",
    ],
    copts = ["-std=c++17"],
    visibility = ["//visibility:public"],
)
