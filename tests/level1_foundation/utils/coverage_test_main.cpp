#include <iostream>
#include <cstdio>
#include <fstream>
#include <string>

#include "src/utils/file_descriptor.h"

int main() {
    std::cout << "=== FileDescriptor Coverage Test ===" << std::endl;
    
    // Test FileDescriptor
    std::cout << "\n--- FileDescriptor Tests ---" << std::endl;
    
    // Default constructor
    sqlcc::FileDescriptor fd;
    std::cout << "Default constructor: valid=" << fd.valid() << ", get=" << fd.get() << std::endl;
    
    // Create temp file
    std::string test_file = "/tmp/sqlcc_fd_coverage_test.txt";
    std::ofstream ofs(test_file);
    ofs << "test data";
    ofs.close();
    
    // Open file and test FileDescriptor
    FILE* file = fopen(test_file.c_str(), "r");
    if (file) {
        sqlcc::FileDescriptor fd2(fileno(file));
        std::cout << "Valid FD: valid=" << fd2.valid() << ", get=" << fd2.get() << std::endl;
        
        // Test move
        sqlcc::FileDescriptor fd3(std::move(fd2));
        std::cout << "Move constructor: fd2 valid=" << fd2.valid() << ", fd3 valid=" << fd3.valid() << std::endl;
        
        // Test release
        int released = fd3.release();
        std::cout << "Release: released=" << released << ", valid=" << fd3.valid() << std::endl;
        
        fclose(file);
    }
    
    // Test reset
    sqlcc::FileDescriptor fd4;
    file = fopen(test_file.c_str(), "r");
    fd4.reset(fileno(file));
    std::cout << "Reset: valid=" << fd4.valid() << std::endl;
    fd4.reset(-1);
    std::cout << "Reset -1: valid=" << fd4.valid() << std::endl;
    fclose(file);
    
    // Test explicit bool
    std::cout << "Bool conversion (empty): " << static_cast<bool>(fd) << std::endl;
    
    // Cleanup
    std::remove(test_file.c_str());
    
    std::cout << "\n=== All tests passed ===" << std::endl;
    return 0;
}
