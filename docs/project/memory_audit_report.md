# SQLCC项目内存审计报告

## 基本信息

- **生成时间**: 2025-12-12 04:34:52
- **审计文件总数**: 137
- **影响文件数**: 23
- **项目根目录**: src/

## 审计结果

### 问题统计

| 文件路径 | 问题数量 | 主要问题类型 |
|---------|---------|-------------|
| `src/bin/isql_main.cpp` | 2 | 裸指针(2)  |
| `src/core/database_manager.cpp` | 1 | 裸指针(1)  |
| `src/dcl_executor.cpp` | 1 | 裸指针(1)  |
| `src/execution/set_operation_executor.cpp` | 2 | 裸指针(2)  |
| `src/isql_network/client_main.cpp` | 9 | 裸指针(9)  |
| `src/isql_network/demo_client.cpp` | 10 | 裸指针(10)  |
| `src/network/encryption.cpp` | 2 | 裸指针(2)  |
| `src/network/network.cpp` | 22 | 裸指针(16) 文件描述符(6)  |
| `src/sql_parser/ast/source_location.cpp` | 1 | 裸指针(1)  |
| `src/sql_parser/lexer_new.cpp` | 1 | new/delete(1)  |
| `src/sql_parser/token.cpp` | 2 | 裸指针(2)  |
| `src/sql_parser/token_new.cpp` | 2 | 裸指针(2)  |
| `src/sqlcc_server/demo_server.cpp` | 1 | 裸指针(1)  |
| `src/sqlcc_server/server_main.cpp` | 1 | 裸指针(1)  |
| `src/storage_engine/b_plus_tree.cpp` | 11 | 裸指针(8) new/delete(3)  |
| `src/storage_engine/buffer_pool.cpp` | 10 | 裸指针(5) new/delete(5)  |
| `src/storage_engine/buffer_pool_new.cpp` | 17 | 裸指针(5) new/delete(12)  |
| `src/storage_engine/buffer_pool_sharded.cpp` | 1 | new/delete(1)  |
| `src/storage_engine/disk_manager.cpp` | 10 | 裸指针(3) new/delete(2) 文件描述符(5)  |
| `src/storage_engine/replace_strategy.cpp` | 2 | 裸指针(2)  |
| `src/storage_engine/storage_engine.cpp` | 1 | 裸指针(1)  |
| `src/storage_engine/table_storage.cpp` | 26 | 裸指针(25) new/delete(1)  |
| `src/unified_executor.cpp` | 2 | 裸指针(1) new/delete(1)  |

### 详细问题列表

#### 📁 src/bin/isql_main.cpp

**问题数量**: 2

**问题 1**:

```
src/bin/isql_main.cpp:155: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char *argv[]) {
```

**问题 2**:

```
src/bin/isql_main.cpp:213: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       char *line = readline("");
```

#### 📁 src/core/database_manager.cpp

**问题数量**: 1

**问题 1**:

```
src/core/database_manager.cpp:424: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:                                 Page *page) {
```

#### 📁 src/dcl_executor.cpp

**问题数量**: 1

**问题 1**:

```
src/dcl_executor.cpp:130: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: ExecutionResult DCLExecutor::executeRevoke(sql_parser::RevokeStatement* stmt) {
```

#### 📁 src/execution/set_operation_executor.cpp

**问题数量**: 2

**问题 1**:

```
src/execution/set_operation_executor.cpp:16: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       memory_limit_(1024 * 1024 * 1024) { // 默认1GB内存限制
```

**问题 2**:

```
src/execution/set_operation_executor.cpp:126: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: SetOperationExecutor::execute_subquery(SelectStatement *subquery) {
```

#### 📁 src/isql_network/client_main.cpp

**问题数量**: 9

**问题 1**:

```
src/isql_network/client_main.cpp:23: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char* argv[]) {
```

**问题 2**:

```
src/isql_network/client_main.cpp:85: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string query = "SELECT * FROM test_table";
```

**问题 3**:

```
src/isql_network/client_main.cpp:116: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* response_header = reinterpret_cast<MessageHeader*>(response.data());
```

**问题 4**:

```
src/isql_network/client_main.cpp:146: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* header = reinterpret_cast<MessageHeader*>(connect_msg.data());
```

**问题 5**:

```
src/isql_network/client_main.cpp:165: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* resp_header = reinterpret_cast<MessageHeader*>(connect_resp.data());
```

**问题 6**:

```
src/isql_network/client_main.cpp:174: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         size_t msg_len = 2 * sizeof(uint32_t) + user_len + pass_len;
```

**问题 7**:

```
src/isql_network/client_main.cpp:184: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         char* body = auth_msg.data() + sizeof(MessageHeader);
```

**问题 8**:

```
src/isql_network/client_main.cpp:187: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::memcpy(body + 2 * sizeof(uint32_t), username.c_str(), user_len);
```

**问题 9**:

```
src/isql_network/client_main.cpp:188: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::memcpy(body + 2 * sizeof(uint32_t) + user_len, password.c_str(), pass_len);
```

#### 📁 src/isql_network/demo_client.cpp

**问题数量**: 10

**问题 1**:

```
src/isql_network/demo_client.cpp:20: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char* argv[]) {
```

**问题 2**:

```
src/isql_network/demo_client.cpp:82: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string query = "SELECT * FROM test_table";
```

**问题 3**:

```
src/isql_network/demo_client.cpp:113: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* response_header = reinterpret_cast<MessageHeader*>(response.data());
```

**问题 4**:

```
src/isql_network/demo_client.cpp:143: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* header = reinterpret_cast<MessageHeader*>(connect_msg.data());
```

**问题 5**:

```
src/isql_network/demo_client.cpp:162: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* resp_header = reinterpret_cast<MessageHeader*>(connect_resp.data());
```

**问题 6**:

```
src/isql_network/demo_client.cpp:171: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         size_t msg_len = 2 * sizeof(uint32_t) + user_len + pass_len;
```

**问题 7**:

```
src/isql_network/demo_client.cpp:181: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         char* body = auth_msg.data() + sizeof(MessageHeader);
```

**问题 8**:

```
src/isql_network/demo_client.cpp:184: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::memcpy(body + 2 * sizeof(uint32_t), username.c_str(), user_len);
```

**问题 9**:

```
src/isql_network/demo_client.cpp:185: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         std::memcpy(body + 2 * sizeof(uint32_t) + user_len, password.c_str(), pass_len);
```

**问题 10**:

```
src/isql_network/demo_client.cpp:199: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* auth_resp_header = reinterpret_cast<MessageHeader*>(auth_resp.data());
```

#### 📁 src/network/encryption.cpp

**问题数量**: 2

**问题 1**:

```
src/network/encryption.cpp:104: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
```

**问题 2**:

```
src/network/encryption.cpp:150: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
```

#### 📁 src/network/network.cpp

**问题数量**: 22

**问题 1**:

```
src/network/network.cpp:367: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     size_t body_size = 2 * sizeof(uint32_t) + username_len + password_len;
```

**问题 2**:

```
src/network/network.cpp:383: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::memcpy(body_span.data() + 2 * sizeof(uint32_t), username.c_str(), username_len);
```

**问题 3**:

```
src/network/network.cpp:384: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::memcpy(body_span.data() + 2 * sizeof(uint32_t) + username_len, password.c_str(), password_len);
```

**问题 4**:

```
src/network/network.cpp:516: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int flags = fcntl(fd_.get(), F_GETFL, 0);
```

**问题 5**:

```
src/network/network.cpp:523: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: void ConnectionHandler::SetTLS(struct ssl_st* ssl, bool enabled) {
```

**问题 6**:

```
src/network/network.cpp:622: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* msg_header = reinterpret_cast<MessageHeader*>(to_send.data());
```

**问题 7**:

```
src/network/network.cpp:624: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* header = reinterpret_cast<MessageHeader*>(to_send.data());
```

**问题 8**:

```
src/network/network.cpp:667: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
```

**问题 9**:

```
src/network/network.cpp:720: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:         MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
```

**问题 10**:

```
src/network/network.cpp:754: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
```

**问题 11**:

```
src/network/network.cpp:762: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     if (header->length < 2 * sizeof(uint32_t)) {
```

**问题 12**:

```
src/network/network.cpp:769: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     if (header->length != 2 * sizeof(uint32_t) + username_len + password_len) {
```

**问题 13**:

```
src/network/network.cpp:773: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string username(body + 2 * sizeof(uint32_t), username_len);
```

**问题 14**:

```
src/network/network.cpp:774: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     std::string password(body + 2 * sizeof(uint32_t) + username_len, password_len);
```

**问题 15**:

```
src/network/network.cpp:805: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
```

**问题 16**:

```
src/network/network.cpp:837: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
```

**问题 17**:

```
src/network/network.cpp:1017: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int nfds = epoll_wait(epoll_fd_.get(), events, 64, 0);
```

**问题 18**:

```
src/network/network.cpp:1019: 文件描述符直接使用 - 建议封装为RAII类
  代码:     for (int i = 0; i < nfds; i++) {
```

**问题 19**:

```
src/network/network.cpp:1025: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:             ConnectionHandler* handler = static_cast<ConnectionHandler*>(events[i].data.ptr);
```

**问题 20**:

```
src/network/network.cpp:1030: 文件描述符直接使用 - 建议封装为RAII类
  代码:                 int fd = handler->GetFd();
```

**问题 21**:

```
src/network/network.cpp:1089: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int fd = handler->GetFd(); // 获取文件描述符值用于epoll
```

**问题 22**:

```
src/network/network.cpp:1098: 文件描述符直接使用 - 建议封装为RAII类
  代码:         int flags = fcntl(fd, F_GETFL, 0);
```

#### 📁 src/sql_parser/ast/source_location.cpp

**问题数量**: 1

**问题 1**:

```
src/sql_parser/ast/source_location.cpp:20: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     if (!other.isValid()) return *this;
```

#### 📁 src/sql_parser/lexer_new.cpp

**问题数量**: 1

**问题 1**:

```
src/sql_parser/lexer_new.cpp:402: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       // Transition to new state
```

#### 📁 src/sql_parser/token.cpp

**问题数量**: 2

**问题 1**:

```
src/sql_parser/token.cpp:26: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     return *this;
```

**问题 2**:

```
src/sql_parser/token.cpp:51: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     return *this;
```

#### 📁 src/sql_parser/token_new.cpp

**问题数量**: 2

**问题 1**:

```
src/sql_parser/token_new.cpp:26: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     return *this;
```

**问题 2**:

```
src/sql_parser/token_new.cpp:51: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     return *this;
```

#### 📁 src/sqlcc_server/demo_server.cpp

**问题数量**: 1

**问题 1**:

```
src/sqlcc_server/demo_server.cpp:36: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char* argv[]) {
```

#### 📁 src/sqlcc_server/server_main.cpp

**问题数量**: 1

**问题 1**:

```
src/sqlcc_server/server_main.cpp:29: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: int main(int argc, char* argv[]) {
```

#### 📁 src/storage_engine/b_plus_tree.cpp

**问题数量**: 11

**问题 1**:

```
src/storage_engine/b_plus_tree.cpp:44: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:  * BPlusTreeNode *node = new BPlusTreeNode(storage_engine, page_id, is_leaf);
```

**问题 2**:

```
src/storage_engine/b_plus_tree.cpp:44: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:  * BPlusTreeNode *node = new BPlusTreeNode(storage_engine, page_id, is_leaf);
```

**问题 3**:

```
src/storage_engine/b_plus_tree.cpp:127: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:  * BPlusTreeInternalNode *internal_node = new
```

**问题 4**:

```
src/storage_engine/b_plus_tree.cpp:191: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page_->GetData();
```

**问题 5**:

```
src/storage_engine/b_plus_tree.cpp:242: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page_->GetData();
```

**问题 6**:

```
src/storage_engine/b_plus_tree.cpp:465: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:  * BPlusTreeLeafNode *leaf_node = new BPlusTreeLeafNode(storage_engine,
```

**问题 7**:

```
src/storage_engine/b_plus_tree.cpp:465: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:  * BPlusTreeLeafNode *leaf_node = new BPlusTreeLeafNode(storage_engine,
```

**问题 8**:

```
src/storage_engine/b_plus_tree.cpp:528: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page_->GetData();
```

**问题 9**:

```
src/storage_engine/b_plus_tree.cpp:579: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page_->GetData();
```

**问题 10**:

```
src/storage_engine/b_plus_tree.cpp:893: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:  * BPlusTreeIndex *index = new BPlusTreeIndex(storage_engine, table_name,
```

**问题 11**:

```
src/storage_engine/b_plus_tree.cpp:893: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:  * BPlusTreeIndex *index = new BPlusTreeIndex(storage_engine, table_name,
```

#### 📁 src/storage_engine/buffer_pool.cpp

**问题数量**: 10

**问题 1**:

```
src/storage_engine/buffer_pool.cpp:116: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char* page_data = static_cast<char*>(page->GetData());
```

**问题 2**:

```
src/storage_engine/buffer_pool.cpp:497: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = page_it->second.get();
```

**问题 3**:

```
src/storage_engine/buffer_pool.cpp:678: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       Page *page = page_it->second.get();
```

**问题 4**:

```
src/storage_engine/buffer_pool.cpp:679: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:       void *page_data = page->GetData();
```

**问题 5**:

```
src/storage_engine/buffer_pool.cpp:712: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:               std::chrono::milliseconds(50 * relock_attempts));
```

**问题 6**:

```
src/storage_engine/buffer_pool.cpp:847: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     SQLCC_LOG_WARN("Failed to acquire buffer pool lock for creating new page, "
```

**问题 7**:

```
src/storage_engine/buffer_pool.cpp:863: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:         "Buffer pool is full, replacing page for new page allocation");
```

**问题 8**:

```
src/storage_engine/buffer_pool.cpp:876: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:           "Failed to replace page in buffer pool for new page allocation";
```

**问题 9**:

```
src/storage_engine/buffer_pool.cpp:895: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   std::string error_msg = "Failed to allocate new page from disk manager";
```

**问题 10**:

```
src/storage_engine/buffer_pool.cpp:904: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   SQLCC_LOG_DEBUG("Allocated new page ID " + std::to_string(*page_id) +
```

#### 📁 src/storage_engine/buffer_pool_new.cpp

**问题数量**: 17

**问题 1**:

```
src/storage_engine/buffer_pool_new.cpp:39: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: BufferPool::BufferPool(DiskManager *disk_manager, size_t pool_size,
```

**问题 2**:

```
src/storage_engine/buffer_pool_new.cpp:77: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: Page *BufferPool::FetchPage(int32_t page_id) {
```

**问题 3**:

```
src/storage_engine/buffer_pool_new.cpp:114: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   // Create new page and load from disk
```

**问题 4**:

```
src/storage_engine/buffer_pool_new.cpp:182: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码: // Create a new page
```

**问题 5**:

```
src/storage_engine/buffer_pool_new.cpp:183: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: Page *BufferPool::NewPage(int32_t *page_id) {
```

**问题 6**:

```
src/storage_engine/buffer_pool_new.cpp:186: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     SQLCC_LOG_WARN("Failed to acquire buffer pool lock for creating new page");
```

**问题 7**:

```
src/storage_engine/buffer_pool_new.cpp:194: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       SQLCC_LOG_ERROR("No pages available for eviction when creating new page");
```

**问题 8**:

```
src/storage_engine/buffer_pool_new.cpp:198: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     if (!ReplacePage(victim_id, -1)) { // -1 means we'll allocate a new page ID
```

**问题 9**:

```
src/storage_engine/buffer_pool_new.cpp:205: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     SQLCC_LOG_ERROR("Failed to allocate new page ID from disk manager");
```

**问题 10**:

```
src/storage_engine/buffer_pool_new.cpp:247: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = it->second;
```

**问题 11**:

```
src/storage_engine/buffer_pool_new.cpp:473: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码: // Replace a page with a new one
```

**问题 12**:

```
src/storage_engine/buffer_pool_new.cpp:491: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *victim_page = victim_it->second;
```

**问题 13**:

```
src/storage_engine/buffer_pool_new.cpp:529: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   // If new_page_id is valid (not -1), we need to load the new page
```

**问题 14**:

```
src/storage_engine/buffer_pool_new.cpp:531: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     // Create new page and load from disk
```

**问题 15**:

```
src/storage_engine/buffer_pool_new.cpp:541: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       SQLCC_LOG_ERROR("Failed to reacquire lock after loading new page");
```

**问题 16**:

```
src/storage_engine/buffer_pool_new.cpp:546: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       SQLCC_LOG_ERROR("Failed to read new page " + std::to_string(new_page_id) +
```

**问题 17**:

```
src/storage_engine/buffer_pool_new.cpp:551: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     // Add new page to buffer pool
```

#### 📁 src/storage_engine/buffer_pool_sharded.cpp

**问题数量**: 1

**问题 1**:

```
src/storage_engine/buffer_pool_sharded.cpp:198: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:       SQLCC_LOG_ERROR("Failed to replace page for new page creation");
```

#### 📁 src/storage_engine/disk_manager.cpp

**问题数量**: 10

**问题 1**:

```
src/storage_engine/disk_manager.cpp:57: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     SQLCC_LOG_INFO("Database file does not exist, creating new file: " +
```

**问题 2**:

```
src/storage_engine/disk_manager.cpp:125: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:  *       2. 计算偏移量：page_id * PAGE_SIZE
```

**问题 3**:

```
src/storage_engine/disk_manager.cpp:279: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: bool DiskManager::ReadPage(int32_t page_id, char *page_data) {
```

**问题 4**:

```
src/storage_engine/disk_manager.cpp:409: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:   SQLCC_LOG_DEBUG("Allocated new page ID: " + std::to_string(page_id) +
```

**问题 5**:

```
src/storage_engine/disk_manager.cpp:510: 文件描述符直接使用 - 建议封装为RAII类
  代码:   int fd = open(db_file_name_.c_str(), O_RDONLY);
```

**问题 6**:

```
src/storage_engine/disk_manager.cpp:522: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     char *data = pair.second;
```

**问题 7**:

```
src/storage_engine/disk_manager.cpp:566: 文件描述符直接使用 - 建议封装为RAII类
  代码:   int fd = open(db_file_name_.c_str(), O_RDONLY);
```

**问题 8**:

```
src/storage_engine/disk_manager.cpp:577: 文件描述符直接使用 - 建议封装为RAII类
  代码:   int result = posix_fadvise(fd, offset, PAGE_SIZE, POSIX_FADV_WILLNEED);
```

**问题 9**:

```
src/storage_engine/disk_manager.cpp:617: 文件描述符直接使用 - 建议封装为RAII类
  代码:   int fd = open(db_file_name_.c_str(), O_RDONLY);
```

**问题 10**:

```
src/storage_engine/disk_manager.cpp:642: 文件描述符直接使用 - 建议封装为RAII类
  代码:     int result = posix_fadvise(fd, offset, size, POSIX_FADV_WILLNEED);
```

#### 📁 src/storage_engine/replace_strategy.cpp

**问题数量**: 2

**问题 1**:

```
src/storage_engine/replace_strategy.cpp:140: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     a1in_capacity_ = static_cast<size_t>(a1in_capacity_ * scale);
```

**问题 2**:

```
src/storage_engine/replace_strategy.cpp:141: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     am_capacity_ = static_cast<size_t>(am_capacity_ * scale);
```

#### 📁 src/storage_engine/storage_engine.cpp

**问题数量**: 1

**问题 1**:

```
src/storage_engine/storage_engine.cpp:60: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: Page* StorageEngine::FetchPage(int32_t page_id) {
```

#### 📁 src/storage_engine/table_storage.cpp

**问题数量**: 26

**问题 1**:

```
src/storage_engine/table_storage.cpp:118: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = AllocateNewPage(table_name);
```

**问题 2**:

```
src/storage_engine/table_storage.cpp:120: 直接使用new操作符 - 建议使用std::make_unique或std::make_shared
  代码:     SQLCC_LOG_ERROR("Failed to allocate new page for table: " + table_name);
```

**问题 3**:

```
src/storage_engine/table_storage.cpp:145: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = storage_engine_->FetchPage(page_id);
```

**问题 4**:

```
src/storage_engine/table_storage.cpp:170: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = storage_engine_->FetchPage(page_id);
```

**问题 5**:

```
src/storage_engine/table_storage.cpp:196: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page *page = storage_engine_->FetchPage(page_id);
```

**问题 6**:

```
src/storage_engine/table_storage.cpp:244: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:     Page *page = storage_engine_->FetchPage(page_id);
```

**问题 7**:

```
src/storage_engine/table_storage.cpp:270: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   Page* page = page_ptr.release();  // 释放unique_ptr的所有权
```

**问题 8**:

```
src/storage_engine/table_storage.cpp:278: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: bool TableStorageManager::InitializePage(Page *page,
```

**问题 9**:

```
src/storage_engine/table_storage.cpp:297: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

**问题 10**:

```
src/storage_engine/table_storage.cpp:350: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

**问题 11**:

```
src/storage_engine/table_storage.cpp:365: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: bool TableStorageManager::DeleteRecordInPage(Page *page, size_t offset) {
```

**问题 12**:

```
src/storage_engine/table_storage.cpp:366: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

**问题 13**:

```
src/storage_engine/table_storage.cpp:386: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

**问题 14**:

```
src/storage_engine/table_storage.cpp:417: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

**问题 15**:

```
src/storage_engine/table_storage.cpp:425: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(&header.next_page_id, data + sizeof(PageType) + 2 * sizeof(int32_t),
```

**问题 16**:

```
src/storage_engine/table_storage.cpp:428: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:          data + sizeof(PageType) + 3 * sizeof(int32_t), sizeof(uint16_t));
```

**问题 17**:

```
src/storage_engine/table_storage.cpp:430: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:          data + sizeof(PageType) + 3 * sizeof(int32_t) + sizeof(uint16_t),
```

**问题 18**:

```
src/storage_engine/table_storage.cpp:433: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:          data + sizeof(PageType) + 3 * sizeof(int32_t) + 2 * sizeof(uint16_t),
```

**问题 19**:

```
src/storage_engine/table_storage.cpp:436: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:          data + sizeof(PageType) + 3 * sizeof(int32_t) + 3 * sizeof(uint16_t),
```

**问题 20**:

```
src/storage_engine/table_storage.cpp:442: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码: void TableStorageManager::WritePageHeader(Page *page,
```

**问题 21**:

```
src/storage_engine/table_storage.cpp:444: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   char *data = page->GetData();
```

**问题 22**:

```
src/storage_engine/table_storage.cpp:451: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(data + sizeof(PageType) + 2 * sizeof(int32_t), &header.next_page_id,
```

**问题 23**:

```
src/storage_engine/table_storage.cpp:453: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(data + sizeof(PageType) + 3 * sizeof(int32_t),
```

**问题 24**:

```
src/storage_engine/table_storage.cpp:455: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(data + sizeof(PageType) + 3 * sizeof(int32_t) + sizeof(uint16_t),
```

**问题 25**:

```
src/storage_engine/table_storage.cpp:457: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(data + sizeof(PageType) + 3 * sizeof(int32_t) + 2 * sizeof(uint16_t),
```

**问题 26**:

```
src/storage_engine/table_storage.cpp:459: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   memcpy(data + sizeof(PageType) + 3 * sizeof(int32_t) + 3 * sizeof(uint16_t),
```

#### 📁 src/unified_executor.cpp

**问题数量**: 2

**问题 1**:

```
src/unified_executor.cpp:1232: 直接使用delete操作符 - 建议使用RAII模式自动管理内存
  代码:     return {false, "Invalid delete statement"};
```

**问题 2**:

```
src/unified_executor.cpp:1914: 裸指针声明 - 建议使用std::unique_ptr或std::shared_ptr
  代码:   ExecutionStrategy *strategy = getStrategy(stmt->getType());
```

## 改进建议

### 1. 智能指针使用原则
- 使用 `std::unique_ptr` 替代裸指针进行独占所有权管理
- 使用 `std::shared_ptr` 替代裸指针进行共享所有权管理
- 使用 `std::make_unique` 和 `std::make_shared` 替代直接使用 `new`

### 2. RAII资源管理
- 使用 RAII 模式管理资源，避免直接使用 `delete`
- 将文件描述符等系统资源封装为 RAII 类
- 实现异常安全的资源管理

### 3. 重构优先级
1. **高优先级**: 裸指针成员变量（内存泄漏风险）
2. **中优先级**: 文件描述符直接使用（资源泄漏风险）
3. **低优先级**: 函数参数裸指针（接口兼容性考虑）

### 4. 验证机制
- 启用 ASan/LSan 进行运行时内存检查
- 定期运行内存审计工具监控改进效果
- 建立自动化测试确保重构质量

