《数据库原理》期末大作业：AI驱动的微型数据库系统开发
1. 核心定位
性质：个人或小组项目（2-4人，需明确分工并独立提交报告）

周期：4-8周（28天）

核心目标：从零开始，全程利用AI辅助，交付一个具备完整核心功能的可执行数据库系统，帮助学生初步掌握 AI 辅助的软件工程，深入理解数据库系统的原理和实现。

关键要求：必须自研存储引擎，禁止使用SQLite等现成库。

最终交付：源码、可执行程序、详细实验报告、5分钟演示视频。

2. 技术栈与环境约束（全部国产化）
    类别 可选方案
    开发语言 C / C++ / Java / Go (选择你最熟悉、AI训练语料最多的)
    AI-IDE 字节 Trae (多模态，无限时长) / 阿里 Qoder (Repo Wiki) / 腾讯 CodeBuddy (中文语义好)
    大模型 IDE内置模型 或 通义/混元/豆包API
    构建与测试 Maven, JUnit 5 / CppUnit, 阿里Java开发手册
    存储 自研页式文件管理，使用本地文件
3. 功能基线要求（必须通过自动化测试）
    模块 核心功能要求 自动化测试基准
    存储引擎 8KB定长页，空间管理，定/变长记录 10万次INSERT后，文件体积≤1.2倍理论值
    SQL解析 解析指定6种SQL语句，生成正确AST 全部测试SQL解析通过，AST打印正确
    索引 B+树，单字段唯一，支持=, >, <, 范围查询 100万主键下，点查页面访问≤4次
    CRUD 插入、点查、范围扫描、更新、删除 1-10万行数据，单操作耗时<5ms (SSD)
    事务 WAL(预写日志) + 两阶段锁，读已提交隔离级别 10线程并发转账1000次，无丢失更新
    工具 交互式命令行isql，支持-f执行脚本 执行1000行脚本无崩溃
4. 四周开发路线图
    周次 核心任务 AI辅助最佳实践（教程核心） 验收点
    W1: 环境与存储 
      1. 搭建环境
      2. 创建项目
      3. 实现页式文件管理 提示词示例：
         “用Java NIO编写一个8KB定长页的文件管理器，包含allocatePage和freePage方法，并提供JUnit测试用例。”
         技巧：让AI生成BufferPool类的骨架，并基于LRU算法实现页面淘汰。
      4. Git仓库
      5. 准备一个10分钟闪电演讲，介绍项目的成员，主要技术框架和路线和开发计划。
    W2: SQL与索引 
      1. 实现SQL解析器
      2. 实现B+树索引 提示词示例：
         “根据以下EBNF语法（粘贴教师提供的语法），生成ANTLR4的g4文件，并创建一个遍历AST的Visitor基类。”
         技巧：在Qoder中，使用Repo Wiki让AI理解整个项目结构，然后提问：“B+树的insert方法如何与我的Page类交互？” 
      3. 通过20条SQL解析测试
      4. 索引单测通过,实现 1M ~1G条数据的索引和测试
     W3: CRUD与事务 
      1. 实现执行器
      2. 实现WAL和锁管理器 提示词示例：
         “编写一个Java类，实现两阶段锁协议，使用ConcurrentHashMap管理S锁(共享锁)和X锁(排他锁)。提供简单的加锁/解锁接口。”
         技巧：将编译错误直接粘贴给AI，让它自我修正。 
      3. 通过CRUD性能测试
      4. 课堂isql转账DEMO
    W4: 集成与报告 
      1. 系统联调
      2. 撰写报告
      3. 录制视频 提示词示例：
         “为我生成一个20页的LaTeX实验报告模板，包含摘要、引言、系统架构图、ER图、事务时序图、性能评估和结论等章节。”
         技巧：使用AI将性能测试数据（如JMH输出）自动转换为LaTeX表格和图表。 
      4. 现场随机SQL测试
      5. 提交所有最终材料
核心任务分步教程与AI引导策略
第一步：环境搭建与项目初始化
AI引导：
提示词：“我是一名学生，要开始一个Java数据库项目。请为我创建一个标准的Maven项目结构，包含src/main/java, src/test/java目录，并在pom.xml中配置JUnit 5的依赖。”

可能问题：AI生成的pom.xml版本可能过旧。解决方案：检查并手动指定较新的版本号。

测试与评估：

运行mvn compile和mvn test，确保项目可以正常编译且测试通过（即使测试是空的）。

第二步：存储引擎实现（W1核心）
AI引导：

提示词：“请实现一个Page类，代表一个8KB的字节缓冲区。再实现一个DiskManager类，使用FileChannel将Page对象读写到磁盘文件‘data.db’中。要求支持按页ID随机读写。”

可能问题：AI可能不理解“自由空间管理”。解决方案：追加提示词：“请为Page类添加一个位图，用于追踪页内哪些槽位是空闲的，并实现insertRecord和deleteRecord方法。”

测试与评估：

单元测试：编写测试，创建100个页，写入后读出，验证数据一致性。

性能测试：运行教师下发的10万次INSERT测试，并使用ls -l检查最终文件大小是否符合要求。

第三步：SQL解析器实现（W2核心）
AI引导：

最佳路径：使用AI生成ANTLR4语法文件。

提示词：“以下是SQL子集的EBNF描述（粘贴EBNF）。请为我生成对应的ANTLR4语法文件(Sql.g4)，并生成Java词法分析和语法分析器。”

后续提示：“根据生成的语法，创建一个SqlVisitor实现，将SELECT ... FROM ... WHERE ...语句解析成一个自定义的SelectStatement对象，包含字段、表名和条件。”

测试与评估：

使用教师下发的20条测试SQL，逐条解析并打印AST或自定义语句对象，确保结构正确。

常见问题：语法歧义。解决方案：将出错的SQL样例和错误信息反馈给AI，要求它修正语法文件。

第四步：B+树索引实现（W2难点）
AI引导：

提示词：“请用Java实现一个B+树索引的核心结构。假设键是整数，值是长整型的页ID。请实现insert(key, value), search(key)和rangeSearch(startKey, endKey)方法。”

关键提示：“我的B+树节点需要存储到之前实现的8KB Page中。请帮我设计一个BPlusTreeLeafNode类，它能够序列化和反序列化到一个Page的字节数组中。”

测试与评估：

正确性：插入一系列键值对，然后进行点查和范围查询，验证结果是否正确。

性能：插入100万个随机主键，统计随机点查的页面访问次数（可通过Mock DiskManager或日志计算），确保≤4次。

第五步：事务实现（W3难点）
AI引导：

WAL提示词：“实现一个简单的预写日志（WAL）类。它有一个logTransaction方法，接收事务ID和一系列修改操作（页ID， 偏移量， 旧数据， 新数据），并将其追加到日志文件。还有一个recover方法，用于系统崩溃后重做已提交的事务。”

锁管理器提示词：“实现一个LockManager，提供acquireLock(transactionId, resource, lockType)和releaseLock(transactionId, resource)方法。锁类型分SHARED和EXCLUSIVE。需要处理锁兼容性和死锁预防（例如超时）。”

测试与评估：

并发测试：严格按照教师要求，编写10线程的随机转账测试。运行多次，检查最终总金额是否一致。

恢复测试：模拟系统崩溃（强制退出），然后重启并调用recover，检查数据是否恢复到一个一致的状态。

系统测试与评估完整指南
1. 分层测试策略
单元测试 (Unit Testing)：为每个核心类（Page, BPlusTree, LockManager）编写测试，覆盖正常和边界情况。

集成测试 (Integration Testing)：测试模块间的交互。例如，测试通过SQL解析器解析出的命令，能否正确调用执行器和索引完成操作。

系统测试 (System Testing)：运行教师下发的全套自动化测试脚本，这是评分的直接依据。

压力与并发测试 (Stress & Concurrency Testing)：自行构造远超基准要求的数据量（如1000万条）和高并发场景，观察系统是否稳定、性能是否平滑下降。

2. 性能评估方法
基准测试 (Benchmarking)：使用JMH（Java）或自定义脚本，对关键操作（INSERT, SELECT）进行量化测试。

** profiling**：使用JVisualVM或Async-Profiler等工具，找出性能瓶颈（是IO？是锁竞争？还是序列化？）。

AI辅助分析：将性能测试的原始数据或图表扔给AI，提示词：“请分析这份数据库性能测试报告，指出可能的性能瓶颈，并给出优化建议。”

3. 正确性验证方法
与SQLite对比：这是最有效的方法。让你的isql和SQLite命令行执行相同的DDL和DML脚本，然后对比查询结果是否完全一致。

模糊测试 (Fuzzing)：用AI生成大量随机的、语法正确的SQL语句，喂给你的数据库，检查是否会崩溃或返回错误结果。

一致性检查：在事务测试后，检查数据库的完整性约束是否仍然保持（如账户总金额不变）。

最终交付清单核对表
源码仓库：Gitee平台，main分支可编译 (mvn clean package)

可执行程序：db-1.0-SNAPSHOT.jar 及启动脚本 (isql.sh/bat)

实验报告 (PDF)：命名规范，包含详细的AI辅助记录章节

演示视频 (MP4)：≤100MB，5分钟，展示核心功能

AI辅助记录表：按要求填写AI-log.xlsx

通过遵循以上结构化的要求和教程，学生将能系统地利用AI作为强大的编程伙伴，不仅顺利完成大作业，更能深刻理解数据库内核的工作原理。
#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
