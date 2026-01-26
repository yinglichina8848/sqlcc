# SQL Parser 函数设计文档

## 函数列表

### operationType_

**定义位置**: `src/sql_parser/set_operation.cpp`

**签名**:
```cpp
            operationType_(operationType),
      leftOperand_(std::move(leftOperand)),
      rightOp...
```

---

### hasLookahead_

**定义位置**: `src/sql_parser/parser.cpp`

**签名**:
```cpp
      hasLookahead_(false),    // HOW: 前瞻机制初始化，避免无效状态
      panicMode_(false) {
```

---

### parseStatement

**定义位置**: `src/sql_parser/parser.cpp`

**签名**:
```cpp
 *   parseStatement()是SQL解析器的核心分派函数，负责识别SQL语句类型
 *   并调用对应的专用解析函数。采用策略模式，将不同语句的解析逻辑
 *   分离到专门的方法中，提...
```

---

### match

**定义位置**: `src/sql_parser/parser.cpp`

**签名**:
```cpp
 *   match()函数是递归下降语法分析器的核心操作之一，实现"匹配-消费"的原子操作。
 *   它是语法规则实现的基础，通过检查当前token类型并在匹配时消费token，
 *   确保语...
```

---

### parseCreateTableStatement

**定义位置**: `src/sql_parser/parser.cpp`

**签名**:
```cpp
 *   parseCreateTableStatement()是DDL解析器的核心函数，负责解析
 *   SQL CREATE TABLE语句的完整语法。CREATE TABLE是数据库中最基础
...
```

---

### check

**定义位置**: `src/sql_parser/parser.cpp`

**签名**:
```cpp
                  check(Type::KEYWORD_UNION) || check(Type::SEMICOLON))) {
```

---

### parseSelectStatement

**定义位置**: `src/sql_parser/parser.cpp`

**签名**:
```cpp
 *   parseSelectStatement()是SQL解析器中最复杂的函数之一，负责解析
 *   SQL SELECT语句的所有组成部分。SELECT语句是SQL中最常用的语句类型，
 * ...
```

---

### parseInsertStatement

**定义位置**: `src/sql_parser/parser.cpp`

**签名**:
```cpp
 *   parseInsertStatement()负责解析SQL INSERT语句，这是关系数据库中最
 *   基础的DML操作之一。INSERT语句用于向表中添加新的数据行，支持单行
 *  ...
```

---

### isolationLevel_

**定义位置**: `src/sql_parser/advanced_sql92_features.cpp`

**签名**:
```cpp
      isolationLevel_(READ_COMMITTED),
      accessMode_(READ_WRITE),
      work_(false),
      hasI...
```

---

### baseType_

**定义位置**: `src/sql_parser/advanced_sql92_features.cpp`

**签名**:
```cpp
      baseType_(baseType),
      characterLength_(0),
      precision_(0),
      scale_(0),
      no...
```

---

### domainName_

**定义位置**: `src/sql_parser/advanced_sql92_features.cpp`

**签名**:
```cpp
      domainName_(domainName),
      dropBehavior_(RESTRICT),
      ifExists_(false) {}

DropDomainS...
```

---

### hasOldTableName_

**定义位置**: `src/sql_parser/advanced_sql92_features.cpp`

**签名**:
```cpp
      hasOldTableName_(false),
      hasNewTableName_(false),
      hasWhenCondition_(false) {}

Enh...
```

---

### columnName_

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.cpp`

**签名**:
```cpp
      columnName_(columnName) {
  if (!columnName.empty()) {
```

---

### decimal_from_string

**定义位置**: `src/sql_parser/decimal.cpp`

**签名**:
```cpp

Decimal decimal_from_string(const std::string& str) {
```

---

### abs

**定义位置**: `src/sql_parser/decimal.cpp`

**签名**:
```cpp

Decimal abs(const Decimal& value) {
```

---

### ceil

**定义位置**: `src/sql_parser/decimal.cpp`

**签名**:
```cpp

Decimal ceil(const Decimal& value) {
```

---

### floor

**定义位置**: `src/sql_parser/decimal.cpp`

**签名**:
```cpp

Decimal floor(const Decimal& value) {
```

---

### round

**定义位置**: `src/sql_parser/decimal.cpp`

**签名**:
```cpp

Decimal round(const Decimal& value, int32_t decimals) {
```

---

### truncate

**定义位置**: `src/sql_parser/decimal.cpp`

**签名**:
```cpp

Decimal truncate(const Decimal& value, int32_t decimals) {
```

---

### power

**定义位置**: `src/sql_parser/decimal.cpp`

**签名**:
```cpp

Decimal power(const Decimal& base, int32_t exponent) {
  if (exponent < 0) {
```

---

### sqrt

**定义位置**: `src/sql_parser/decimal.cpp`

**签名**:
```cpp

Decimal sqrt(const Decimal& value) {
  if (value.is_negative()) {
```

---

### referenced_columns_

**定义位置**: `src/sql_parser/constraint_validators.cpp`

**签名**:
```cpp
      referenced_columns_(referenced_columns), constraint_name_(constraint_name),
      on_delete_ac...
```

---

### minutes_

**定义位置**: `src/sql_parser/datetime.cpp`

**签名**:
```cpp
      minutes_(other.minutes_), seconds_(other.seconds_), milliseconds_(other.milliseconds_) {}

Int...
```

---

### date_from_string

**定义位置**: `src/sql_parser/datetime.cpp`

**签名**:
```cpp

DateTime date_from_string(const std::string& str) {
```

---

### interval_from_string

**定义位置**: `src/sql_parser/datetime.cpp`

**签名**:
```cpp

Interval interval_from_string(const std::string& str) {
```

---

### JsonBoolean

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp
  explicit JsonBoolean(bool value) : value_(value) {
```

---

### JsonNumber

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp
  explicit JsonNumber(double value) : value_(value) {
```

---

### JsonString

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp
  explicit JsonString(const std::string& value) : value_(value) {
```

---

### json_null

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp

Json json_null() {
```

---

### json_bool

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp
Json json_bool(bool value) {
```

---

### json_int

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp
Json json_int(int value) {
```

---

### json_int64

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp
Json json_int64(int64_t value) {
```

---

### json_double

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp
Json json_double(double value) {
```

---

### json_string

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp
Json json_string(const std::string& value) {
```

---

### json_array

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp
Json json_array() {
```

---

### json_object

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp
Json json_object() {
```

---

### json_from_string

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp
Json json_from_string(const std::string& str) {
```

---

### parse_value

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp

Json parse_value(const std::string& str, size_t& pos) {
```

---

### skip_whitespace

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp

void skip_whitespace(const std::string& str, size_t& pos) {
  while (pos < str.length() && std::iss...
```

---

### parse_object

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp

Json parse_object(const std::string& str, size_t& pos) {
  if (str[pos] != '{') {
```

---

### parse_array

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp

Json parse_array(const std::string& str, size_t& pos) {
  if (str[pos] != '[') {
```

---

### parse_bool

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp

bool parse_bool(const std::string& str, size_t& pos) {
  if (pos + 4 <= str.length() && str.substr(...
```

---

### parse_null

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp

void parse_null(const std::string& str, size_t& pos) {
  if (pos + 4 <= str.length() && str.substr(...
```

---

### parse_number

**定义位置**: `src/sql_parser/json.cpp`

**签名**:
```cpp

double parse_number(const std::string& str, size_t& pos) {
```

---

### referenced_table_

**定义位置**: `src/sql_parser/constraint.cpp`

**签名**:
```cpp
      referenced_table_(referenced_table),
      referenced_columns_(referenced_columns),
      name...
```

---

### isPrimaryKey_

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.cpp`

**签名**:
```cpp
      isPrimaryKey_(false), isNullable_(true), isUnique_(false),
      isAutoIncrement_(false), prec...
```

---

### selectAll_

**定义位置**: `src/sql_parser/ast/dml/ast_dml_nodes.cpp`

**签名**:
```cpp
      selectAll_(false), distinct_(false), hasLimit_(false), hasOffset_(false) {}

SelectStatement::...
```

---

### ifExists_

**定义位置**: `src/sql_parser/ast_nodes.cpp`

**签名**:
```cpp
      ifExists_(false) {}

DropStatement::~DropStatement() {}

DropStatement::DropStatement(ObjectTy...
```

---

### columnDef_

**定义位置**: `src/sql_parser/ast_nodes.cpp`

**签名**:
```cpp
      columnDef_("temp", "temp") {}  // 提供默认的ColumnDefinition构造参数

AlterStatement::~AlterStatement()...
```

---

### unique_

**定义位置**: `src/sql_parser/ast_nodes.cpp`

**签名**:
```cpp
      unique_(false) {
```

---

### hasTableName_

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.cpp`

**签名**:
```cpp
      hasTableName_(false) {}

DropIndexStatement::~DropIndexStatement() {}

void DropIndexStatement...
```

---

### withPassword_

**定义位置**: `src/sql_parser/ast_nodes.cpp`

**签名**:
```cpp
      withPassword_(false) {}

CreateUserStatement::~CreateUserStatement() {}

void CreateUserStatem...
```

---

### tableName_

**定义位置**: `src/sql_parser/ast_nodes.cpp`

**签名**:
```cpp
      tableName_(tableName), hasCondition_(false) {}

TriggerDefinition::~TriggerDefinition() {
```

---

### cte_name_

**定义位置**: `src/sql_parser/recursive_query.cpp`

**签名**:
```cpp
      cte_name_(std::move(cte_name)),
      base_query_(std::move(base_query)),
      recursive_quer...
```

---

### isIdentifierStart

**定义位置**: `src/sql_parser/lexer.cpp`

**签名**:
```cpp
bool isIdentifierStart(char c) {
```

---

### isIdentifierPart

**定义位置**: `src/sql_parser/lexer.cpp`

**签名**:
```cpp

bool isIdentifierPart(char c) {
```

---

### isDigit

**定义位置**: `src/sql_parser/lexer.cpp`

**签名**:
```cpp

bool isDigit(char c) {
```

---

### isWhitespace

**定义位置**: `src/sql_parser/lexer.cpp`

**签名**:
```cpp

bool isWhitespace(char c) {
```

---

### current_state_

**定义位置**: `src/sql_parser/lexer.cpp`

**签名**:
```cpp
      current_state_(LexerState::START) {
```

---

### frameEnd_

**定义位置**: `src/sql_parser/window_function.cpp`

**签名**:
```cpp
      frameEnd_(FrameBoundary::CURRENT_ROW) {
}

void WindowSpecification::setPartitionBy(std::vecto...
```

---

### defaultPrecision

**定义位置**: `src/sql_parser/data_types.cpp`

**签名**:
```cpp
      defaultPrecision(precision), defaultScale(scale) {}

// ==================== DataTypeManager 实...
```

---

### parseBinaryOp

**定义位置**: `src/sql_parser/expression_parser.cpp`

**签名**:
```cpp

  return parseBinaryOp([this]() {
```

---

### tokenToOperatorKind

**定义位置**: `src/sql_parser/operator_mapper.cpp`

**签名**:
```cpp

OperatorKind tokenToOperatorKind(Token::Type type) {
  switch (type) {
```

---

### getOperatorPrecedence

**定义位置**: `src/sql_parser/operator_mapper.cpp`

**签名**:
```cpp

int getOperatorPrecedence(OperatorKind op) {
  switch (op) {
```

---

### main

**定义位置**: `src/sql_parser/ast/binary_expression_test.cpp`

**签名**:
```cpp

int main() {
```

---

### objectName_

**定义位置**: `src/sql_parser/ast/utilities/ast_utility_nodes.cpp`

**签名**:
```cpp
      objectName_(objectName), userName_(userName) {}

RevokeStatement::~RevokeStatement() {}

void ...
```

---

### right_

**定义位置**: `src/sql_parser/ast/expressions/ast_expression_nodes.cpp`

**签名**:
```cpp
      right_(std::move(right)) {}

BinaryExpression::~BinaryExpression() {}

void BinaryExpression::...
```

---

### setPrecision

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void setPrecision(int precision) {
```

---

### setScale

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void setScale(int scale) {
```

---

### setFormat

**定义位置**: `src/sql_parser/data_types.h`

**签名**:
```cpp
    void setFormat(Format format) {
```

---

### setNull

**定义位置**: `src/sql_parser/data_types.h`

**签名**:
```cpp
    void setNull(bool null = true) {
```

---

### validateArchitectureConstraints

**定义位置**: `src/sql_parser/architecture_safeguards.h`

**签名**:
```cpp
inline void validateArchitectureConstraints() {
```

---

### constexpr

**定义位置**: `src/sql_parser/architecture_safeguards.h`

**签名**:
```cpp

    if constexpr (sizeof(ExpressionParser) == 0) {
```

---

### reportArchitectureViolation

**定义位置**: `src/sql_parser/architecture_safeguards.h`

**签名**:
```cpp
inline void reportArchitectureViolation(const std::string& location,
                               ...
```

---

### FunctionParameter

**定义位置**: `src/sql_parser/function_ast.h`

**签名**:
```cpp

        FunctionParameter(const std::string& n, const std::string& t,
                         Para...
```

---

### Statement

**定义位置**: `src/sql_parser/ast/statement.h`

**签名**:
```cpp

    explicit Statement(Type type) : type_(type) {
```

---

### visit

**定义位置**: `src/sql_parser/ast/node_visitor.h`

**签名**:
```cpp
  virtual void visit(Expression&) {}
  virtual void visit(NumericLiteralExpression&) {}
  virtual vo...
```

---

### DebugPrintVisitor

**定义位置**: `src/sql_parser/ast/debug_printer.h`

**签名**:
```cpp
  explicit DebugPrintVisitor(std::ostream& os = std::cout)
      : os_(os) {
```

---

### indent

**定义位置**: `src/sql_parser/ast/debug_printer.h`

**签名**:
```cpp
  void indent() {
```

---

### NumericLiteralExpression

**定义位置**: `src/sql_parser/ast/expression.h`

**签名**:
```cpp
  explicit NumericLiteralExpression(double value) : value_(value) {
```

---

### StringLiteralExpression

**定义位置**: `src/sql_parser/ast/expression.h`

**签名**:
```cpp
  explicit StringLiteralExpression(const std::string& value) : value_(value) {
```

---

### BooleanLiteralExpression

**定义位置**: `src/sql_parser/ast/expression.h`

**签名**:
```cpp
  explicit BooleanLiteralExpression(bool value) : value_(value) {
```

---

### IdentifierExpression

**定义位置**: `src/sql_parser/ast/expression.h`

**签名**:
```cpp
  explicit IdentifierExpression(const std::string& name) : name_(name) {
```

---

### FunctionCallExpression

**定义位置**: `src/sql_parser/ast/expression.h`

**签名**:
```cpp
  FunctionCallExpression(const std::string& name, std::vector<ExprPtr> arguments)
      : name_(name...
```

---

### BinaryExpression

**定义位置**: `src/sql_parser/ast/expression.h`

**签名**:
```cpp
  BinaryExpression(ExprPtr left, ExprPtr right, OperatorKind op)
      : left_(std::move(left)), rig...
```

---

### setName

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void setName(const std::string &name) {
```

---

### setPrimaryKey

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void setPrimaryKey(bool primaryKey = true) {
```

---

### setNullable

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void setNullable(bool nullable = true) {
```

---

### setUnique

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp

    void setUnique(bool unique) {
```

---

### setForeignKey

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void setForeignKey(bool foreignKey = true) {
```

---

### setAutoIncrement

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void setAutoIncrement(bool autoIncrement = true) {
```

---

### setIsPrimaryKey

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**签名**:
```cpp
  void setIsPrimaryKey(bool isPrimaryKey) {
```

---

### setIsNullable

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**签名**:
```cpp
  void setIsNullable(bool isNullable) {
```

---

### setIsUnique

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**签名**:
```cpp
  void setIsUnique(bool isUnique) {
```

---

### setIsAutoIncrement

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**签名**:
```cpp
  void setIsAutoIncrement(bool isAutoIncrement) {
```

---

### setObjectName

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void setObjectName(const std::string &name) {
```

---

### setDatabaseName

**定义位置**: `src/sql_parser/ast/utilities/ast_utility_nodes.h`

**签名**:
```cpp
    void setDatabaseName(const std::string &name) {
```

---

### setTableName

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void setTableName(const std::string &tableName) {
```

---

### CompositeSelectStatement

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**签名**:
```cpp
  CompositeSelectStatement() : Statement(Statement::COMPOSITE_SELECT) {
```

---

### addSelectStatement

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**签名**:
```cpp

  void addSelectStatement(std::unique_ptr<SelectStatement> stmt) {
```

---

### addSetOperation

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**签名**:
```cpp

  void addSetOperation(std::unique_ptr<SetOperation> op) {
```

---

### LoadDataStatement

**定义位置**: `src/sql_parser/ast/load_data_ast.h`

**签名**:
```cpp
    LoadDataStatement() : Statement(Type::LOAD_DATA) {
```

---

### setIfExists

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp

    void setIfExists(bool ifExists) {
```

---

### setDropBehavior

**定义位置**: `src/sql_parser/function/function_ddl.h`

**签名**:
```cpp
    void setDropBehavior(DropBehavior behavior) {
```

---

### setAction

**定义位置**: `src/sql_parser/function/function_ddl.h`

**签名**:
```cpp
    void setAction(Action action) {
```

---

### setNewName

**定义位置**: `src/sql_parser/function/function_ddl.h`

**签名**:
```cpp
    void setNewName(const std::string& new_name) {
```

---

### setNewSchema

**定义位置**: `src/sql_parser/function/function_ddl.h`

**签名**:
```cpp
    void setNewSchema(const std::string& new_schema) {
```

---

### setPrivileges

**定义位置**: `src/sql_parser/ast/utilities/ast_utility_nodes.h`

**签名**:
```cpp
    void setPrivileges(const std::vector<std::string> &privileges) {
```

---

### setObjectType

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void setObjectType(ObjectType type) {
```

---

### setUsers

**定义位置**: `src/sql_parser/ast/utilities/ast_utility_nodes.h`

**签名**:
```cpp
    void setUsers(const std::vector<std::string> &users) {
```

---

### setTransactionName

**定义位置**: `src/sql_parser/ast/utilities/ast_utility_nodes.h`

**签名**:
```cpp
    void setTransactionName(const std::string &name) {
```

---

### setIsolationLevel

**定义位置**: `src/sql_parser/ast/utilities/ast_utility_nodes.h`

**签名**:
```cpp
    void setIsolationLevel(const std::string &level) {
```

---

### setShowType

**定义位置**: `src/sql_parser/ast/utilities/ast_utility_nodes.h`

**签名**:
```cpp
    void setShowType(const std::string &type) {
```

---

### setFileName

**定义位置**: `src/sql_parser/ast/utilities/ast_utility_nodes.h`

**签名**:
```cpp
    void setFileName(const std::string &name) {
```

---

### setColumnNames

**定义位置**: `src/sql_parser/ast/dml/ast_dml_nodes.h`

**签名**:
```cpp
    void setColumnNames(const std::vector<std::string> &columns) {
```

---

### setFromTables

**定义位置**: `src/sql_parser/ast/dml/ast_dml_nodes.h`

**签名**:
```cpp
    void setFromTables(const std::vector<std::string> &tables) {
```

---

### setDistinct

**定义位置**: `src/sql_parser/ast/dml/ast_dml_nodes.h`

**签名**:
```cpp
    void setDistinct(bool distinct) {
```

---

### setTableNames

**定义位置**: `src/sql_parser/ast/dml/ast_dml_nodes.h`

**签名**:
```cpp
    void setTableNames(const std::vector<std::string> &tables) {
```

---

### setType

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void setType(Type type) {
```

---

### setOperator

**定义位置**: `src/sql_parser/ast/expressions/ast_expression_nodes.h`

**签名**:
```cpp
    void setOperator(OperatorKind op) {
```

---

### setValue

**定义位置**: `src/sql_parser/ast/expressions/ast_expression_nodes.h`

**签名**:
```cpp
    void setValue(const std::string &value) {
```

---

### setFunctionName

**定义位置**: `src/sql_parser/ast/expressions/ast_expression_nodes.h`

**签名**:
```cpp
    void setFunctionName(const std::string &name) {
```

---

### setConstraintName

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void setConstraintName(const std::string &name) {
```

---

### addColumn

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void addColumn(const std::string &column) {
```

---

### setReferencedTable

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void setReferencedTable(const std::string &table) {
```

---

### addReferencedColumn

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void addReferencedColumn(const std::string &column) {
```

---

### setCheckExpression

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void setCheckExpression(const std::string &expression) {
```

---

### addConstraint

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void addConstraint(std::unique_ptr<TableConstraint> constraint) {
```

---

### setAlterType

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void setAlterType(const std::string &type) {
```

---

### setColumnDefinition

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void setColumnDefinition(std::unique_ptr<ColumnDefinition> column) {
```

---

### setTableConstraint

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**签名**:
```cpp
    void setTableConstraint(std::unique_ptr<TableConstraint> constraint) {
```

---

