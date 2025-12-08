# OpenSSL第三方依赖构建配置

load("@rules_foreign_cc//foreign_cc:defs.bzl", "configure_make")

configure_make(
    name = "openssl",
    lib_source = "@openssl//:srcs",
    out_static_libs = ["libssl.a", "libcrypto.a"],
    out_shared_libs = ["libssl.so", "libcrypto.so"],
    configure_options = [
        "--prefix=/usr/local",
        "--openssldir=/usr/local/ssl",
        "no-shared",
        "no-asm",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "ssl",
    hdrs = glob(["include/openssl/*.h"]),
    includes = ["include"],
    linkopts = ["-lssl", "-lcrypto"],
    visibility = ["//visibility:public"],
    deps = [":openssl"],
)

cc_library(
    name = "crypto",
    hdrs = glob(["include/openssl/*.h"]),
    includes = ["include"],
    linkopts = ["-lcrypto"],
    visibility = ["//visibility:public"],
    deps = [":openssl"],
)