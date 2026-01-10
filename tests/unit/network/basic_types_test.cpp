/**
 * @file basic_types_test.cpp
 * @brief 基本类型测试 - 不包含任何头文件依赖
 *
 * 测试最基本的C++类型和概念，验证编译环境正常
 */

#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <vector>

// Test basic enum functionality
enum TestMessageType {
    TEST_CONNECT = 0,
    TEST_DISCONNECT = 1,
    TEST_DATA = 2
};

enum TestConnectionState {
    TEST_DISCONNECTED = 0,
    TEST_CONNECTED = 1,
    TEST_ERROR = 2
};

// Test basic struct
struct TestMessageHeader {
    uint32_t magic;
    uint32_t length;
    uint16_t type;
    uint16_t flags;
    uint32_t sequence_id;
};

// Test basic class
class TestException {
public:
    TestException(const std::string& message) : message_(message) {}
    const std::string& GetMessage() const { return message_; }
private:
    std::string message_;
};

// Test message type enum values
TEST(BasicTypesTest, TestEnumValues) {
    EXPECT_EQ(TEST_CONNECT, 0);
    EXPECT_EQ(TEST_DISCONNECT, 1);
    EXPECT_EQ(TEST_DATA, 2);

    EXPECT_EQ(TEST_DISCONNECTED, 0);
    EXPECT_EQ(TEST_CONNECTED, 1);
    EXPECT_EQ(TEST_ERROR, 2);
}

// Test struct creation and access
TEST(BasicTypesTest, TestStructOperations) {
    TestMessageHeader header = {
        .magic = 0x12345678,
        .length = 100,
        .type = TEST_DATA,
        .flags = 0x01,
        .sequence_id = 42
    };

    EXPECT_EQ(header.magic, 0x12345678U);
    EXPECT_EQ(header.length, 100U);
    EXPECT_EQ(header.type, TEST_DATA);
    EXPECT_EQ(header.flags, 0x01);
    EXPECT_EQ(header.sequence_id, 42U);
}

// Test exception class
TEST(BasicTypesTest, TestExceptionClass) {
    TestException ex("Test error message");

    EXPECT_EQ(ex.GetMessage(), "Test error message");

    // Test copy construction
    TestException ex2 = ex;
    EXPECT_EQ(ex2.GetMessage(), "Test error message");
}

// Test std::string operations
TEST(BasicTypesTest, TestStringOperations) {
    std::string str = "Hello World";

    EXPECT_EQ(str.length(), 11U);
    EXPECT_EQ(str.substr(0, 5), "Hello");
    EXPECT_EQ(str.find("World"), 6U);

    // Test string concatenation
    std::string combined = str + " Test";
    EXPECT_EQ(combined, "Hello World Test");
}

// Test std::vector operations
TEST(BasicTypesTest, TestVectorOperations) {
    std::vector<int> vec;

    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    EXPECT_EQ(vec.size(), 3U);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);

    // Test vector resizing
    vec.resize(5, 0);
    EXPECT_EQ(vec.size(), 5U);
    EXPECT_EQ(vec[4], 0);
}

// Test std::shared_ptr operations
TEST(BasicTypesTest, TestSharedPtrOperations) {
    std::shared_ptr<std::string> ptr1 = std::make_shared<std::string>("test");

    EXPECT_EQ(*ptr1, "test");
    EXPECT_EQ(ptr1.use_count(), 1);

    // Test copy construction
    std::shared_ptr<std::string> ptr2 = ptr1;
    EXPECT_EQ(ptr1.use_count(), 2);
    EXPECT_EQ(ptr2.use_count(), 2);
    EXPECT_EQ(*ptr2, "test");

    // Test reset
    ptr1.reset();
    EXPECT_EQ(ptr1, nullptr);
    EXPECT_EQ(ptr2.use_count(), 1);
}

// Test boolean logic
TEST(BasicTypesTest, TestBooleanLogic) {
    bool a = true;
    bool b = false;

    EXPECT_TRUE(a);
    EXPECT_FALSE(b);
    EXPECT_TRUE(a && !b);
    EXPECT_TRUE(a || b);
    EXPECT_FALSE(a && b);
    EXPECT_TRUE(!(!a || b));
}

// Test integer arithmetic
TEST(BasicTypesTest, TestIntegerArithmetic) {
    int x = 10;
    int y = 5;

    EXPECT_EQ(x + y, 15);
    EXPECT_EQ(x - y, 5);
    EXPECT_EQ(x * y, 50);
    EXPECT_EQ(x / y, 2);
    EXPECT_EQ(x % y, 0);

    // Test overflow protection (in safe ranges)
    EXPECT_LT(x, 100);
    EXPECT_GT(y, 0);
}

// Test size_t operations
TEST(BasicTypesTest, TestSizeTOperations) {
    size_t a = 100;
    size_t b = 50;

    EXPECT_EQ(a + b, 150U);
    EXPECT_EQ(a - b, 50U);
    EXPECT_EQ(a * 2, 200U);
    EXPECT_EQ(a / 2, 50U);

    EXPECT_TRUE(a > b);
    EXPECT_TRUE(b < a);
    EXPECT_TRUE(a >= b);
    EXPECT_TRUE(b <= a);
}

// Test memory layout
TEST(BasicTypesTest, TestMemoryLayout) {
    // Test basic type sizes
    EXPECT_EQ(sizeof(int), 4U);
    EXPECT_EQ(sizeof(uint32_t), 4U);
    EXPECT_EQ(sizeof(uint16_t), 2U);
    EXPECT_EQ(sizeof(bool), 1U);

    // Test struct size
    TestMessageHeader header = {};
    EXPECT_EQ(sizeof(header), 16U); // 4*4 + 2*2 = 16 bytes

    // Test pointer size
    void* ptr = nullptr;
    EXPECT_TRUE(sizeof(ptr) >= 4U); // At least 32-bit
}

// Test nullptr operations
TEST(BasicTypesTest, TestNullptrOperations) {
    void* ptr = nullptr;
    EXPECT_EQ(ptr, nullptr);

    std::shared_ptr<std::string> shared_ptr = nullptr;
    EXPECT_EQ(shared_ptr, nullptr);

    // Test nullptr comparisons
    EXPECT_TRUE(ptr == nullptr);
    EXPECT_FALSE(ptr != nullptr);
}

// Test compilation constants
TEST(BasicTypesTest, TestCompilationConstants) {
    // Test that basic constants work
    const uint32_t MAGIC_NUMBER = 0x53434C53; // 'SQLC'
    const size_t MAX_SIZE = 1024 * 1024; // 1MB

    EXPECT_EQ(MAGIC_NUMBER, 0x53434C53U);
    EXPECT_EQ(MAX_SIZE, 1048576U);

    // Test bitwise operations
    EXPECT_EQ(MAGIC_NUMBER & 0xFF, 0x53);
    EXPECT_EQ(MAGIC_NUMBER >> 24, 0x53);
}

// Test that all basic operations work together
TEST(BasicTypesTest, TestIntegratedOperations) {
    // Create a message header
    TestMessageHeader header = {
        .magic = 0x53434C53,
        .length = 256,
        .type = TEST_DATA,
        .flags = 0x01,
        .sequence_id = 12345
    };

    // Test all fields
    EXPECT_EQ(header.magic, 0x53434C53U);
    EXPECT_EQ(header.length, 256U);
    EXPECT_EQ(header.type, TEST_DATA);
    EXPECT_EQ(header.flags, 0x01);
    EXPECT_EQ(header.sequence_id, 12345U);

    // Test that we can create vectors of structs
    std::vector<TestMessageHeader> headers;
    headers.push_back(header);
    headers.push_back(header);

    EXPECT_EQ(headers.size(), 2U);
    EXPECT_EQ(headers[0].sequence_id, 12345U);
    EXPECT_EQ(headers[1].sequence_id, 12345U);

    // Test exception handling
    try {
        throw TestException("Test exception");
    } catch (const TestException& e) {
        EXPECT_EQ(e.GetMessage(), "Test exception");
    }
}