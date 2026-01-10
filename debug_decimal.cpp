#include "sql_parser/decimal.h"
#include <iostream>

int main() {
    sqlcc::Decimal a("10.5");
    sqlcc::Decimal b("3.2");

    std::cout << "a: " << a.to_string() << std::endl;
    std::cout << "b: " << b.to_string() << std::endl;

    sqlcc::Decimal prod = a * b;
    std::cout << "a * b: " << prod.to_string() << std::endl;

    sqlcc::Decimal quot = a / b;
    std::cout << "a / b: " << quot.to_string() << std::endl;

    sqlcc::Decimal mod = a % b;
    std::cout << "a % b: " << mod.to_string() << std::endl;

    // Test multiply_strings directly
    std::string result = sqlcc::Decimal::multiply_strings("105", "32");
    std::cout << "multiply_strings(\"105\", \"32\"): " << result << std::endl;

    return 0;
}
