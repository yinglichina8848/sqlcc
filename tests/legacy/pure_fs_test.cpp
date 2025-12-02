#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

int main() {
    try {
        std::cout << "=== Pure Filesystem Test ===" << std::endl;
        
        // 清理之前的测试数据
        fs::remove_all("./pure_test");
        fs::create_directories("./pure_test");
        
        // 第一部分：创建数据库目录
        std::cout << "Part 1: Creating database directories..." << std::endl;
        
        fs::create_directories("./pure_test/mydb");
        fs::create_directories("./pure_test/yourdb");
        
        std::cout << "Created directories: mydb, yourdb" << std::endl;
        
        // 在数据库目录中创建一些文件来模拟表
        std::ofstream file1("./pure_test/mydb/users.tbl");
        file1 << "id INT\nname VARCHAR(255)\nage INT\n";
        file1.close();
        
        std::ofstream file2("./pure_test/yourdb/products.tbl");
        file2 << "id INT\nname VARCHAR(255)\nprice DECIMAL\n";
        file2.close();
        
        std::cout << "Created sample table files" << std::endl;
        
        // 列出所有数据库
        std::cout << "Current databases:" << std::endl;
        for (const auto& entry : fs::directory_iterator("./pure_test")) {
            if (entry.is_directory()) {
                std::cout << "  - " << entry.path().filename().string() << std::endl;
            }
        }
        
        std::cout << "\nPart 1 completed.\n" << std::endl;
        
        // 第二部分：验证持久化
        std::cout << "Part 2: Verifying persistence..." << std::endl;
        
        // 检查数据库目录是否存在
        if (fs::exists("./pure_test/mydb")) {
            std::cout << "Database 'mydb' exists!" << std::endl;
        } else {
            std::cout << "Database 'mydb' does not exist!" << std::endl;
        }
        
        if (fs::exists("./pure_test/yourdb")) {
            std::cout << "Database 'yourdb' exists!" << std::endl;
        } else {
            std::cout << "Database 'yourdb' does not exist!" << std::endl;
        }
        
        // 检查表文件是否存在
        if (fs::exists("./pure_test/mydb/users.tbl")) {
            std::cout << "Table 'users.tbl' exists in 'mydb'!" << std::endl;
        } else {
            std::cout << "Table 'users.tbl' does not exist in 'mydb'!" << std::endl;
        }
        
        if (fs::exists("./pure_test/yourdb/products.tbl")) {
            std::cout << "Table 'products.tbl' exists in 'yourdb'!" << std::endl;
        } else {
            std::cout << "Table 'products.tbl' does not exist in 'yourdb'!" << std::endl;
        }
        
        // 列出所有数据库
        std::cout << "Current databases:" << std::endl;
        for (const auto& entry : fs::directory_iterator("./pure_test")) {
            if (entry.is_directory()) {
                std::cout << "  - " << entry.path().filename().string() << std::endl;
            }
        }
        
        std::cout << "\nPart 2 completed. Test finished successfully!" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}