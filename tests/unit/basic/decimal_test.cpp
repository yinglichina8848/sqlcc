#include "sql_parser/decimal.h"
#include <gtest/gtest.h>
#include <limits>
#include <cmath>

namespace sqlcc {
namespace test {

class DecimalTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

// 测试基本构造函数
TEST_F(DecimalTest, BasicConstructors) {
  // 默认构造函数
  Decimal d1;
  EXPECT_EQ(d1.to_string(), "0");

  // int64构造函数
  Decimal d2(123);
  EXPECT_EQ(d2.to_string(), "123");

  Decimal d3(-456);
  EXPECT_EQ(d3.to_string(), "-456");

  // double构造函数
  Decimal d4(123.45);
  EXPECT_EQ(d4.to_string(), "123.45");

  // 字符串构造函数
  Decimal d5("789.123");
  EXPECT_EQ(d5.to_string(), "789.123");

  Decimal d6("-999.999");
  EXPECT_EQ(d6.to_string(), "-999.999");
}

// 测试拷贝构造函数和赋值
TEST_F(DecimalTest, CopyAndAssignment) {
  Decimal d1("123.456");
  Decimal d2(d1); // 拷贝构造
  EXPECT_EQ(d2.to_string(), "123.456");

  Decimal d3;
  d3 = d1; // 赋值
  EXPECT_EQ(d3.to_string(), "123.456");

  Decimal d4;
  d4 = 789; // 从int赋值
  EXPECT_EQ(d4.to_string(), "789");

  Decimal d5;
  d5 = 123.456; // 从double赋值
  EXPECT_EQ(d5.to_string(), "123.456");

  Decimal d6;
  d6 = "999.999"; // 从字符串赋值
  EXPECT_EQ(d6.to_string(), "999.999");
}

// 测试算术运算
TEST_F(DecimalTest, ArithmeticOperations) {
  Decimal a("10.5");
  Decimal b("3.2");

  // 加法
  Decimal sum = a + b;
  EXPECT_EQ(sum.to_string(), "13.7");

  // 减法
  Decimal diff = a - b;
  EXPECT_EQ(diff.to_string(), "7.3");

  // 乘法
  Decimal prod = a * b;
  EXPECT_EQ(prod.to_string(), "33.60");

  // 除法
  Decimal quot = a / b;
  EXPECT_EQ(quot.to_string(), "3.28125");

  // 取模
  Decimal mod = a % b;
  EXPECT_EQ(mod.to_string(), "0.9");
}

// 测试复合赋值运算
TEST_F(DecimalTest, CompoundAssignment) {
  Decimal a("10.0");

  a += Decimal("5.5");
  EXPECT_EQ(a.to_string(), "15.5");

  a -= Decimal("3.0");
  EXPECT_EQ(a.to_string(), "12.5");

  a *= Decimal("2.0");
  EXPECT_EQ(a.to_string(), "25.0");

  a /= Decimal("5.0");
  EXPECT_EQ(a.to_string(), "5.0");

  a %= Decimal("3.0");
  EXPECT_EQ(a.to_string(), "2.0");
}

// 测试比较运算
TEST_F(DecimalTest, ComparisonOperations) {
  Decimal a("10.5");
  Decimal b("3.2");
  Decimal c("10.5");

  EXPECT_TRUE(a > b);
  EXPECT_TRUE(b < a);
  EXPECT_TRUE(a >= c);
  EXPECT_TRUE(a <= c);
  EXPECT_TRUE(a == c);
  EXPECT_TRUE(a != b);

  EXPECT_FALSE(a < b);
  EXPECT_FALSE(b > a);
  EXPECT_FALSE(a != c);
  EXPECT_FALSE(a == b);
}

// 测试一元运算
TEST_F(DecimalTest, UnaryOperations) {
  Decimal a("10.5");
  Decimal b = -a;
  EXPECT_EQ(b.to_string(), "-10.5");

  Decimal c = +a;
  EXPECT_EQ(c.to_string(), "10.5");
}

// 测试类型转换
TEST_F(DecimalTest, TypeConversions) {
  Decimal d("123.456");

  // 转换为int64
  int64_t int_val = d.to_int64();
  EXPECT_EQ(int_val, 123);

  // 转换为double
  double double_val = d.to_double();
  EXPECT_DOUBLE_EQ(double_val, 123.456);

  // 转换为字符串
  std::string str_val = d.to_string();
  EXPECT_EQ(str_val, "123.456");

  // 测试零值
  Decimal zero;
  EXPECT_EQ(zero.to_int64(), 0);
  EXPECT_DOUBLE_EQ(zero.to_double(), 0.0);
}

// 测试数学函数
TEST_F(DecimalTest, MathematicalFunctions) {
  Decimal d("10.7");

  // 绝对值
  Decimal abs_val = d.abs();
  EXPECT_EQ(abs_val.to_string(), "10.7");

  Decimal neg_d("-10.7");
  Decimal abs_neg = neg_d.abs();
  EXPECT_EQ(abs_neg.to_string(), "10.7");

  // 向上取整
  Decimal ceil_val = d.ceil();
  EXPECT_EQ(ceil_val.to_string(), "11");

  // 向下取整
  Decimal floor_val = d.floor();
  EXPECT_EQ(floor_val.to_string(), "10");

  // 四舍五入
  Decimal round_val = d.round(1);
  EXPECT_EQ(round_val.to_string(), "10.7");

  Decimal round_test("10.75");
  Decimal round_result = round_test.round(1);
  EXPECT_EQ(round_result.to_string(), "10.8");

  // 截断
  Decimal trunc_val = d.truncate(1);
  EXPECT_EQ(trunc_val.to_string(), "10.7");
}

// 测试精度和刻度
TEST_F(DecimalTest, PrecisionAndScale) {
  Decimal d("123.456");

  EXPECT_EQ(d.precision(), 28); // 默认精度
  EXPECT_EQ(d.scale(), 3);      // 3位小数

  d.set_scale(2);
  EXPECT_EQ(d.scale(), 2);
  EXPECT_EQ(d.to_string(), "123.46");

  d.set_precision(10);
  EXPECT_EQ(d.precision(), 10);
}

// 测试边界条件
TEST_F(DecimalTest, EdgeCases) {
  // 零值处理
  Decimal zero("0");
  EXPECT_TRUE(zero.is_zero());
  EXPECT_EQ(zero.to_string(), "0");

  Decimal zero2("0.000");
  EXPECT_TRUE(zero2.is_zero());
  EXPECT_EQ(zero2.to_string(), "0");

  // 负零
  Decimal neg_zero("-0");
  EXPECT_TRUE(neg_zero.is_zero());
  EXPECT_EQ(neg_zero.to_string(), "0");

  // 极大值（测试字符串处理）
  std::string large_str(20, '9');
  large_str.insert(10, ".");
  Decimal large(large_str);
  EXPECT_EQ(large.to_string(), large_str);

  // 极小值
  Decimal small("0.000000000000000001");
  EXPECT_EQ(small.to_string(), "0.000000000000000001");
}

// 测试字符串解析
TEST_F(DecimalTest, StringParsing) {
  // 正常格式
  Decimal d1("123.456");
  EXPECT_EQ(d1.to_string(), "123.456");

  // 整数
  Decimal d2("789");
  EXPECT_EQ(d2.to_string(), "789");

  // 纯小数
  Decimal d3("0.123");
  EXPECT_EQ(d3.to_string(), "0.123");

  // 前导零
  Decimal d4("00123.456000");
  EXPECT_EQ(d4.to_string(), "123.456");

  // 正号
  Decimal d5("+123.456");
  EXPECT_EQ(d5.to_string(), "123.456");

  // 负号
  Decimal d6("-123.456");
  EXPECT_EQ(d6.to_string(), "-123.456");
}

// 测试大数运算
TEST_F(DecimalTest, LargeNumbers) {
  Decimal a("999999999999999999");
  Decimal b("1");
  Decimal sum = a + b;
  EXPECT_EQ(sum.to_string(), "1000000000000000000");

  Decimal prod = a * Decimal("2");
  EXPECT_EQ(prod.to_string(), "1999999999999999998");
}

// 测试精度保持
TEST_F(DecimalTest, PrecisionPreservation) {
  Decimal a("1.234567890123456789");
  Decimal b("9.876543210987654321");

  Decimal sum = a + b;
  EXPECT_EQ(sum.to_string(), "11.11111110111111111");

  Decimal diff = b - a;
  EXPECT_EQ(diff.to_string(), "8.641975320864197532");
}

// 测试除法精度
TEST_F(DecimalTest, DivisionPrecision) {
  Decimal a("10");
  Decimal b("3");

  Decimal quot = a / b;
  std::string result = quot.to_string();
  // 除法结果应该有足够的精度
  EXPECT_TRUE(result.length() > 3); // 至少有几位小数
}

// 测试异常情况
TEST_F(DecimalTest, ExceptionHandling) {
  // 除零
  Decimal a("10");
  Decimal zero("0");
  EXPECT_THROW(a / zero, std::runtime_error);
  EXPECT_THROW(a % zero, std::runtime_error);

  // 无效字符串
  EXPECT_THROW(Decimal("abc"), std::runtime_error);
  EXPECT_THROW(Decimal("12.34.56"), std::runtime_error);
  EXPECT_THROW(Decimal(""), std::runtime_error);
}

// 测试全局函数
TEST_F(DecimalTest, GlobalFunctions) {
  Decimal d("-10.5");

  // abs函数
  Decimal abs_result = abs(d);
  EXPECT_EQ(abs_result.to_string(), "10.5");

  // ceil函数
  Decimal ceil_result = ceil(d);
  EXPECT_EQ(ceil_result.to_string(), "-10");

  // floor函数
  Decimal floor_result = floor(d);
  EXPECT_EQ(floor_result.to_string(), "-11");

  // round函数
  Decimal round_result = round(d, 0);
  EXPECT_EQ(round_result.to_string(), "-11");

  // truncate函数
  Decimal trunc_result = truncate(d, 0);
  EXPECT_EQ(trunc_result.to_string(), "-10");
}

// 测试流操作
TEST_F(DecimalTest, StreamOperations) {
  Decimal d("123.456");

  // 输出流
  std::stringstream ss_out;
  ss_out << d;
  EXPECT_EQ(ss_out.str(), "123.456");

  // 输入流
  std::stringstream ss_in("789.123");
  Decimal d2;
  ss_in >> d2;
  EXPECT_EQ(d2.to_string(), "789.123");
}

// 测试静态常量
TEST_F(DecimalTest, StaticConstants) {
  EXPECT_EQ(Decimal::ZERO.to_string(), "0");
  EXPECT_EQ(Decimal::ONE.to_string(), "1");
  EXPECT_EQ(Decimal::MINUS_ONE.to_string(), "-1");
}

// 测试幂运算和平方根
TEST_F(DecimalTest, AdvancedMath) {
  Decimal base("2");
  Decimal power_result = power(base, 3);
  EXPECT_EQ(power_result.to_string(), "8");

  Decimal sqrt_result = sqrt(Decimal("16"));
  EXPECT_EQ(sqrt_result.to_string(), "4");

  // 负数平方根应该抛出异常
  EXPECT_THROW(sqrt(Decimal("-4")), std::runtime_error);

  // 负数幂次应该抛出异常
  EXPECT_THROW(power(base, -1), std::runtime_error);
}

// 测试最大值和最小值
TEST_F(DecimalTest, MinMaxValues) {
  Decimal max_val = Decimal::max_value();
  Decimal min_val = Decimal::min_value();

  // max_value应该是正数
  EXPECT_TRUE(max_val > Decimal::ZERO);

  // min_value应该是负数
  EXPECT_TRUE(min_val < Decimal::ZERO);

  // min_value的绝对值应该等于max_value
  EXPECT_EQ((-min_val).to_string(), max_val.to_string());
}

// 测试大整数转换
TEST_F(DecimalTest, LargeIntegerConversion) {
  // 测试接近int64最大值的数
  int64_t large_int = 9223372036854775807LL; // INT64_MAX
  Decimal d(large_int);
  EXPECT_EQ(d.to_int64(), large_int);

  // 测试超出int64范围的数
  std::string very_large = "9223372036854775808"; // INT64_MAX + 1
  Decimal d2(very_large);
  EXPECT_THROW(d2.to_int64(), std::runtime_error);
}

// 测试浮点数转换
TEST_F(DecimalTest, DoubleConversion) {
  double test_values[] = {0.0, 1.0, -1.0, 123.456, -789.123, 0.000001};

  for (double val : test_values) {
    Decimal d(val);
    double converted = d.to_double();
    // 允许小误差
    EXPECT_NEAR(converted, val, 1e-10);
  }

  // 测试特殊值
  EXPECT_THROW(Decimal(std::numeric_limits<double>::infinity()), std::runtime_error);
  EXPECT_THROW(Decimal(std::numeric_limits<double>::quiet_NaN()), std::runtime_error);
}

// 性能测试（基本检查）
TEST_F(DecimalTest, PerformanceBasic) {
  // 基本性能检查，确保运算不会无限循环
  Decimal a("123456789.123456789");
  Decimal b("987654321.987654321");

  // 执行一些运算
  Decimal result = (a + b) * (a - b) / (a + b);
  EXPECT_TRUE(result.to_double() >= 0); // 结果应该是合理的

  // 批量运算测试
  for (int i = 0; i < 100; ++i) {
    Decimal temp = a + Decimal(std::to_string(i));
    EXPECT_TRUE(temp > a);
  }
}

} // namespace test
} // namespace sqlcc
