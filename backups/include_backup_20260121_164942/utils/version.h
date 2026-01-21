#ifndef SQLCC_UTILS_VERSION_H
#define SQLCC_UTILS_VERSION_H

#include <string>

// Version information for SQLCC
#define SQLCC_VERSION_MAJOR 1
#define SQLCC_VERSION_MINOR 3
#define SQLCC_VERSION_PATCH 2

#define SQLCC_VERSION "1.3.2"
#define SQLCC_VERSION_STRING "SQLCC v1.3.2"

namespace sqlcc {
namespace utils {

// Get version string
inline std::string get_version_string() {
    return SQLCC_VERSION_STRING;
}

// Get version as string
inline std::string get_version() {
    return SQLCC_VERSION;
}

// Get version components
inline int get_version_major() {
    return SQLCC_VERSION_MAJOR;
}

inline int get_version_minor() {
    return SQLCC_VERSION_MINOR;
}

inline int get_version_patch() {
    return SQLCC_VERSION_PATCH;
}

} // namespace utils
} // namespace sqlcc

#endif // SQLCC_UTILS_VERSION_H