#pragma once

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace sqlcc {

class Decimal {
public:
  // 构造函数
  Decimal();
  Decimal(int64_t value);
  Decimal(double value);
  Decimal(const std::string& str);
  Decimal(const Decimal& other);
  Decimal(Decimal&& other) noexcept;

  // 析构函数
  ~Decimal();

  // 赋值操作符
  Decimal& operator=(const Decimal& other);
  Decimal& operator=(Decimal&& other) noexcept;
  Decimal& operator=(int64_t value);
  Decimal& operator=(double value);
  Decimal& operator=(const std::string& str);

  // 算术操作符
  Decimal operator+(const Decimal& other) const;
  Decimal operator-(const Decimal& other) const;
  Decimal operator*(const Decimal& other) const;
  Decimal operator/(const Decimal& other) const;
  Decimal operator%(const Decimal& other) const;

  // 复合赋值操作符
  Decimal& operator+=(const Decimal& other);
  Decimal& operator-=(const Decimal& other);
  Decimal& operator*=(const Decimal& other);
  Decimal& operator/=(const Decimal& other);
  Decimal& operator%=(const Decimal& other);

  // 比较操作符
  bool operator==(const Decimal& other) const;
  bool operator!=(const Decimal& other) const;
  bool operator<(const Decimal& other) const;
  bool operator<=(const Decimal& other) const;
  bool operator>(const Decimal& other) const;
  bool operator>=(const Decimal& other) const;

  // 一元操作符
  Decimal operator-() const;
  Decimal operator+() const;

  // 类型转换
  int64_t to_int64() const;
  double to_double() const;
  std::string to_string() const;

  // 属性查询
  int32_t precision() const { return precision_; }
  int32_t scale() const { return scale_; }
  bool is_zero() const;
  bool is_negative() const;
  bool is_positive() const { return !is_zero() && !is_negative(); }

  // 精度和刻度设置
  void set_precision(int32_t precision);
  void set_scale(int32_t scale);

  // 数学函数
  Decimal abs() const;
  Decimal ceil() const;
  Decimal floor() const;
  Decimal round(int32_t decimals = 0) const;
  Decimal truncate(int32_t decimals = 0) const;

  // 静态常量
  static const Decimal ZERO;
  static const Decimal ONE;
  static const Decimal MINUS_ONE;

  // 最大/最小值常量
  static Decimal max_value();
  static Decimal min_value();

private:
  // 内部表示：使用字符串存储精确数值
  std::string value_;  // 数字字符串，不含小数点
  bool negative_;      // 负数标志
  int32_t precision_;  // 总精度（整数位+小数位）
  int32_t scale_;      // 小数位数

  // 内部辅助方法
  void normalize();
  void from_string(const std::string& str);
  void from_int64(int64_t value);
  void from_double(double value);

  static std::string add_strings(const std::string& a, const std::string& b);
  static std::string subtract_strings(const std::string& a, const std::string& b);
  static std::string multiply_strings(const std::string& a, const std::string& b);
  static std::string divide_strings(const std::string& a, const std::string& b, int32_t scale);

  static int compare_strings(const std::string& a, const std::string& b);

  // 移除前导零
  static std::string remove_leading_zeros(const std::string& str);
  // 移除尾随零
  static std::string remove_trailing_zeros(const std::string& str);

  // 调整小数位数
  void adjust_scale(int32_t new_scale);

  // 验证精度和刻度
  void validate_precision_scale() const;

  // 常量定义
  static constexpr int32_t DEFAULT_PRECISION = 28;
  static constexpr int32_t DEFAULT_SCALE = 8;
  static constexpr int32_t MAX_PRECISION = 38;
  static constexpr int32_t MAX_SCALE = 18;
};

// 流操作符
std::ostream& operator<<(std::ostream& os, const Decimal& decimal);
std::istream& operator>>(std::istream& is, Decimal& decimal);

// 字符串转换函数
Decimal decimal_from_string(const std::string& str);
std::string decimal_to_string(const Decimal& decimal);

// 数学函数
Decimal abs(const Decimal& value);
Decimal ceil(const Decimal& value);
Decimal floor(const Decimal& value);
Decimal round(const Decimal& value, int32_t decimals = 0);
Decimal truncate(const Decimal& value, int32_t decimals = 0);

// 幂运算
Decimal power(const Decimal& base, int32_t exponent);

// 平方根（近似值）
Decimal sqrt(const Decimal& value);

} // namespace sqlcc
