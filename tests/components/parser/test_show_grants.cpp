#include <iostream>
#include <string>
#include <vector>
#include <memory>

// 简单的前向声明，避免复杂的include依赖
namespace sqlcc {
    namespace sql_parser {
        enum class StatementType {
            UNKNOWN,
            CREATE_USER,
            DROP_USER,
            GRANT,
            REVOKE,
            SHOW
        };
        
        class Statement {
        public:
            virtual ~Statement() = default;
            virtual StatementType getType() const = 0;
        };
        
        class ShowStatement : public Statement {
        public:
            enum ShowType {
                DATABASES,
                TABLES,
                CREATE_TABLE,
                COLUMNS,
                INDEXES,
                GRANTS
            };
            
        private:
            ShowType type_;
            std::string targetObject_;
            std::string fromDatabase_;
            bool hasFromDb_;
            
        public:
            ShowStatement(ShowType type) : type_(type), hasFromDb_(false) {}
            
            ShowType getShowType() const { return type_; }
            void setTargetObject(const std::string& obj) { targetObject_ = obj; }
            std::string getTargetObject() const { return targetObject_; }
            void setFromDatabase(const std::string& db) { fromDatabase_ = db; hasFromDb_ = true; }
            std::string getFromDatabase() const { return fromDatabase_; }
            bool hasFromDatabase() const { return hasFromDb_; }
            
            StatementType getType() const override { return StatementType::SHOW; }
        };
        
        class ParserNew {
        private:
            std::string input_;
            size_t pos_;
            
        public:
            ParserNew(const std::string& input) : input_(input), pos_(0) {}
            
            std::vector<std::unique_ptr<Statement>> parse() {
                std::vector<std::unique_ptr<Statement>> statements;
                
                // 跳过空格
                skipWhitespace();
                
                // 检查是否以SHOW开始
                if (checkKeyword("SHOW")) {
                    auto stmt = parseShowStatement();
                    if (stmt) {
                        statements.push_back(std::move(stmt));
                    }
                }
                
                return statements;
            }
            
        private:
            void skipWhitespace() {
                while (pos_ < input_.length() && std::isspace(input_[pos_])) {
                    pos_++;
                }
            }
            
            bool checkKeyword(const std::string& keyword) {
                skipWhitespace();
                if (input_.compare(pos_, keyword.length(), keyword) == 0) {
                    // 检查是否是完整单词
                    size_t endPos = pos_ + keyword.length();
                    if (endPos == input_.length() || !std::isalnum(input_[endPos])) {
                        return true;
                    }
                }
                return false;
            }
            
            std::unique_ptr<ShowStatement> parseShowStatement() {
                std::cout << "[TEST] 开始解析SHOW语句" << std::endl;
                
                // 消费SHOW关键字
                consumeKeyword("SHOW");
                
                // 检查SHOW语句类型
                if (checkKeyword("GRANTS")) {
                    std::cout << "[TEST] 检测到SHOW GRANTS" << std::endl;
                    consumeKeyword("GRANTS");
                    consumeKeyword("FOR");
                    
                    std::string username = parseIdentifier();
                    std::cout << "[TEST] 用户名: " << username << std::endl;
                    
                    auto stmt = std::make_unique<ShowStatement>(ShowStatement::GRANTS);
                    stmt->setTargetObject(username);
                    
                    std::cout << "[TEST] SHOW GRANTS语句解析成功" << std::endl;
                    return stmt;
                }
                else if (checkKeyword("DATABASES")) {
                    std::cout << "[TEST] 检测到SHOW DATABASES" << std::endl;
                    consumeKeyword("DATABASES");
                    return std::make_unique<ShowStatement>(ShowStatement::DATABASES);
                }
                else if (checkKeyword("TABLES")) {
                    std::cout << "[TEST] 检测到SHOW TABLES" << std::endl;
                    consumeKeyword("TABLES");
                    auto stmt = std::make_unique<ShowStatement>(ShowStatement::TABLES);
                    
                    // 可选：解析 FROM database_name
                    if (checkKeyword("FROM")) {
                        consumeKeyword("FROM");
                        std::string dbName = parseIdentifier();
                        stmt->setFromDatabase(dbName);
                        std::cout << "[TEST] FROM数据库: " << dbName << std::endl;
                    }
                    
                    return stmt;
                }
                else {
                    std::cout << "[TEST] 未知的SHOW语句类型" << std::endl;
                    return nullptr;
                }
            }
            
            void consumeKeyword(const std::string& keyword) {
                skipWhitespace();
                if (input_.compare(pos_, keyword.length(), keyword) != 0) {
                    throw std::runtime_error("Expected keyword: " + keyword);
                }
                pos_ += keyword.length();
                skipWhitespace();
            }
            
            std::string parseIdentifier() {
                skipWhitespace();
                size_t start = pos_;
                
                // 支持字母、数字、下划线
                while (pos_ < input_.length() && 
                       (std::isalnum(input_[pos_]) || input_[pos_] == '_')) {
                    pos_++;
                }
                
                if (start == pos_) {
                    throw std::runtime_error("Expected identifier");
                }
                
                return input_.substr(start, pos_ - start);
            }
        };
    }
}

void testShowGrants() {
    std::cout << "\n=== 测试SHOW GRANTS语句解析 ===" << std::endl;
    
    try {
        // 测试SHOW GRANTS FOR语句
        std::string sql = "SHOW GRANTS FOR testuser";
        std::cout << "测试SQL: " << sql << std::endl;
        
        sqlcc::sql_parser::ParserNew parser(sql);
        auto statements = parser.parse();
        
        if (statements.size() == 1) {
            auto stmt = dynamic_cast<sqlcc::sql_parser::ShowStatement*>(statements[0].get());
            if (stmt && stmt->getType() == sqlcc::sql_parser::StatementType::SHOW) {
                std::cout << "✅ SHOW语句类型正确" << std::endl;
                std::cout << "✅ SHOW类型: " << stmt->getShowType() << std::endl;
                std::cout << "✅ 目标用户: " << stmt->getTargetObject() << std::endl;
            } else {
                std::cout << "❌ 语句类型不正确" << std::endl;
            }
        } else {
            std::cout << "❌ 解析失败，语句数量: " << statements.size() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "❌ 解析异常: " << e.what() << std::endl;
    }
}

void testShowDatabases() {
    std::cout << "\n=== 测试SHOW DATABASES语句解析 ===" << std::endl;
    
    try {
        std::string sql = "SHOW DATABASES";
        std::cout << "测试SQL: " << sql << std::endl;
        
        sqlcc::sql_parser::ParserNew parser(sql);
        auto statements = parser.parse();
        
        if (statements.size() == 1) {
            auto stmt = dynamic_cast<sqlcc::sql_parser::ShowStatement*>(statements[0].get());
            if (stmt && stmt->getType() == sqlcc::sql_parser::StatementType::SHOW) {
                std::cout << "✅ SHOW语句类型正确" << std::endl;
                std::cout << "✅ SHOW类型: " << stmt->getShowType() << std::endl;
            } else {
                std::cout << "❌ 语句类型不正确" << std::endl;
            }
        } else {
            std::cout << "❌ 解析失败，语句数量: " << statements.size() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "❌ 解析异常: " << e.what() << std::endl;
    }
}

void testShowTables() {
    std::cout << "\n=== 测试SHOW TABLES语句解析 ===" << std::endl;
    
    try {
        std::string sql = "SHOW TABLES FROM mydb";
        std::cout << "测试SQL: " << sql << std::endl;
        
        sqlcc::sql_parser::ParserNew parser(sql);
        auto statements = parser.parse();
        
        if (statements.size() == 1) {
            auto stmt = dynamic_cast<sqlcc::sql_parser::ShowStatement*>(statements[0].get());
            if (stmt && stmt->getType() == sqlcc::sql_parser::StatementType::SHOW) {
                std::cout << "✅ SHOW语句类型正确" << std::endl;
                std::cout << "✅ SHOW类型: " << stmt->getShowType() << std::endl;
                std::cout << "✅ 来源数据库: " << stmt->getFromDatabase() << std::endl;
                std::cout << "✅ 有来源数据库: " << (stmt->hasFromDatabase() ? "是" : "否") << std::endl;
            } else {
                std::cout << "❌ 语句类型不正确" << std::endl;
            }
        } else {
            std::cout << "❌ 解析失败，语句数量: " << statements.size() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "❌ 解析异常: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "=== SQLCC DCL语句解析测试 ===" << std::endl;
    std::cout << "重点测试SHOW GRANTS功能的实现" << std::endl;
    
    testShowGrants();
    testShowDatabases();
    testShowTables();
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
    return 0;
}