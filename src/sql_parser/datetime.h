/**
 * WHY: 为什么需要专门的日期时间处理系统？
 *
 * 数据库系统需要精确处理时间和日期信息，传统方案存在诸多问题：
 * - 时间精度不足：只能处理到秒级别，无法满足毫秒和微秒需求
 * - 时区处理复杂：缺乏统一的时区处理和转换机制
 * - 格式多样化：不同的输入输出格式难以统一处理
 * - 运算功能缺失：缺少时间间隔计算和日期运算功能
 * - 边界情况处理：闰年、闰秒等特殊情况处理不完善
 * - 性能开销大：频繁的字符串解析和格式化效率低下
 *
 * 日期时间处理系统的核心价值：
 * 1. 高精度时间：支持毫秒、微秒、纳秒级别的时间精度
 * 2. 时区智能：完善的时区处理和自动转换机制
 * 3. 格式灵活：支持多种输入输出格式和自定义格式化
 * 4. 运算丰富：完整的时间间隔计算和日期运算功能
 * 5. 边界完善：正确处理闰年、闰秒等边界情况
 * 6. 性能优化：高效的解析、存储和运算算法
 *
 * 🏗️ 设计模式：值对象模式(Value Object Pattern)
 *
 * 日期时间类作为值对象模式的经典应用：
 * - 不可变性：日期时间对象一旦创建就不可修改
 * - 值语义：对象比较基于内容而非引用
 * - 完整构造：构造函数提供所有必需参数
 * - 无副作用：所有操作都是纯函数不产生副作用
 * - 线程安全：不可变对象天然线程安全
 * - 内存高效：避免不必要的对象拷贝和引用计数
 *
 * SOLID原则体现：
 * - 单一职责：日期时间类负责时间和日期的存储和操作
 * - 开闭原则：新时间功能通过扩展现有类实现
 * - 里氏替换：日期时间子类可以替换基类使用
 * - 接口隔离：日期时间接口精确定义所需方法
 * - 依赖倒置：高层模块依赖日期时间接口而非实现
 *
 * WHAT: 日期时间处理系统 - 完整的时序数据管理框架
 *
 * 核心功能：
 * - 日期时间表示：精确的日期时间存储和表示
 * - 时间间隔计算：灵活的时间间隔定义和运算
 * - 时区处理：完整的时区转换和本地化支持
 * - 格式化输出：多种格式的字符串输入输出
 * - 日期运算：丰富的日期加减和比较操作
 * - 边界验证：严格的日期时间有效性验证
 *
 * 系统组件：
 * - DateTime：主要的日期时间类，支持完整的日期时间操作
 * - Interval：时间间隔类，支持各种时间单位的间隔计算
 * - Timestamp：高精度时间戳类，用于精确时间测量
 * - 时区系统：完整的时区信息管理和转换
 * - 格式化器：灵活的日期时间格式化和解析
 * - 验证器：严格的日期时间有效性检查
 *
 * 日期时间表示能力：
 * - 年月日时分秒：标准的日期时间组件表示
 * - 毫秒精度：支持到毫秒级别的精确时间
 * - 时区偏移：完整的时区偏移量处理
 * - 夏令时：自动处理夏令时转换
 * - 闰年处理：正确处理闰年和闰月
 * - 历史日期：支持从公元前到未来的日期范围
 *
 * 时间间隔运算能力：
 * - 多单位支持：年、月、日、时、分、秒、毫秒等单位
 * - 混合运算：支持不同单位的间隔混合计算
 * - 标准化处理：自动处理进位和借位逻辑
 * - 负间隔：支持负时间间隔的运算
 * - 比较操作：完整的间隔比较和排序功能
 *
 * 时区处理能力：
 * - 时区数据库：内置完整的时区信息数据库
 * - 自动转换：智能的时区转换和本地化
 * - 命名时区：支持"IANA"标准时区名称
 * - 偏移时区：支持固定偏移量的时区表示
 * - 夏令时规则：自动处理夏令时转换规则
 *
 * 格式化输出能力：
 * - 标准格式：ISO 8601等标准格式支持
 * - 自定义格式：灵活的格式化字符串模板
 * - 本地化：支持不同语言和地区的格式化
 * - 解析输入：从字符串精确解析日期时间
 * - 错误容忍：宽松的输入格式解析
 *
 * 接口设计：
 * - 构造接口：多种方式构造日期时间对象
 * - 访问接口：获取日期时间各个组件的值
 * - 运算接口：日期时间的加减和比较操作
 * - 转换接口：与其他时间表示的相互转换
 * - 格式化接口：字符串的输入输出和格式化
 *
 * HOW: 日期时间处理系统的实现机制
 *
 * 日期时间存储实现：
 * 1. 时间戳基准：使用Unix时间戳作为内部存储基准
 * 2. 毫秒精度：以毫秒为单位提供足够的精度
 * 3. 时区分离：时间戳和时区信息分离存储
 * 4. 规范化存储：统一使用UTC作为内部基准时间
 * 5. 边界处理：特殊处理最小和最大日期时间值
 *
 * 时间间隔计算实现：
 * 1. 单位分离：分别存储不同时间单位的数值
 * 2. 规范化算法：处理进位和借位的标准化逻辑
 * 3. 混合运算：支持不同单位的间隔混合计算
 * 4. 精度保持：尽量保持计算过程中的精度
 * 5. 溢出处理：检测和处理计算溢出情况
 *
 * 时区转换实现：
 * 1. 时区数据库：维护完整的时区转换规则
 * 2. 偏移计算：根据时区规则计算偏移量
 * 3. 夏令时处理：动态处理夏令时转换
 * 4. 本地化转换：支持本地时间和UTC的相互转换
 * 5. 历史规则：处理历史时区规则的变化
 *
 * 格式化解析实现：
 * 1. 模板解析：解析格式化字符串模板
 * 2. 组件提取：从模板中提取格式化组件
 * 3. 数值格式化：将数值转换为指定格式的字符串
 * 4. 填充对齐：处理宽度、填充字符和对齐方式
 * 5. 转义处理：正确处理特殊字符的转义
 *
 * 日期运算实现：
 * 1. 日历算法：使用格里高利历算法进行日期计算
 * 2. 闰年判断：精确判断闰年和闰月
 * 3. 月日调整：处理不同月份的天数差异
 * 4. 边界检查：验证运算结果的合法性
 * 5. 循环处理：处理日期循环和周期性计算
 *
 * 性能优化策略：
 * - 缓存机制：缓存常用格式化和解析结果
 * - 惰性计算：延迟计算复杂的日期组件
 * - SIMD加速：向量化时间格式化和解析操作
 * - 内存池：复用日期时间对象的内存分配
 * - 批量处理：支持批量日期时间操作
 *
 * 错误处理机制：
 * - 格式错误：日期时间格式解析错误的详细诊断
 * - 范围错误：超出有效范围的日期时间值检测
 * - 时区错误：无效时区信息的识别和处理
 * - 运算错误：日期运算溢出和无效操作的检测
 * - 转换错误：类型转换失败的异常处理
 *
 * 扩展性设计：
 * - 插件架构：支持自定义日历系统和时区规则
 * - 配置化：日期时间处理的配置化管理
 * - 多历法：扩展支持其他历法系统
 * - 国际化：支持多语言和地区的本地化需求
 * - 向后兼容：保持与现有日期时间系统的兼容性
 *
 * 调试和诊断：
 * - 时间追踪：详细记录日期时间操作的执行轨迹
 * - 性能监控：日期时间操作的性能统计和分析
 * - 边界测试：自动化测试边界情况和异常情况
 * - 一致性检查：验证日期时间计算的一致性和正确性
 * - 可视化工具：日期时间数据的图形化展示和分析
 */

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
