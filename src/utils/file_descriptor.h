#ifndef SQLCC_UTILS_FILE_DESCRIPTOR_H
#define SQLCC_UTILS_FILE_DESCRIPTOR_H

#include <unistd.h>
#include <utility>

namespace sqlcc {

/**
 * @brief RAII wrapper for file descriptors
 *
 * Automatically closes the file descriptor when it goes out of scope.
 */
class FileDescriptor {
public:
    // Default constructor
    FileDescriptor() : fd_(-1) {}

    // Constructor taking ownership of a file descriptor
    explicit FileDescriptor(int fd) : fd_(fd) {}

    // Move constructor
    FileDescriptor(FileDescriptor&& other) noexcept : fd_(other.fd_) {
        other.fd_ = -1;
    }

    // Move assignment
    FileDescriptor& operator=(FileDescriptor&& other) noexcept {
        if (this != &other) {
            close();
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

    // Destructor
    ~FileDescriptor() {
        close();
    }

    // Delete copy constructor and assignment
    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    // Get the file descriptor
    int get() const { return fd_; }

    // Check if valid
    bool valid() const { return fd_ >= 0; }

    // Explicit bool conversion
    explicit operator bool() const { return valid(); }

    // Release ownership (caller takes responsibility for closing)
    int release() {
        int temp = fd_;
        fd_ = -1;
        return temp;
    }

    // Close the file descriptor
    void close() {
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
    }

    // Reset with a new file descriptor
    void reset(int fd = -1) {
        close();
        fd_ = fd;
    }

private:
    int fd_;
};

} // namespace sqlcc

#endif // SQLCC_UTILS_FILE_DESCRIPTOR_H