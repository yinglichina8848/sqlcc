《数据库原理》期末大作业：AI驱动的微型数据库系统开发--完整指南

本指南记录咨询AI以完成 数据库原理期末作业的全部过程，方便新手进行学习：
一. Win11  开发环境准备

   安装 TRAE，参考以下文章，或者自行搜索教程：
   https://www.cnblogs.com/aidigitialfuture/p/18875823/trae-domestic-version-installation-tutorial-1jadig

   安装 WSL 环境+Ubuntu，准备类 Ubuntu 的开发环境（个人爱好，习惯 纯Windows 环境的，可以跳过，但是要自行准备 C++/Java 等开发环境）
   这部分的教程很多，请自行搜索和安装。

二. 项目准备
   登录 Gitee，完成个人身份注册（手机号+验证码即可），创建一个软件仓库，我本人定义的是 sqlcc，准备用C++开发一个简单的SQL数据库。

   咨询Kimi，如何实现本地开发环境和 Gitee 的同步，及Kimi的回答 ：
------------------------------------------------------------------------------------------------------------------------------------------ 
   “准备采用 C/C++ 完成数据库系统的开发，本地环境：Win11 WSL +Ubuntu 24.04 ，TRAE 的开发工具，Gitee上定义了仓库：https://gitee.com/yinglichina/sqlcc 
怎么配置本地项目，并和 Gitee 同步？”
------------------------------------------------------------------------------------------------------------------------------------------ 
下面给出一份“从零到可编译、可推送”的完整流程，全部在 Win11 WSL + Ubuntu 24.04 里完成。
照着做 10 分钟就能跑通：
本地目录建好
CMake 工程能编译
第一次 commit & push 到 Gitee
后续只需 git pull / git push 即可同步。
一、WSL 环境一次性准备
1. 升级系统并装常用工具
sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential gdb cmake git ninja-clang-format clang-tidy
2. 配置 Git 全局身份（已配过可跳过）
git config --global user.name "你的名字"
git config --global user.email "你的邮箱@xxx.com"
3. 生成 SSH key 并绑定 Gitee
ssh-keygen -t ed25519 -C "你的邮箱@xxx.com"   # 一路回车
cat ~/.ssh/id_ed25519.pub                     # 复制输出
登录 Gitee → 设置 → SSH 公钥 → 新增，粘贴保存
测试连通
ssh -T git@gitee.com   # 看到 “Hi xxx, You’ve successfully authenticated” 即可
二、Clone 空仓库到本地
进入你的常用工作区（比如 ~/proj）
mkdir -p ~/proj && cd ~/proj
git clone git@gitee.com:yinglichina/sqlcc.git
cd sqlcc
此时目录里只有 .git 和 README.md
三、建立 CMake C++ 工程骨架
1. 直接一键生成标准布局
mkdir -p {src,include,tests,docs,scripts}
touch README.md .gitignore CMakeLists.txt
2. 写 .gitignore（防止把 build 产物提交）
cat > .gitignore <<'EOF'
/build/
/cmake-build-*/
/.vscode/
/.idea/
*.o
*.a
*.so
*.exe
compile_commands.json
EOF
3. 写根目录 CMakeLists.txt（C++20，警告全开）
cat > CMakeLists.txt <<'EOF'
cmake_minimum_required(VERSION 3.22)
project(sqlcc VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # 给 clangd/trae 补全用
警告集合
add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion)
include_directories(${CMAKE_SOURCE_DIR}/include)
核心静态库
add_subdirectory(src)
可执行文件
add_executable(sqlcc_cli main.cc)
target_link_libraries(sqlcc_cli PRIVATE sqlcc_core)
单元测试（可选）
enable_testing()
add_subdirectory(tests)
EOF
4. src 目录
cd src
cat > CMakeLists.txt <<'EOF'
add_library(sqlcc_core STATIC
buffer_pool.cc
disk_manager.cc
bpt.cc
index.cc
record.cc
sql_parser.cc
)
target_include_directories(sqlcc_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
EOF
5. 先放一个最小 main.cc 能编译过
cd ~/proj/sqlcc
cat > main.cc <<'EOF'
#include <iostream>
6. 放一个版本头
cat > include/version.h <<'EOF'
#pragma once
#define SQLCC_VERSION "0.1.0"
EOF
四、第一次本地编译
1. 新建 build 目录（out-of-source）
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
2. 运行
./build/sqlcc_cli
输出：SqlCC 0.1.0 startup! 说明链路通了
五、提交并推送到 Gitee
git add .
git commit -m "feat: init CMake skeleton, CLI prints version"
git push origin main        # 或 master，看仓库默认分支名
六、TRAE 开发工具无缝衔接
用 TRAE 打开 \\wsl$\Ubuntu-24.04\home\<user>\proj\sqlcc 即可，
因为 compile_commands.json 已生成，补全、跳转、clang-tidy 实时检查全部生效。
若 TRAE 找不到 clangd，可在 settings.json 里加：
"clangd.path": "/usr/bin/clangd"
七、后续开发节奏（建议脚本化）
一键 rebuild & test
cat > scripts/dev.sh <<'EOF'
#!/usr/bin/env bash
set -e
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
EOF
chmod +x scripts/dev.sh
每天写完代码：
./scripts/dev.sh && git add -A && git commit -m "xxx" && git push
八、小组协作建议
分支模型
main（保护分支）← develop ← feature/xxx
每人每天 git pull --rebase origin main 保持最新。
分工文件级避免冲突
比如 A 做 buffer_pool.，B 做 bpt.，C 做 sql_parser.*，提前在 CMakeLists.txt 里占位。
Code Review
Gitee 上提 Pull Request，至少 1 人 review 后合并，保证质量。
九、常见坑速查
WSL 路径大小写不敏感，但 Linux 区分，注意 #include 大小写。
如果 push 时提示 “permission denied”，先 ssh-add ~/.ssh/id_ed25519。
若 Windows 侧也装 Git，确保 WSL 内外的 global config 一致，避免 author 错乱。
到此，本地项目与 Gitee 已完全同步，随时可写新模块、提交、推送。
下一步就是真正开始写存储引擎啦，祝期末大作业一次通过！
------------------------------------------------------------------------------------------------------------------------------------------ 
以上内容有待验证，第一步大体结束。
