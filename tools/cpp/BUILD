load(":cc_toolchain_config.bzl", "cc_toolchain_config")

package(default_visibility = ["//visibility:public"])

cc_toolchain_config(name = "local_cc_toolchain_config")

filegroup(
    name = "empty",
    srcs = [],
)

cc_toolchain_suite(
    name = "toolchain",
    toolchains = {
        "k8": "local_cc_toolchain",
    },
)

cc_toolchain(
    name = "local_cc_toolchain",
    toolchain_config = ":local_cc_toolchain_config",
    all_files = ":empty",
    compiler_files = ":empty",
    strip_files = ":empty",
    objcopy_files = ":empty",
    linker_files = ":empty",
    dwp_files = ":empty",
    toolchain_identifier = "local_cc_toolchain",
)