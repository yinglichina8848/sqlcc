#include "datetime.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace sqlcc {

// ==================== Interval 类实现 ====================

Interval::Interval() : years_(0), months_(0), days_(0), hours_(0), minutes_(0), seconds_(0), milliseconds_(0) {}

Interval::Interval(int64_t value, Unit unit) : Interval() {
  switch (unit) {
    case MICROSECOND: milliseconds_ = value / 1000; break;
    case MILLISECOND: milliseconds_ = value; break;
    case SECOND: seconds_ = value; break;
    case MINUTE: minutes_ = value; break;
    case HOUR: hours_ = value; break;
    case DAY: days_ = value; break;
    case WEEK: days_ = value * 7; break;
    case MONTH: months_ = value; break;
    case YEAR: years_ = value; break;
  }
  normalize();
}

Interval::Interval(int years, int months, int days, int hours, int minutes, int seconds, int milliseconds)
    : years_(years), months_(months), days_(days), hours_(hours), minutes_(minutes), seconds_(seconds), milliseconds_(milliseconds) {
  normalize();
}

Interval::Interval(const std::string& interval_str) : Interval() {
  // 简化的间隔字符串解析，如 "1 year 2 months 3 days"
  std::istringstream iss(interval_str);
  std::string token;
  while (iss >> token) {
    int64_t value;
    std::string unit;
    if (std::isdigit(token[0]) || (token[0] == '-' && token.size() > 1)) {
      value = std::stoll(token);
      if (iss >> unit) {
        if (unit == "year" || unit == "years") years_ += value;
        else if (unit == "month" || unit == "months") months_ += value;
        else if (unit == "day" || unit == "days") days_ += value;
        else if (unit == "hour" || unit == "hours") hours_ += value;
        else if (unit == "minute" || unit == "minutes") minutes_ += value;
        else if (unit == "second" || unit == "seconds") seconds_ += value;
        else if (unit == "millisecond" || unit == "milliseconds") milliseconds_ += value;
      }
    }
  }
  normalize();
}

Interval::Interval(const Interval& other)
    : years_(other.years_), months_(other.months_), days_(other.days_), hours_(other.hours_),
      minutes_(other.minutes_), seconds_(other.seconds_), milliseconds_(other.milliseconds_) {}

Interval::Interval(Interval&& other) noexcept
    : years_(other.years_), months_(other.months_), days_(other.days_), hours_(other.hours_),
      minutes_(other.minutes_), seconds_(other.seconds_), milliseconds_(other.milliseconds_) {
  other.years_ = other.months_ = other.days_ = other.hours_ = other.minutes_ = other.seconds_ = other.milliseconds_ = 0;
}

Interval::~Interval() {}

Interval& Interval::operator=(const Interval& other) {
  if (this != &other) {
    years_ = other.years_;
    months_ = other.months_;
    days_ = other.days_;
    hours_ = other.hours_;
    minutes_ = other.minutes_;
    seconds_ = other.seconds_;
    milliseconds_ = other.milliseconds_;
  }
  return *this;
}

Interval& Interval::operator=(Interval&& other) noexcept {
  if (this != &other) {
    years_ = other.years_;
    months_ = other.months_;
    days_ = other.days_;
    hours_ = other.hours_;
    minutes_ = other.minutes_;
    seconds_ = other.seconds_;
    milliseconds_ = other.milliseconds_;
    other.years_ = other.months_ = other.days_ = other.hours_ = other.minutes_ = other.seconds_ = other.milliseconds_ = 0;
  }
  return *this;
}

// 算术操作符
Interval Interval::operator+(const Interval& other) const {
  Interval result(*this);
  result += other;
  return result;
}

Interval Interval::operator-(const Interval& other) const {
  Interval result(*this);
  result -= other;
  return result;
}

Interval Interval::operator*(int64_t factor) const {
  Interval result(*this);
  result *= factor;
  return result;
}

Interval Interval::operator/(int64_t divisor) const {
  if (divisor == 0) throw std::runtime_error("Division by zero");
  Interval result(*this);
  result /= divisor;
  return result;
}

Interval& Interval::operator+=(const Interval& other) {
  years_ += other.years_;
  months_ += other.months_;
  days_ += other.days_;
  hours_ += other.hours_;
  minutes_ += other.minutes_;
  seconds_ += other.seconds_;
  milliseconds_ += other.milliseconds_;
  normalize();
  return *this;
}

Interval& Interval::operator-=(const Interval& other) {
  years_ -= other.years_;
  months_ -= other.months_;
  days_ -= other.days_;
  hours_ -= other.hours_;
  minutes_ -= other.minutes_;
  seconds_ -= other.seconds_;
  milliseconds_ -= other.milliseconds_;
  normalize();
  return *this;
}

Interval& Interval::operator*=(int64_t factor) {
  years_ *= factor;
  months_ *= factor;
  days_ *= factor;
  hours_ *= factor;
  minutes_ *= factor;
  seconds_ *= factor;
  milliseconds_ *= factor;
  normalize();
  return *this;
}

Interval& Interval::operator/=(int64_t divisor) {
  if (divisor == 0) throw std::runtime_error("Division by zero");
  years_ /= divisor;
  months_ /= divisor;
  days_ /= divisor;
  hours_ /= divisor;
  minutes_ /= divisor;
  seconds_ /= divisor;
  milliseconds_ /= divisor;
  normalize();
  return *this;
}

// 比较操作符
bool Interval::operator==(const Interval& other) const {
  return years_ == other.years_ && months_ == other.months_ && days_ == other.days_ &&
         hours_ == other.hours_ && minutes_ == other.minutes_ && seconds_ == other.seconds_ &&
         milliseconds_ == other.milliseconds_;
}

bool Interval::operator!=(const Interval& other) const { return !(*this == other); }
bool Interval::operator<(const Interval& other) const { return to_milliseconds() < other.to_milliseconds(); }
bool Interval::operator<=(const Interval& other) const { return to_milliseconds() <= other.to_milliseconds(); }
bool Interval::operator>(const Interval& other) const { return to_milliseconds() > other.to_milliseconds(); }
bool Interval::operator>=(const Interval& other) const { return to_milliseconds() >= other.to_milliseconds(); }

// 类型转换
std::string Interval::to_string() const {
  std::stringstream ss;
  bool has_content = false;

  if (years_ != 0) { ss << years_ << " year" << (std::abs(years_) != 1 ? "s" : ""); has_content = true; }
  if (months_ != 0) { if (has_content) ss << " "; ss << months_ << " month" << (std::abs(months_) != 1 ? "s" : ""); has_content = true; }
  if (days_ != 0) { if (has_content) ss << " "; ss << days_ << " day" << (std::abs(days_) != 1 ? "s" : ""); has_content = true; }
  if (hours_ != 0) { if (has_content) ss << " "; ss << hours_ << " hour" << (std::abs(hours_) != 1 ? "s" : ""); has_content = true; }
  if (minutes_ != 0) { if (has_content) ss << " "; ss << minutes_ << " minute" << (std::abs(minutes_) != 1 ? "s" : ""); has_content = true; }
  if (seconds_ != 0) { if (has_content) ss << " "; ss << seconds_ << " second" << (std::abs(seconds_) != 1 ? "s" : ""); has_content = true; }
  if (milliseconds_ != 0) { if (has_content) ss << " "; ss << milliseconds_ << " millisecond" << (std::abs(milliseconds_) != 1 ? "s" : ""); has_content = true; }

  if (!has_content) return "0 seconds";
  return ss.str();
}

int64_t Interval::to_milliseconds() const {
  // 近似转换，忽略月份和年份的复杂性
  return milliseconds_ +
         seconds_ * 1000LL +
         minutes_ * 60LL * 1000LL +
         hours_ * 60LL * 60LL * 1000LL +
         days_ * 24LL * 60LL * 60LL * 1000LL +
         months_ * 30LL * 24LL * 60LL * 60LL * 1000LL +  // 近似每月30天
         years_ * 365LL * 24LL * 60LL * 60LL * 1000LL;   // 近似每年365天
}

// 组件访问
int64_t Interval::years() const { return years_; }
int64_t Interval::months() const { return months_; }
int64_t Interval::days() const { return days_; }
int64_t Interval::hours() const { return hours_; }
int64_t Interval::minutes() const { return minutes_; }
int64_t Interval::seconds() const { return seconds_; }
int64_t Interval::milliseconds() const { return milliseconds_; }

// 静态方法
Interval Interval::from_milliseconds(int64_t ms) { return Interval(ms, MILLISECOND); }
Interval Interval::from_seconds(int64_t seconds) { return Interval(seconds, SECOND); }
Interval Interval::from_minutes(int64_t minutes) { return Interval(minutes, MINUTE); }
Interval Interval::from_hours(int64_t hours) { return Interval(hours, HOUR); }
Interval Interval::from_days(int64_t days) { return Interval(days, DAY); }

void Interval::normalize() {
  // 处理进位（简化实现）
  while (milliseconds_ >= 1000) { seconds_ += milliseconds_ / 1000; milliseconds_ %= 1000; }
  while (milliseconds_ <= -1000) { seconds_ += milliseconds_ / 1000; milliseconds_ = -((-milliseconds_) % 1000); }

  while (seconds_ >= 60) { minutes_ += seconds_ / 60; seconds_ %= 60; }
  while (seconds_ <= -60) { minutes_ += seconds_ / 60; seconds_ = -((-seconds_) % 60); }

  while (minutes_ >= 60) { hours_ += minutes_ / 60; minutes_ %= 60; }
  while (minutes_ <= -60) { hours_ += minutes_ / 60; minutes_ = -((-minutes_) % 60); }

  while (hours_ >= 24) { days_ += hours_ / 24; hours_ %= 24; }
  while (hours_ <= -24) { days_ += hours_ / 24; hours_ = -((-hours_) % 24); }

  // 月份和年份的处理更复杂，这里简化
  while (months_ >= 12) { years_ += months_ / 12; months_ %= 12; }
  while (months_ <= -12) { years_ += months_ / 12; months_ = -((-months_) % 12); }
}

// ==================== DateTime 类实现 ====================

DateTime::DateTime() : timestamp_ms_(0), timezone_offset_minutes_(0) {}

DateTime::DateTime(int64_t timestamp_ms) : timestamp_ms_(timestamp_ms), timezone_offset_minutes_(0) {}

DateTime::DateTime(int year, int month, int day, int hour, int minute, int second, int millisecond)
    : timezone_offset_minutes_(0) {
  from_components(year, month, day, hour, minute, second, millisecond);
}

DateTime::DateTime(const std::string& datetime_str) : timezone_offset_minutes_(0) {
  // 简化的日期时间字符串解析
  // 支持格式: YYYY-MM-DD HH:MM:SS.mmm 或 YYYY-MM-DDTHH:MM:SS.mmmZ
  std::tm tm = {};
  std::istringstream iss(datetime_str);

  if (datetime_str.find('T') != std::string::npos) {
    // ISO 8601格式
    iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (iss.fail()) throw std::runtime_error("Invalid datetime format");

    // 处理毫秒部分
    if (datetime_str.find('.') != std::string::npos) {
      size_t dot_pos = datetime_str.find('.');
      size_t ms_end = datetime_str.find('Z', dot_pos);
      if (ms_end == std::string::npos) ms_end = datetime_str.find('+', dot_pos);
      if (ms_end == std::string::npos) ms_end = datetime_str.find('-', dot_pos);
      if (ms_end == std::string::npos) ms_end = datetime_str.size();

      std::string ms_str = datetime_str.substr(dot_pos + 1, ms_end - dot_pos - 1);
      tm.tm_sec += std::stoi(ms_str) / 1000;  // 简化处理
    }
  } else {
    // 标准格式
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (iss.fail()) throw std::runtime_error("Invalid datetime format");
  }

  // 转换为时间戳
  timestamp_ms_ = std::mktime(&tm) * 1000LL;
}

DateTime::DateTime(const DateTime& other)
    : timestamp_ms_(other.timestamp_ms_), timezone_offset_minutes_(other.timezone_offset_minutes_) {}

DateTime::DateTime(DateTime&& other) noexcept
    : timestamp_ms_(other.timestamp_ms_), timezone_offset_minutes_(other.timezone_offset_minutes_) {}

DateTime::~DateTime() {}

// 赋值操作符
DateTime& DateTime::operator=(const DateTime& other) {
  if (this != &other) {
    timestamp_ms_ = other.timestamp_ms_;
    timezone_offset_minutes_ = other.timezone_offset_minutes_;
  }
  return *this;
}

DateTime& DateTime::operator=(DateTime&& other) noexcept {
  if (this != &other) {
    timestamp_ms_ = other.timestamp_ms_;
    timezone_offset_minutes_ = other.timezone_offset_minutes_;
  }
  return *this;
}

// 比较操作符
bool DateTime::operator==(const DateTime& other) const { return timestamp_ms_ == other.timestamp_ms_; }
bool DateTime::operator!=(const DateTime& other) const { return !(*this == other); }
bool DateTime::operator<(const DateTime& other) const { return timestamp_ms_ < other.timestamp_ms_; }
bool DateTime::operator<=(const DateTime& other) const { return timestamp_ms_ <= other.timestamp_ms_; }
bool DateTime::operator>(const DateTime& other) const { return timestamp_ms_ > other.timestamp_ms_; }
bool DateTime::operator>=(const DateTime& other) const { return timestamp_ms_ >= other.timestamp_ms_; }

// 算术操作符
DateTime DateTime::operator+(const Interval& interval) const {
  DateTime result(*this);
  result += interval;
  return result;
}

DateTime DateTime::operator-(const Interval& interval) const {
  DateTime result(*this);
  result -= interval;
  return result;
}

Interval DateTime::operator-(const DateTime& other) const {
  int64_t diff_ms = timestamp_ms_ - other.timestamp_ms_;
  return Interval::from_milliseconds(diff_ms);
}

DateTime& DateTime::operator+=(const Interval& interval) {
  // 简化的实现，实际需要考虑月份和年份的复杂性
  timestamp_ms_ += interval.to_milliseconds();
  return *this;
}

DateTime& DateTime::operator-=(const Interval& interval) {
  timestamp_ms_ -= interval.to_milliseconds();
  return *this;
}

// 类型转换
std::string DateTime::to_string() const {
  std::time_t time = timestamp_ms_ / 1000;
  std::tm* tm = std::localtime(&time);
  std::stringstream ss;
  ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");

  int ms = timestamp_ms_ % 1000;
  if (ms != 0) {
    ss << "." << std::setfill('0') << std::setw(3) << ms;
  }

  return ss.str();
}

std::string DateTime::to_iso_string() const {
  std::time_t time = timestamp_ms_ / 1000;
  std::tm* tm = std::gmtime(&time);
  std::stringstream ss;
  ss << std::put_time(tm, "%Y-%m-%dT%H:%M:%S");

  int ms = timestamp_ms_ % 1000;
  if (ms != 0) {
    ss << "." << std::setfill('0') << std::setw(3) << ms;
  }

  ss << "Z";
  return ss.str();
}

int64_t DateTime::to_timestamp_ms() const { return timestamp_ms_; }
int64_t DateTime::to_timestamp_s() const { return timestamp_ms_ / 1000; }

// 日期时间组件访问
int DateTime::year() const { int y, m, d, h, min, s, ms; to_components(y, m, d, h, min, s, ms); return y; }
int DateTime::month() const { int y, m, d, h, min, s, ms; to_components(y, m, d, h, min, s, ms); return m; }
int DateTime::day() const { int y, m, d, h, min, s, ms; to_components(y, m, d, h, min, s, ms); return d; }
int DateTime::hour() const { int y, m, d, h, min, s, ms; to_components(y, m, d, h, min, s, ms); return h; }
int DateTime::minute() const { int y, m, d, h, min, s, ms; to_components(y, m, d, h, min, s, ms); return min; }
int DateTime::second() const { int y, m, d, h, min, s, ms; to_components(y, m, d, h, min, s, ms); return s; }
int DateTime::millisecond() const { int y, m, d, h, min, s, ms; to_components(y, m, d, h, min, s, ms); return ms; }

// 静态方法
DateTime DateTime::now() {
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
  return DateTime(millis.count());
}

DateTime DateTime::today() {
  DateTime now_dt = now();
  return DateTime(now_dt.year(), now_dt.month(), now_dt.day());
}

DateTime DateTime::from_timestamp_ms(int64_t timestamp_ms) {
  return DateTime(timestamp_ms);
}

DateTime DateTime::from_timestamp_s(int64_t timestamp_s) {
  return DateTime(timestamp_s * 1000LL);
}

// 私有方法实现
void DateTime::from_components(int year, int month, int day, int hour, int minute, int second, int millisecond) {
  if (!is_valid_date(year, month, day)) {
    throw std::runtime_error("Invalid date");
  }

  std::tm tm = {};
  tm.tm_year = year - 1900;
  tm.tm_mon = month - 1;
  tm.tm_mday = day;
  tm.tm_hour = hour;
  tm.tm_min = minute;
  tm.tm_sec = second;

  timestamp_ms_ = std::mktime(&tm) * 1000LL + millisecond;
}

void DateTime::to_components(int& year, int& month, int& day, int& hour, int& minute, int& second, int& millisecond) const {
  std::time_t time = timestamp_ms_ / 1000;
  std::tm* tm = std::localtime(&time);

  year = tm->tm_year + 1900;
  month = tm->tm_mon + 1;
  day = tm->tm_mday;
  hour = tm->tm_hour;
  minute = tm->tm_min;
  second = tm->tm_sec;
  millisecond = timestamp_ms_ % 1000;
}

bool DateTime::is_valid_date(int year, int month, int day) const {
  if (year < 1900 || year > 9999) return false;
  if (month < 1 || month > 12) return false;
  if (day < 1 || day > 31) return false;

  static const int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int max_days = days_in_month[month];

  if (month == 2 && is_leap_year(year)) max_days = 29;

  return day <= max_days;
}

int DateTime::days_in_month(int year, int month) {
  static const int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month == 2 && is_leap_year(year)) return 29;
  return days_in_month[month];
}

bool DateTime::is_leap_year(int year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 未实现的日期时间操作（简化）
void DateTime::set_year(int year) { /* 简化实现 */ }
void DateTime::set_month(int month) { /* 简化实现 */ }
void DateTime::set_day(int day) { /* 简化实现 */ }
void DateTime::set_hour(int hour) { /* 简化实现 */ }
void DateTime::set_minute(int minute) { /* 简化实现 */ }
void DateTime::set_second(int second) { /* 简化实现 */ }
void DateTime::set_millisecond(int millisecond) { /* 简化实现 */ }
void DateTime::set_timezone_offset(int offset_minutes) { timezone_offset_minutes_ = offset_minutes; }
int DateTime::timezone_offset() const { return timezone_offset_minutes_; }
std::string DateTime::timezone_name() const { return "UTC"; }

DateTime DateTime::add_years(int years) const { return *this + Interval(years, Interval::YEAR); }
DateTime DateTime::add_months(int months) const { return *this + Interval(months, Interval::MONTH); }
DateTime DateTime::add_days(int days) const { return *this + Interval(days, Interval::DAY); }
DateTime DateTime::add_hours(int hours) const { return *this + Interval(hours, Interval::HOUR); }
DateTime DateTime::add_minutes(int minutes) const { return *this + Interval(minutes, Interval::MINUTE); }
DateTime DateTime::add_seconds(int seconds) const { return *this + Interval(seconds, Interval::SECOND); }
DateTime DateTime::add_milliseconds(int milliseconds) const { return *this + Interval(milliseconds, Interval::MILLISECOND); }

bool DateTime::is_leap_year() const { return is_leap_year(year()); }
int DateTime::day_of_week() const { /* 简化实现 */ return 0; }
int DateTime::day_of_year() const { /* 简化实现 */ return 0; }
int DateTime::week_of_year() const { /* 简化实现 */ return 0; }
int DateTime::days_in_month() const { return days_in_month(year(), month()); }
int DateTime::days_in_year() const { return is_leap_year() ? 366 : 365; }

DateTime DateTime::parse(const std::string& str, const std::string& format) {
  return DateTime(str); // 简化实现
}

std::string DateTime::format(const std::string& format_str) const {
  return to_string(); // 简化实现
}

bool DateTime::is_valid() const {
  return timestamp_ms_ >= -2208988800000LL && timestamp_ms_ <= 4102444800000LL; // 1900-2100年范围
}

// ==================== Timestamp 类实现 ====================

Timestamp::Timestamp() : seconds_(0), nanoseconds_(0) {}
Timestamp::Timestamp(int64_t seconds, int32_t nanoseconds) : seconds_(seconds), nanoseconds_(nanoseconds) {}
Timestamp::Timestamp(const std::string& timestamp_str) : seconds_(0), nanoseconds_(0) {
  // 简化的时间戳字符串解析
  size_t dot_pos = timestamp_str.find('.');
  if (dot_pos != std::string::npos) {
    seconds_ = std::stoll(timestamp_str.substr(0, dot_pos));
    std::string ns_str = timestamp_str.substr(dot_pos + 1);
    nanoseconds_ = std::stoi(ns_str.substr(0, std::min(size_t(9), ns_str.size())));
  } else {
    seconds_ = std::stoll(timestamp_str);
  }
}

Timestamp::Timestamp(const Timestamp& other) : seconds_(other.seconds_), nanoseconds_(other.nanoseconds_) {}
Timestamp::Timestamp(Timestamp&& other) noexcept : seconds_(other.seconds_), nanoseconds_(other.nanoseconds_) {}

Timestamp::~Timestamp() {}

Timestamp& Timestamp::operator=(const Timestamp& other) {
  if (this != &other) {
    seconds_ = other.seconds_;
    nanoseconds_ = other.nanoseconds_;
  }
  return *this;
}

Timestamp& Timestamp::operator=(Timestamp&& other) noexcept {
  if (this != &other) {
    seconds_ = other.seconds_;
    nanoseconds_ = other.nanoseconds_;
  }
  return *this;
}

bool Timestamp::operator==(const Timestamp& other) const {
  return seconds_ == other.seconds_ && nanoseconds_ == other.nanoseconds_;
}

bool Timestamp::operator!=(const Timestamp& other) const { return !(*this == other); }
bool Timestamp::operator<(const Timestamp& other) const {
  return seconds_ < other.seconds_ || (seconds_ == other.seconds_ && nanoseconds_ < other.nanoseconds_);
}

bool Timestamp::operator<=(const Timestamp& other) const {
  return *this < other || *this == other;
}

bool Timestamp::operator>(const Timestamp& other) const {
  return !(*this <= other);
}

bool Timestamp::operator>=(const Timestamp& other) const {
  return !(*this < other);
}

std::string Timestamp::to_string() const {
  std::stringstream ss;
  ss << seconds_;
  if (nanoseconds_ != 0) {
    ss << "." << std::setfill('0') << std::setw(9) << nanoseconds_;
  }
  return ss.str();
}

int64_t Timestamp::seconds() const { return seconds_; }
int32_t Timestamp::nanoseconds() const { return nanoseconds_; }
double Timestamp::to_double() const { return static_cast<double>(seconds_) + static_cast<double>(nanoseconds_) / 1e9; }

Timestamp Timestamp::now() {
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
  auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration) -
                    std::chrono::duration_cast<std::chrono::nanoseconds>(seconds);
  return Timestamp(seconds.count(), nanoseconds.count());
}

Timestamp Timestamp::from_double(double timestamp) {
  int64_t seconds = static_cast<int64_t>(timestamp);
  int32_t nanoseconds = static_cast<int32_t>((timestamp - seconds) * 1e9);
  return Timestamp(seconds, nanoseconds);
}

// ==================== 全局函数 ====================

DateTime date_from_string(const std::string& str) { return DateTime(str); }
std::string date_to_string(const DateTime& date) { return date.to_string(); }

Interval interval_from_string(const std::string& str) { return Interval(str); }
std::string interval_to_string(const Interval& interval) { return interval.to_string(); }

// 流操作符
std::ostream& operator<<(std::ostream& os, const DateTime& dt) {
  os << dt.to_string();
  return os;
}

std::istream& operator>>(std::istream& is, DateTime& dt) {
  std::string str;
  is >> str;
  dt = DateTime(str);
  return is;
}

std::ostream& operator<<(std::ostream& os, const Interval& interval) {
  os << interval.to_string();
  return os;
}

std::istream& operator>>(std::istream& is, Interval& interval) {
  std::string str;
  is >> str;
  interval = Interval(str);
  return is;
}

std::ostream& operator<<(std::ostream& os, const Timestamp& ts) {
  os << ts.to_string();
  return os;
}

std::istream& operator>>(std::istream& is, Timestamp& ts) {
  std::string str;
  is >> str;
  ts = Timestamp(str);
  return is;
}

// 全局常量定义
const DateTime EPOCH(0);
const DateTime MIN_DATETIME(-2208988800000LL); // 1900-01-01
const DateTime MAX_DATETIME(4102444800000LL);  // 2100-01-01

} // namespace sqlcc
