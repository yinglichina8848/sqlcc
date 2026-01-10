#include <iostream>
#include <string>
#include "include/utils/logger.h"

int main() {
    std::cout << "Testing basic logging functionality..." << std::endl;

    // Test basic logging functions
    sqlcc::Logger::GetInstance().Debug("This is a debug message");
    sqlcc::Logger::GetInstance().Info("This is an info message");
    sqlcc::Logger::GetInstance().Warn("This is a warning message");
    sqlcc::Logger::GetInstance().Error("This is an error message");

    // Test string message
    std::string msg = "Test message";
    sqlcc::Logger::GetInstance().Info("String message: " + msg);

    // Test vector message
    std::vector<std::string> vec = {"msg1", "msg2", "msg3"};
    for (const auto& v : vec) {
        sqlcc::Logger::GetInstance().Info("Vector message: " + v);
    }

    // Test shared pointer
    std::shared_ptr<std::string> sp = std::make_shared<std::string>("smart pointer test");
    sqlcc::Logger::GetInstance().Info("Shared pointer: " + *sp);

    std::cout << "Basic logging test completed successfully!" << std::endl;

    // Test basic types
    std::cout << "Testing basic types..." << std::endl;
    int i = 42;
    long l = 1234567890L;
    size_t s = 100;
    double d = 3.14159;
    float f = 2.71828f;
    bool b = true;
    char c = 'A';

    std::cout << "int: " << i << std::endl;
    std::cout << "long: " << l << std::endl;
    std::cout << "size_t: " << s << std::endl;
    std::cout << "double: " << d << std::endl;
    std::cout << "float: " << f << std::endl;
    std::cout << "bool: " << (b ? "true" : "false") << std::endl;
    std::cout << "char: " << c << std::endl;

    std::cout << "Basic types test completed successfully!" << std::endl;

    // Test basic operations
    std::cout << "Testing basic operations..." << std::endl;
    int a = 15, bb = 25;
    std::cout << "Addition: " << (a + bb) << std::endl;
    std::cout << "Subtraction: " << (a - bb) << std::endl;
    std::cout << "Multiplication: " << (a * bb) << std::endl;
    std::cout << "Division: " << (bb / a) << std::endl;
    std::cout << "Equal: " << ((a == bb) ? "true" : "false") << std::endl;
    std::cout << "Not equal: " << ((a != bb) ? "true" : "false") << std::endl;
    std::cout << "Less than: " << ((a < bb) ? "true" : "false") << std::endl;
    std::cout << "Greater than: " << ((a > bb) ? "true" : "false") << std::endl;
    std::cout << "AND: " << ((a > 10 && bb < 30) ? "true" : "false") << std::endl;
    std::cout << "OR: " << ((a < 10 || bb > 20) ? "true" : "false") << std::endl;
    std::cout << "NOT: " << ((!b) ? "true" : "false") << std::endl;

    std::cout << "Basic operations test completed successfully!" << std::endl;

    std::cout << "All tests passed!" << std::endl;
    return 0;
}
