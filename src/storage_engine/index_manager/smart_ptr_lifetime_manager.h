#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include <functional>

namespace sqlcc {

template<typename T>
class SmartPtrLifetimeManager {
public:
    SmartPtrLifetimeManager() = default;
    ~SmartPtrLifetimeManager();

    std::shared_ptr<T> GetOrCreate(const std::string& key,
                                    std::function<std::shared_ptr<T>()> creator);
    void Release(const std::string& key);
    void Clear();

private:
    std::mutex mutex_;
    std::unordered_map<std::string, std::weak_ptr<T>> weak_registry_;
};

} // namespace sqlcc
