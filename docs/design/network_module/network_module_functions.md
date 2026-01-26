# Network Module 函数设计文档

## 函数列表

### max_connections_

**定义位置**: `src/network/server_network_manager.cpp`

**签名**:
```cpp
      max_connections_(max_connections),
      running_(false),
      session_manager_(std::make_sha...
```

---

### signalHandler

**定义位置**: `src/network/server_main.cpp`

**签名**:
```cpp
void signalHandler(int signal) {
```

---

### main

**定义位置**: `src/network/server_main.cpp`

**签名**:
```cpp

int main(int argc, char* argv[]) {
```

---

### authenticated_

**定义位置**: `src/network/session.cpp`

**签名**:
```cpp
      authenticated_(false),
      encryption_disabled_(false),
      authentication_disabled_(false...
```

---

### session_manager_

**定义位置**: `src/network/network.cpp`

**签名**:
```cpp
      session_manager_(std::make_shared<SessionManager>()),
      user_manager_(std::make_shared<::s...
```

---

### max_log_entries_

**定义位置**: `src/network/network_exception_handler.cpp`

**签名**:
```cpp
      max_log_entries_(1000),
      performance_monitoring_enabled_(true),
      start_time_(std::ch...
```

---

### max_throughput_

**定义位置**: `src/network/network_exception_handler.cpp`

**签名**:
```cpp
      max_throughput_(100 * 1024 * 1024), // 100MB/s
      max_exception_rate_(1.0), // 1 exception ...
```

---

### state_change_callback_

**定义位置**: `src/network/connection_state_machine.cpp`

**签名**:
```cpp
      state_change_callback_(nullptr),
      connect_timeout_(std::chrono::seconds(30)),
      recon...
```

---

### max_buffer_size_

**定义位置**: `src/network/data_transmission_validator.cpp`

**签名**:
```cpp
      max_buffer_size_(128 * 1024 * 1024),     // 128MB
      fragment_size_(1024 * 1024),          ...
```

---

### MessageHeader

**定义位置**: `src/network/data_transmission_validator.h`

**签名**:
```cpp

    MessageHeader() : magic(0), version(0), type(0), length(0), sequence(0), checksum(0) {
```

---

### NetworkRequest

**定义位置**: `include/network/multi_threaded_network_manager.h`

**签名**:
```cpp

  NetworkRequest(int id, const std::string& data, std::shared_ptr<ConnectionState> conn)
      : cl...
```

---

### NetworkResponse

**定义位置**: `include/network/multi_threaded_network_manager.h`

**签名**:
```cpp

  NetworkResponse(int id, const std::string& data, bool ok = true)
      : client_id(id), response_...
```

---

### KeyRotationPolicy

**定义位置**: `include/network/key_rotation_policy.h`

**签名**:
```cpp
    explicit KeyRotationPolicy(size_t interval_messages = 1000)
        : interval_(interval_message...
```

---

### ConnectionStateMachine

**定义位置**: `include/network/connection_state.h`

**签名**:
```cpp
    ConnectionStateMachine()
        : current_state_(ConnectionState::DISCONNECTED),
          targ...
```

---

### transition

**定义位置**: `include/network/connection_state.h`

**签名**:
```cpp
    bool transition(ConnectionEvent event) {
```

---

### set_target_state

**定义位置**: `include/network/connection_state.h`

**签名**:
```cpp
    void set_target_state(ConnectionState target_state) {
```

---

### wait_for_state_change

**定义位置**: `include/network/connection_state.h`

**签名**:
```cpp
    bool wait_for_state_change(ConnectionState expected_state, 
                               std::...
```

---

### set_error

**定义位置**: `include/network/connection_state.h`

**签名**:
```cpp
    void set_error(const std::string& error_message) {
```

---

### reset

**定义位置**: `include/network/connection_state.h`

**签名**:
```cpp
    void reset() {
```

---

### is_valid_transition

**定义位置**: `include/network/connection_state.h`

**签名**:
```cpp
    bool is_valid_transition(ConnectionState current, ConnectionEvent event) const {
        switch ...
```

---

### get_next_state

**定义位置**: `include/network/connection_state.h`

**签名**:
```cpp
    ConnectionState get_next_state(ConnectionState current, ConnectionEvent event) const {
        i...
```

---

### ConnectionTimeoutManager

**定义位置**: `include/network/connection_state.h`

**签名**:
```cpp
    explicit ConnectionTimeoutManager(std::chrono::milliseconds connect_timeout = 
                 ...
```

---

### set_connect_timeout

**定义位置**: `include/network/connection_state.h`

**签名**:
```cpp
    void set_connect_timeout(std::chrono::milliseconds timeout) {
        if (timeout <= std::chrono...
```

---

### set_authentication_timeout

**定义位置**: `include/network/connection_state.h`

**签名**:
```cpp
    void set_authentication_timeout(std::chrono::milliseconds timeout) {
        if (timeout <= std:...
```

---

### update_activity_time

**定义位置**: `include/network/connection_state.h`

**签名**:
```cpp
    void update_activity_time() {
```

---

### set_timeout_enabled

**定义位置**: `include/network/connection_state.h`

**签名**:
```cpp
    void set_timeout_enabled(bool enabled) {
```

---

