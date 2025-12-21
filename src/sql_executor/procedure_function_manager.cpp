#include "sql_executor/procedure_function_manager.h"
#include <iostream>
#include <ctime>

// ProcedureFunctionManager 实现
namespace sqlcc {
namespace sql_executor {

ProcedureFunctionManager& ProcedureFunctionManager::getInstance() {
  static ProcedureFunctionManager instance;
  return instance;
}

bool ProcedureFunctionManager::createProcedure(const sql_parser::CreateProcedureStatement& stmt) {
  const auto& def = stmt.getProcedureDefinition(); // 修正为正确的接口
  std::string procedureName = def.getName();

  if (procedureExists(procedureName)) {
    return false; // 存储过程已存在
  }

  ProcedureInfo info;
  info.name = procedureName;
  info.parameters = def.getParameters();
  info.body = def.getBody();
  info.owner = "current_user"; // TODO: 从上下文获取当前用户
  info.created_time = time(nullptr);

  procedures_[procedureName] = info;
  return true;
}

bool ProcedureFunctionManager::dropProcedure(const std::string& procedureName) {
  auto it = procedures_.find(procedureName);
  if (it != procedures_.end()) {
    procedures_.erase(it);
    return true;
  }
  return false;
}

std::unique_ptr<sql_parser::CallProcedureStatement> ProcedureFunctionManager::callProcedure(const std::string& name) {
  if (procedureExists(name)) {
    return std::make_unique<sql_parser::CallProcedureStatement>(name);
  }
  return nullptr;
}

bool ProcedureFunctionManager::procedureExists(const std::string& procedureName) const {
  return procedures_.find(procedureName) != procedures_.end();
}

bool ProcedureFunctionManager::createFunction(const sql_parser::CreateFunctionStatement& stmt) {
  const auto& def = stmt.getFunctionDefinition();
  std::string functionName = def.getName();

  if (functionExists(functionName)) {
    return false; // 函数已存在
  }

  FunctionInfo info;
  info.name = functionName;
  info.parameters = def.getParameters();
  info.returnDataType = def.getReturnType();
  info.body = def.getBody();
  info.language = def.getLanguage();
  info.deterministic = def.isDeterministic();
  info.owner = "current_user"; // TODO: 从上下文获取当前用户
  info.created_time = time(nullptr);

  functions_[functionName] = info;
  return true;
}

bool ProcedureFunctionManager::dropFunction(const std::string& functionName) {
  auto it = functions_.find(functionName);
  if (it != functions_.end()) {
    functions_.erase(it);
    return true;
  }
  return false;
}

bool ProcedureFunctionManager::functionExists(const std::string& functionName) const {
  return functions_.find(functionName) != functions_.end();
}

std::string ProcedureFunctionManager::executeProcedure(const sql_parser::CallProcedureStatement& stmt) {
  std::string procedureName = stmt.getProcedureName();
  auto it = procedures_.find(procedureName);
  if (it != procedures_.end()) {
    // TODO: 实际的存储过程执行逻辑
    return "Procedure '" + procedureName + "' executed successfully";
  }
  return "Procedure '" + procedureName + "' not found";
}

std::string ProcedureFunctionManager::executeFunction(const std::string& functionName, const std::vector<std::string>& args) {
  auto it = functions_.find(functionName);
  if (it != functions_.end()) {
    // TODO: 实际的函数执行逻辑
    return "Function '" + functionName + "' executed with " + std::to_string(args.size()) + " arguments";
  }
  return "Function '" + functionName + "' not found";
}

void ProcedureFunctionManager::setVariable(const std::string& varName, const std::string& value) {
  variables_[varName] = value;
}

std::string ProcedureFunctionManager::getVariable(const std::string& varName) const {
  auto it = variables_.find(varName);
  if (it != variables_.end()) {
    return it->second;
  }
  return "";
}

bool ProcedureFunctionManager::variableExists(const std::string& varName) const {
  return variables_.find(varName) != variables_.end();
}

} // namespace sql_executor
} // namespace sqlcc
