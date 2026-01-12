#include <stdexcept>
#include <string>

namespace sqlcc {

class Exception : public std::runtime_error {
public:
    explicit Exception(const std::string& message);
};

Exception::Exception(const std::string& message)
    : std::runtime_error(message) {
}

} // namespace sqlcc