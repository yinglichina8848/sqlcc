# Transaction Manager 函数设计文档

## 函数列表

### timestamp_

**定义位置**: `src/transaction/savepoint_manager.cpp`

**签名**:
```cpp
      timestamp_(std::chrono::system_clock::now()) {}

Savepoint::~Savepoint() {}

void Savepoint::a...
```

---

