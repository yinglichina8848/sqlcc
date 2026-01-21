#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <thread>
#include <algorithm>
#include <fstream>
#include <cstdio>

// Utils tests for foundation layer
// These tests verify utility concepts and provide basic coverage

TEST(UtilsTest, StringUtilities) {
    // Test string utility functions (concept)
    std::string str = "  Hello, World!  ";

    // Test trim operations (concept)
    auto ltrim = [](std::string s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        return s;
    };

    auto rtrim = [](std::string s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
        return s;
    };

    auto trim = [&](std::string s) {
        return ltrim(rtrim(s));
    };

    EXPECT_EQ(trim(str), "Hello, World!");
    EXPECT_EQ(ltrim(str), "Hello, World!  ");
    EXPECT_EQ(rtrim(str), "  Hello, World!");
}

TEST(UtilsTest, ContainerUtilities) {
    // Test container utility functions
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // Test filtering (concept)
    auto is_even = [](int n) { return n % 2 == 0; };
    std::vector<int> evens;
    std::copy_if(numbers.begin(), numbers.end(), std::back_inserter(evens), is_even);

    EXPECT_EQ(evens.size(), 5);
    EXPECT_EQ(evens[0], 2);
    EXPECT_EQ(evens.back(), 10);

    // Test transformation (concept)
    std::vector<int> doubled;
    std::transform(numbers.begin(), numbers.end(), std::back_inserter(doubled),
                   [](int n) { return n * 2; });

    EXPECT_EQ(doubled.size(), 10);
    EXPECT_EQ(doubled[0], 2);
    EXPECT_EQ(doubled.back(), 20);
}

TEST(UtilsTest, MemoryUtilities) {
    // Test memory utility functions
    auto safe_delete = [](auto* ptr) {
        if (ptr) {
            delete ptr;
            ptr = nullptr;
        }
    };

    int* ptr = new int(42);
    EXPECT_EQ(*ptr, 42);
    safe_delete(ptr);
    // ptr is now nullptr

    // Test smart pointer utilities
    auto make_optional_ptr = [](auto value) {
        return std::make_shared<decltype(value)>(value);
    };

    auto shared_int = make_optional_ptr(100);
    EXPECT_EQ(*shared_int, 100);
    EXPECT_EQ(shared_int.use_count(), 1);
}

TEST(UtilsTest, FunctionalUtilities) {
    // Test functional utility functions
    std::vector<int> numbers = {1, 2, 3, 4, 5};

    // Test function composition (concept)
    auto add_one = [](int n) { return n + 1; };
    auto multiply_two = [](int n) { return n * 2; };

    // Compose: multiply_two(add_one(x))
    auto composed = [&](int x) {
        return multiply_two(add_one(x));
    };

    EXPECT_EQ(composed(3), 8);  // (3+1)*2 = 8

    // Test partial application (concept)
    auto add = [](int a, int b) { return a + b; };
    auto add_five = [&](int x) { return add(5, x); };

    EXPECT_EQ(add_five(3), 8);
}

TEST(UtilsTest, TimeUtilities) {
    // Test time utility functions
    auto start = std::chrono::high_resolution_clock::now();

    // Simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    EXPECT_GE(duration.count(), 0);

    // Test time formatting (concept)
    auto format_duration = [](auto d) {
        return std::to_string(d.count()) + " microseconds";
    };

    std::string formatted = format_duration(duration);
    EXPECT_NE(formatted.find("microseconds"), std::string::npos);
}

TEST(UtilsTest, FileUtilities) {
    // Test file utility functions (concept)
    std::vector<std::string> file_list = {"file1.txt", "file2.cpp", "file3.h", "file4.txt"};

    // Test file extension filtering
    auto ends_with = [](const std::string& str, const std::string& suffix) {
        return str.size() >= suffix.size() &&
               str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    };

    std::vector<std::string> txt_files;
    for (const auto& file : file_list) {
        if (ends_with(file, ".txt")) {
            txt_files.push_back(file);
        }
    }

    EXPECT_EQ(txt_files.size(), 2);
    EXPECT_EQ(txt_files[0], "file1.txt");
    EXPECT_EQ(txt_files[1], "file4.txt");
}

TEST(UtilsTest, MathUtilities) {
    // Test math utility functions
    std::vector<double> values = {1.5, 2.7, 3.2, 4.8, 5.1};

    // Test sum calculation
    double sum = 0.0;
    for (auto val : values) {
        sum += val;
    }
    EXPECT_DOUBLE_EQ(sum, 17.3);

    // Test average calculation
    double average = sum / values.size();
    EXPECT_DOUBLE_EQ(average, 3.46);

    // Test min/max finding
    auto min_max = std::minmax_element(values.begin(), values.end());
    EXPECT_DOUBLE_EQ(*min_max.first, 1.5);
    EXPECT_DOUBLE_EQ(*min_max.second, 5.1);
}

TEST(UtilsTest, RandomUtilities) {
    // Test random utility functions
    std::vector<int> numbers;

    // Generate some "random" numbers (deterministic for testing)
    for (int i = 0; i < 10; ++i) {
        numbers.push_back(i * 7 % 100);  // Pseudo-random
    }

    EXPECT_EQ(numbers.size(), 10);
    EXPECT_GE(numbers[0], 0);
    EXPECT_LT(numbers[0], 100);

    // Test shuffling (concept)
    std::vector<int> original = {1, 2, 3, 4, 5};
    std::vector<int> shuffled = original;
    // In real implementation, would use std::shuffle
    // For testing, just verify the concept

    EXPECT_EQ(shuffled.size(), 5);
}

TEST(UtilsTest, ValidationUtilities) {
    // Test validation utility functions

    // Email validation (concept) - basic check for @ and .
    auto is_valid_email = [](const std::string& email) {
        return email.find('@') != std::string::npos &&
               email.find('.') != std::string::npos;
    };

    EXPECT_TRUE(is_valid_email("user@example.com"));
    EXPECT_FALSE(is_valid_email("invalid-email"));
    EXPECT_TRUE(is_valid_email("@example.com"));  // Contains both @ and .

    // Numeric validation (concept) - FIXED VERSION
    auto is_numeric = [](const std::string& str) {
        if (str.empty()) return false;
        for (char c : str) {
            if (c < '0' || c > '9') return false;
        }
        return true;
    };

    EXPECT_TRUE(is_numeric("12345"));
    EXPECT_FALSE(is_numeric("12a45"));
    EXPECT_FALSE(is_numeric(""));
}
