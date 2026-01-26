# SQL Parser 类设计文档

## 类列表

### JsonValue

**定义位置**: `src/sql_parser/json/json_value.h`

**定义**:
```cpp

class JsonValue {
public:
    enum Type {
        NULL_VALUE,
        BOOLEAN,
        NUMBER,
        STRING,
        ARRAY,
        OBJECT
    };

    explicit JsonValue(Type type);

private:
    T...
```

**构造函数**:
- `JsonValue`

**公有方法**:
- `explicit JsonValue`

---

### JsonNull

**定义位置**: `src/sql_parser/json.cpp`

**定义**:
```cpp
class JsonNull : public JsonValue {
public:
  Json::Type type() const override { return Json::NULL_VALUE; }
  std::string to_string() const override { return "null"; }
  std::string to_string_formatte...
```

**构造函数**:
- `clone`

---

### JsonBoolean

**定义位置**: `src/sql_parser/json.cpp`

**定义**:
```cpp
class JsonBoolean : public JsonValue {
public:
  explicit JsonBoolean(bool value) : value_(value) {}
  Json::Type type() const override { return Json::BOOLEAN; }
  std::string to_string() const overri...
```

**构造函数**:
- `clone`

---

### JsonNumber

**定义位置**: `src/sql_parser/json.cpp`

**定义**:
```cpp
class JsonNumber : public JsonValue {
public:
  explicit JsonNumber(double value) : value_(value) {}
  Json::Type type() const override { return Json::NUMBER; }
  std::string to_string() const overrid...
```

**构造函数**:
- `str`
- `to_string_formatted`
- `clone`

**公有方法**:
- `string to_string_formatted`

---

### JsonString

**定义位置**: `src/sql_parser/json.cpp`

**定义**:
```cpp
class JsonString : public JsonValue {
public:
  explicit JsonString(const std::string& value) : value_(value) {}
  Json::Type type() const override { return Json::STRING; }
  std::string to_string() c...
```

**构造函数**:
- `to_string_formatted`
- `clone`

**公有方法**:
- `string to_string_formatted`

---

### Lexer

**定义位置**: `src/sql_parser/lexer.h`

**定义**:
```cpp
class Lexer {
public:
  /**
   * @brief 构造函数
   *
   * @param input 要分析的SQL字符串
   */
  explicit Lexer(const std::string &input);

  /**
   * @brief 获取下一个Token
   *
   * 从当前位置开始分析，生成并返回下一个Token。
   * 如...
```

**构造函数**:
- `Lexer`
- `nextToken`
- `advance`
- `setupTransitionTable`
- `createToken`
- `createIdentifierToken`
- `createKeywordToken`
- `createNumberToken`
- `createStringToken`
- `createOperatorToken`
- `createPunctuationToken`
- `handleLineComment`
- `handleBlockComment`
- `reportError`

**公有方法**:
- `explicit Lexer`
- `Token nextToken`
- `char advance`
- `void setupTransitionTable`
- `Token createToken`
- `Token createIdentifierToken`
- `Token createKeywordToken`
- `Token createNumberToken`
- `Token createStringToken`
- `Token createOperatorToken`
- `Token createPunctuationToken`
- `void handleLineComment`
- `void handleBlockComment`
- `void reportError`

---

### Json

**定义位置**: `src/sql_parser/json.h`

**定义**:
```cpp
class Json {
public:
  // JSON类型枚举
  enum Type {
    NULL_VALUE,
    BOOLEAN,
    NUMBER,
    STRING,
    ARRAY,
    OBJECT
  };

  // 构造函数
  Json();
  Json(std::nullptr_t);  // null值
  Json(bool valu...
```

**构造函数**:
- `Json`
- `Json`
- `Json`
- `Json`
- `Json`
- `Json`
- `Json`
- `Json`
- `Json`
- `Json`
- `set_null`
- `set_bool`
- `set_int`
- `set_int64`
- `set_double`
- `set_string`
- `clear`
- `push_back`
- `pop_back`
- `back`
- `erase`
- `parse`
- `from_file`
- `merge`
- `ensure_array`
- `ensure_object`

**析构函数**:
- `Json`

**公有方法**:
- `构造函数
  Json`
- `null值
  Json`
- `布尔值
  Json`
- `整数
  Json`
- `长整数
  Json`
- `浮点数
  Json`
- `字符串
  Json`
- `C字符串
  Json`
- `值设置
  void set_null`
- `void set_bool`
- `void set_int`
- `void set_int64`
- `void set_double`
- `void set_string`
- `数组大小或对象键值对数量
  void clear`
- `void push_back`
- `添加数组元素
  void pop_back`
- `检查键是否存在
  void erase`
- `static Json parse`
- `static Json from_file`
- `对象初始化辅助方法
  void ensure_array`
- `void ensure_object`

---

### JsonPath

**定义位置**: `src/sql_parser/json.h`

**定义**:
```cpp
class JsonPath {
public:
  JsonPath(const std::string& path);
  ~JsonPath();

  // 查询执行
  Json query(const Json& json) const;
  std::vector<Json> query_all(const Json& json) const;

  // 路径验证
  bool i...
```

**构造函数**:
- `JsonPath`
- `JsonPath`

**析构函数**:
- `JsonPath`

---

### JsonSchema

**定义位置**: `src/sql_parser/json.h`

**定义**:
```cpp
class JsonSchema {
public:
  JsonSchema(const Json& schema);
  ~JsonSchema();

  // 验证
  bool validate(const Json& json) const;
  std::string validation_error() const;

  // 模式检查
  bool is_valid_schem...
```

**构造函数**:
- `JsonSchema`
- `JsonSchema`

**析构函数**:
- `JsonSchema`

---

### JsonBuilder

**定义位置**: `src/sql_parser/json.h`

**定义**:
```cpp
class JsonBuilder {
public:
  JsonBuilder();
  ~JsonBuilder();

  // 开始构建对象
  JsonBuilder& object();
  JsonBuilder& end_object();

  // 开始构建数组
  JsonBuilder& array();
  JsonBuilder& end_array();

  //...
```

**构造函数**:
- `JsonBuilder`
- `JsonBuilder`
- `object`
- `end_object`
- `array`
- `end_array`
- `key`
- `value`
- `add_null`
- `add_bool`
- `add_int`
- `add_int64`
- `add_double`
- `add_string`
- `build`

**析构函数**:
- `JsonBuilder`

**公有方法**:
- `获取结果
  Json build`

---

### ForeignKeyConstraint

**定义位置**: `src/sql_parser/constraint/foreign_key_constraint.h`

**定义**:
```cpp
class ForeignKeyConstraint {
public:
    /**
     * @brief 级联操作类型
     */
    enum CascadeAction {
        RESTRICT,    ///< 限制删除/更新
        CASCADE,     ///< 级联删除/更新
        SET_NULL,    ///< 设置为空
  ...
```

**构造函数**:
- `ForeignKeyConstraint`

---

### CheckConstraint

**定义位置**: `src/sql_parser/constraint/check_constraint.h`

**定义**:
```cpp
class CheckConstraint {
public:
    /**
     * @brief 构造函数
     * @param condition 检查条件表达式
     * @param name 约束名称（可选）
     */
    CheckConstraint(std::unique_ptr<Expression> condition,
              ...
```

**构造函数**:
- `CheckConstraint`

---

### PrimaryKeyConstraint

**定义位置**: `src/sql_parser/constraint/primary_key_constraint.h`

**定义**:
```cpp
class PrimaryKeyConstraint {
public:
    /**
     * @brief 构造函数
     * @param columns 主键列名列表
     * @param name 约束名称（可选）
     */
    PrimaryKeyConstraint(const std::vector<std::string>& columns,
     ...
```

**构造函数**:
- `PrimaryKeyConstraint`

---

### UniqueConstraint

**定义位置**: `src/sql_parser/constraint/unique_constraint.h`

**定义**:
```cpp
class UniqueConstraint {
public:
    /**
     * @brief 构造函数
     * @param columns 唯一约束列名列表
     * @param name 约束名称（可选）
     */
    UniqueConstraint(const std::vector<std::string>& columns,
           ...
```

**构造函数**:
- `UniqueConstraint`

---

### NotNullConstraint

**定义位置**: `src/sql_parser/constraint/not_null_constraint.h`

**定义**:
```cpp
class NotNullConstraint {
public:
    /**
     * @brief 构造函数
     * @param column 非空约束列名
     * @param name 约束名称（可选）
     */
    NotNullConstraint(const std::string& column, const std::string& name = ...
```

**构造函数**:
- `NotNullConstraint`

---

### AssertionConstraint

**定义位置**: `src/sql_parser/constraint/assertion_constraint.h`

**定义**:
```cpp
class AssertionConstraint {
public:
    /**
     * @brief 构造函数
     * @param condition 断言条件表达式
     * @param name 约束名称（可选）
     */
    AssertionConstraint(std::unique_ptr<Expression> condition,
      ...
```

**构造函数**:
- `AssertionConstraint`

---

### SelectParser

**定义位置**: `src/sql_parser/select_parser.h`

**定义**:
```cpp
class SelectParser {
public:
  /**
   * 构造函数 - 依赖注入所需的解析器
   * @param tokens Token流，提供token访问接口
   * @param expr_parser 表达式解析器，用于解析WHERE等条件表达式
   */
  SelectParser(TokenStream& tokens, ExpressionParse...
```

**构造函数**:
- `SelectParser`
- `parse`
- `parseSelectClause`
- `parseFromClause`
- `parseJoinClause`
- `parseWhereClause`
- `parseGroupByClause`
- `parseHavingClause`
- `parseOrderByClause`
- `parseSelectItem`

**公有方法**:
- `void parseSelectClause`
- `void parseFromClause`
- `void parseWhereClause`
- `void parseGroupByClause`
- `void parseHavingClause`
- `void parseOrderByClause`
- `string parseSelectItem`

---

### Decimal

**定义位置**: `src/sql_parser/decimal.h`

**定义**:
```cpp

class Decimal {
public:
  // 构造函数
  Decimal();
  Decimal(int64_t value);
  Decimal(double value);
  Decimal(const std::string& str);
  Decimal(const Decimal& other);
  Decimal(Decimal&& other) noexce...
```

**构造函数**:
- `Decimal`
- `Decimal`
- `Decimal`
- `Decimal`
- `Decimal`
- `Decimal`
- `is_positive`
- `set_precision`
- `set_scale`
- `max_value`
- `min_value`
- `normalize`
- `from_string`
- `from_int64`
- `from_double`
- `add_strings`
- `subtract_strings`
- `multiply_strings`
- `divide_strings`
- `compare_strings`
- `remove_leading_zeros`
- `remove_trailing_zeros`
- `adjust_scale`

**析构函数**:
- `Decimal`

**公有方法**:
- `构造函数
  Decimal`
- `bool is_positive`
- `精度和刻度设置
  void set_precision`
- `void set_scale`
- `static Decimal max_value`
- `static Decimal min_value`
- `内部辅助方法
  void normalize`
- `void from_string`
- `void from_int64`
- `void from_double`
- `string add_strings`
- `string subtract_strings`
- `string multiply_strings`
- `string divide_strings`
- `static int compare_strings`
- `string remove_leading_zeros`
- `string remove_trailing_zeros`
- `调整小数位数
  void adjust_scale`

---

### DecimalValue

**定义位置**: `src/sql_parser/data_types.h`

**定义**:
```cpp
class DecimalValue {
public:
    DecimalValue();
    DecimalValue(int64_t value, int precision = 18, int scale = 0);
    DecimalValue(const std::string& str);
    DecimalValue(double value, int precis...
```

**构造函数**:
- `DecimalValue`
- `DecimalValue`
- `DecimalValue`
- `DecimalValue`
- `normalize`
- `from_string`
- `from_double`
- `stringToInt64`

**公有方法**:
- `小数位数

    void normalize`
- `void from_string`
- `void from_double`
- `static int64_t stringToInt64`

---

### DateTimeValue

**定义位置**: `src/sql_parser/data_types.h`

**定义**:
```cpp
class DateTimeValue {
public:
    enum class Format {
        DATE,           // YYYY-MM-DD
        TIME,           // HH:MM:SS
        TIMESTAMP,      // YYYY-MM-DD HH:MM:SS
        DATETIME        /...
```

**构造函数**:
- `DateTimeValue`
- `DateTimeValue`
- `DateTimeValue`
- `now`
- `today`
- `parseString`
- `formatTimePoint`

**公有方法**:
- `static DateTimeValue now`
- `static DateTimeValue today`
- `time_point parseString`
- `string formatTimePoint`

---

### DataValue

**定义位置**: `src/sql_parser/data_types.h`

**定义**:
```cpp
class DataValue {
public:
    DataValue();
    DataValue(DataType type);
    DataValue(int64_t int_val);
    DataValue(double double_val);
    DataValue(const std::string& str_val);
    DataValue(bool...
```

**构造函数**:
- `DataValue`
- `DataValue`
- `DataValue`
- `DataValue`
- `DataValue`
- `DataValue`
- `DataValue`
- `DataValue`
- `DataValue`
- `DataValue`
- `setInt64`
- `setDouble`
- `setString`
- `setBool`
- `setDecimal`
- `setDateTime`
- `deserialize`
- `copyFrom`
- `moveFrom`
- `cleanup`

**析构函数**:
- `DataValue`

**公有方法**:
- `拷贝构造函数和赋值运算符
    DataValue`
- `值设置
    void setInt64`
- `void setDouble`
- `void setString`
- `void setBool`
- `void setDecimal`
- `void setDateTime`
- `static DataValue deserialize`
- `void copyFrom`
- `void moveFrom`
- `void cleanup`

---

### DataTypeManager

**定义位置**: `src/sql_parser/data_types.h`

**定义**:
```cpp
class DataTypeManager {
public:
    static DataTypeManager& getInstance();

    // 类型信息查询
    const DataTypeInfo* getTypeInfo(DataType type) const;
    const DataTypeInfo* getTypeInfo(const std::strin...
```

**构造函数**:
- `getInstance`
- `DataTypeManager`

---

### JSONParser

**定义位置**: `src/sql_parser/json_parser.h`

**定义**:
```cpp
class JSONParser {
public:
    /**
     * @brief 构造函数
     */
    JSONParser();

    /**
     * @brief 析构函数
     */
    ~JSONParser();

    /**
     * @brief 解析JSON路径表达式
     * @param json_path JSON路径...
```

**构造函数**:
- `JSONParser`
- `JSONParser`
- `ParseJSONPath`
- `JSON_QUERY`
- `JSON_VALUE`
- `JSON_ARRAYAGG`
- `JSON_OBJECTAGG`
- `ValidateJSONData`
- `Reset`
- `ParseArrayIndices`
- `ValidateJSONPathSyntax`
- `ExtractPropertyNames`
- `IsValidPathCharacter`
- `IsValidPropertyName`
- `TrimQuotes`
- `SplitPath`
- `ParseFunctionCall`
- `SetError`

**析构函数**:
- `JSONParser`

**公有方法**:
- `bool ParseJSONPath`
- `bool ParseJSONQueryExpression`
- `bool ParseJSONValueExpression`
- `bool ParseJSONArrayAggExpression`
- `bool ParseJSONObjectAggExpression`
- `bool ValidateJSONData`
- `void Reset`
- `bool ValidateJSONPathSyntax`
- `私有辅助方法
    bool IsValidPathCharacter`
- `bool IsValidPropertyName`
- `string TrimQuotes`
- `bool ParseFunctionCall`
- `void SetError`

---

### SetOperation

**定义位置**: `src/sql_parser/ast/dml/ast_dml_nodes.h`

**定义**:
```cpp

class SetOperation {
public:
    SetOperation(SetOperationType type);
    ~SetOperation();

    // Getters
    SetOperationType getType() const { return type_; }
    const std::unique_ptr<SelectState...
```

**构造函数**:
- `SetOperation`
- `SetOperation`
- `setLeft`
- `setRight`

**析构函数**:
- `SetOperation`

**公有方法**:
- `void setLeft`
- `void setRight`

---

### ExpressionParser

**定义位置**: `src/sql_parser/expression_parser.h`

**定义**:
```cpp
class ExpressionParser {
public:
  /**
   * 构造函数 - 依赖注入TokenStream
   * @param tokens Token流，提供token访问接口
   */
  explicit ExpressionParser(TokenStream& tokens);

  /**
   * parseExpression - 表达式解析主入口
...
```

**构造函数**:
- `ExpressionParser`
- `parseExpression`
- `解析逻辑或表达式`
- `parseLogicalAnd`
- `parseEquality`
- `parseComparison`
- `parseTerm`
- `parseFactor`
- `parseUnary`
- `func`
- `func`
- `parseBinaryExpression`
- `parsing`

**公有方法**:
- `explicit ExpressionParser`
- `binary operator parsing`

---

### DDLParser

**定义位置**: `src/sql_parser/ddl_parser.h`

**定义**:
```cpp
class DDLParser {
public:
    DDLParser(Lexer& lexer, Token& currentToken, Token& lookaheadToken, bool& hasLookahead);

    // DDL语句解析
    std::unique_ptr<CreateStatement> parseCreateStatement();
    ...
```

**构造函数**:
- `DDLParser`
- `parseCreateStatement`
- `parseCreateTableStatement`
- `parseCreateDatabaseStatement`
- `parseCreateProcedureStatement`
- `parseCreateTriggerStatement`
- `parseCreateViewStatement`
- `parseDropStatement`
- `parseAlterStatement`
- `parseCreateIndexStatement`
- `parseDropIndexStatement`
- `parseColumnDefinitions`
- `parseColumnDefinition`
- `parseDataType`
- `parseDefaultValue`
- `parseTableConstraint`
- `parseQualifiedName`
- `parseIdentifier`
- `advance`
- `match`
- `consume`
- `reportError`
- `parseColumnNames`

**公有方法**:
- `string parseDataType`
- `string parseDefaultValue`
- `void parseTableConstraint`
- `string parseQualifiedName`
- `string parseIdentifier`
- `Token管理辅助方法
    void advance`
- `bool match`
- `void consume`
- `methods
    void reportError`

---

### SavepointStatement

**定义位置**: `src/sql_parser/advanced_sql92_features.h`

**定义**:
```cpp
class SavepointStatement : public Statement {
public:
  SavepointStatement(const std::string &savepointName);
  ~SavepointStatement();

  const std::string &getSavepointName() const;
  void accept(Nod...
```

**构造函数**:
- `SavepointStatement`
- `SavepointStatement`
- `accept`

**析构函数**:
- `SavepointStatement`

**公有方法**:
- `void accept`

---

### ReleaseSavepointStatement

**定义位置**: `src/sql_parser/advanced_sql92_features.h`

**定义**:
```cpp
class ReleaseSavepointStatement : public Statement {
public:
  ReleaseSavepointStatement(const std::string &savepointName);
  ~ReleaseSavepointStatement();

  const std::string &getSavepointName() con...
```

**构造函数**:
- `ReleaseSavepointStatement`
- `ReleaseSavepointStatement`
- `accept`

**析构函数**:
- `ReleaseSavepointStatement`

**公有方法**:
- `void accept`

---

### SetTransactionStatement

**定义位置**: `src/sql_parser/advanced_sql92_features.h`

**定义**:
```cpp
class SetTransactionStatement : public Statement {
public:
  enum IsolationLevel {
    READ_UNCOMMITTED,
    READ_COMMITTED,
    REPEATABLE_READ,
    SERIALIZABLE
  };

  enum AccessMode { READ_ONLY, ...
```

**构造函数**:
- `SetTransactionStatement`
- `SetTransactionStatement`
- `setIsolationLevel`
- `setAccessMode`
- `setWork`
- `accept`

**析构函数**:
- `SetTransactionStatement`

**公有方法**:
- `void setIsolationLevel`
- `void setAccessMode`
- `void setWork`
- `void accept`

---

### DomainDefinition

**定义位置**: `src/sql_parser/advanced_sql92_features.h`

**定义**:
```cpp
class DomainDefinition {
public:
  enum BaseType {
    CHARACTER,
    DECIMAL,
    NUMERIC,
    INTEGER,
    BIGINT,
    SMALLINT,
    FLOAT,
    REAL,
    DOUBLE_PRECISION,
    BOOLEAN,
    DATE,
   ...
```

**构造函数**:
- `DomainDefinition`
- `DomainDefinition`
- `setCharacterLength`
- `setPrecision`
- `setScale`
- `setDefaultValue`
- `setCheckConstraint`
- `setNotNull`

**析构函数**:
- `DomainDefinition`

**公有方法**:
- `void setCharacterLength`
- `void setPrecision`
- `void setScale`
- `void setDefaultValue`
- `void setCheckConstraint`
- `void setNotNull`

---

### CreateDomainStatement

**定义位置**: `src/sql_parser/advanced_sql92_features.h`

**定义**:
```cpp
class CreateDomainStatement : public Statement {
public:
  CreateDomainStatement(std::unique_ptr<DomainDefinition> domainDef);
  ~CreateDomainStatement();

  const DomainDefinition &getDomainDefinitio...
```

**构造函数**:
- `CreateDomainStatement`
- `CreateDomainStatement`
- `takeDomainDefinition`
- `accept`

**析构函数**:
- `CreateDomainStatement`

**公有方法**:
- `void accept`

---

### AlterDomainStatement

**定义位置**: `src/sql_parser/advanced_sql92_features.h`

**定义**:
```cpp
class AlterDomainStatement : public Statement {
public:
  enum Action {
    SET_DEFAULT,
    DROP_DEFAULT,
    ADD_CONSTRAINT,
    DROP_CONSTRAINT
  };

  AlterDomainStatement(const std::string &domai...
```

**构造函数**:
- `AlterDomainStatement`
- `AlterDomainStatement`
- `setDefaultValue`
- `setConstraintName`
- `setConstraintDefinition`
- `accept`

**析构函数**:
- `AlterDomainStatement`

**公有方法**:
- `void setDefaultValue`
- `void setConstraintName`
- `void setConstraintDefinition`
- `void accept`

---

### DropDomainStatement

**定义位置**: `src/sql_parser/advanced_sql92_features.h`

**定义**:
```cpp
class DropDomainStatement : public Statement {
public:
  enum DropBehavior { RESTRICT, CASCADE };

  DropDomainStatement(const std::string &domainName);
  ~DropDomainStatement();

  const std::string ...
```

**构造函数**:
- `DropDomainStatement`
- `DropDomainStatement`
- `setDropBehavior`
- `setIfExists`
- `accept`

**析构函数**:
- `DropDomainStatement`

**公有方法**:
- `void setDropBehavior`
- `void setIfExists`
- `void accept`

---

### FunctionDefinition

**定义位置**: `src/sql_parser/function/function_definition.h`

**定义**:
```cpp
class FunctionDefinition {
public:
    /**
     * @brief 构造函数
     * @param name 函数名称
     * @param return_type 返回值类型
     */
    FunctionDefinition(const std::string& name, const std::string& return_...
```

**构造函数**:
- `FunctionDefinition`
- `FunctionDefinition`
- `addParameter`
- `addCharacteristic`
- `setBody`
- `setLanguage`
- `characteristicToString`
- `stringToCharacteristic`

**析构函数**:
- `FunctionDefinition`

**公有方法**:
- `void addParameter`
- `void addCharacteristic`
- `void setBody`
- `void setLanguage`
- `string characteristicToString`
- `static FunctionCharacteristic stringToCharacteristic`

---

### CreateFunctionStatement

**定义位置**: `src/sql_parser/function/create_function.h`

**定义**:
```cpp
class CreateFunctionStatement : public DDLStatement {
public:
    CreateFunctionStatement(const std::string& function_name,
                           const std::vector<FunctionParameter>& parameters,...
```

**构造函数**:
- `CreateFunctionStatement`

---

### DropFunctionStatement

**定义位置**: `src/sql_parser/function/function_ddl.h`

**定义**:
```cpp
class DropFunctionStatement : public Statement {
public:
    /**
     * @brief 删除行为枚举
     */
    enum DropBehavior {
        RESTRICT, ///< 限制删除（默认）
        CASCADE   ///< 级联删除
    };

    /**
     *...
```

**构造函数**:
- `DropFunctionStatement`

**公有方法**:
- `explicit DropFunctionStatement`

---

### EnhancedTriggerDefinition

**定义位置**: `src/sql_parser/advanced_sql92_features.h`

**定义**:
```cpp
class EnhancedTriggerDefinition : public TriggerDefinition {
public:
  EnhancedTriggerDefinition(const std::string &name, Timing timing, Event event,
                           Level level, const std:...
```

**构造函数**:
- `EnhancedTriggerDefinition`
- `EnhancedTriggerDefinition`
- `setOldTableName`
- `setNewTableName`
- `addVariable`
- `setWhenCondition`

**析构函数**:
- `EnhancedTriggerDefinition`

**公有方法**:
- `OLD和NEW引用支持
  void setOldTableName`
- `void setNewTableName`
- `触发器变量支持
  void addVariable`
- `WHEN条件支持
  void setWhenCondition`

---

### AlterTableAction

**定义位置**: `src/sql_parser/advanced_sql92_features.h`

**定义**:
```cpp
class AlterTableAction {
public:
  enum ActionType {
    ADD_COLUMN,
    DROP_COLUMN,
    ALTER_COLUMN,
    RENAME_COLUMN,
    ADD_CONSTRAINT,
    DROP_CONSTRAINT,
    DISABLE_TRIGGER,
    ENABLE_TRIG...
```

**构造函数**:
- `AlterTableAction`
- `AlterTableAction`
- `setColumnDefinition`
- `setColumnName`
- `setNewColumnDefinition`
- `setOldColumnName`
- `setConstraint`
- `setConstraintName`
- `setTriggerName`

**析构函数**:
- `AlterTableAction`

**公有方法**:
- `COLUMN支持
  void setColumnDefinition`
- `COLUMN支持
  void setColumnName`
- `COLUMN支持
  void setNewColumnDefinition`
- `COLUMN支持
  void setOldColumnName`
- `CONSTRAINT操作支持
  void setConstraint`
- `void setConstraintName`
- `TRIGGER操作支持
  void setTriggerName`

---

### EnhancedAlterTableStatement

**定义位置**: `src/sql_parser/advanced_sql92_features.h`

**定义**:
```cpp
class EnhancedAlterTableStatement : public Statement {
public:
  EnhancedAlterTableStatement(const std::string &tableName);
  ~EnhancedAlterTableStatement();

  const std::string &getTableName() const...
```

**构造函数**:
- `EnhancedAlterTableStatement`
- `EnhancedAlterTableStatement`
- `addAction`
- `accept`

**析构函数**:
- `EnhancedAlterTableStatement`

**公有方法**:
- `void addAction`
- `void accept`

---

### StatementParser

**定义位置**: `src/sql_parser/statement_parser.h`

**定义**:
```cpp
class StatementParser {
public:
  /**
   * 构造函数 - 依赖注入所需的解析器
   * @param tokens Token流，提供token访问接口
   * @param expr_parser 表达式解析器，用于解析条件表达式
   */
  StatementParser(TokenStream& tokens, ExpressionParse...
```

**构造函数**:
- `StatementParser`
- `parseStatement`
- `VALUES`
- `parseUpdateStatement`
- `parseDeleteStatement`
- `parseCreateStatement`
- `parseDropStatement`
- `parseAlterStatement`
- `parseGrantStatement`
- `parseRevokeStatement`
- `parseUseStatement`
- `parseShowStatement`
- `parseLoadDataStatement`
- `parseIdentifier`
- `parseColumnDefinition`
- `parseDataType`
- `parseDefaultValue`
- `parseTableConstraint`
- `parseCreateTableStatement`
- `parseCreateDatabaseStatement`
- `parseCreateUserStatement`
- `parseDropUserStatement`
- `parseCreateProcedureStatement`
- `parseCreateTriggerStatement`
- `parseCreateViewStatement`
- `parseCreateIndexStatement`
- `parseDropIndexStatement`

**公有方法**:
- `string parseIdentifier`
- `string parseDataType`
- `string parseDefaultValue`
- `void parseTableConstraint`

---

### Interval

**定义位置**: `src/sql_parser/datetime.h`

**定义**:
```cpp
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
  Interval(int64_t value...
```

**构造函数**:
- `Interval`
- `Interval`
- `Interval`
- `Interval`
- `Interval`
- `Interval`
- `set_years`
- `set_months`
- `set_days`
- `set_hours`
- `set_minutes`
- `set_seconds`
- `set_milliseconds`
- `from_milliseconds`
- `from_seconds`
- `from_minutes`
- `from_hours`
- `from_days`
- `normalize`

**析构函数**:
- `Interval`

**公有方法**:
- `构造函数
  Interval`
- `等格式
  Interval`
- `组件设置
  void set_years`
- `void set_months`
- `void set_days`
- `void set_hours`
- `void set_minutes`
- `void set_seconds`
- `void set_milliseconds`
- `static Interval from_milliseconds`
- `static Interval from_seconds`
- `static Interval from_minutes`
- `static Interval from_hours`
- `static Interval from_days`
- `void normalize`

---

### Timestamp

**定义位置**: `src/sql_parser/datetime.h`

**定义**:
```cpp
class Timestamp {
public:
  // 构造函数
  Timestamp();
  Timestamp(int64_t seconds, int32_t nanoseconds = 0);
  Timestamp(const std::string& timestamp_str);
  Timestamp(const Timestamp& other);
  Timestam...
```

**构造函数**:
- `Timestamp`
- `Timestamp`
- `Timestamp`
- `Timestamp`
- `Timestamp`
- `now`
- `from_double`

**析构函数**:
- `Timestamp`

**公有方法**:
- `构造函数
  Timestamp`
- `static Timestamp now`
- `static Timestamp from_double`

---

### TokenStream

**定义位置**: `src/sql_parser/token_stream.h`

**定义**:
```cpp
class TokenStream {
public:
    /**
     * @brief 构造函数 - 初始化token流管理器
     * @param lexer 词法分析器引用，用于获取token
     */
    explicit TokenStream(Lexer& lexer);

    /**
     * @brief 获取当前token
     * @ret...
```

**构造函数**:
- `TokenStream`
- `peek`
- `advance`
- `expect`

**公有方法**:
- `explicit TokenStream`
- `void advance`
- `void expect`

---

### WithRecursiveClause

**定义位置**: `src/sql_parser/recursive_query.h`

**定义**:
```cpp
class WithRecursiveClause : public Statement {
public:
    /**
     * @brief 构造函数
     * @param cte_name CTE名称
     * @param base_query 基础查询
     * @param recursive_query 递归查询
     */
    WithRecursiv...
```

**构造函数**:
- `WithRecursiveClause`
- `WithRecursiveClause`
- `accept`

**析构函数**:
- `WithRecursiveClause`

**公有方法**:
- `void accept`

---

### Token

**定义位置**: `src/sql_parser/token.h`

**定义**:
```cpp
class Token {
public:
  /**
   * @brief 默认构造函数
   */
  Token();

  /**
   * @brief 带参数构造函数
   *
   * @param type Token类型
   * @param lexeme 词素字符串
   * @param line 行号（从1开始）
   * @param column 列号（从1开始）
...
```

**构造函数**:
- `Token`
- `Token`
- `getTypeName`

**公有方法**:
- `string getTypeName`

---

### WindowFunction

**定义位置**: `src/sql_parser/window_function.h`

**定义**:
```cpp
class WindowFunction : public ast::Expression {
private:
    FunctionType functionType_;
    std::string functionName_;
    std::unique_ptr<ast::Expression> expression_;
    std::unique_ptr<class Wind...
```

**构造函数**:
- `WindowFunction`
- `setExpression`
- `setWindowSpecification`

**公有方法**:
- `void setExpression`
- `void setWindowSpecification`

---

### WindowSpecification

**定义位置**: `src/sql_parser/window_function.h`

**定义**:
```cpp
class WindowSpecification : public Statement {
private:
    std::vector<std::string> partitionByColumns_;
    std::vector<std::string> orderByColumns_;
    std::vector<bool> orderByAscending_;
    Fra...
```

**构造函数**:
- `WindowSpecification`
- `accept`
- `setOrderBy`
- `setFrame`

**公有方法**:
- `void accept`
- `排序相关方法
    void setOrderBy`
- `窗口帧相关方法
    void setFrame`

---

### TriggerManager

**定义位置**: `src/sql_parser/trigger_manager.h`

**定义**:
```cpp
class TriggerManager {
public:
    // 构造函数
    TriggerManager();
    explicit TriggerManager(const std::string& name);

    // 析构函数
    ~TriggerManager();

    // 禁用拷贝
    TriggerManager(const Trigger...
```

**构造函数**:
- `TriggerManager`
- `TriggerManager`
- `TriggerManager`
- `initialize`
- `shutdown`
- `set_name`

**析构函数**:
- `TriggerManager`

**公有方法**:
- `构造函数
    TriggerManager`
- `explicit TriggerManager`
- `公共方法
    void initialize`
- `void shutdown`
- `void set_name`

---

### TriggerParser

**定义位置**: `src/sql_parser/trigger_parser.h`

**定义**:
```cpp
class TriggerParser {
public:
    /**
     * @brief 构造函数
     */
    TriggerParser();

    /**
     * @brief 析构函数
     */
    ~TriggerParser();

    /**
     * @brief 解析CREATE TRIGGER语句
     * @param ...
```

**构造函数**:
- `TriggerParser`
- `TriggerParser`
- `ParseCreateTrigger`
- `ParseDropTrigger`
- `ParseAlterTrigger`
- `ParseTriggerName`
- `ParseTriggerType`
- `ParseTriggerEvent`
- `ParseTriggerTable`
- `ParseTriggerCondition`
- `ParseTriggerAction`
- `ValidateTriggerSyntax`
- `Reset`
- `ParseForEachRow`
- `ParseForEachStatement`
- `ParseWhenCondition`
- `ParseTriggerFunction`
- `IsValidTriggerName`
- `IsValidTableName`
- `IsValidTriggerType`
- `IsValidTriggerEvent`
- `TrimQuotes`
- `ExtractSubstring`
- `SplitByKeywords`
- `SetError`
- `ParseTriggerDefinition`
- `ParseTriggerBody`

**析构函数**:
- `TriggerParser`

**公有方法**:
- `bool ParseCreateTrigger`
- `bool ParseDropTrigger`
- `bool ParseAlterTrigger`
- `string ParseTriggerName`
- `string ParseTriggerType`
- `string ParseTriggerEvent`
- `string ParseTriggerTable`
- `string ParseTriggerCondition`
- `string ParseTriggerAction`
- `bool ValidateTriggerSyntax`
- `void Reset`
- `bool ParseForEachRow`
- `bool ParseForEachStatement`
- `string ParseWhenCondition`
- `私有辅助方法
    bool IsValidTriggerName`
- `bool IsValidTableName`
- `bool IsValidTriggerType`
- `bool IsValidTriggerEvent`
- `string TrimQuotes`
- `string ExtractSubstring`
- `void SetError`
- `bool ParseTriggerDefinition`
- `bool ParseTriggerBody`

---

### Parser

**定义位置**: `src/sql_parser/parser.h`

**定义**:
```cpp
class Parser final {
public:
  /**
   * @brief Parser构造函数
   * @param input SQL输入字符串，由词法分析器处理
   */
  Parser(const std::string& input);

  /**
   * WHAT: parse - 解析SQL语句的主入口

   * 处理完整的SQL脚本，包含多个语句。返回...
```

**构造函数**:
- `Parser`
- `循环调用parseStatement`
- `clearErrors`
- `advance`
- `match`
- `consume`
- `reportError`
- `synchronize`
- `isCreateViewStatement`
- `isCreateUserStatement`
- `isDropUserStatement`
- `parsing`
- `parseCreateStatement`
- `parseCreateTableStatement`
- `parseCreateDatabaseStatement`
- `parseCreateProcedureStatement`
- `parseCreateTriggerStatement`
- `parseCreateViewStatement`
- `parseDropStatement`
- `parseAlterStatement`
- `parseSelectStatement`
- `parseInsertStatement`
- `parseUpdateStatement`
- `parseDeleteStatement`
- `parseUseStatement`
- `parseShowStatement`
- `parseCreateIndexStatement`
- `parseDropIndexStatement`
- `parseCreateUserStatement`
- `parseDropUserStatement`
- `parseGrantStatement`
- `parseRevokeStatement`
- `parseLoadDataStatement`
- `parseColumnNames`
- `parseExpressions`
- `COUNT`
- `parseJoinClause`
- `parseColumnDefinitions`
- `parseColumnDefinition`
- `parseDataType`
- `parseDefaultValue`
- `parseTableConstraint`
- `initializeSyncTokens`
- `parseQualifiedName`
- `parseIdentifier`
- `parseStringLiteral`
- `parseIntLiteral`
- `parseCompositeSelectStatement`
- `parseSetOperation`
- `parseUnion`
- `parseIntersect`
- `parseExcept`
- `parseSetOperationType`

**公有方法**:
- `void clearErrors`
- `methods
  void advance`
- `bool match`
- `void consume`
- `handling
  void reportError`
- `void synchronize`
- `VIEW
  bool isCreateViewStatement`
- `USER
  bool isCreateUserStatement`
- `USER
  bool isDropUserStatement`
- `Statement parsing`
- `string parseDataType`
- `string parseDefaultValue`
- `void parseTableConstraint`
- `methods
  void initializeSyncTokens`
- `string parseQualifiedName`
- `string parseIdentifier`
- `string parseStringLiteral`
- `int parseIntLiteral`
- `parsing
  SetOperationType parseSetOperationType`

---

### NodeVisitor

**定义位置**: `src/sql_parser/ast/expression.h`

**定义**:
```cpp

class NodeVisitor;

class Expression : public ASTNode {
public:
  virtual ~Expression() = default;
  virtual ExpressionType getType() const = 0;
  virtual void accept(NodeVisitor& visitor) = 0;
};
```

---

### CreateRoleStatement

**定义位置**: `src/sql_parser/ast/ast_dcl_statements.h`

**定义**:
```cpp
class CreateRoleStatement : public Statement {
public:
    CreateRoleStatement(const std::string& role_name);
    virtual ~CreateRoleStatement();

    std::string getTypeName() const;
    virtual void...
```

**构造函数**:
- `CreateRoleStatement`
- `CreateRoleStatement`

**析构函数**:
- `CreateRoleStatement`

---

### DropRoleStatement

**定义位置**: `src/sql_parser/ast/ast_dcl_statements.h`

**定义**:
```cpp
class DropRoleStatement : public Statement {
public:
    DropRoleStatement(const std::string& role_name);
    virtual ~DropRoleStatement();

    std::string getTypeName() const;
    virtual void accep...
```

**构造函数**:
- `DropRoleStatement`
- `DropRoleStatement`

**析构函数**:
- `DropRoleStatement`

---

### GrantRoleStatement

**定义位置**: `src/sql_parser/ast/ast_dcl_statements.h`

**定义**:
```cpp
class GrantRoleStatement : public Statement {
public:
    GrantRoleStatement(const std::string& role_name, const std::string& grantee);
    virtual ~GrantRoleStatement();

    std::string getTypeName(...
```

**构造函数**:
- `GrantRoleStatement`
- `GrantRoleStatement`

**析构函数**:
- `GrantRoleStatement`

---

### RevokeRoleStatement

**定义位置**: `src/sql_parser/ast/ast_dcl_statements.h`

**定义**:
```cpp
class RevokeRoleStatement : public Statement {
public:
    RevokeRoleStatement(const std::string& role_name, const std::string& grantee);
    virtual ~RevokeRoleStatement();

    std::string getTypeNa...
```

**构造函数**:
- `RevokeRoleStatement`
- `RevokeRoleStatement`

**析构函数**:
- `RevokeRoleStatement`

---

### NumericLiteralExpression

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**定义**:
```cpp

class NumericLiteralExpression : public Expression {
public:
  NumericLiteralExpression(double value);
  ~NumericLiteralExpression();

  double getValue() const;
  virtual std::string getTypeName() c...
```

**构造函数**:
- `NumericLiteralExpression`
- `NumericLiteralExpression`
- `accept`

**析构函数**:
- `NumericLiteralExpression`

**公有方法**:
- `virtual void accept`

---

### StringLiteralExpression

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**定义**:
```cpp

class StringLiteralExpression : public Expression {
public:
  StringLiteralExpression(const std::string &value);
  ~StringLiteralExpression();

  const std::string &getValue() const;
  virtual std::s...
```

**构造函数**:
- `StringLiteralExpression`
- `StringLiteralExpression`
- `accept`

**析构函数**:
- `StringLiteralExpression`

**公有方法**:
- `virtual void accept`

---

### BooleanLiteralExpression

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**定义**:
```cpp

class BooleanLiteralExpression : public Expression {
public:
  BooleanLiteralExpression(bool value);
  ~BooleanLiteralExpression();

  bool getValue() const;
  virtual std::string getTypeName() const...
```

**构造函数**:
- `BooleanLiteralExpression`
- `BooleanLiteralExpression`
- `accept`

**析构函数**:
- `BooleanLiteralExpression`

**公有方法**:
- `virtual void accept`

---

### NullLiteralExpression

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**定义**:
```cpp

class NullLiteralExpression : public Expression {
public:
  NullLiteralExpression();
  ~NullLiteralExpression();

  virtual std::string getTypeName() const override;
  virtual void accept(NodeVisitor...
```

**构造函数**:
- `NullLiteralExpression`
- `NullLiteralExpression`
- `accept`

**析构函数**:
- `NullLiteralExpression`

**公有方法**:
- `virtual void accept`

---

### IdentifierExpression

**定义位置**: `src/sql_parser/ast/expressions/ast_expression_nodes.h`

**定义**:
```cpp

class IdentifierExpression : public Expression {
public:
    IdentifierExpression(const std::string &name);
    ~IdentifierExpression() override;
    
    void accept(NodeVisitor &visitor) override;
...
```

**构造函数**:
- `IdentifierExpression`

---

### FunctionCallExpression

**定义位置**: `src/sql_parser/ast/expressions/ast_expression_nodes.h`

**定义**:
```cpp

class FunctionCallExpression : public Expression {
public:
    FunctionCallExpression(const std::string &functionName);
    ~FunctionCallExpression() override;
    
    void accept(NodeVisitor &visit...
```

**构造函数**:
- `FunctionCallExpression`
- `setArguments`

**公有方法**:
- `void setArguments`

---

### BinaryExpression

**定义位置**: `src/sql_parser/ast/expressions/ast_expression_nodes.h`

**定义**:
```cpp

class BinaryExpression : public Expression {
public:
    BinaryExpression(std::unique_ptr<Expression> left, OperatorKind op, std::unique_ptr<Expression> right);
    ~BinaryExpression() override;
    ...
```

**构造函数**:
- `BinaryExpression`
- `setLeft`
- `setRight`

**公有方法**:
- `Setters
    void setLeft`
- `void setRight`

---

### ConstraintValidator

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**定义**:
```cpp
class ConstraintValidator {
public:
  virtual ~ConstraintValidator() = default;

  /**
   * 验证记录是否满足约束条件
   * @param record 要验证的记录
   * @param metadata 表元数据
   * @param table_name 表名
   * @return 验证结果...
```

---

### PrimaryKeyValidator

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**定义**:
```cpp
class PrimaryKeyValidator : public ConstraintValidator {
public:
  PrimaryKeyValidator(const std::vector<std::string>& columns, const std::string& constraint_name = "");
  ~PrimaryKeyValidator() overr...
```

**构造函数**:
- `PrimaryKeyValidator`

---

### UniqueKeyValidator

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**定义**:
```cpp
class UniqueKeyValidator : public ConstraintValidator {
public:
  UniqueKeyValidator(const std::vector<std::string>& columns, const std::string& constraint_name = "");
  ~UniqueKeyValidator() override...
```

**构造函数**:
- `UniqueKeyValidator`

---

### ForeignKeyValidator

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**定义**:
```cpp
class ForeignKeyValidator : public ConstraintValidator {
public:
  ForeignKeyValidator(const std::vector<std::string>& columns,
                     const std::string& referenced_table,
              ...
```

**构造函数**:
- `ForeignKeyValidator`
- `setOnDeleteAction`
- `setOnUpdateAction`

**公有方法**:
- `void setOnDeleteAction`
- `void setOnUpdateAction`

---

### CheckConstraintValidator

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**定义**:
```cpp
class CheckConstraintValidator : public ConstraintValidator {
public:
  CheckConstraintValidator(const std::string& expression, const std::string& constraint_name = "");
  ~CheckConstraintValidator() ...
```

**构造函数**:
- `CheckConstraintValidator`

---

### ConstraintManager

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**定义**:
```cpp
class ConstraintManager {
public:
  static ConstraintManager& getInstance();

  // 添加约束验证器
  void addValidator(const std::string& table_name,
                   std::unique_ptr<ConstraintValidator> va...
```

**构造函数**:
- `getInstance`
- `addValidator`
- `removeValidator`
- `clearValidators`
- `ConstraintManager`

**公有方法**:
- `添加约束验证器
  void addValidator`
- `移除约束验证器
  void removeValidator`
- `清空表的所有约束
  void clearValidators`

---

### TableConstraint

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**定义**:
```cpp

class TableConstraint {
public:
    enum Type { PRIMARY_KEY, FOREIGN_KEY, UNIQUE, CHECK };

    TableConstraint(Type type, const std::string &name = "");
    ~TableConstraint();

    // Getters
    T...
```

**构造函数**:
- `TableConstraint`
- `TableConstraint`
- `addColumn`
- `addReferencedColumn`

**析构函数**:
- `TableConstraint`

**公有方法**:
- `void addColumn`
- `void addReferencedColumn`

---

### WhereClause

**定义位置**: `src/sql_parser/ast/dml/ast_dml_nodes.h`

**定义**:
```cpp

class WhereClause {
public:
    WhereClause(std::unique_ptr<Expression> condition);
    ~WhereClause();

    // Getters
    const std::unique_ptr<Expression> &getCondition() const { return condition_...
```

**构造函数**:
- `WhereClause`
- `WhereClause`
- `setCondition`

**析构函数**:
- `WhereClause`

**公有方法**:
- `Setters
    void setCondition`

---

### CreateStatement

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**定义**:
```cpp

class CreateStatement : public Statement {
public:
    enum ObjectType { DATABASE, TABLE, INDEX, VIEW, PROCEDURE, TRIGGER };

    CreateStatement(ObjectType objectType);
    CreateStatement(ObjectTyp...
```

**构造函数**:
- `CreateStatement`
- `CreateStatement`
- `addColumn`
- `addConstraint`
- `setSelectStatement`

**公有方法**:
- `void addColumn`
- `void addConstraint`
- `void setSelectStatement`

---

### CreateViewStatement

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**定义**:
```cpp

class CreateViewStatement : public Statement {
public:
    CreateViewStatement(const std::string &viewName);
    ~CreateViewStatement() override;

    void accept(NodeVisitor &visitor) override;

   ...
```

**构造函数**:
- `CreateViewStatement`
- `addColumnName`
- `setSelectStatement`

**公有方法**:
- `void addColumnName`
- `void setSelectStatement`

---

### AlterViewStatement

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**定义**:
```cpp

class AlterViewStatement : public Statement {
public:
  AlterViewStatement(const std::string &viewName);
  ~AlterViewStatement();

  const std::string &getViewName() const;
  const std::vector<std::s...
```

**构造函数**:
- `AlterViewStatement`
- `AlterViewStatement`
- `addColumnName`
- `setSelectStatement`
- `accept`

**析构函数**:
- `AlterViewStatement`

**公有方法**:
- `void addColumnName`
- `void setSelectStatement`
- `void accept`

---

### DropViewStatement

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**定义**:
```cpp

class DropViewStatement : public Statement {
public:
  enum DropBehavior { RESTRICT, CASCADE };

  DropViewStatement(const std::string &viewName);
  ~DropViewStatement();

  const std::string &getVie...
```

**构造函数**:
- `DropViewStatement`
- `DropViewStatement`
- `setDropBehavior`
- `setIfExists`
- `accept`

**析构函数**:
- `DropViewStatement`

**公有方法**:
- `void setDropBehavior`
- `void setIfExists`
- `void accept`

---

### JoinClause

**定义位置**: `src/sql_parser/ast/dml/ast_dml_nodes.h`

**定义**:
```cpp

class JoinClause {
public:
    JoinClause(const std::string &tableName, JoinType type);
    ~JoinClause();

    // Getters
    const std::string &getTableName() const { return tableName_; }
    JoinT...
```

**构造函数**:
- `JoinClause`
- `JoinClause`
- `setCondition`

**析构函数**:
- `JoinClause`

**公有方法**:
- `void setCondition`

---

### SelectStatement

**定义位置**: `src/sql_parser/ast/dml/ast_dml_nodes.h`

**定义**:
```cpp

class SelectStatement : public Statement {
public:
    SelectStatement();
    ~SelectStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::vect...
```

**构造函数**:
- `SelectStatement`
- `setSelectList`
- `setWhereClause`
- `setJoinClauses`
- `setGroupBy`
- `setHaving`
- `setOrderBy`

**公有方法**:
- `Setters
    void setSelectList`
- `void setWhereClause`
- `void setJoinClauses`
- `void setGroupBy`
- `void setHaving`
- `void setOrderBy`

---

### CompositeSelectStatement

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**定义**:
```cpp

class CompositeSelectStatement : public Statement {
public:
  CompositeSelectStatement() : Statement(Statement::COMPOSITE_SELECT) {}
  ~CompositeSelectStatement() override = default;

  void addSelec...
```

**构造函数**:
- `addSelectStatement`
- `addSetOperation`
- `getStatementCount`
- `getOperationCount`
- `hasSetOperations`
- `accept`

**公有方法**:
- `void addSelectStatement`
- `void addSetOperation`
- `size_t getStatementCount`
- `size_t getOperationCount`
- `bool hasSetOperations`
- `void accept`

---

### InsertStatement

**定义位置**: `src/sql_parser/ast/dml/ast_dml_nodes.h`

**定义**:
```cpp

class InsertStatement : public Statement {
public:
    InsertStatement(const std::string &tableName);
    ~InsertStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    //...
```

**构造函数**:
- `InsertStatement`
- `setValues`
- `setSelectStatement`

**公有方法**:
- `void setValues`
- `void setSelectStatement`

---

### UpdateStatement

**定义位置**: `src/sql_parser/ast/dml/ast_dml_nodes.h`

**定义**:
```cpp

class UpdateStatement : public Statement {
public:
    UpdateStatement(const std::string &tableName);
    ~UpdateStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    //...
```

**构造函数**:
- `UpdateStatement`
- `setAssignments`
- `setWhereClause`

**公有方法**:
- `void setAssignments`
- `void setWhereClause`

---

### DeleteStatement

**定义位置**: `src/sql_parser/ast/dml/ast_dml_nodes.h`

**定义**:
```cpp

class DeleteStatement : public Statement {
public:
    DeleteStatement(const std::vector<std::string> &tableNames);
    ~DeleteStatement() override;
    
    void accept(NodeVisitor &visitor) overrid...
```

**构造函数**:
- `DeleteStatement`
- `setWhereClause`

**公有方法**:
- `void setWhereClause`

---

### DropStatement

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**定义**:
```cpp

class DropStatement : public Statement {
public:
    enum ObjectType { DATABASE, TABLE, INDEX, VIEW, PROCEDURE, TRIGGER, USER };

    DropStatement(ObjectType objectType, const std::string &objectNam...
```

**构造函数**:
- `DropStatement`

---

### AlterStatement

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**定义**:
```cpp

class AlterStatement : public Statement {
public:
    enum ObjectType { DATABASE, TABLE };

    AlterStatement(ObjectType objectType);
    AlterStatement(ObjectType objectType, const std::string &obj...
```

**构造函数**:
- `AlterStatement`
- `AlterStatement`
- `setTableName`
- `setColumnDefinition`
- `setTableConstraint`

**公有方法**:
- `void setTableName`
- `void setColumnDefinition`
- `void setTableConstraint`

---

### UseStatement

**定义位置**: `src/sql_parser/ast/utilities/ast_utility_nodes.h`

**定义**:
```cpp

class UseStatement : public Statement {
public:
    UseStatement(const std::string &databaseName);
    ~UseStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Gette...
```

**构造函数**:
- `UseStatement`

---

### CreateIndexStatement

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**定义**:
```cpp

class CreateIndexStatement : public Statement {
public:
    CreateIndexStatement(const std::string &indexName,
                       const std::string &tableName,
                       const std::s...
```

**构造函数**:
- `CreateIndexStatement`
- `addColumn`

**公有方法**:
- `void addColumn`

---

### DropIndexStatement

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**定义**:
```cpp

class DropIndexStatement : public Statement {
public:
    DropIndexStatement(const std::string &indexName);
    ~DropIndexStatement() override;

    void accept(NodeVisitor &visitor) override;

    c...
```

**构造函数**:
- `DropIndexStatement`

---

### CreateUserStatement

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**定义**:
```cpp

class CreateUserStatement : public Statement {
public:
    CreateUserStatement(const std::string &userName, const std::string &password);
    ~CreateUserStatement() override;
    void accept(NodeVisi...
```

**构造函数**:
- `CreateUserStatement`

---

### DropUserStatement

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**定义**:
```cpp

class DropUserStatement : public Statement {
public:
    DropUserStatement(const std::string &userName);
    ~DropUserStatement() override;
    void accept(NodeVisitor &visitor) override;

    const ...
```

**构造函数**:
- `DropUserStatement`

---

### GrantStatement

**定义位置**: `src/sql_parser/ast/utilities/ast_utility_nodes.h`

**定义**:
```cpp

class GrantStatement : public Statement {
public:
    GrantStatement();
    ~GrantStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::vector<...
```

**构造函数**:
- `GrantStatement`

---

### RevokeStatement

**定义位置**: `src/sql_parser/ast/utilities/ast_utility_nodes.h`

**定义**:
```cpp

class RevokeStatement : public Statement {
public:
    RevokeStatement();
    ~RevokeStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::vect...
```

**构造函数**:
- `RevokeStatement`

---

### ShowStatement

**定义位置**: `src/sql_parser/ast/utilities/ast_utility_nodes.h`

**定义**:
```cpp

class ShowStatement : public Statement {
public:
    ShowStatement(const std::string &showType);
    ~ShowStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getter...
```

**构造函数**:
- `ShowStatement`

---

### CommitStatement

**定义位置**: `src/sql_parser/ast/utilities/ast_utility_nodes.h`

**定义**:
```cpp

class CommitStatement : public Statement {
public:
    CommitStatement();
    ~CommitStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::stri...
```

**构造函数**:
- `CommitStatement`

---

### RollbackStatement

**定义位置**: `src/sql_parser/ast/utilities/ast_utility_nodes.h`

**定义**:
```cpp

class RollbackStatement : public Statement {
public:
    RollbackStatement();
    ~RollbackStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std...
```

**构造函数**:
- `RollbackStatement`

---

### ProcedureParameter

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**定义**:
```cpp

class ProcedureParameter {
public:
  enum Mode { IN, OUT, INOUT };

  ProcedureParameter(const std::string &name, const std::string &type,
                     Mode mode);
  ~ProcedureParameter();

 ...
```

**构造函数**:
- `ProcedureParameter`
- `ProcedureParameter`

**析构函数**:
- `ProcedureParameter`

---

### CreateProcedureStatement

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**定义**:
```cpp

class CreateProcedureStatement : public Statement {
public:
    CreateProcedureStatement(const std::string &procedureName);
    ~CreateProcedureStatement() override;
    void accept(NodeVisitor &visi...
```

**构造函数**:
- `CreateProcedureStatement`

---

### CallProcedureStatement

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**定义**:
```cpp

class CallProcedureStatement : public Statement {
public:
  CallProcedureStatement(const std::string &name);
  ~CallProcedureStatement();

  void addArgument(std::unique_ptr<Expression> arg);
  const...
```

**构造函数**:
- `CallProcedureStatement`
- `CallProcedureStatement`
- `addArgument`
- `accept`

**析构函数**:
- `CallProcedureStatement`

**公有方法**:
- `void addArgument`
- `void accept`

---

### DropProcedureStatement

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**定义**:
```cpp

class DropProcedureStatement : public Statement {
public:
    DropProcedureStatement(const std::string &procedureName);
    ~DropProcedureStatement() override;
    void accept(NodeVisitor &visitor) o...
```

**构造函数**:
- `DropProcedureStatement`

---

### TriggerDefinition

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**定义**:
```cpp

class TriggerDefinition {
public:
  enum Timing { BEFORE, AFTER, INSTEAD_OF };

  enum Event { INSERT, UPDATE, DELETE };

  enum Level { ROW, STATEMENT };

  TriggerDefinition(const std::string &name...
```

**构造函数**:
- `TriggerDefinition`
- `TriggerDefinition`
- `getTableName`
- `setCondition`
- `setBody`

**析构函数**:
- `TriggerDefinition`

**公有方法**:
- `string getTableName`
- `void setCondition`
- `void setBody`

---

### CreateTriggerStatement

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**定义**:
```cpp

class CreateTriggerStatement : public Statement {
public:
    CreateTriggerStatement(const std::string &triggerName);
    ~CreateTriggerStatement() override;
    void accept(NodeVisitor &visitor) ove...
```

**构造函数**:
- `CreateTriggerStatement`

---

### DropTriggerStatement

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**定义**:
```cpp

class DropTriggerStatement : public Statement {
public:
    DropTriggerStatement(const std::string &triggerName);
    ~DropTriggerStatement() override;
    void accept(NodeVisitor &visitor) override;...
```

**构造函数**:
- `DropTriggerStatement`

---

### AlterTriggerStatement

**定义位置**: `src/sql_parser/ast/ast_nodes.h`

**定义**:
```cpp

class AlterTriggerStatement : public Statement {
public:
  enum Action { ENABLE, DISABLE };

  AlterTriggerStatement(const std::string &name, Action action);
  ~AlterTriggerStatement();

  const std:...
```

**构造函数**:
- `AlterTriggerStatement`
- `AlterTriggerStatement`
- `accept`

**析构函数**:
- `AlterTriggerStatement`

**公有方法**:
- `void accept`

---

### ASTNode

**定义位置**: `src/sql_parser/ast/ast_node.h`

**定义**:
```cpp

class ASTNode {
public:
  virtual ~ASTNode() = default;
  virtual std::string getTypeName() const { return "ASTNode"; }
};
```

---

### FunctionParameter

**定义位置**: `src/sql_parser/function/function_parameter.h`

**定义**:
```cpp
class FunctionParameter {
public:
    FunctionParameter(const std::string& name, DataType data_type,
                     ParameterMode mode = ParameterMode::IN,
                     const std::string...
```

**构造函数**:
- `FunctionParameter`
- `set_default_value`

**公有方法**:
- `void set_default_value`

---

### AlterFunctionStatement

**定义位置**: `src/sql_parser/function/alter_function.h`

**定义**:
```cpp
class AlterFunctionStatement : public DDLStatement {
public:
    // 修改操作类型
    enum class AlterType {
        RENAME,           // 重命名函数
        SET_SCHEMA,       // 更改模式
        OWNER_TO,         // ...
```

**构造函数**:
- `AlterFunctionStatement`

---

### FunctionCallStatement

**定义位置**: `src/sql_parser/function/function_call.h`

**定义**:
```cpp
class FunctionCallStatement : public Statement {
public:
    /**
     * @brief 构造函数
     * @param function_name 函数名
     */
    explicit FunctionCallStatement(const std::string& function_name);

    /...
```

**构造函数**:
- `FunctionCallStatement`
- `addArgument`

**公有方法**:
- `explicit FunctionCallStatement`
- `void addArgument`

---

### ParseError

**定义位置**: `src/sql_parser/errors/error_core.h`

**定义**:
```cpp
class ParseError {
public:
    /**
     * @brief 构造函数
     *
     * @param type 错误类型
     * @param severity 错误严重程度
     * @param message 错误消息
     * @param location 错误位置
     */
    ParseError(ErrorTy...
```

**构造函数**:
- `ParseError`
- `setSuggestion`
- `setContext`

**公有方法**:
- `void setSuggestion`
- `void setContext`

---

### ErrorCollector

**定义位置**: `src/sql_parser/errors/error_core.h`

**定义**:
```cpp
class ErrorCollector {
public:
    /**
     * @brief 构造函数
     */
    ErrorCollector();

    /**
     * @brief 析构函数
     */
    ~ErrorCollector() = default;

    /**
     * @brief 添加错误
     *
     * @...
```

**构造函数**:
- `ErrorCollector`
- `addError`
- `addWarning`
- `addInfo`
- `clear`
- `clearErrors`
- `clearWarnings`

**公有方法**:
- `void addError`
- `void addWarning`
- `void addInfo`
- `void clear`
- `void clearErrors`
- `void clearWarnings`

---

### ParserCore

**定义位置**: `src/sql_parser/parsers/parser_core.h`

**定义**:
```cpp

class ParserCore {
protected:
    // Single source of truth for parser state
    TokenStream& tokens_;

    // Error recovery
    std::vector<std::string> errors_;
    bool panic_mode_;
    std::unor...
```

**构造函数**:
- `ParserCore`
- `match`
- `consume`
- `advance`
- `peek`
- `reportError`
- `synchronize`
- `resetPanicMode`
- `setSyncTokens`
- `parseIdentifier`
- `parseQualifiedName`
- `parseStringLiteral`
- `parseIntLiteral`
- `parseColumnNames`

**公有方法**:
- `基础token匹配方法
    bool match`
- `void consume`
- `前瞻token管理
    void advance`
- `错误处理方法
    void reportError`
- `void synchronize`
- `void resetPanicMode`
- `void setSyncTokens`
- `string parseIdentifier`
- `string parseQualifiedName`
- `string parseStringLiteral`
- `int parseIntLiteral`

---

### ParserDCL

**定义位置**: `src/sql_parser/parsers/dcl/parser_dcl.h`

**定义**:
```cpp

class ParserDCL : public ParserCore {
public:
    ParserDCL(TokenStream& tokens);
    
    // DCL语句解析方法
    std::unique_ptr<ASTNode> parseGrantStatement();
    std::unique_ptr<ASTNode> parseRevokeSta...
```

**构造函数**:
- `ParserDCL`
- `parseGrantStatement`
- `parseRevokeStatement`
- `parsePrivilegeList`
- `parseObjectType`
- `parseObjectName`
- `parseUserNames`
- `isValidPrivilege`
- `isValidObjectType`

**公有方法**:
- `string parseObjectType`
- `string parseObjectName`
- `权限验证方法
    bool isValidPrivilege`
- `bool isValidObjectType`

---

### ParserDML

**定义位置**: `src/sql_parser/parsers/dml/parser_dml.h`

**定义**:
```cpp

class ParserDML : public ParserCore {
private:
    std::unique_ptr<SelectParser> select_parser_;

public:
    ParserDML(TokenStream& tokens);
    
    // DML语句解析方法
    std::unique_ptr<ASTNode> parseS...
```

**构造函数**:
- `ParserDML`
- `parseSelectStatement`
- `parseInsertStatement`
- `parseUpdateStatement`
- `parseDeleteStatement`
- `parseWhereClause`
- `parseJoinClause`
- `parseSetOperation`
- `parseInsertColumnNames`
- `parseInsertValues`
- `parseInsertSelect`
- `parseUpdateAssignments`
- `parseDeleteTableNames`

---

### ParserHelpers

**定义位置**: `src/sql_parser/parsers/helpers/parser_helpers.h`

**定义**:
```cpp

class ParserHelpers : public ParserCore {
private:
    std::unique_ptr<ExpressionParser> expression_parser_;

public:
    ParserHelpers(TokenStream& tokens);
    
    // 辅助解析方法
    std::vector<std::s...
```

**构造函数**:
- `ParserHelpers`
- `parseColumnNames`
- `parseExpressions`
- `parseQualifiedName`
- `parseIdentifier`
- `parseStringLiteral`
- `parseIntLiteral`
- `parseFloatLiteral`
- `parseCompositeSelectStatement`
- `parseSetOperation`
- `parseUnion`
- `parseIntersect`
- `parseExcept`
- `parseSetOperationType`
- `isSetOperation`
- `parseUseStatement`
- `parseShowStatement`
- `parseLoadDataStatement`
- `parseExpression`

**公有方法**:
- `string parseQualifiedName`
- `string parseIdentifier`
- `string parseStringLiteral`
- `int64_t parseIntLiteral`
- `double parseFloatLiteral`
- `SetOperationType parseSetOperationType`
- `bool isSetOperation`

---

### ParserTCL

**定义位置**: `src/sql_parser/parsers/tcl/parser_tcl.h`

**定义**:
```cpp

class ParserTCL : public ParserCore {
public:
    ParserTCL(TokenStream& tokens);
    
    // TCL语句解析方法
    std::unique_ptr<ASTNode> parseCommitStatement();
    std::unique_ptr<ASTNode> parseRollback...
```

**构造函数**:
- `ParserTCL`
- `parseCommitStatement`
- `parseRollbackStatement`
- `parseBeginStatement`
- `parseTransactionMode`
- `parseIsolationLevel`

**公有方法**:
- `string parseTransactionMode`
- `string parseIsolationLevel`

---

### ParserDDL

**定义位置**: `src/sql_parser/parsers/ddl/parser_ddl.h`

**定义**:
```cpp

class ParserDDL : public ParserCore {
public:
    ParserDDL(TokenStream& tokens);
    
    // DDL语句解析方法
    std::unique_ptr<ASTNode> parseCreateStatement();
    std::unique_ptr<ASTNode> parseCreateTa...
```

**构造函数**:
- `ParserDDL`
- `parseCreateStatement`
- `parseCreateTableStatement`
- `parseCreateDatabaseStatement`
- `parseCreateIndexStatement`
- `parseCreateViewStatement`
- `parseCreateUserStatement`
- `parseCreateProcedureStatement`
- `parseCreateTriggerStatement`
- `parseDropStatement`
- `parseDropTableStatement`
- `parseDropDatabaseStatement`
- `parseDropIndexStatement`
- `parseDropViewStatement`
- `parseDropUserStatement`
- `parseDropProcedureStatement`
- `parseDropTriggerStatement`
- `parseAlterStatement`
- `parseAlterTableStatement`
- `parseAlterDatabaseStatement`
- `parseColumnDefinition`
- `parseTableConstraint`
- `parseColumnDefinitions`
- `parseTableConstraints`

---

### BeginStatement

**定义位置**: `src/sql_parser/ast/utilities/ast_utility_nodes.h`

**定义**:
```cpp

class BeginStatement : public Statement {
public:
    BeginStatement();
    ~BeginStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::string ...
```

**构造函数**:
- `BeginStatement`

---

### LoadDataStatement

**定义位置**: `src/sql_parser/ast/utilities/ast_utility_nodes.h`

**定义**:
```cpp

class LoadDataStatement : public Statement {
public:
    LoadDataStatement(const std::string &fileName, const std::string &tableName);
    ~LoadDataStatement() override;
    
    void accept(NodeVisi...
```

**构造函数**:
- `LoadDataStatement`

---

### UnaryExpression

**定义位置**: `src/sql_parser/ast/expressions/ast_expression_nodes.h`

**定义**:
```cpp

class UnaryExpression : public Expression {
public:
    UnaryExpression(OperatorKind op, std::unique_ptr<Expression> operand);
    ~UnaryExpression() override;
    
    void accept(NodeVisitor &visit...
```

**构造函数**:
- `UnaryExpression`
- `setOperand`

**公有方法**:
- `void setOperand`

---

### LiteralExpression

**定义位置**: `src/sql_parser/ast/expressions/ast_expression_nodes.h`

**定义**:
```cpp

class LiteralExpression : public Expression {
public:
    LiteralExpression(const std::string &value, LiteralType type);
    ~LiteralExpression() override;
    
    void accept(NodeVisitor &visitor) ...
```

**构造函数**:
- `LiteralExpression`

---

### ColumnDefinition

**定义位置**: `src/sql_parser/ast/ddl/ast_ddl_nodes.h`

**定义**:
```cpp

class ColumnDefinition {
public:
    ColumnDefinition(const std::string &name, const std::string &type);
    ~ColumnDefinition();

    // Getters
    const std::string &getName() const { return name_...
```

**构造函数**:
- `ColumnDefinition`
- `ColumnDefinition`
- `setType`
- `setDefaultValue`

**析构函数**:
- `ColumnDefinition`

**公有方法**:
- `void setType`
- `void setDefaultValue`

---

