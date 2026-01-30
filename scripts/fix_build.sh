#!/bin/bash
# Script to add complex_subquery_test to BUILD.bazel

BUILD_FILE="tests/unit/parser/BUILD.bazel"

# Find the line after expression_parser_test ends and add complex_subquery_test
line_num=$(grep -n 'name = "expression_parser_test"' "$BUILD_FILE" | cut -d: -f1)
if [ -n "$line_num" ]; then
    # Find the end of the expression_parser_test block (the closing parenthesis)
    end_line=$(sed -n "${line_num},\$p" "$BUILD_FILE" | grep -n "^)" | head -1 | cut -d: -f1)
    end_line=$((line_num + end_line - 1))
    
    # Insert complex_subquery_test configuration after expression_parser_test
    sed -i "${end_line}r -" "$BUILD_FILE" << 'EOF'

cc_test(
    name = "complex_subquery_test",
    srcs = ["complex_subquery_test.cpp"],
    copts = [
        "-std=c++20",
        "-stdlib=libc++",
        "-Iinclude",
    ],
    linkopts = [
        "-stdlib=libc++",
        "-lc++abi",
    ],
    deps = [
        "@com_google_googletest//:gtest_main",
        "//src/sql_parser:sql_parser",
    ],
)
EOF
    
    # Add complex_subquery_test to test_suite tests list
    sed -i '/":expression_parser_test",/i\        ":complex_subquery_test",' "$BUILD_FILE"
    
    echo "Successfully added complex_subquery_test to BUILD.bazel"
else
    echo "Could not find expression_parser_test in BUILD.bazel"
fi
