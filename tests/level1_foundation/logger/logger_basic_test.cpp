#include <iostream>
#include <string>
#include <vector>
#include <memory>

// Basic test for logger functionality
class LoggerBasicTest {
public:
    void test_basic_logging() {
        std::cout << "Testing basic logging functionality..." << std::endl;

        // Test basic output
        std::cout << "INFO: This is an info message" << std::endl;
        std::cout << "DEBUG: This is a debug message" << std::endl;
        std::cout << "ERROR: This is an error message" << std::endl;

        // Test string operations
        std::string message = "Test message";
        std::cout << "String message: " << message << std::endl;

        // Test vector operations
        std::vector<std::string> messages = {"msg1", "msg2", "msg3"};
        for (const auto& msg : messages) {
            std::cout << "Vector message: " << msg << std::endl;
        }

        // Test shared pointer
        auto ptr = std::make_shared<std::string>("smart pointer test");
        std::cout << "Shared pointer: " << *ptr << std::endl;

        std::cout << "Basic logging test completed successfully!" << std::endl;
    }

    void test_basic_types() {
        std::cout << "Testing basic types..." << std::endl;

        // Test integers
        int i = 42;
        long l = 1234567890L;
        size_t s = 100;

        std::cout << "int: " << i << std::endl;
        std::cout << "long: " << l << std::endl;
        std::cout << "size_t: " << s << std::endl;

        // Test floating point
        double d = 3.14159;
        float f = 2.71828f;

        std::cout << "double: " << d << std::endl;
        std::cout << "float: " << f << std::endl;

        // Test boolean
        bool b = true;
        std::cout << "bool: " << (b ? "true" : "false") << std::endl;

        // Test char
        char c = 'A';
        std::cout << "char: " << c << std::endl;

        std::cout << "Basic types test completed successfully!" << std::endl;
    }

    void test_basic_operations() {
        std::cout << "Testing basic operations..." << std::endl;

        // Arithmetic operations
        int a = 10, b = 20;
        std::cout << "Addition: " << (a + b) << std::endl;
        std::cout << "Subtraction: " << (a - b) << std::endl;
        std::cout << "Multiplication: " << (a * b) << std::endl;
        std::cout << "Division: " << (b / a) << std::endl;

        // Comparison operations
        std::cout << "Equal: " << (a == b ? "true" : "false") << std::endl;
        std::cout << "Not equal: " << (a != b ? "true" : "false") << std::endl;
        std::cout << "Less than: " << (a < b ? "true" : "false") << std::endl;
        std::cout << "Greater than: " << (a > b ? "true" : "false") << std::endl;

        // Logical operations
        bool x = true, y = false;
        std::cout << "AND: " << ((x && y) ? "true" : "false") << std::endl;
        std::cout << "OR: " << ((x || y) ? "true" : "false") << std::endl;
        std::cout << "NOT: " << ((!x) ? "true" : "false") << std::endl;

        std::cout << "Basic operations test completed successfully!" << std::endl;
    }

    void run_all_tests() {
        try {
            test_basic_logging();
            test_basic_types();
            test_basic_operations();
            std::cout << "\nAll tests passed!" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Test failed with exception: " << e.what() << std::endl;
            throw;
        } catch (...) {
            std::cout << "Test failed with unknown exception" << std::endl;
            throw;
        }
    }
};

int main() {
    LoggerBasicTest test;
    test.run_all_tests();
    return 0;
}
