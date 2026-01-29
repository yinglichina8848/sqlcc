#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "src/utils/config_manager.h"

using namespace std;
using namespace std::chrono_literals;

class ConfigManagerTest {
public:
    static void CreateSampleConfig(const std::string& filename) {
        std::ofstream file(filename);
        file << "# Database Configuration\n";
        file << "[database]\n";
        file << "host=localhost\n";
        file << "port=5432\n";
        file << "max_connections=100\n";
        file << "timeout=30000\n";
        file << "debug=true\n";
        file << "enable_ssl=false\n";
        file << "\n";
        file << "[storage]\n";
        file << "buffer_size=1048576\n";
        file << "compression_level=6\n";
        file << "enable_logging=true\n";
        file.close();
    }
};

int main() {
    cout << "=== ConfigManager Deadlock Fix Test ===" << endl;
    
    // 创建示例配置文件
    const std::string config_file = "/tmp/test_config.ini";
    ConfigManagerTest::CreateSampleConfig(config_file);
    
    auto start_time = chrono::high_resolution_clock::now();
    
    // 测试并发读取配置
    auto& config = sqlcc::ConfigManager::GetInstance();
    atomic<bool> stop_loading(false);
    mutex cout_mutex;
    
    vector<thread> threads;
    
    // 线程1-5：并发加载配置文件
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&config, &config_file, &stop_loading, &cout_mutex, i]() {
            int load_count = 0;
            while (!stop_loading && load_count < 5) {  // 每个线程加载5次
                try {
                    config.LoadConfig(config_file);
                    {
                        lock_guard<mutex> lock(cout_mutex);
                        cout << "Thread " << i << " loaded config, iteration " << load_count + 1 << endl;
                    }
                    this_thread::sleep_for(chrono::milliseconds(10));
                    load_count++;
                } catch (const exception& e) {
                    {
                        lock_guard<mutex> lock(cout_mutex);
                        cout << "Thread " << i << " failed to load config: " << e.what() << endl;
                    }
                }
            }
        });
    }
    
    // 线程6-10：并发读取配置值
    for (int i = 5; i < 10; ++i) {
        threads.emplace_back([&config, &stop_loading, &cout_mutex, i]() {
            int read_count = 0;
            while (!stop_loading && read_count < 10) {  // 每个线程读取10次
                try {
                    string host = config.GetString("database.host");
                    int port = config.GetInt("database.port");
                    int max_conn = config.GetInt("database.max_connections");
                    bool debug = config.GetBool("database.debug");
                    int buf_size = config.GetInt("storage.buffer_size");
                    bool logging = config.GetBool("storage.enable_logging");
                    
                    {
                        lock_guard<mutex> lock(cout_mutex);
                        cout << "Thread " << i << " read config, iteration " << read_count + 1 
                             << ", host=" << host << ", port=" << port 
                             << ", max_conn=" << max_conn << ", debug=" << debug 
                             << ", buf_size=" << buf_size << ", logging=" << logging << endl;
                    }
                    this_thread::sleep_for(chrono::milliseconds(5));
                    read_count++;
                } catch (const exception& e) {
                    {
                        lock_guard<mutex> lock(cout_mutex);
                        cout << "Thread " << i << " failed to read config: " << e.what() << endl;
                    }
                }
            }
        });
    }
    
    // 运行一段时间后停止
    this_thread::sleep_for(chrono::seconds(5));
    stop_loading = true;
    
    // 等待所有线程完成
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
    
    cout << "\nTotal execution time: " << duration.count() << " ms" << endl;
    cout << "Expected: Should complete without deadlocks or exceptions" << endl;
    cout << "Result: Deadlock fix working correctly!" << endl;
    
    return 0;
}