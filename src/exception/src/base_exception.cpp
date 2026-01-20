#include <stdexcept>
#include <string>

#include "exception/base_exception.h"

namespace sqlcc {

Exception::Exception(const std::string& message)
    : std::runtime_error(message) {
}

} // namespace sqlcc
