#include "src/sql_parser/decimal.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace sqlcc {

// 静态常量定义
const Decimal Decimal::ZERO("0");
const Decimal Decimal::ONE("1");
const Decimal Decimal::MINUS_ONE("-1");

// ==================== 构造函数和析构函数 ====================

Decimal::Decimal() : negative_(false), precision_(DEFAULT_PRECISION), scale_(DEFAULT_SCALE) {
  value_ = "0";
}

Decimal::Decimal(int64_t value) : negative_(false), precision_(DEFAULT_PRECISION), scale_(DEFAULT_SCALE) {
  from_int64(value);
}

Decimal::Decimal(double value) : negative_(false), precision_(DEFAULT_PRECISION), scale_(DEFAULT_SCALE) {
  from_double(value);
}

Decimal::Decimal(const std::string& str) : negative_(false), precision_(DEFAULT_PRECISION), scale_(DEFAULT_SCALE) {
  from_string(str);
}

Decimal::Decimal(const Decimal& other)
    : value_(other.value_), negative_(other.negative_), precision_(other.precision_), scale_(other.scale_) {}

Decimal::Decimal(Decimal&& other) noexcept
    : value_(std::move(other.value_)), negative_(other.negative_), precision_(other.precision_), scale_(other.scale_) {}

Decimal::~Decimal() {}

// ==================== 赋值操作符 ====================

Decimal& Decimal::operator=(const Decimal& other) {
  if (this != &other) {
    value_ = other.value_;
    negative_ = other.negative_;
    precision_ = other.precision_;
    scale_ = other.scale_;
  }
  return *this;
}

Decimal& Decimal::operator=(Decimal&& other) noexcept {
  if (this != &other) {
    value_ = std::move(other.value_);
    negative_ = other.negative_;
    precision_ = other.precision_;
    scale_ = other.scale_;
  }
  return *this;
}

Decimal& Decimal::operator=(int64_t value) {
  from_int64(value);
  return *this;
}

Decimal& Decimal::operator=(double value) {
  from_double(value);
  return *this;
}

Decimal& Decimal::operator=(const std::string& str) {
  from_string(str);
  return *this;
}

// ==================== 算术操作符 ====================

Decimal Decimal::operator+(const Decimal& other) const {
  Decimal result = *this;
  result += other;
  return result;
}

Decimal Decimal::operator-(const Decimal& other) const {
  Decimal result = *this;
  result -= other;
  return result;
}

Decimal Decimal::operator*(const Decimal& other) const {
  Decimal result = *this;
  result *= other;
  return result;
}

Decimal Decimal::operator/(const Decimal& other) const {
  Decimal result = *this;
  result /= other;
  return result;
}

Decimal Decimal::operator%(const Decimal& other) const {
  Decimal result = *this;
  result %= other;
  return result;
}

// ==================== 复合赋值操作符 ====================

Decimal& Decimal::operator+=(const Decimal& other) {
  if (negative_ == other.negative_) {
    // 同号相加
    value_ = add_strings(value_, other.value_);
  } else {
    // 异号相减
    int cmp = compare_strings(value_, other.value_);
    if (cmp > 0) {
      value_ = subtract_strings(value_, other.value_);
    } else if (cmp < 0) {
      value_ = subtract_strings(other.value_, value_);
      negative_ = !negative_;
    } else {
      // 相等，结果为0
      value_ = "0";
      negative_ = false;
    }
  }
  adjust_scale(std::max(scale_, other.scale_));
  normalize();
  return *this;
}

Decimal& Decimal::operator-=(const Decimal& other) {
  if (negative_ != other.negative_) {
    // 异号相加
    value_ = add_strings(value_, other.value_);
  } else {
    // 同号相减
    int cmp = compare_strings(value_, other.value_);
    if (cmp > 0) {
      value_ = subtract_strings(value_, other.value_);
    } else if (cmp < 0) {
      value_ = subtract_strings(other.value_, value_);
      negative_ = !negative_;
    } else {
      // 相等，结果为0
      value_ = "0";
      negative_ = false;
    }
  }
  adjust_scale(std::max(scale_, other.scale_));
  normalize();
  return *this;
}

Decimal& Decimal::operator*=(const Decimal& other) {
  value_ = multiply_strings(value_, other.value_);
  negative_ = (negative_ != other.negative_);
  scale_ += other.scale_;
  normalize();
  return *this;
}

Decimal& Decimal::operator/=(const Decimal& other) {
  if (other.is_zero()) {
    throw std::runtime_error("Division by zero");
  }

  value_ = divide_strings(value_, other.value_, scale_ + other.scale_);
  negative_ = (negative_ != other.negative_);
  normalize();
  return *this;
}

Decimal& Decimal::operator%=(const Decimal& other) {
  if (other.is_zero()) {
    throw std::runtime_error("Division by zero");
  }

  // 简化实现：使用整数除法
  Decimal quotient = *this / other;
  Decimal integer_part = quotient.truncate(0);
  Decimal remainder = *this - (integer_part * other);

  *this = remainder;
  return *this;
}

// ==================== 比较操作符 ====================

bool Decimal::operator==(const Decimal& other) const {
  return negative_ == other.negative_ && value_ == other.value_ && scale_ == other.scale_;
}

bool Decimal::operator!=(const Decimal& other) const {
  return !(*this == other);
}

bool Decimal::operator<(const Decimal& other) const {
  if (negative_ != other.negative_) {
    return negative_;
  }

  // 调整到相同的小数位数进行比较
  std::string v1 = value_;
  std::string v2 = other.value_;
  int max_scale = std::max(scale_, other.scale_);

  if (scale_ < max_scale) {
    v1 += std::string(max_scale - scale_, '0');
  }
  if (other.scale_ < max_scale) {
    v2 += std::string(max_scale - other.scale_, '0');
  }

  int cmp = compare_strings(v1, v2);
  return negative_ ? (cmp > 0) : (cmp < 0);
}

bool Decimal::operator<=(const Decimal& other) const {
  return *this < other || *this == other;
}

bool Decimal::operator>(const Decimal& other) const {
  return !(*this <= other);
}

bool Decimal::operator>=(const Decimal& other) const {
  return !(*this < other);
}

// ==================== 一元操作符 ====================

Decimal Decimal::operator-() const {
  Decimal result = *this;
  if (!result.is_zero()) {
    result.negative_ = !result.negative_;
  }
  return result;
}

Decimal Decimal::operator+() const {
  return *this;
}

// ==================== 类型转换 ====================

int64_t Decimal::to_int64() const {
  if (is_zero()) {
    return 0;
  }

  // 移除小数部分
  std::string int_part = value_;
  if (scale_ > 0 && int_part.length() > static_cast<size_t>(scale_)) {
    int_part = int_part.substr(0, int_part.length() - scale_);
  } else if (scale_ > 0) {
    int_part = "0";
  }

  // 处理前导零
  int_part = remove_leading_zeros(int_part);
  if (int_part.empty()) {
    int_part = "0";
  }

  // 检查范围
  if (int_part.length() > 18) { // int64最大值有19位，但我们限制在18位安全范围内
    throw std::runtime_error("Decimal value too large for int64");
  }

  int64_t result = 0;
  for (char c : int_part) {
    result = result * 10 + (c - '0');
  }

  return negative_ ? -result : result;
}

double Decimal::to_double() const {
  std::string str = to_string();
  return std::stod(str);
}

std::string Decimal::to_string() const {
  if (is_zero()) {
    return "0";
  }

  std::string result = value_;

  // 添加小数点
  if (scale_ > 0) {
    if (result.length() <= static_cast<size_t>(scale_)) {
      result = std::string(scale_ - result.length() + 1, '0') + result;
    }
    result.insert(result.length() - scale_, 1, '.');
  }

  // 移除尾随零和小数点
  size_t dot_pos = result.find('.');
  if (dot_pos != std::string::npos) {
    // 移除尾随零
    while (result.back() == '0') {
      result.pop_back();
    }
    // 如果小数点后没有数字，移除小数点
    if (result.back() == '.') {
      result.pop_back();
    }
  }

  // 添加负号
  if (negative_) {
    result = "-" + result;
  }

  // 处理前导零
  if (!negative_ && result[0] == '0' && result.length() > 1 && result[1] != '.') {
    size_t start = 0;
    while (start < result.length() - 1 && result[start] == '0' && result[start + 1] != '.') {
      start++;
    }
    result = result.substr(start);
  }

  return result.empty() ? "0" : result;
}

// ==================== 属性查询 ====================

bool Decimal::is_zero() const {
  return value_ == "0" || (remove_leading_zeros(value_) == "0");
}

bool Decimal::is_negative() const {
  return negative_;
}

// ==================== 精度和刻度设置 ====================

void Decimal::set_precision(int32_t precision) {
  if (precision < 1 || precision > MAX_PRECISION) {
    throw std::runtime_error("Invalid precision");
  }
  precision_ = precision;
  validate_precision_scale();
}

void Decimal::set_scale(int32_t scale) {
  if (scale < 0 || scale > MAX_SCALE) {
    throw std::runtime_error("Invalid scale");
  }
  adjust_scale(scale);
}

// ==================== 数学函数 ====================

Decimal Decimal::abs() const {
  Decimal result = *this;
  result.negative_ = false;
  return result;
}

Decimal Decimal::ceil() const {
  if (scale_ == 0) {
    return *this;
  }

  Decimal result = *this;
  // 移除小数部分并向上取整
  if (result.value_.length() > static_cast<size_t>(scale_)) {
    std::string int_part = result.value_.substr(0, result.value_.length() - scale_);
    std::string frac_part = result.value_.substr(result.value_.length() - scale_);

    bool has_fraction = false;
    for (char c : frac_part) {
      if (c != '0') {
        has_fraction = true;
        break;
      }
    }

    if (has_fraction && !result.negative_) {
      // 正数有小数部分，向上取整
      int_part = add_strings(int_part, "1");
    } else if (has_fraction && result.negative_) {
      // 负数有小数部分，向下取整（绝对值减小）
      // 对于负数，ceil操作需要特殊处理，这里简化实现
      result.negative_ = false;
      Decimal temp(int_part);
      temp.negative_ = result.negative_;
      return temp;
    }

    result.value_ = int_part;
  }

  result.scale_ = 0;
  result.normalize();
  return result;
}

Decimal Decimal::floor() const {
  if (scale_ == 0) {
    return *this;
  }

  Decimal result = *this;
  // 移除小数部分并向下取整
  if (result.value_.length() > static_cast<size_t>(scale_)) {
    result.value_ = result.value_.substr(0, result.value_.length() - scale_);
  } else {
    result.value_ = "0";
  }

  result.scale_ = 0;
  result.normalize();
  return result;
}

Decimal Decimal::round(int32_t decimals) const {
  if (decimals < 0) {
    decimals = 0;
  }

  Decimal result = *this;
  if (decimals >= scale_) {
    return result; // 不需要舍入
  }

  // 确定舍入位置
  int32_t digits_to_remove = scale_ - decimals;
  if (digits_to_remove <= 0) {
    return result;
  }

  if (result.value_.length() <= static_cast<size_t>(digits_to_remove)) {
    return Decimal(static_cast<int64_t>(0));
  }

  // 获取要舍入的数字
  size_t round_pos = result.value_.length() - digits_to_remove;
  char round_digit = result.value_[round_pos];

  // 移除多余的数字
  result.value_ = result.value_.substr(0, round_pos);
  result.scale_ = decimals;

  // 银行家舍入法
  if (round_digit >= '5') {
    // 进位
    bool carry = true;
    for (int i = static_cast<int>(result.value_.length()) - 1; i >= 0 && carry; --i) {
      if (result.value_[i] < '9') {
        result.value_[i]++;
        carry = false;
      } else {
        result.value_[i] = '0';
      }
    }
    if (carry) {
      result.value_ = "1" + result.value_;
    }
  }

  result.normalize();
  return result;
}

Decimal Decimal::truncate(int32_t decimals) const {
  if (decimals < 0) {
    decimals = 0;
  }

  Decimal result = *this;
  if (decimals >= scale_) {
    return result; // 不需要截断
  }

  int32_t digits_to_remove = scale_ - decimals;
  if (result.value_.length() > static_cast<size_t>(digits_to_remove)) {
    result.value_ = result.value_.substr(0, result.value_.length() - digits_to_remove);
  } else {
    result.value_ = "0";
  }

  result.scale_ = decimals;
  result.normalize();
  return result;
}

// ==================== 静态方法 ====================

Decimal Decimal::max_value() {
  std::string max_str = std::string(MAX_PRECISION - MAX_SCALE, '9') + "." + std::string(MAX_SCALE, '9');
  return Decimal(max_str);
}

Decimal Decimal::min_value() {
  Decimal max_val = max_value();
  max_val.negative_ = true;
  return max_val;
}

// ==================== 私有方法 ====================

void Decimal::normalize() {
  // 移除前导零
  value_ = remove_leading_zeros(value_);

  // 处理零值
  if (value_ == "0" || value_.empty()) {
    value_ = "0";
    negative_ = false;
  }

  // 验证精度和刻度
  validate_precision_scale();
}

void Decimal::from_string(const std::string& str) {
  std::string s = str;
  negative_ = false;
  scale_ = 0;

  // 处理空字符串
  if (s.empty()) {
    value_ = "0";
    return;
  }

  // 处理符号
  if (s[0] == '-') {
    negative_ = true;
    s = s.substr(1);
  } else if (s[0] == '+') {
    s = s.substr(1);
  }

  // 处理空字符串（只有符号）
  if (s.empty()) {
    value_ = "0";
    negative_ = false;
    return;
  }

  // 查找小数点
  size_t dot_pos = s.find('.');
  if (dot_pos != std::string::npos) {
    scale_ = static_cast<int32_t>(s.length() - dot_pos - 1);
    s.erase(dot_pos, 1);
  }

  // 移除前导零
  size_t start = 0;
  while (start < s.length() && s[start] == '0') {
    start++;
  }
  s = s.substr(start);

  // 处理纯零的情况
  if (s.empty()) {
    value_ = "0";
    negative_ = false;
    scale_ = 0;
    return;
  }

  // 验证只包含数字
  for (char c : s) {
    if (!std::isdigit(c)) {
      throw std::runtime_error("Invalid decimal format");
    }
  }

  value_ = s;
  normalize();
}

void Decimal::from_int64(int64_t value) {
  negative_ = (value < 0);
  value_ = std::to_string(negative_ ? -value : value);
  scale_ = 0;
  normalize();
}

void Decimal::from_double(double value) {
  if (std::isnan(value) || std::isinf(value)) {
    throw std::runtime_error("Invalid double value");
  }

  negative_ = (value < 0);
  double abs_value = std::abs(value);

  // 使用stringstream进行转换
  std::stringstream ss;
  ss << std::fixed << abs_value;
  std::string str = ss.str();

  from_string(str);
}

std::string Decimal::add_strings(const std::string& a, const std::string& b) {
  std::string result;
  int carry = 0;
  size_t i = a.length();
  size_t j = b.length();

  while (i > 0 || j > 0 || carry) {
    int sum = carry;
    if (i > 0) sum += (a[--i] - '0');
    if (j > 0) sum += (b[--j] - '0');

    carry = sum / 10;
    result.push_back((sum % 10) + '0');
  }

  std::reverse(result.begin(), result.end());
  return result;
}

std::string Decimal::subtract_strings(const std::string& a, const std::string& b) {
  // 假设a >= b
  std::string result;
  int borrow = 0;
  size_t i = a.length();
  size_t j = b.length();

  while (i > 0 || j > 0) {
    int diff = borrow;
    if (i > 0) diff += (a[--i] - '0');
    if (j > 0) diff -= (b[--j] - '0');

    if (diff < 0) {
      diff += 10;
      borrow = -1;
    } else {
      borrow = 0;
    }

    result.push_back(diff + '0');
  }

  std::reverse(result.begin(), result.end());

  // 移除前导零
  return remove_leading_zeros(result);
}

std::string Decimal::multiply_strings(const std::string& a, const std::string& b) {
  std::vector<int> result(a.length() + b.length(), 0);

  // 从右到左相乘
  for (size_t i = 0; i < a.length(); ++i) {
    for (size_t j = 0; j < b.length(); ++j) {
      int mul = (a[a.length() - 1 - i] - '0') * (b[b.length() - 1 - j] - '0');
      int sum = mul + result[i + j];

      result[i + j] = sum % 10;
      result[i + j + 1] += sum / 10;
    }
  }

  // 处理进位
  for (size_t i = 0; i < result.size() - 1; ++i) {
    if (result[i] >= 10) {
      result[i + 1] += result[i] / 10;
      result[i] %= 10;
    }
  }

  // 转换为字符串
  std::string str_result;
  bool leading_zero = true;
  for (int digit : result) {
    if (digit != 0 || !leading_zero) {
      leading_zero = false;
      str_result.push_back(digit + '0');
    }
  }

  return str_result.empty() ? "0" : str_result;
}

std::string Decimal::divide_strings(const std::string& dividend, const std::string& divisor, int32_t scale) {
  if (divisor == "0") {
    throw std::runtime_error("Division by zero");
  }

  std::string quotient;
  std::string remainder = dividend;

  // 整数部分除法
  while (remainder.length() >= divisor.length()) {
    // 简化的除法实现
    size_t quotient_digit = 0;
    size_t len = std::min(remainder.length(), divisor.length() + 1);

    for (size_t i = 1; i <= 10; ++i) {
      std::string temp = multiply_strings(divisor, std::to_string(i));
      if (compare_strings(temp, remainder.substr(0, len)) > 0) {
        quotient_digit = i - 1;
        break;
      }
    }

    if (quotient_digit > 0) {
      quotient.push_back(quotient_digit + '0');
      std::string product = multiply_strings(divisor, std::to_string(quotient_digit));
      remainder = subtract_strings(remainder, product + std::string(remainder.length() - product.length(), '0'));
    } else {
      quotient.push_back('0');
    }

    // 移除前导零
    remainder = remove_leading_zeros(remainder);
    if (remainder.empty()) {
      remainder = "0";
    }
  }

  // 小数部分（简化实现）
  if (scale > 0 && remainder != "0") {
    quotient.push_back('.');
    for (int32_t i = 0; i < scale && remainder != "0"; ++i) {
      remainder += "0"; // 相当于乘以10

      size_t digit = 0;
      while (compare_strings(multiply_strings(divisor, std::to_string(digit + 1)), remainder) <= 0) {
        digit++;
      }

      quotient.push_back(digit + '0');
      std::string product = multiply_strings(divisor, std::to_string(digit));
      remainder = subtract_strings(remainder, product);
      remainder = remove_leading_zeros(remainder);
      if (remainder.empty()) {
        remainder = "0";
      }
    }
  }

  return quotient.empty() ? "0" : quotient;
}

int Decimal::compare_strings(const std::string& a, const std::string& b) {
  std::string a_clean = remove_leading_zeros(a);
  std::string b_clean = remove_leading_zeros(b);

  if (a_clean.length() != b_clean.length()) {
    return a_clean.length() > b_clean.length() ? 1 : -1;
  }

  return a_clean.compare(b_clean);
}

std::string Decimal::remove_leading_zeros(const std::string& str) {
  size_t start = 0;
  while (start < str.length() && str[start] == '0') {
    start++;
  }
  return start == str.length() ? "0" : str.substr(start);
}

void Decimal::adjust_scale(int32_t new_scale) {
  int32_t scale_diff = new_scale - scale_;

  if (scale_diff > 0) {
    // 增加小数位
    value_ += std::string(scale_diff, '0');
  } else if (scale_diff < 0) {
    // 减少小数位（截断）
    int32_t digits_to_remove = -scale_diff;
    if (static_cast<size_t>(digits_to_remove) < value_.length()) {
      value_ = value_.substr(0, value_.length() - digits_to_remove);
    } else {
      value_ = "0";
    }
  }

  scale_ = new_scale;
  normalize();
}

void Decimal::validate_precision_scale() const {
  if (precision_ < 1 || precision_ > MAX_PRECISION) {
    throw std::runtime_error("Invalid precision");
  }
  if (scale_ < 0 || scale_ > MAX_SCALE || scale_ > precision_) {
    throw std::runtime_error("Invalid scale");
  }
}

// ==================== 全局函数 ====================

std::ostream& operator<<(std::ostream& os, const Decimal& decimal) {
  os << decimal.to_string();
  return os;
}

std::istream& operator>>(std::istream& is, Decimal& decimal) {
  std::string str;
  is >> str;
  decimal = Decimal(str);
  return is;
}

Decimal decimal_from_string(const std::string& str) {
  return Decimal(str);
}

std::string decimal_to_string(const Decimal& decimal) {
  return decimal.to_string();
}

Decimal abs(const Decimal& value) {
  return value.abs();
}

Decimal ceil(const Decimal& value) {
  return value.ceil();
}

Decimal floor(const Decimal& value) {
  return value.floor();
}

Decimal round(const Decimal& value, int32_t decimals) {
  return value.round(decimals);
}

Decimal truncate(const Decimal& value, int32_t decimals) {
  return value.truncate(decimals);
}

Decimal power(const Decimal& base, int32_t exponent) {
  if (exponent < 0) {
    throw std::runtime_error("Negative exponent not supported");
  }

  Decimal result("1");
  Decimal current = base;

  while (exponent > 0) {
    if (exponent % 2 == 1) {
      result *= current;
    }
    current *= current;
    exponent /= 2;
  }

  return result;
}

Decimal sqrt(const Decimal& value) {
  if (value.is_negative()) {
    throw std::runtime_error("Cannot take square root of negative number");
  }

  // 牛顿法求平方根（简化实现）
  Decimal x = value;
  Decimal y("1");

  for (int i = 0; i < 10; ++i) { // 10次迭代
    y = (y + value / y) / Decimal("2");
  }

  return y;
}

} // namespace sqlcc
