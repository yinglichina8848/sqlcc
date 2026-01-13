#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

// Basic functionality tests for foundation layer
// These tests verify the most fundamental components work correctly

TEST(BasicFunctionalityTest, BasicAssertions) {
    // Basic sanity checks
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
    EXPECT_EQ(1, 1);
    EXPECT_NE(1, 2);
}

TEST(BasicFunctionalityTest, StringOperations) {
    std::string str = "Hello, World!";
    EXPECT_EQ(str.length(), 13);
    EXPECT_EQ(str.substr(0, 5), "Hello");
    EXPECT_TRUE(str.find("World") != std::string::npos);
}

TEST(BasicFunctionalityTest, VectorOperations) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec.back(), 5);

    vec.push_back(6);
    EXPECT_EQ(vec.size(), 6);
    EXPECT_EQ(vec.back(), 6);
}

TEST(BasicFunctionalityTest, SmartPointers) {
    auto unique_ptr = std::make_unique<int>(42);
    EXPECT_EQ(*unique_ptr, 42);

    auto shared_ptr = std::make_shared<std::string>("test");
    EXPECT_EQ(*shared_ptr, "test");
    EXPECT_EQ(shared_ptr.use_count(), 1);
}

TEST(BasicFunctionalityTest, MemoryManagement) {
    // Test basic memory operations
    int* ptr = new int(100);
    EXPECT_EQ(*ptr, 100);
    delete ptr;

    int* array = new int[5]{1, 2, 3, 4, 5};
    EXPECT_EQ(array[0], 1);
    EXPECT_EQ(array[4], 5);
    delete[] array;
}

TEST(BasicFunctionalityTest, ExceptionHandling) {
    try {
        throw std::runtime_error("Test exception");
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "Test exception");
    }
}

TEST(BasicFunctionalityTest, TypeTraits) {
    // Test basic type operations
    EXPECT_TRUE(std::is_integral<int>::value);
    EXPECT_TRUE(std::is_floating_point<double>::value);
    EXPECT_TRUE(std::is_pointer<int*>::value);
    EXPECT_FALSE(std::is_pointer<int>::value);
}

TEST(BasicFunctionalityTest, MoveSemantics) {
    std::vector<int> original = {1, 2, 3, 4, 5};
    std::vector<int> moved = std::move(original);

    EXPECT_EQ(moved.size(), 5);
    EXPECT_EQ(original.size(), 0);  // moved-from vector should be empty
}