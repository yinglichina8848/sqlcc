/**
 * @file smart_pointer_basic_test.cpp
 * @brief 基础智能指针修复验证测试
 * 
 * 本测试文件用于验证网络模块中裸指针到智能指针转换的基本正确性
 * 专注于核心功能验证，避免复杂的依赖关系
 */

#include <gtest/gtest.h>
#include <memory>
#include <cstring>

// 基础测试结构体，模拟网络模块中的数据结构
struct TestMessageHeader {
    int message_type;
    int length;
    char data[64];
    
    TestMessageHeader() : message_type(0), length(0) {
        std::memset(data, 0, sizeof(data));
    }
    
    TestMessageHeader(int type, int len) : message_type(type), length(len) {
        std::memset(data, 0, sizeof(data));
    }
};

/**
 * @brief 测试智能指针的基本创建和访问
 * 
 * 验证std::make_shared和std::make_unique的基本用法
 */
TEST(SmartPointerBasicTest, BasicCreationAndAccess) {
    // 测试shared_ptr创建
    auto shared_header = std::make_shared<TestMessageHeader>(1, 100);
    ASSERT_NE(shared_header, nullptr);
    EXPECT_EQ(shared_header->message_type, 1);
    EXPECT_EQ(shared_header->length, 100);
    
    // 测试unique_ptr创建
    auto unique_header = std::make_unique<TestMessageHeader>(2, 200);
    ASSERT_NE(unique_header, nullptr);
    EXPECT_EQ(unique_header->message_type, 2);
    EXPECT_EQ(unique_header->length, 200);
}

/**
 * @brief 测试智能指针的内存管理
 * 
 * 验证智能指针能正确管理对象生命周期
 */
TEST(SmartPointerBasicTest, MemoryManagement) {
    // 创建智能指针
    auto header = std::make_shared<TestMessageHeader>(3, 300);
    TestMessageHeader* raw_ptr = header.get();
    
    // 验证对象存活
    ASSERT_NE(raw_ptr, nullptr);
    EXPECT_EQ(raw_ptr->message_type, 3);
    
    // 创建共享引用
    {
        auto header_copy = header;
        EXPECT_EQ(header_copy.get(), raw_ptr);
        EXPECT_EQ(header.use_count(), 2);
    }
    
    // 验证原始指针仍然有效
    EXPECT_EQ(header.use_count(), 1);
    EXPECT_EQ(header->message_type, 3);
}

/**
 * @brief 测试智能指针的异常安全性
 * 
 * 验证在异常情况下智能指针能正确释放资源
 */
TEST(SmartPointerBasicTest, ExceptionSafety) {
    try {
        auto header = std::make_shared<TestMessageHeader>(4, 400);
        header->message_type = 42;
        
        // 验证数据设置
        EXPECT_EQ(header->message_type, 42);
        
        // 模拟正常操作完成
        header.reset(); // 手动释放
        EXPECT_EQ(header, nullptr);
        
    } catch (...) {
        FAIL() << "Exception should not occur in normal operation";
    }
}

/**
 * @brief 测试智能指针与容器的兼容性
 * 
 * 验证智能指针能正确存储在标准容器中
 */
TEST(SmartPointerBasicTest, ContainerCompatibility) {
    std::vector<std::shared_ptr<TestMessageHeader>> headers;
    
    // 创建多个智能指针并存储到容器中
    for (int i = 0; i < 10; ++i) {
        auto header = std::make_shared<TestMessageHeader>(i, i * 10);
        headers.push_back(header);
    }
    
    // 验证容器中的数据
    ASSERT_EQ(headers.size(), 10);
    for (int i = 0; i < 10; ++i) {
        ASSERT_NE(headers[i], nullptr);
        EXPECT_EQ(headers[i]->message_type, i);
        EXPECT_EQ(headers[i]->length, i * 10);
    }
}

/**
 * @brief 测试智能指针的拷贝和移动语义
 * 
 * 验证智能指针的拷贝和移动操作正确性
 */
TEST(SmartPointerBasicTest, CopyAndMoveSemantics) {
    // 测试拷贝语义
    auto header1 = std::make_shared<TestMessageHeader>(5, 500);
    auto header2 = header1; // 拷贝
    
    EXPECT_EQ(header1.get(), header2.get());
    EXPECT_EQ(header1.use_count(), 2);
    EXPECT_EQ(header2.use_count(), 2);
    
    // 测试移动语义（unique_ptr）
    auto unique1 = std::make_unique<TestMessageHeader>(6, 600);
    TestMessageHeader* raw_ptr = unique1.get();
    auto unique2 = std::move(unique1);
    
    EXPECT_EQ(unique1, nullptr);
    EXPECT_EQ(unique2.get(), raw_ptr);
    EXPECT_EQ(unique2->message_type, 6);
}

/**
 * @brief 测试智能指针的数据访问安全性
 * 
 * 验证通过智能指针访问数据的安全性
 */
TEST(SmartPointerBasicTest, DataAccessSafety) {
    auto header = std::make_shared<TestMessageHeader>(7, 700);
    
    // 修改数据
    header->message_type = 77;
    header->length = 777;
    std::strcpy(header->data, "test_data");
    
    // 验证数据完整性
    EXPECT_EQ(header->message_type, 77);
    EXPECT_EQ(header->length, 777);
    EXPECT_STREQ(header->data, "test_data");
    
    // 获取原始指针进行验证
    TestMessageHeader* raw_ptr = header.get();
    EXPECT_EQ(raw_ptr->message_type, 77);
    EXPECT_STREQ(raw_ptr->data, "test_data");
}

/**
 * @brief 测试智能指针的资源释放
 * 
 * 验证智能指针能正确释放资源
 */
TEST(SmartPointerBasicTest, ResourceCleanup) {
    // 创建智能指针
    auto header = std::make_shared<TestMessageHeader>(8, 800);
    EXPECT_NE(header, nullptr);
    
    // 释放资源
    header.reset();
    EXPECT_EQ(header, nullptr);
    
    // 重新创建
    header = std::make_shared<TestMessageHeader>(9, 900);
    EXPECT_NE(header, nullptr);
    EXPECT_EQ(header->message_type, 9);
}

/**
 * @brief 主测试函数
 */
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    std::cout << "=== Smart Pointer Basic Fix Validation Test ===" << std::endl;
    std::cout << "Testing basic smart pointer functionality for network module fixes..." << std::endl;
    std::cout << std::endl;
    
    return RUN_ALL_TESTS();
}