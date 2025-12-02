#include "execution_context.h"
#include "permission_validator.h"
#include <sstream>

namespace sqlcc {

ExecutionContext::ExecutionContext()
    : current_user("root"),            // 默认管理员用户（兼容旧代码）
      current_database(""),            // 默认无数据库（兼容旧代码）
      current_user_("root"),           // 默认管理员用户
      current_database_(""),           // 默认无数据库
      is_transactional_(false),        // 默认不处于事务中
      transaction_id_(""),             // 默认无事务ID
      read_only_(false),               // 默认不是只读
      records_affected(0),             // 影响的行数初始为0（兼容旧代码）
      rows_affected_(0),               // 影响的行数初始为0
      rows_returned_(0),               // 返回的行数初始为0
      execution_time_ms_(0),           // 执行时间初始为0
      used_index(false),               // 默认未使用索引（兼容旧代码）
      execution_plan("未优化"),        // 默认执行计划（兼容旧代码）
      used_index_(false),              // 默认未使用索引
      execution_plan_("未优化"),       // 默认执行计划
      plan_details_(""),               // 默认无计划详情
      optimized_plan_(""),             // 默认无优化计划
      query_optimized_(false),         // 默认未优化
      optimization_rules_(),           // 默认无优化规则
      index_info_(""),                 // 默认无索引信息
      cost_estimate_(0.0),             // 默认成本估算为0
      has_error_(false),               // 默认无错误
      error_message_(""),              // 默认无错误信息
      db_manager(nullptr),             // 默认无数据库管理器（兼容旧代码）
      user_manager(nullptr),           // 默认无用户管理器（兼容旧代码）
      system_db(nullptr),              // 默认无系统数据库（兼容旧代码）
      db_manager_(nullptr),            // 默认无数据库管理器
      user_manager_(nullptr),          // 默认无用户管理器
      system_db_(nullptr),             // 默认无系统数据库
      permission_validator_(nullptr) { // 默认无权限验证器
}

ExecutionContext::ExecutionContext(const std::string &current_user,
                                   const std::string &current_database,
                                   bool is_transactional)
    : current_user(current_user.empty() ? "root" : current_user),
      current_database(current_database),
      current_user_(current_user.empty() ? "root" : current_user),
      current_database_(current_database), is_transactional_(is_transactional),
      transaction_id_(""), read_only_(false), records_affected(0),
      rows_affected_(0), rows_returned_(0), execution_time_ms_(0),
      used_index(false), execution_plan("未优化"), used_index_(false),
      execution_plan_("未优化"), plan_details_(""), optimized_plan_(""),
      query_optimized_(false), optimization_rules_(), index_info_(""),
      cost_estimate_(0.0), has_error_(false), error_message_(""),
      db_manager(nullptr), user_manager(nullptr), system_db(nullptr),
      db_manager_(nullptr), user_manager_(nullptr), system_db_(nullptr),
      permission_validator_(nullptr) {}

ExecutionContext::ExecutionContext(std::shared_ptr<DatabaseManager> db_manager,
                                   std::shared_ptr<UserManager> user_manager,
                                   std::shared_ptr<SystemDatabase> system_db)
    : current_user("admin"),
      current_database(db_manager ? db_manager->GetCurrentDatabase() : ""),
      current_user_("admin"),
      current_database_(db_manager ? db_manager->GetCurrentDatabase() : ""),
      is_transactional_(false), transaction_id_(""), read_only_(false),
      records_affected(0), rows_affected_(0), rows_returned_(0),
      execution_time_ms_(0), used_index(false), execution_plan("未优化"),
      used_index_(false), execution_plan_("未优化"), plan_details_(""),
      optimized_plan_(""), query_optimized_(false), optimization_rules_(),
      index_info_(""), cost_estimate_(0.0), has_error_(false),
      error_message_(""), db_manager(db_manager), user_manager(user_manager),
      system_db(system_db), db_manager_(db_manager),
      user_manager_(user_manager), system_db_(system_db),
      permission_validator_(nullptr) {}

ExecutionContext::~ExecutionContext() {
  // 析构函数，无需特殊操作
}

// ==== 获取器和设置器 ====

const std::string &ExecutionContext::get_current_user() const {
  return current_user_;
}

void ExecutionContext::set_current_user(const std::string &user) {
  current_user_ = user;
}

const std::string &ExecutionContext::get_current_database() const {
  return current_database_;
}

void ExecutionContext::set_current_database(const std::string &database) {
  current_database_ = database;
}

bool ExecutionContext::is_transactional() const { return is_transactional_; }

void ExecutionContext::set_transactional(bool is_transactional) {
  is_transactional_ = is_transactional;
}

const std::string &ExecutionContext::get_transaction_id() const {
  return transaction_id_;
}

void ExecutionContext::set_transaction_id(const std::string &transaction_id) {
  transaction_id_ = transaction_id;
}

bool ExecutionContext::is_read_only() const { return read_only_; }

void ExecutionContext::set_read_only(bool read_only) { read_only_ = read_only; }

size_t ExecutionContext::get_rows_affected() const { return rows_affected_; }

void ExecutionContext::set_rows_affected(size_t rows) { rows_affected_ = rows; }

void ExecutionContext::increment_rows_affected(size_t rows) {
  rows_affected_ += rows;
}

size_t ExecutionContext::get_rows_returned() const { return rows_returned_; }

void ExecutionContext::set_rows_returned(size_t rows) { rows_returned_ = rows; }

size_t ExecutionContext::get_execution_time_ms() const {
  return execution_time_ms_;
}

void ExecutionContext::set_execution_time_ms(size_t time) {
  execution_time_ms_ = time;
}

bool ExecutionContext::is_used_index() const { return used_index_; }

void ExecutionContext::set_used_index(bool used_index) {
  used_index_ = used_index;
}

const std::string &ExecutionContext::get_execution_plan() const {
  return execution_plan_;
}

void ExecutionContext::set_execution_plan(const std::string &execution_plan) {
  execution_plan_ = execution_plan;
}

const std::string &ExecutionContext::get_plan_details() const {
  return plan_details_;
}

void ExecutionContext::set_plan_details(const std::string &plan_details) {
  plan_details_ = plan_details;
}

const std::string &ExecutionContext::get_optimized_plan() const {
  return optimized_plan_;
}

void ExecutionContext::set_optimized_plan(const std::string &optimized_plan) {
  optimized_plan_ = optimized_plan;
}

bool ExecutionContext::is_query_optimized() const { return query_optimized_; }

void ExecutionContext::set_query_optimized(bool query_optimized) {
  query_optimized_ = query_optimized;
}

const std::vector<std::string> &
ExecutionContext::get_optimization_rules() const {
  return optimization_rules_;
}

void ExecutionContext::set_optimization_rules(
    const std::vector<std::string> &optimization_rules) {
  optimization_rules_ = optimization_rules;
}

const std::string &ExecutionContext::get_index_info() const {
  return index_info_;
}

void ExecutionContext::set_index_info(const std::string &index_info) {
  index_info_ = index_info;
}

double ExecutionContext::get_cost_estimate() const { return cost_estimate_; }

void ExecutionContext::set_cost_estimate(double cost_estimate) {
  cost_estimate_ = cost_estimate;
}

bool ExecutionContext::has_error() const { return has_error_; }

void ExecutionContext::set_error(bool has_error,
                                 const std::string &error_message) {
  has_error_ = has_error;
  error_message_ = error_message;
}

const std::string &ExecutionContext::get_error_message() const {
  return error_message_;
}

void ExecutionContext::clear_error() {
  has_error_ = false;
  error_message_ = "";
}

// ==== 管理器相关 ====

std::shared_ptr<DatabaseManager> ExecutionContext::get_db_manager() const {
  return db_manager_;
}

void ExecutionContext::set_db_manager(
    std::shared_ptr<DatabaseManager> db_manager) {
  db_manager_ = db_manager;
}

std::shared_ptr<UserManager> ExecutionContext::get_user_manager() const {
  return user_manager_;
}

void ExecutionContext::set_user_manager(
    std::shared_ptr<UserManager> user_manager) {
  user_manager_ = user_manager;
}

std::shared_ptr<SystemDatabase> ExecutionContext::get_system_db() const {
  return system_db_;
}

void ExecutionContext::set_system_db(
    std::shared_ptr<SystemDatabase> system_db) {
  system_db_ = system_db;
}

// ==== 权限验证相关 ====

void ExecutionContext::set_permission_validator(
    std::shared_ptr<PermissionValidator> validator) {
  permission_validator_ = validator;
}

std::shared_ptr<PermissionValidator>
ExecutionContext::get_permission_validator() const {
  return permission_validator_;
}

// ==== 上下文操作 ====

void ExecutionContext::reset() {
  // 重置执行上下文，保留当前用户、数据库信息和管理器指针
  is_transactional_ = false;
  transaction_id_.clear();
  read_only_ = false;

  // 重置兼容旧代码的公共成员变量
  records_affected = 0;
  used_index = false;
  execution_plan = "未优化";

  // 重置新成员变量
  rows_affected_ = 0;
  rows_returned_ = 0;
  execution_time_ms_ = 0;
  used_index_ = false;
  execution_plan_ = "未优化";
  plan_details_.clear();
  optimized_plan_.clear();
  query_optimized_ = false;
  optimization_rules_.clear();
  index_info_.clear();
  cost_estimate_ = 0.0;
  has_error_ = false;
  error_message_.clear();
  // 保留权限验证器和管理器指针
}

std::shared_ptr<ExecutionContext> ExecutionContext::clone() const {
  auto cloned = std::make_shared<ExecutionContext>();

  // 复制兼容旧代码的公共成员变量
  cloned->current_user = current_user;
  cloned->current_database = current_database;
  cloned->records_affected = records_affected;
  cloned->used_index = used_index;
  cloned->execution_plan = execution_plan;
  cloned->db_manager = db_manager;
  cloned->user_manager = user_manager;
  cloned->system_db = system_db;

  // 复制新成员变量
  cloned->current_user_ = current_user_;
  cloned->current_database_ = current_database_;
  cloned->is_transactional_ = is_transactional_;
  cloned->transaction_id_ = transaction_id_;
  cloned->read_only_ = read_only_;
  cloned->rows_affected_ = rows_affected_;
  cloned->rows_returned_ = rows_returned_;
  cloned->execution_time_ms_ = execution_time_ms_;
  cloned->used_index_ = used_index_;
  cloned->execution_plan_ = execution_plan_;
  cloned->plan_details_ = plan_details_;
  cloned->optimized_plan_ = optimized_plan_;
  cloned->query_optimized_ = query_optimized_;
  cloned->optimization_rules_ = optimization_rules_;
  cloned->index_info_ = index_info_;
  cloned->cost_estimate_ = cost_estimate_;
  cloned->has_error_ = has_error_;
  cloned->error_message_ = error_message_;
  cloned->db_manager_ = db_manager_;
  cloned->user_manager_ = user_manager_;
  cloned->system_db_ = system_db_;
  cloned->permission_validator_ = permission_validator_; // 共享权限验证器
  return cloned;
}

// ==== 调试信息 ====

std::string ExecutionContext::to_string() const {
  std::ostringstream oss;
  oss << "ExecutionContext {" << std::endl;
  oss << "  current_user: '" << current_user_ << "'" << std::endl;
  oss << "  current_database: '" << current_database_ << "'" << std::endl;
  oss << "  is_transactional: " << (is_transactional_ ? "true" : "false")
      << std::endl;
  oss << "  transaction_id: '" << transaction_id_ << "'" << std::endl;
  oss << "  read_only: " << (read_only_ ? "true" : "false") << std::endl;
  oss << "  rows_affected: " << rows_affected_ << std::endl;
  oss << "  rows_returned: " << rows_returned_ << std::endl;
  oss << "  execution_time_ms: " << execution_time_ms_ << std::endl;
  oss << "  used_index: " << (used_index_ ? "true" : "false") << std::endl;
  oss << "  execution_plan: '" << execution_plan_ << "'" << std::endl;
  oss << "  plan_details: '" << plan_details_ << "'" << std::endl;
  oss << "  optimized_plan: '" << optimized_plan_ << "'" << std::endl;
  oss << "  query_optimized: " << (query_optimized_ ? "true" : "false")
      << std::endl;
  oss << "  optimization_rules: [";
  for (size_t i = 0; i < optimization_rules_.size(); ++i) {
    if (i > 0)
      oss << ", ";
    oss << "'" << optimization_rules_[i] << "'";
  }
  oss << "]" << std::endl;
  oss << "  index_info: '" << index_info_ << "'" << std::endl;
  oss << "  cost_estimate: " << cost_estimate_ << std::endl;
  oss << "  has_error: " << (has_error_ ? "true" : "false") << std::endl;
  oss << "  error_message: '" << error_message_ << "'" << std::endl;
  oss << "  has_db_manager: " << (db_manager_ ? "true" : "false") << std::endl;
  oss << "  has_user_manager: " << (user_manager_ ? "true" : "false")
      << std::endl;
  oss << "  has_system_db: " << (system_db_ ? "true" : "false") << std::endl;
  oss << "  has_permission_validator: "
      << (permission_validator_ ? "true" : "false") << std::endl;
  oss << "}";
  return oss.str();
}

} // namespace sqlcc