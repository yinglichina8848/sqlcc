#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <fstream>
#include <mutex>
#include "src/utils/config_manager.h"

using namespace std;
using namespace std::chrono_literals;

int main() {
    cout << "Simple ConfigManager Test" << endl;
    
    // 创建示例配置文件
    const std::string config_file = "/tmp/simple_test_config.ini";
    std::ofstream file(config_file);
    file << "[database]\n";
    file << "host=localhost\n";
    file << "port=5432\n";
    file.close();
    
    // 获取ConfigManager单例
    auto& config = sqlcc::ConfigManager::GetInstance();
    
    // 简单测试 - 加载配置并读取
    config.LoadConfig(config_file);
    
    string host = config.GetString("database.host");
    int port = config.GetInt("database.port");
    
    cout << "Read config - host: " << host << ", port: " << port << endl;
    
    // 并发测试 - 启动少量线程快速测试
    vector<thread> threads;
    
    // 两个线程同时加载配置
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back([&config, &config_file, i]() {
            for (int j = 0; j < 3; ++j) {
                config.LoadConfig(config_file);
                cout << "Thread " << i << " load iteration " << j << endl;
                this_thread::sleep_for(10ms);
            }
        });
    }
    
    // 两个线程同时读取配置
    for (int i = 2; i < 4; ++i) {
        threads.emplace_back([&config, i]() {
            for (int j = 0; j < 3; ++j) {
                string host = config.GetString("database.host");
                int port = config.GetInt("database.port");
                cout << "Thread " << i << " read iteration " << j 
                     << ", host: " << host << ", port: " << port << endl;
                this_thread::sleep_for(5ms);
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    cout << "Simple test completed without deadlock!" << endl;
    return 0;
}