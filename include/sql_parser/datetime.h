#pragma once

#include <string>
#include <chrono>
#include <ctime>
#include <memory>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <cstdint>

namespace sqlcc {

// 前向声明
class Interval;

class DateTime {
public:
  // 构造函数
  DateTime();
  DateTime(int64_t timestamp_ms); // 从毫秒时间戳构造
  DateTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0, int millisecond = 0);
  DateTime(const std::string& datetime_str);
  DateTime(const DateTime& other);
  DateTime(DateTime&& other) noexcept;

  // 析构函数
  ~DateTime();

  // 赋值操作符
  DateTime& operator=(const DateTime& other);
  DateTime& operator=(DateTime&& other) noexcept;

  // 比较操作符
  bool operator==(const DateTime& other) const;
  bool operator!=(const DateTime& other) const;
  bool operator<(const DateTime& other) const;
  bool operator<=(const DateTime& other) const;
  bool operator>(const DateTime& other) const;
  bool operator>=(const DateTime& other) const;

  // 算术操作符
  DateTime operator+(const Interval& interval) const;
  DateTime operator-(const Interval& interval) const;
  Interval operator-(const DateTime& other) const;

  // 复合赋值操作符
  DateTime& operator+=(const Interval& interval);
  DateTime& operator-=(const Interval& interval);

  // 类型转换
  std::string to_string() const;
  std::string to_iso_string() const; // ISO 8601格式
  int64_t to_timestamp_ms() const;   // 毫秒时间戳
  int64_t to_timestamp_s() const;    // 秒时间戳

  // 日期时间组件访问
  int year() const;
  int month() const;      // 1-12
  int day() const;        // 1-31
  int hour() const;       // 0-23
  int minute() const;     // 0-59
  int second() const;     // 0-59
  int millisecond() const; // 0-999

  // 日期时间组件设置
  void set_year(int year);
  void set_month(int month);
  void set_day(int day);
  void set_hour(int hour);
  void set_minute(int minute);
  void set_second(int second);
  void set_millisecond(int millisecond);

  // 时区相关
  void set_timezone_offset(int offset_minutes); // 设置时区偏移（分钟）
  int timezone_offset() const; // 获取时区偏移（分钟）
  std::string timezone_name() const; // 获取时区名称

  // 日期时间操作
  DateTime add_years(int years) const;
  DateTime add_months(int months) const;
  DateTime add_days(int days) const;
  DateTime add_hours(int hours) const;
  DateTime add_minutes(int minutes) const;
  DateTime add_seconds(int seconds) const;
  DateTime add_milliseconds(int milliseconds) const;

  // 日期时间查询
  bool is_leap_year() const;        // 是否闰年
  int day_of_week() const;          // 星期几 (0=周日, 1=周一, ..., 6=周六)
  int day_of_year() const;          // 年中的第几天 (1-366)
  int week_of_year() const;         // 年中的第几周 (1-53)
  int days_in_month() const;        // 月份的天数
  int days_in_year() const;         // 年份的天数

  // 静态方法
  static DateTime now();                    // 当前时间
  static DateTime today();                  // 今天的日期（时间为00:00:00）
  static DateTime from_timestamp_ms(int64_t timestamp_ms);
  static DateTime from_timestamp_s(int64_t timestamp_s);
  static DateTime parse(const std::string& str, const std::string& format = ""); // 格式化解析

  // 格式化输出
  std::string format(const std::string& format_str) const;

  // 验证
  bool is_valid() const;

private:
  // 内部表示：使用毫秒时间戳
  int64_t timestamp_ms_;
  int timezone_offset_minutes_; // 时区偏移（分钟）

  // 内部辅助方法
  void from_components(int year, int month, int day, int hour, int minute, int second, int millisecond);
  void to_components(int& year, int& month, int& day, int& hour, int& minute, int& second, int& millisecond) const;
  bool is_valid_date(int year, int month, int day) const;
  static int days_in_month(int year, int month);
  static bool is_leap_year(int year);
};

// 时间间隔类
class Interval {
public:
  enum Unit {
    MICROSECOND,
    MILLISECOND,
    SECOND,
    MINUTE,
    HOUR,
    DAY,
    WEEK,
    MONTH,
    YEAR
  };

  // 构造函数
  Interval();
  Interval(int64_t value, Unit unit = MILLISECOND);
  Interval(int years, int months, int days, int hours, int minutes, int seconds, int milliseconds);
  Interval(const std::string& interval_str); // 解析"1 year 2 months"等格式
  Interval(const Interval& other);
  Interval(Interval&& other) noexcept;

  // 析构函数
  ~Interval();

  // 赋值操作符
  Interval& operator=(const Interval& other);
  Interval& operator=(Interval&& other) noexcept;

  // 算术操作符
  Interval operator+(const Interval& other) const;
  Interval operator-(const Interval& other) const;
  Interval operator*(int64_t factor) const;
  Interval operator/(int64_t divisor) const;

  // 复合赋值操作符
  Interval& operator+=(const Interval& other);
  Interval& operator-=(const Interval& other);
  Interval& operator*=(int64_t factor);
  Interval& operator/=(int64_t divisor);

  // 比较操作符
  bool operator==(const Interval& other) const;
  bool operator!=(const Interval& other) const;
  bool operator<(const Interval& other) const;
  bool operator<=(const Interval& other) const;
  bool operator>(const Interval& other) const;
  bool operator>=(const Interval& other) const;

  // 类型转换
  std::string to_string() const;
  int64_t to_milliseconds() const; // 转换为毫秒（近似值）

  // 组件访问
  int64_t years() const;
  int64_t months() const;
  int64_t days() const;
  int64_t hours() const;
  int64_t minutes() const;
  int64_t seconds() const;
  int64_t milliseconds() const;

  // 组件设置
  void set_years(int64_t years);
  void set_months(int64_t months);
  void set_days(int64_t days);
  void set_hours(int64_t hours);
  void set_minutes(int64_t minutes);
  void set_seconds(int64_t seconds);
  void set_milliseconds(int64_t milliseconds);

  // 静态方法
  static Interval from_milliseconds(int64_t ms);
  static Interval from_seconds(int64_t seconds);
  static Interval from_minutes(int64_t minutes);
  static Interval from_hours(int64_t hours);
  static Interval from_days(int64_t days);

private:
  // 内部表示：分别存储不同的时间单位
  int64_t years_;
  int64_t months_;
  int64_t days_;
  int64_t hours_;
  int64_t minutes_;
  int64_t seconds_;
  int64_t milliseconds_;

  // 标准化间隔（处理进位）
  void normalize();
};

// 时间戳类（用于高精度时间戳）
class Timestamp {
public:
  // 构造函数
  Timestamp();
  Timestamp(int64_t seconds, int32_t nanoseconds = 0);
  Timestamp(const std::string& timestamp_str);
  Timestamp(const Timestamp& other);
  Timestamp(Timestamp&& other) noexcept;

  // 析构函数
  ~Timestamp();

  // 赋值操作符
  Timestamp& operator=(const Timestamp& other);
  Timestamp& operator=(Timestamp&& other) noexcept;

  // 比较操作符
  bool operator==(const Timestamp& other) const;
  bool operator!=(const Timestamp& other) const;
  bool operator<(const Timestamp& other) const;
  bool operator<=(const Timestamp& other) const;
  bool operator>(const Timestamp& other) const;
  bool operator>=(const Timestamp& other) const;

  // 类型转换
  std::string to_string() const;
  int64_t seconds() const;
  int32_t nanoseconds() const;
  double to_double() const; // 转换为双精度浮点数

  // 静态方法
  static Timestamp now();
  static Timestamp from_double(double timestamp);

private:
  int64_t seconds_;      // 秒数
  int32_t nanoseconds_;  // 纳秒数 (0-999999999)
};

// 全局函数
DateTime date_from_string(const std::string& str);
std::string date_to_string(const DateTime& date);

Interval interval_from_string(const std::string& str);
std::string interval_to_string(const Interval& interval);

// 流操作符
std::ostream& operator<<(std::ostream& os, const DateTime& dt);
std::istream& operator>>(std::istream& is, DateTime& dt);

std::ostream& operator<<(std::ostream& os, const Interval& interval);
std::istream& operator>>(std::istream& is, Interval& interval);

std::ostream& operator<<(std::ostream& os, const Timestamp& ts);
std::istream& operator>>(std::istream& is, Timestamp& ts);

// 时间常量
extern const DateTime EPOCH;        // 1970-01-01 00:00:00
extern const DateTime MIN_DATETIME; // 最小日期时间
extern const DateTime MAX_DATETIME; // 最大日期时间

} // namespace sqlcc
