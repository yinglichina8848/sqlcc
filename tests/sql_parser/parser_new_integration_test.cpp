#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>

/**
 * @brief Independent Parser New Integration Test
 *
 * Demonstrates Parser New functionality using mock classes.
 * This implementation creates independent test for the重构后的SQL Parser.
 */

namespace parser_new_test {

// Mock Token types for testing
enum class TokenType {
    SELECT, INSERT, UPDATE, DELETE,
    CREATE, DROP, ALTER,
    FROM, WHERE, INTO, SET, VALUES,
    TABLE, DATABASE, INDEX,
    IDENTIFIER, NUMBER, STRING,
    LEFT_PAREN, RIGHT_PAREN,
    COMMA, SEMICOLON, EQUALS,
    GREATER_THAN, LESS_THAN,
    AND, OR, NOT,
    PRIMARY_KEY, FOREIGN_KEY,
    VARCHAR, INT, FLOAT, BOOLEAN,
    END_OF_FILE, UNKNOWN
};

struct MockToken {
    TokenType type;
    std::string lexeme;
    size_t line;
    size_t column;
    
    MockToken(TokenType t, const std::string& l, size_t ln = 1, size_t col = 1)
        : type(t), lexeme(l), line(ln), column(col) {}
    
    std::string toString() const {
        std::ostringstream ss;
        ss << "<" << tokenTypeToString() << ":'" << lexeme << "'>";
        return ss.str();
    }
    
private:
    std::string tokenTypeToString() const {
        switch (type) {
            case TokenType::SELECT: return "SELECT";
            case TokenType::INSERT: return "INSERT";
            case TokenType::UPDATE: return "UPDATE";
            case TokenType::DELETE: return "DELETE";
            case TokenType::CREATE: return "CREATE";
            case TokenType::DROP: return "DROP";
            case TokenType::ALTER: return "ALTER";
            case TokenType::FROM: return "FROM";
            case TokenType::WHERE: return "WHERE";
            case TokenType::INTO: return "INTO";
            case TokenType::SET: return "SET";
            case TokenType::VALUES: return "VALUES";
            case TokenType::TABLE: return "TABLE";
            case TokenType::DATABASE: return "DATABASE";
            case TokenType::INDEX: return "INDEX";
            case TokenType::IDENTIFIER: return "IDENTIFIER";
            case TokenType::NUMBER: return "NUMBER";
            case TokenType::STRING: return "STRING";
            case TokenType::LEFT_PAREN: return "(";
            case TokenType::RIGHT_PAREN: return ")";
            case TokenType::COMMA: return ",";
            case TokenType::SEMICOLON: return ";";
            case TokenType::EQUALS: return "=";
            case TokenType::GREATER_THAN: return ">";
            case TokenType::LESS_THAN: return "<";
            case TokenType::AND: return "AND";
            case TokenType::OR: return "OR";
            case TokenType::NOT: return "NOT";
            case TokenType::PRIMARY_KEY: return "PRIMARY_KEY";
            case TokenType::FOREIGN_KEY: return "FOREIGN_KEY";
            case TokenType::VARCHAR: return "VARCHAR";
            case TokenType::INT: return "INT";
            case TokenType::FLOAT: return "FLOAT";
            case TokenType::BOOLEAN: return "BOOLEAN";
            case TokenType::END_OF_FILE: return "EOF";
            default: return "UNKNOWN";
        }
    }
};

// Mock LexerNew for testing
class MockLexerNew {
public:
    explicit MockLexerNew(const std::string& input) : input_(input), position_(0), line_(1), column_(1) {
        tokenize();
    }
    
    std::vector<MockToken> getTokens() const { return tokens_; }
    
private:
    void tokenize() {
        size_t pos = 0;
        while (pos < input_.length()) {
            char c = input_[pos];
            
            // Skip whitespace
            if (std::isspace(c)) {
                if (c == '\n') { line_++; column_ = 1; }
                else { column_++; }
                pos++;
                continue;
            }
            
            // Comments
            if (c == '-' && pos + 1 < input_.length() && input_[pos + 1] == '-') {
                while (pos < input_.length() && input_[pos] != '\n') pos++;
                continue;
            }
            
            // Numbers
            if (std::isdigit(c)) {
                size_t start = pos;
                while (pos < input_.length() && std::isdigit(input_[pos])) pos++;
                if (pos < input_.length() && input_[pos] == '.') {
                    pos++;
                    while (pos < input_.length() && std::isdigit(input_[pos])) pos++;
                }
                std::string number = input_.substr(start, pos - start);
                tokens_.emplace_back(TokenType::NUMBER, number, line_, column_);
                column_ += pos - start;
                continue;
            }
            
            // Strings
            if (c == '\'') {
                size_t start = pos;
                pos++;
                while (pos < input_.length() && input_[pos] != '\'') pos++;
                if (pos < input_.length()) pos++; // Include closing quote
                std::string str = input_.substr(start, pos - start);
                tokens_.emplace_back(TokenType::STRING, str, line_, column_);
                column_ += pos - start;
                continue;
            }
            
            // Identifiers and Keywords
            if (std::isalpha(c) || c == '_') {
                size_t start = pos;
                pos++;
                while (pos < input_.length() && (std::isalnum(input_[pos]) || input_[pos] == '_')) pos++;
                std::string word = input_.substr(start, pos - start);
                
                // Check for keywords
                TokenType type = getKeywordType(word);
                tokens_.emplace_back(type, word, line_, column_);
                column_ += pos - start;
                continue;
            }
            
            // Single character tokens
            std::unordered_map<char, TokenType> singleTokens = {
                {'(', TokenType::LEFT_PAREN},
                {')', TokenType::RIGHT_PAREN},
                {',', TokenType::COMMA},
                {';', TokenType::SEMICOLON},
                {'=', TokenType::EQUALS},
                {'>', TokenType::GREATER_THAN},
                {'<', TokenType::LESS_THAN},
                {'?', TokenType::UNKNOWN} // Placeholder for ? placeholder
            };
            
            if (singleTokens.count(c)) {
                std::string s(1, c);
                tokens_.emplace_back(singleTokens[c], s, line_, column_);
                column_++;
                pos++;
                continue;
            }
            
            // Unknown character - skip
            tokens_.emplace_back(TokenType::UNKNOWN, std::string(1, c), line_, column_);
            column_++;
            pos++;
        }
        
        tokens_.emplace_back(TokenType::END_OF_FILE, "", line_, column_);
    }
    
    TokenType getKeywordType(const std::string& word) const {
        static const std::unordered_map<std::string, TokenType> keywords = {
            {"SELECT", TokenType::SELECT},
            {"INSERT", TokenType::INSERT},
            {"UPDATE", TokenType::UPDATE},
            {"DELETE", TokenType::DELETE},
            {"CREATE", TokenType::CREATE},
            {"DROP", TokenType::DROP},
            {"ALTER", TokenType::ALTER},
            {"FROM", TokenType::FROM},
            {"WHERE", TokenType::WHERE},
            {"INTO", TokenType::INTO},
            {"SET", TokenType::SET},
            {"VALUES", TokenType::VALUES},
            {"TABLE", TokenType::TABLE},
            {"DATABASE", TokenType::DATABASE},
            {"INDEX", TokenType::INDEX},
            {"AND", TokenType::AND},
            {"OR", TokenType::OR},
            {"NOT", TokenType::NOT},
            {"PRIMARY", TokenType::PRIMARY_KEY},
            {"FOREIGN", TokenType::FOREIGN_KEY},
            {"VARCHAR", TokenType::VARCHAR},
            {"INT", TokenType::INT},
            {"FLOAT", TokenType::FLOAT},
            {"BOOLEAN", TokenType::BOOLEAN}
        };
        
        auto it = keywords.find(word);
        return (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;
    }
    
    std::string input_;
    size_t position_;
    size_t line_;
    size_t column_;
    std::vector<MockToken> tokens_;
};

// Mock AST Node types
enum class ASTNodeType {
    SELECT_STATEMENT,
    INSERT_STATEMENT,
    UPDATE_STATEMENT,
    DELETE_STATEMENT,
    CREATE_STATEMENT,
    DROP_STATEMENT,
    ALTER_STATEMENT,
    EXPRESSION,
    COLUMN_DEFINITION,
    TABLE_REFERENCE
};

class MockASTNode {
public:
    explicit MockASTNode(ASTNodeType type, const std::string& value = "") 
        : type_(type), value_(value) {}
    
    virtual ~MockASTNode() = default;
    
    ASTNodeType getType() const { return type_; }
    const std::string& getValue() const { return value_; }
    const std::vector<std::unique_ptr<MockASTNode>>& getChildren() const { return children_; }
    
    void addChild(std::unique_ptr<MockASTNode> child) {
        children_.push_back(std::move(child));
    }
    
    std::string toString() const {
        std::ostringstream ss;
        ss << astNodeTypeToString() << (value_.empty() ? "" : "('" + value_ + "')");
        if (!children_.empty()) {
            ss << " [";
            for (size_t i = 0; i < children_.size(); ++i) {
                if (i > 0) ss << ", ";
                ss << children_[i]->toString();
            }
            ss << "]";
        }
        return ss.str();
    }
    
private:
    std::string astNodeTypeToString() const {
        switch (type_) {
            case ASTNodeType::SELECT_STATEMENT: return "SELECT";
            case ASTNodeType::INSERT_STATEMENT: return "INSERT";
            case ASTNodeType::UPDATE_STATEMENT: return "UPDATE";
            case ASTNodeType::DELETE_STATEMENT: return "DELETE";
            case ASTNodeType::CREATE_STATEMENT: return "CREATE";
            case ASTNodeType::DROP_STATEMENT: return "DROP";
            case ASTNodeType::ALTER_STATEMENT: return "ALTER";
            case ASTNodeType::EXPRESSION: return "EXPR";
            case ASTNodeType::COLUMN_DEFINITION: return "COLUMN";
            case ASTNodeType::TABLE_REFERENCE: return "TABLE";
            default: return "UNKNOWN";
        }
    }
    
    ASTNodeType type_;
    std::string value_;
    std::vector<std::unique_ptr<MockASTNode>> children_;
};

// Mock ParserNew for testing
class MockParserNew {
public:
    explicit MockParserNew(const std::string& input) : input_(input), lexer_(input) {}
    
    std::vector<std::unique_ptr<MockASTNode>> parse() {
        std::vector<std::unique_ptr<MockASTNode>> statements;
        const auto& tokens = lexer_.getTokens();
        size_t pos = 0;
        
        while (pos < tokens.size() && tokens[pos].type != TokenType::END_OF_FILE) {
            auto stmt = parseStatement(tokens, pos);
            if (stmt) {
                statements.push_back(std::move(stmt));
            }
            // Skip to next statement
            while (pos < tokens.size() && tokens[pos].type != TokenType::SEMICOLON && 
                   tokens[pos].type != TokenType::END_OF_FILE) {
                pos++;
            }
            if (pos < tokens.size() && tokens[pos].type == TokenType::SEMICOLON) {
                pos++; // Skip semicolon
            }
        }
        
        return statements;
    }
    
private:
    std::unique_ptr<MockASTNode> parseStatement(const std::vector<MockToken>& tokens, size_t& pos) {
        if (pos >= tokens.size()) return nullptr;
        
        TokenType type = tokens[pos].type;
        std::string keyword = tokens[pos].lexeme;
        pos++;
        
        switch (type) {
            case TokenType::SELECT:
                return parseSelectStatement(tokens, pos);
            case TokenType::INSERT:
                return parseInsertStatement(tokens, pos);
            case TokenType::UPDATE:
                return parseUpdateStatement(tokens, pos);
            case TokenType::DELETE:
                return parseDeleteStatement(tokens, pos);
            case TokenType::CREATE:
                return parseCreateStatement(tokens, pos);
            case TokenType::DROP:
                return parseDropStatement(tokens, pos);
            default:
                return std::make_unique<MockASTNode>(ASTNodeType::EXPRESSION, keyword);
        }
    }
    
    std::unique_ptr<MockASTNode> parseSelectStatement(const std::vector<MockToken>& tokens, size_t& pos) {
        auto node = std::make_unique<MockASTNode>(ASTNodeType::SELECT_STATEMENT, "SELECT");
        
        // Parse select list
        if (pos < tokens.size() && tokens[pos].type == TokenType::ASTERISK) {
            node->addChild(std::make_unique<MockASTNode>(ASTNodeType::EXPRESSION, "*"));
            pos++;
        } else {
            // Parse column list
            auto columns = parseColumnList(tokens, pos);
            node->addChild(std::move(columns));
        }
        
        // Parse FROM clause
        if (pos < tokens.size() && tokens[pos].type == TokenType::FROM) {
            pos++;
            if (pos < tokens.size()) {
                node->addChild(std::make_unique<MockASTNode>(ASTNodeType::TABLE_REFERENCE, tokens[pos].lexeme));
                pos++;
            }
        }
        
        // Parse WHERE clause
        if (pos < tokens.size() && tokens[pos].type == TokenType::WHERE) {
            pos++;
            auto where = std::make_unique<MockASTNode>(ASTNodeType::EXPRESSION, "WHERE");
            if (pos < tokens.size()) {
                where->addChild(std::make_unique<MockASTNode>(ASTNodeType::EXPRESSION, tokens[pos].lexeme));
                pos++;
            }
            node->addChild(std::move(where));
        }
        
        return node;
    }
    
    std::unique_ptr<MockASTNode> parseInsertStatement(const std::vector<MockToken>& tokens, size_t& pos) {
        auto node = std::make_unique<MockASTNode>(ASTNodeType::INSERT_STATEMENT, "INSERT");
        
        // INTO keyword
        if (pos < tokens.size() && tokens[pos].type == TokenType::INTO) {
            pos++;
        }
        
        // Table name
        if (pos < tokens.size()) {
            node->addChild(std::make_unique<MockASTNode>(ASTNodeType::TABLE_REFERENCE, tokens[pos].lexeme));
            pos++;
        }
        
        // Column list
        if (pos < tokens.size() && tokens[pos].type == TokenType::LEFT_PAREN) {
            pos++;
            auto columns = parseColumnList(tokens, pos);
            node->addChild(std::move(columns));
        }
        
        // VALUES keyword
        if (pos < tokens.size() && tokens[pos].type == TokenType::VALUES) {
            pos++;
        }
        
        // Values
        if (pos < tokens.size() && tokens[pos].type == TokenType::LEFT_PAREN) {
            pos++;
            auto values = parseValueList(tokens, pos);
            node->addChild(std::move(values));
        }
        
        return node;
    }
    
    std::unique_ptr<MockASTNode> parseUpdateStatement(const std::vector<MockToken>& tokens, size_t& pos) {
        auto node = std::make_unique<MockASTNode>(ASTNodeType::UPDATE_STATEMENT, "UPDATE");
        
        // Table name
        if (pos < tokens.size()) {
            node->addChild(std::make_unique<MockASTNode>(ASTNodeType::TABLE_REFERENCE, tokens[pos].lexeme));
            pos++;
        }
        
        // SET keyword
        if (pos < tokens.size() && tokens[pos].type == TokenType::SET) {
            pos++;
        }
        
        // Column assignments
        if (pos < tokens.size()) {
            node->addChild(std::make_unique<MockASTNode>(ASTNodeType::EXPRESSION, "SET_CLAUSE"));
            pos++;
        }
        
        return node;
    }
    
    std::unique_ptr<MockASTNode> parseDeleteStatement(const std::vector<MockToken>& tokens, size_t& pos) {
        auto node = std::make_unique<MockASTNode>(ASTNodeType::DELETE_STATEMENT, "DELETE");
        
        // FROM keyword
        if (pos < tokens.size() && tokens[pos].type == TokenType::FROM) {
            pos++;
        }
        
        // Table name
        if (pos < tokens.size()) {
            node->addChild(std::make_unique<MockASTNode>(ASTNodeType::TABLE_REFERENCE, tokens[pos].lexeme));
            pos++;
        }
        
        return node;
    }
    
    std::unique_ptr<MockASTNode> parseCreateStatement(const std::vector<MockToken>& tokens, size_t& pos) {
        auto node = std::make_unique<MockASTNode>(ASTNodeType::CREATE_STATEMENT, "CREATE");
        
        // TABLE or DATABASE keyword
        if (pos < tokens.size()) {
            std::string objectType = tokens[pos].lexeme;
            node->addChild(std::make_unique<MockASTNode>(ASTNodeType::EXPRESSION, objectType));
            pos++;
            
            // Object name
            if (pos < tokens.size()) {
                node->addChild(std::make_unique<MockASTNode>(ASTNodeType::TABLE_REFERENCE, tokens[pos].lexeme));
                pos++;
            }
        }
        
        return node;
    }
    
    std::unique_ptr<MockASTNode> parseDropStatement(const std::vector<MockToken>& tokens, size_t& pos) {
        auto node = std::make_unique<MockASTNode
