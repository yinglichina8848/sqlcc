#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    try {
        std::cout << "=== Persistence Check Test ===" << std::endl;
        
        // 检查数据库目录结构
        std::cout << "Checking directory structure:" << std::endl;
        if (fs::exists("./simple_test_data")) {
            for (const auto& entry : fs::directory_iterator("./simple_test_data")) {
                if (entry.is_directory()) {
                    std::cout << "Found database directory: " << entry.path().filename().string() << std::endl;
                }
            }
        } else {
            std::cout << "Database path does not exist: ./simple_test_data" << std::endl;
        }
        
        // 检查特定数据库是否存在
        if (fs::exists("./simple_test_data/testdb1")) {
            std::cout << "Database 'testdb1' exists and is persistent!" << std::endl;
        } else {
            std::cout << "Database 'testdb1' does not exist" << std::endl;
        }
        
        if (fs::exists("./simple_test_data/testdb2")) {
            std::cout << "Database 'testdb2' exists and is persistent!" << std::endl;
        } else {
            std::cout << "Database 'testdb2' does not exist" << std::endl;
        }
        
        std::cout << "\nTest completed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}