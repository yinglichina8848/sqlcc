# Abseil库的自定义构建配置
# 用于解决系统头文件依赖问题

cc_library(
    name = "absl_base",
    srcs = glob([
        "absl/base/*.cc",
    ]),
    hdrs = glob([
        "absl/base/*.h",
    ]),
    copts = [
        "-I/usr/include",
        "-I/usr/include/c++/15",
        "-I/usr/include/x86_64-linux-gnu/c++/15",
        "-I/usr/lib/gcc/x86_64-linux-gnu/15/include",
    ],
    includes = [
        ".",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "absl_strings",
    srcs = glob([
        "absl/strings/*.cc",
    ], exclude = [
        "absl/strings/internal/*.cc",
    ]),
    hdrs = glob([
        "absl/strings/*.h",
    ]),
    copts = [
        "-I/usr/include",
        "-I/usr/include/c++/15",
        "-I/usr/include/x86_64-linux-gnu/c++/15",
        "-I/usr/lib/gcc/x86_64-linux-gnu/15/include",
    ],
    includes = [
        ".",
    ],
    deps = [
        ":absl_base",
    ],
    visibility = ["//visibility:public"],
)