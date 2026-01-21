/**
 * @file function_definition.cpp
 * @brief 函数定义类实现
 */

#include "function/function_definition.h"
#include <algorithm>
#include <cctype>

namespace sqlcc {
namespace sql_parser {

FunctionDefinition::FunctionDefinition(const std::string& name, const std::string& return_type)
    : name_(name), return_type_(return_type) {
}

FunctionDefinition::~FunctionDefinition() {
}

void FunctionDefinition::addParameter(const FunctionParameter& param) {
    parameters_.push_back(param);
}

void FunctionDefinition::addCharacteristic(const std::string& characteristic) {
    characteristics_.push_back(characteristic);
}

void FunctionDefinition::setBody(const std::string& body) {
    body_ = body;
}

void FunctionDefinition::setLanguage(const std::string& language) {
    language_ = language;
}

bool FunctionDefinition::isDeterministic() const {
    for (const auto& characteristic : characteristics_) {
        if (characteristic == "DETERMINISTIC" || 
            characteristic == "deterministic") {
            return true;
        } else if (characteristic == "NOT DETERMINISTIC" || 
                   characteristic == "NOT_DETERMINISTIC" ||
                   characteristic == "not deterministic") {
            return false;
        }
    }
    return false; // 默认非确定性
}

bool FunctionDefinition::containsSql() const {
    for (const auto& characteristic : characteristics_) {
        if (characteristic == "CONTAINS SQL" || 
            characteristic == "CONTAINS_SQL" ||
            characteristic == "contains sql") {
            return true;
        }
    }
    return false;
}

bool FunctionDefinition::readsSqlData() const {
    for (const auto& characteristic : characteristics_) {
        if (characteristic == "READS SQL DATA" || 
            characteristic == "READS_SQL_DATA" ||
            characteristic == "reads sql data") {
            return true;
        }
    }
    return false;
}

bool FunctionDefinition::modifiesSqlData() const {
    for (const auto& characteristic : characteristics_) {
        if (characteristic == "MODIFIES SQL DATA" || 
            characteristic == "MODIFIES_SQL_DATA" ||
            characteristic == "modifies sql data") {
            return true;
        }
    }
    return false;
}

std::string FunctionDefinition::characteristicToString(FunctionCharacteristic characteristic) {
    switch (characteristic) {
        case FunctionCharacteristic::DETERMINISTIC:
            return "DETERMINISTIC";
        case FunctionCharacteristic::NOT_DETERMINISTIC:
            return "NOT_DETERMINISTIC";
        case FunctionCharacteristic::CONTAINS_SQL:
            return "CONTAINS_SQL";
        case FunctionCharacteristic::READS_SQL_DATA:
            return "READS_SQL_DATA";
        case FunctionCharacteristic::MODIFIES_SQL_DATA:
            return "MODIFIES_SQL_DATA";
        default:
            return "UNKNOWN";
    }
}

FunctionCharacteristic FunctionDefinition::stringToCharacteristic(const std::string& str) {
    std::string upper_str = str;
    std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(), ::toupper);
    
    if (upper_str.find("DETERMINISTIC") != std::string::npos) {
        if (upper_str.find("NOT") != std::string::npos) {
            return FunctionCharacteristic::NOT_DETERMINISTIC;
        } else {
            return FunctionCharacteristic::DETERMINISTIC;
        }
    } else if (upper_str.find("CONTAINS SQL") != std::string::npos) {
        return FunctionCharacteristic::CONTAINS_SQL;
    } else if (upper_str.find("READS SQL DATA") != std::string::npos) {
        return FunctionCharacteristic::READS_SQL_DATA;
    } else if (upper_str.find("MODIFIES SQL DATA") != std::string::npos) {
        return FunctionCharacteristic::MODIFIES_SQL_DATA;
    }
    
    return FunctionCharacteristic::NOT_DETERMINISTIC; // 默认
}

} // namespace sql_parser
} // namespace sqlcc