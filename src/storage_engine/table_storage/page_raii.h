#pragma once

#include <memory>
#include <stdexcept>
#include "../../page/page.h"
#include "../storage_engine.h"

namespace sqlcc {
namespace storage_engine {
} // namespace storage_engine
} // namespace sqlcc

namespace sqlcc {
namespace storage_engine {
namespace table_storage {

class PageRAII {
public:
    // Constructor - takes ownership of the page through RAII pattern
    PageRAII(Page* page, std::shared_ptr<StorageEngine> storage_engine, int32_t page_id);

    // Destructor - automatically unpins the page
    ~PageRAII();

    // Move constructor
    PageRAII(PageRAII&& other) noexcept;

    // Move assignment operator
    PageRAII& operator=(PageRAII&& other) noexcept;

    // Delete copy operations for RAII pattern
    PageRAII(const PageRAII&) = delete;
    PageRAII& operator=(const PageRAII&) = delete;

    // Access methods
    char* GetData();
    const char* GetData() const;

    // Manual unpin method
    void Unpin(bool is_dirty = false);

private:
    Page* page_;                              ///< Pointer to the page
    std::shared_ptr<StorageEngine> storage_engine_;  ///< Storage engine instance
    int32_t page_id_;                         ///< Page ID
    bool pinned_;                             ///< Whether the page is still pinned
};

} // namespace table_storage
} // namespace storage_engine
} // namespace sqlcc
