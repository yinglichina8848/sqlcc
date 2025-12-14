# 内存安全修正任务计划

## 目标
根据内存审计工具分析报告，修正核心源码5个高风险文件的内存安全问题，并验证编译测试。

## 任务清单
- [x] 读取内存审计工具分析报告
- [x] 分析高风险文件列表  
- [x] 识别具体的内存安全问题
- [ ] 修正 src/storage_engine/table_storage.cpp (26个问题) - 进行中
  - [x] 添加异常安全检查
  - [ ] 修正裸指针参数问题
  - [ ] 改进内存分配安全性
  - [ ] 验证函数完整性
- [ ] 修正 src/network/network.cpp (22个问题)
- [ ] 修正 src/storage_engine/buffer_pool_new.cpp (17个问题) 
- [ ] 修正 src/storage_engine/b_plus_tree.cpp (11个问题)
- [ ] 修正 src/storage_engine/disk_manager.cpp (10个问题)
- [ ] 编译核心代码验证
- [ ] 运行测试验证修正效果

## 高风险文件详情
1. **table_storage.cpp** - 裸指针参数和成员变量，数据页管理核心组件
2. **network.cpp** - 裸指针+文件描述符，网络通信模块
3. **buffer_pool_new.cpp** - new/delete操作，缓冲池管理
4. **b_plus_tree.cpp** - 裸指针+new/delete，索引系统
5. **disk_manager.cpp** - 文件描述符管理，磁盘I/O管理

## 开始时间
2025-12-12 05:47:14

## 当前进度
- 第一个文件(table_storage.cpp)正在进行异常安全改进
- 还需要修正4个高风险文件
- 编译验证和测试验证待完成
