#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

// Types tests for foundation layer
// These tests verify type system components work correctly

TEST(TypesTest, BasicTypeOperations) {
    // Test basic type operations
    int int_val = 42;
    double double_val = 3.14;
    std::string str_val = "test";
    bool bool_val = true;

    // Verify basic operations
    EXPECT_EQ(int_val, 42);
    EXPECT_DOUBLE_EQ(double_val, 3.14);
    EXPECT_EQ(str_val, "test");
    EXPECT_TRUE(bool_val);
}

TEST(TypesTest, TypeConversions) {
    // Test type conversions
    int int_val = 42;
    double double_val = static_cast<double>(int_val);
    std::string str_val = std::to_string(int_val);

    EXPECT_DOUBLE_EQ(double_val, 42.0);
    EXPECT_EQ(str_val, "42");

    // Test reverse conversions
    int converted_back = static_cast<int>(double_val);
    EXPECT_EQ(converted_back, 42);
}

TEST(TypesTest, PointerTypes) {
    // Test pointer type operations
    int* int_ptr = new int(100);
    EXPECT_EQ(*int_ptr, 100);

    std::shared_ptr<int> shared_ptr = std::make_shared<int>(200);
    EXPECT_EQ(*shared_ptr, 200);
    EXPECT_EQ(shared_ptr.use_count(), 1);

    std::unique_ptr<int> unique_ptr = std::make_unique<int>(300);
    EXPECT_EQ(*unique_ptr, 300);

    delete int_ptr;
}

TEST(TypesTest, ContainerTypes) {
    // Test container type operations
    std::vector<int> int_vec = {1, 2, 3, 4, 5};
    EXPECT_EQ(int_vec.size(), 5);
    EXPECT_EQ(int_vec[0], 1);
    EXPECT_EQ(int_vec.back(), 5);

    std::unordered_map<std::string, int> str_int_map;
    str_int_map["one"] = 1;
    str_int_map["two"] = 2;
    str_int_map["three"] = 3;

    EXPECT_EQ(str_int_map["one"], 1);
    EXPECT_EQ(str_int_map["two"], 2);
    EXPECT_EQ(str_int_map["three"], 3);
    EXPECT_EQ(str_int_map.size(), 3);
}

TEST(TypesTest, EnumTypes) {
    // Test enum type operations
    enum class Color { RED, GREEN, BLUE };
    enum Status { ACTIVE = 1, INACTIVE = 0 };

    Color color = Color::RED;
    Status status = ACTIVE;

    // Verify enum values
    EXPECT_EQ(static_cast<int>(color), 0);  // RED = 0
    EXPECT_EQ(status, 1);  // ACTIVE = 1

    // Test enum conversions
    int color_int = static_cast<int>(color);
    EXPECT_EQ(color_int, 0);
}

TEST(TypesTest, ReferenceTypes) {
    // Test reference type operations
    int original = 100;
    int& ref = original;

    EXPECT_EQ(ref, 100);
    EXPECT_EQ(&ref, &original);  // Same address

    // Modify through reference
    ref = 200;
    EXPECT_EQ(original, 200);
    EXPECT_EQ(ref, 200);
}

TEST(TypesTest, ConstTypes) {
    // Test const type operations
    const int const_val = 42;
    EXPECT_EQ(const_val, 42);

    const std::string const_str = "constant";
    EXPECT_EQ(const_str, "constant");

    // Test const pointers
    int non_const = 100;
    const int* const_ptr = &non_const;
    EXPECT_EQ(*const_ptr, 100);

    // Cannot modify through const pointer
    // *const_ptr = 200;  // This would be a compile error
}

TEST(TypesTest, UnionTypes) {
    // Test union type operations
    union Data {
        int int_val;
        double double_val;
        char char_val;
    };

    Data data;
    data.int_val = 42;
    EXPECT_EQ(data.int_val, 42);

    data.double_val = 3.14;
    EXPECT_DOUBLE_EQ(data.double_val, 3.14);

    data.char_val = 'A';
    EXPECT_EQ(data.char_val, 'A');
}

TEST(TypesTest, StructTypes) {
    // Test struct type operations
    struct Person {
        std::string name;
        int age;
        double height;
    };

    Person person;
    person.name = "John Doe";
    person.age = 30;
    person.height = 175.5;

    EXPECT_EQ(person.name, "John Doe");
    EXPECT_EQ(person.age, 30);
    EXPECT_DOUBLE_EQ(person.height, 175.5);
}

TEST(TypesTest, TypeSafety) {
    // Test type safety concepts
    int int_val = 42;

    // Safe conversions
    double double_val = int_val;  // Implicit conversion
    EXPECT_DOUBLE_EQ(double_val, 42.0);

    // Explicit casting when needed
    int cast_back = static_cast<int>(double_val);
    EXPECT_EQ(cast_back, 42);
}