#include <gtest/gtest.h>
#include "page.h"
#include "exception.h"
#include "logger.h"
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

class PageEnhancedTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前的初始化
    }

    void TearDown() override {
        // 测试后的清理
    }
};

// 测试默认构造函数
TEST_F(PageEnhancedTest, DefaultConstructor) {
    Page page;
    
    // 默认构造的页面ID应该是-1
    EXPECT_EQ(page.GetPageId(), -1);
    
    // 数据应该被初始化为0
    char* data = page.GetData();
    for (size_t i = 0; i < 100; ++i) {  // 只检查前100字节，避免检查整个页面
        EXPECT_EQ(data[i], 0);
    }
}

// 测试带参数的构造函数
TEST_F(PageEnhancedTest, ParameterizedConstructor) {
    const int32_t test_page_id = 42;
    Page page(test_page_id);
    
    // 页面ID应该被正确设置
    EXPECT_EQ(page.GetPageId(), test_page_id);
    
    // 数据应该被初始化为0
    char* data = page.GetData();
    for (size_t i = 0; i < 100; ++i) {  // 只检查前100字节，避免检查整个页面
        EXPECT_EQ(data[i], 0);
    }
}

// 测试写入数据 - 成功的情况
TEST_F(PageEnhancedTest, WriteDataSuccess) {
    const int32_t test_page_id = 1;
    Page page(test_page_id);
    
    const std::string test_data = "Hello, World!";
    const size_t offset = 100;
    
    // 写入数据
    page.WriteData(offset, test_data.c_str(), test_data.size());
    
    // 验证数据是否正确写入
    char read_data[100];
    page.ReadData(offset, read_data, test_data.size());
    EXPECT_EQ(std::string(read_data, test_data.size()), test_data);
}

// 测试写入数据 - 边界情况
TEST_F(PageEnhancedTest, WriteDataBoundary) {
    const int32_t test_page_id = 2;
    Page page(test_page_id);
    
    const std::string test_data = "Boundary";
    const size_t offset = PAGE_SIZE - test_data.size();
    
    // 在页面边界写入数据
    page.WriteData(offset, test_data.c_str(), test_data.size());
    
    // 验证数据是否正确写入
    char read_data[100];
    page.ReadData(offset, read_data, test_data.size());
    EXPECT_EQ(std::string(read_data, test_data.size()), test_data);
}

// 测试写入数据 - 超出边界
TEST_F(PageEnhancedTest, WriteDataOutOfBounds) {
    const int32_t test_page_id = 3;
    Page page(test_page_id);
    
    const std::string test_data = "This will cause an exception";
    const size_t offset = PAGE_SIZE - 10;  // 故意设置一个会导致越界的偏移量
    
    // 尝试写入超出边界的数据，应该抛出异常
    EXPECT_THROW({
        page.WriteData(offset, test_data.c_str(), test_data.size());
    }, PageException);
}

// 测试写入数据 - 偏移量为0
TEST_F(PageEnhancedTest, WriteDataOffsetZero) {
    const int32_t test_page_id = 4;
    Page page(test_page_id);
    
    const std::string test_data = "Start of page";
    const size_t offset = 0;
    
    // 在页面开始处写入数据
    page.WriteData(offset, test_data.c_str(), test_data.size());
    
    // 验证数据是否正确写入
    char read_data[100];
    page.ReadData(offset, read_data, test_data.size());
    EXPECT_EQ(std::string(read_data, test_data.size()), test_data);
}

// 测试写入数据 - 空数据
TEST_F(PageEnhancedTest, WriteDataEmpty) {
    const int32_t test_page_id = 5;
    Page page(test_page_id);
    
    const size_t offset = 100;
    
    // 写入空数据
    page.WriteData(offset, nullptr, 0);
    
    // 不需要验证，因为空数据不应该改变任何内容
}

// 测试读取数据 - 成功的情况
TEST_F(PageEnhancedTest, ReadDataSuccess) {
    const int32_t test_page_id = 6;
    Page page(test_page_id);
    
    const std::string test_data = "Read test data";
    const size_t offset = 200;
    
    // 先写入数据
    page.WriteData(offset, test_data.c_str(), test_data.size());
    
    // 读取数据
    char read_data[100];
    page.ReadData(offset, read_data, test_data.size());
    
    // 验证数据是否正确读取
    EXPECT_EQ(std::string(read_data, test_data.size()), test_data);
}

// 测试读取数据 - 边界情况
TEST_F(PageEnhancedTest, ReadDataBoundary) {
    const int32_t test_page_id = 7;
    Page page(test_page_id);
    
    const std::string test_data = "Boundary read";
    const size_t offset = PAGE_SIZE - test_data.size();
    
    // 先写入数据
    page.WriteData(offset, test_data.c_str(), test_data.size());
    
    // 在页面边界读取数据
    char read_data[100];
    page.ReadData(offset, read_data, test_data.size());
    
    // 验证数据是否正确读取
    EXPECT_EQ(std::string(read_data, test_data.size()), test_data);
}

// 测试读取数据 - 超出边界
TEST_F(PageEnhancedTest, ReadDataOutOfBounds) {
    const int32_t test_page_id = 8;
    Page page(test_page_id);
    
    const size_t offset = PAGE_SIZE - 10;  // 故意设置一个会导致越界的偏移量
    char read_data[100];
    
    // 尝试读取超出边界的数据，应该抛出异常
    EXPECT_THROW({
        page.ReadData(offset, read_data, 100);
    }, PageException);
}

// 测试读取数据 - 偏移量为0
TEST_F(PageEnhancedTest, ReadDataOffsetZero) {
    const int32_t test_page_id = 9;
    Page page(test_page_id);
    
    const std::string test_data = "Start of page read";
    const size_t offset = 0;
    
    // 先写入数据
    page.WriteData(offset, test_data.c_str(), test_data.size());
    
    // 在页面开始处读取数据
    char read_data[100];
    page.ReadData(offset, read_data, test_data.size());
    
    // 验证数据是否正确读取
    EXPECT_EQ(std::string(read_data, test_data.size()), test_data);
}

// 测试读取数据 - 空数据
TEST_F(PageEnhancedTest, ReadDataEmpty) {
    const int32_t test_page_id = 10;
    Page page(test_page_id);
    
    const size_t offset = 100;
    
    // 读取空数据
    page.ReadData(offset, nullptr, 0);
    
    // 不需要验证，因为空数据不应该读取任何内容
}

// 测试多次读写操作
TEST_F(PageEnhancedTest, MultipleReadWriteOperations) {
    const int32_t test_page_id = 11;
    Page page(test_page_id);
    
    // 写入多个数据块
    const std::string data1 = "First block";
    const std::string data2 = "Second block";
    const std::string data3 = "Third block";
    
    const size_t offset1 = 100;
    const size_t offset2 = 200;
    const size_t offset3 = 300;
    
    page.WriteData(offset1, data1.c_str(), data1.size());
    page.WriteData(offset2, data2.c_str(), data2.size());
    page.WriteData(offset3, data3.c_str(), data3.size());
    
    // 读取并验证每个数据块
    char read_data[100];
    
    page.ReadData(offset1, read_data, data1.size());
    EXPECT_EQ(std::string(read_data, data1.size()), data1);
    
    page.ReadData(offset2, read_data, data2.size());
    EXPECT_EQ(std::string(read_data, data2.size()), data2);
    
    page.ReadData(offset3, read_data, data3.size());
    EXPECT_EQ(std::string(read_data, data3.size()), data3);
}

// 测试覆盖写入
TEST_F(PageEnhancedTest, OverwriteData) {
    const int32_t test_page_id = 12;
    Page page(test_page_id);
    
    const std::string original_data = "Original data";
    const std::string new_data = "New data that overwrites";
    const size_t offset = 150;
    
    // 写入原始数据
    page.WriteData(offset, original_data.c_str(), original_data.size());
    
    // 覆盖写入新数据
    page.WriteData(offset, new_data.c_str(), new_data.size());
    
    // 读取并验证数据已被覆盖
    char read_data[100];
    page.ReadData(offset, read_data, new_data.size());
    EXPECT_EQ(std::string(read_data, new_data.size()), new_data);
}

// 测试部分覆盖写入
TEST_F(PageEnhancedTest, PartialOverwriteData) {
    const int32_t test_page_id = 13;
    Page page(test_page_id);
    
    const std::string original_data = "Original long data";
    const std::string new_data = "New";
    const size_t offset = 150;
    
    // 写入原始数据
    page.WriteData(offset, original_data.c_str(), original_data.size());
    
    // 部分覆盖写入新数据
    page.WriteData(offset, new_data.c_str(), new_data.size());
    
    // 读取并验证数据已被部分覆盖
    char read_data[100];
    page.ReadData(offset, read_data, original_data.size());
    
    // 前面部分应该是新数据，后面部分应该是原始数据
    EXPECT_EQ(std::string(read_data, new_data.size()), new_data);
    EXPECT_EQ(std::string(read_data + new_data.size(), original_data.size() - new_data.size()), 
              original_data.substr(new_data.size()));
}

// 测试大块数据写入
TEST_F(PageEnhancedTest, LargeDataWrite) {
    const int32_t test_page_id = 14;
    Page page(test_page_id);
    
    // 创建一个较大的数据块（但不超过页面大小）
    const size_t data_size = PAGE_SIZE / 2;  // 页面大小的一半
    std::vector<char> large_data(data_size, 'X');
    
    const size_t offset = 0;
    
    // 写入大块数据
    page.WriteData(offset, large_data.data(), data_size);
    
    // 读取并验证数据
    std::vector<char> read_data(data_size);
    page.ReadData(offset, read_data.data(), data_size);
    
    // 验证数据是否正确
    for (size_t i = 0; i < data_size; ++i) {
        EXPECT_EQ(read_data[i], 'X');
    }
}

// 测试二进制数据
TEST_F(PageEnhancedTest, BinaryData) {
    const int32_t test_page_id = 15;
    Page page(test_page_id);
    
    // 创建包含各种二进制值的数据
    std::vector<char> binary_data(256);
    for (int i = 0; i < 256; ++i) {
        binary_data[i] = static_cast<char>(i);
    }
    
    const size_t offset = 100;
    
    // 写入二进制数据
    page.WriteData(offset, binary_data.data(), binary_data.size());
    
    // 读取并验证数据
    std::vector<char> read_data(256);
    page.ReadData(offset, read_data.data(), read_data.size());
    
    // 验证数据是否正确
    for (int i = 0; i < 256; ++i) {
        EXPECT_EQ(read_data[i], static_cast<char>(i));
    }
}

}  // namespace sqlcc