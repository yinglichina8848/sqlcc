/**
 * @file config_lifecycle.h
 * @brief Defines a robust, RAII-based lifecycle and access framework for managing system configuration.
 *
 * @WHY
 * In a complex system like a database, configuration is not a simple, static entity. It needs to be:
 * 1.  **Dynamically Loaded & Reloaded**: Configuration can change during runtime. The system must handle these changes gracefully without restarts.
 * 2.  **Snapshot & Versioned**: To ensure consistency, different parts of the system operating on a single task must see the *same* configuration, even if it's being updated concurrently. Snapshots provide this stable view. Versioning allows for reliable rollback.
 * 3.  **Lifecycle-Aware**: Initialization, updates, and shutdown are distinct phases. The system must manage these states to prevent access to uninitialized or shut-down configuration.
 * 4.  **Exception-Safe**: Accessing configuration should not leave resources hanging or the system in an inconsistent state if an error occurs. The RAII (Resource Acquisition Is Initialization) pattern is perfect for this.
 *
 * This file provides the infrastructure to solve these problems.
 *
 * @WHAT
 * This header defines three core components that work together:
 * 1.  **`ConfigLifecycleManager`**: A central state machine that manages the overall lifecycle (initialization, readiness, shutdown) of the configuration system. It owns the `ConfigSnapshotManager` and orchestrates updates and rollbacks.
 * 2.  **`ConfigRAIIAccessor`**: A lightweight RAII "guard" class. Its sole purpose is to acquire a *stable snapshot* of the configuration from the `ConfigLifecycleManager` upon creation and hold it for a short-lived scope. When the accessor goes out of scope, the snapshot reference is automatically released. This is the primary mechanism for safe, temporary access to configuration.
 * 3.  **`SafeConfigAccessor`**: A template-based utility class that provides convenient, type-safe methods (`GetValue<T>`, `SetValue<T>`) for reading from or converting values for a `ConfigRAIIAccessor`. It handles the underlying `std::variant` conversions, simplifying data access for clients.
 *
 * @HOW
 * The intended workflow for a client (e.g., a function executing a query) is:
 * 1.  Create a `ConfigRAIIAccessor` instance on the stack, passing it a pointer to the global `ConfigLifecycleManager`.
 *     ```cpp
 *     ConfigRAIIAccessor accessor(&config_lifecycle_manager);
 *     ```
 * 2.  The accessor's constructor obtains the *current*, valid configuration snapshot. This snapshot is immutable and will not change for the lifetime of the `accessor` object.
 * 3.  Use the `SafeConfigAccessor` utility to read values in a type-safe manner.
 *     ```cpp
 *     int timeout = SafeConfigAccessor<int>::GetValue(accessor, "query.timeout_ms", 1000);
 *     ```
 * 4.  When the function exits, the `accessor` is destroyed, releasing its reference to the snapshot. This entire process is exception-safe.
 *
 * This design separates the high-level state management (`ConfigLifecycleManager`) from the low-level, safe access pattern (`ConfigRAIIAccessor`), providing both flexibility and security.
 */

#ifndef SQLCC_CONFIG_LIFECYCLE_H_
#define SQLCC_CONFIG_LIFECYCLE_H_

#include "config_snapshot.h"
#include <memory>
#include <functional>
#include <exception>
#include <chrono>

namespace sqlcc {

/**
 * @brief Defines the possible states of the configuration system.
 * This enum enforces a strict state machine for configuration management.
 */
enum class ConfigLifecycleState {
    UNINITIALIZED,    // The manager has not been initialized.
    INITIALIZING,     // The manager is currently loading the initial configuration.
    READY,           // The manager is initialized and ready to serve configuration snapshots.
    UPDATING,        // The manager is in the process of applying a new configuration snapshot.
    SHUTTING_DOWN,   // The manager is releasing resources.
    SHUTDOWN         // The manager is fully shut down.
};

/**
 * @brief Custom exception for errors related to the configuration lifecycle.
 * Provides context about the state of the manager when the error occurred.
 */
class ConfigLifecycleException : public std::exception {
private:
    std::string message_;
    ConfigLifecycleState state_;
    
public:
    ConfigLifecycleException(const std::string& message, ConfigLifecycleState state)
        : message_(message), state_(state) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
    ConfigLifecycleState GetState() const {
        return state_;
    }
};

/**
 * @brief Manages the overall state and versioning of system configuration.
 *
 * @details This class acts as a state machine and the central authority for configuration.
 * It is responsible for:
 * - Owning the history of configuration snapshots (`ConfigSnapshotManager`).
 * - Managing the lifecycle state (e.g., UNINITIALIZED, READY, SHUTDOWN).
 * - Coordinating atomic updates and rollbacks to configuration.
 * - Providing a single point of access for RAII-based configuration accessor.
 * It is expected to be a long-lived object (e.g., a singleton) within the system.
 */
class ConfigLifecycleManager {
private:
    mutable std::shared_mutex state_mutex_;
    std::atomic<ConfigLifecycleState> state_{ConfigLifecycleState::UNINITIALIZED};
    ConfigSnapshotManager snapshot_manager_;
    std::string current_config_path_;
    
    // Lifecycle hook callbacks.
    std::function<void()> on_initialize_;
    std::function<void()> on_shutdown_;
    std::function<void(const std::string&)> on_config_change_;
    
    // Performance and health monitoring.
    std::chrono::steady_clock::time_point init_time_;
    std::chrono::steady_clock::time_point last_update_time_;
    std::atomic<size_t> update_count_{0};
    std::atomic<size_t> error_count_{0};

public:
    /**
     * @brief Constructs the lifecycle manager.
     * @param config_path Optional initial path to the configuration file.
     */
    explicit ConfigLifecycleManager(const std::string& config_path = "")
        : current_config_path_(config_path) {
        init_time_ = std::chrono::steady_clock::now();
    }
    
    /**
     * @brief Destructor that ensures a clean shutdown.
     */
    ~ConfigLifecycleManager() {
        Shutdown();
    }
    
    // The manager is a central resource, so it should not be copyable.
    ConfigLifecycleManager(const ConfigLifecycleManager&) = delete;
    ConfigLifecycleManager& operator=(const ConfigLifecycleManager&) = delete;
    
    // It can, however, be moved.
    ConfigLifecycleManager(ConfigLifecycleManager&&) noexcept = default;
    ConfigLifecycleManager& operator=(ConfigLifecycleManager&&) noexcept = default;

    /**
     * @brief Initializes the configuration manager, making it ready.
     * @param config_path Path to the configuration file to load. If empty, uses the path provided in the constructor.
     * @return True if initialization was successful, false if already initialized.
     * @throws ConfigLifecycleException if initialization fails.
     */
    bool Initialize(const std::string& config_path = "") {
        std::unique_lock<std::shared_mutex> lock(state_mutex_);
        
        if (state_.load() != ConfigLifecycleState::UNINITIALIZED) {
            return false;
        }
        
        try {
            state_.store(ConfigLifecycleState::INITIALIZING);
            
            if (!config_path.empty()) {
                current_config_path_ = config_path;
            }
            
            // Execute user-defined initialization hook.
            if (on_initialize_) {
                on_initialize_();
            }
            
            // Create a default empty snapshot to ensure a valid state.
            auto default_snapshot = ConfigSnapshotFactory::CreateEmptySnapshot(
                GenerateVersionId("init_"), "Default configuration snapshot");
            
            if (!snapshot_manager_.AddSnapshot(default_snapshot)) {
                throw ConfigLifecycleException("Failed to add default snapshot", ConfigLifecycleState::INITIALIZING);
            }
            
            state_.store(ConfigLifecycleState::READY);
            return true;
            
        } catch (const std::exception& e) {
            state_.store(ConfigLifecycleState::UNINITIALIZED);
            ++error_count_;
            throw ConfigLifecycleException(
                std::string("Initialization failed: ") + e.what(), 
                ConfigLifecycleState::UNINITIALIZED);
        }
    }

    /**
     * @brief Shuts down the manager and cleans up resources.
     * This is idempotent; calling it multiple times has no effect.
     * @return True always.
     * @throws ConfigLifecycleException if a catastrophic failure occurs during shutdown.
     */
    bool Shutdown() {
        std::unique_lock<std::shared_mutex> lock(state_mutex_);
        
        if (state_.load() == ConfigLifecycleState::SHUTDOWN || 
            state_.load() == ConfigLifecycleState::SHUTTING_DOWN) {
            return true;
        }
        
        try {
            state_.store(ConfigLifecycleState::SHUTTING_DOWN);
            
            // Execute user-defined shutdown hook.
            if (on_shutdown_) {
                on_shutdown_();
            }
            
            // Clear all historical snapshots.
            auto version_ids = snapshot_manager_.GetAllVersionIds();
            for (const auto& version_id : version_ids) {
                snapshot_manager_.RemoveSnapshot(version_id);
            }
            
            state_.store(ConfigLifecycleState::SHUTDOWN);
            return true;
            
        } catch (const std::exception& e) {
            state_.store(ConfigLifecycleState::SHUTDOWN);
            ++error_count_;
            throw ConfigLifecycleException(
                std::string("Shutdown failed: ") + e.what(), 
                ConfigLifecycleState::SHUTDOWN);
        }
    }

    /**
     * @brief Gets the current state of the lifecycle manager.
     * @return The current ConfigLifecycleState.
     */
    ConfigLifecycleState GetState() const {
        return state_.load();
    }

    /**
     * @brief Checks if the manager is ready to serve configuration.
     * @return True if the state is READY, otherwise false.
     */
    bool IsReady() const {
        return state_.load() == ConfigLifecycleState::READY;
    }

    /**
     * @brief Gets a shared pointer to the current configuration snapshot.
     * This is the core method used by RAII accessors.
     * @return A thread-safe, shared pointer to the immutable current snapshot.
     * @throws ConfigLifecycleException if the manager is not in a READY state.
     */
    ConfigSnapshot::SnapshotPtr GetCurrentSnapshot() const {
        if (!IsReady()) {
            throw ConfigLifecycleException("Config manager not ready", state_.load());
        }
        
        std::shared_lock<std::shared_mutex> lock(state_mutex_);
        return snapshot_manager_.GetCurrentSnapshot();
    }

    /**
     * @brief Atomically updates the configuration by adding a new snapshot.
     * @param snapshot The new snapshot to make current.
     * @return True on success.
     * @throws ConfigLifecycleException if the update fails.
     */
    bool UpdateSnapshot(const ConfigSnapshot::SnapshotPtr& snapshot) {
        if (!IsReady()) {
            return false;
        }
        
        std::unique_lock<std::shared_mutex> lock(state_mutex_);
        
        try {
            state_.store(ConfigLifecycleState::UPDATING);
            
            if (!snapshot) {
                throw ConfigLifecycleException("Invalid snapshot", ConfigLifecycleState::UPDATING);
            }
            
            if (!snapshot_manager_.AddSnapshot(snapshot)) {
                throw ConfigLifecycleException("Failed to add snapshot", ConfigLifecycleState::UPDATING);
            }
            
            last_update_time_ = std::chrono::steady_clock::now();
            ++update_count_;
            
            // Trigger the configuration change callback.
            if (on_config_change_) {
                on_config_change_(snapshot->GetMetadata().version_id);
            }
            
            state_.store(ConfigLifecycleState::READY);
            return true;
            
        } catch (const std::exception& e) {
            state_.store(ConfigLifecycleState::READY);  // Attempt to recover to a ready state.
            ++error_count_;
            throw ConfigLifecycleException(
                std::string("Update failed: ") + e.what(), 
                ConfigLifecycleState::READY);
        }
    }

    /**
     * @brief Atomically rolls back the configuration to a previous version.
     * @param version_id The ID of the target snapshot to restore.
     * @return True on success.
     * @throws ConfigLifecycleException if the rollback fails.
     */
    bool RollbackToVersion(const std::string& version_id) {
        if (!IsReady()) {
            return false;
        }
        
        std::unique_lock<std::shared_mutex> lock(state_mutex_);
        
        try {
            state_.store(ConfigLifecycleState::UPDATING);
            
            if (!snapshot_manager_.RollbackToVersion(version_id)) {
                throw ConfigLifecycleException("Rollback failed", ConfigLifecycleState::UPDATING);
            }
            
            last_update_time_ = std::chrono::steady_clock::now();
            ++update_count_;
            
            // Trigger the configuration change callback.
            if (on_config_change_) {
                on_config_change_(version_id);
            }
            
            state_.store(ConfigLifecycleState::READY);
            return true;
            
        } catch (const std::exception& e) {
            state_.store(ConfigLifecycleState::READY);
            ++error_count_;
            throw ConfigLifecycleException(
                std::string("Rollback failed: ") + e.what(), 
                ConfigLifecycleState::READY);
        }
    }

    /**
     * @brief Gets a reference to the underlying snapshot manager.
     * @return A mutable reference to ConfigSnapshotManager.
     */
    ConfigSnapshotManager& GetSnapshotManager() {
        return snapshot_manager_;
    }

    /**
     * @brief Gets a const reference to the underlying snapshot manager.
     * @return A const reference to ConfigSnapshotManager.
     */
    const ConfigSnapshotManager& GetSnapshotManager() const {
        return snapshot_manager_;
    }

    /**
     * @brief Registers a callback function to be executed upon initialization.
     * @param callback The function to call.
     */
    void SetInitializeCallback(std::function<void()> callback) {
        std::unique_lock<std::shared_mutex> lock(state_mutex_);
        on_initialize_ = std::move(callback);
    }

    /**
     * @brief Registers a callback function to be executed upon shutdown.
     * @param callback The function to call.
     */
    void SetShutdownCallback(std::function<void()> callback) {
        std::unique_lock<std::shared_mutex> lock(state_mutex_);
        on_shutdown_ = std::move(callback);
    }

    /**
     * @brief Registers a callback function to be executed after a configuration change.
     * @param callback The function to call, which receives the new version ID.
     */
    void SetConfigChangeCallback(std::function<void(const std::string&)> callback) {
        std::unique_lock<std::shared_mutex> lock(state_mutex_);
        on_config_change_ = std::move(callback);
    }

    /**
     * @brief Gathers and returns performance and health statistics.
     * @return A formatted string containing key statistics.
     */
    std::string GetStatistics() const {
        std::shared_lock<std::shared_mutex> lock(state_mutex_);
        
        auto now = std::chrono::steady_clock::now();
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - init_time_).count();
        auto last_update_seconds = std::chrono::duration_cast<std::chrono::seconds>(
            now - last_update_time_).count();
        
        std::stringstream ss;
        ss << "Config Lifecycle Statistics:" << std::endl;
        ss << "  State: " << static_cast<int>(state_.load()) << std::endl;
        ss << "  Uptime: " << uptime << " seconds" << std::endl;
        ss << "  Update Count: " << update_count_.load() << std::endl;
        ss << "  Error Count: " << error_count_.load() << std::endl;
        ss << "  Last Update: " << last_update_seconds << " seconds ago" << std::endl;
        ss << "  Snapshot Count: " << snapshot_manager_.GetSnapshotCount() << std::endl;
        ss << "  Current Version: " << snapshot_manager_.GetCurrentVersionId() << std::endl;
        
        return ss.str();
    }
};

/**
 * @brief An RAII guard for safe, temporary, and consistent access to configuration.
 *
 * @details This class is the primary client-facing tool for reading configuration.
 * When created, it "locks in" the current configuration snapshot from the
 * `ConfigLifecycleManager`. This snapshot is guaranteed to be stable for the lifetime
 * of the accessor object, even if the global configuration is updated in the background.
 * It is lightweight and designed to be created on the stack for short-lived operations.
 */
class ConfigRAIIAccessor {
private:
    ConfigLifecycleManager* lifecycle_manager_;
    ConfigSnapshot::SnapshotPtr snapshot_;
    std::string accessor_id_;
    std::chrono::steady_clock::time_point access_time_;
    bool is_valid_;

public:
    /**
     * @brief Constructs the accessor and acquires the current configuration snapshot.
     * @param lifecycle_manager Pointer to the central lifecycle manager.
     * @param accessor_id An optional identifier for debugging and monitoring.
     * @throws ConfigLifecycleException if the manager is not ready or snapshot acquisition fails.
     */
    explicit ConfigRAIIAccessor(
        ConfigLifecycleManager* lifecycle_manager,
        const std::string& accessor_id = "")
        : lifecycle_manager_(lifecycle_manager),
          accessor_id_(accessor_id.empty() ? "default_accessor" : accessor_id),
          access_time_(std::chrono::steady_clock::now()),
          is_valid_(false) {
        
        if (!lifecycle_manager_) {
            throw ConfigLifecycleException("Invalid lifecycle manager", ConfigLifecycleState::UNINITIALIZED);
        }
        
        try {
            snapshot_ = lifecycle_manager_->GetCurrentSnapshot();
            if (!snapshot_) {
                throw ConfigLifecycleException("Failed to get current snapshot", 
                    lifecycle_manager_->GetState());
            }
            
            is_valid_ = true;
            
        } catch (const std::exception& e) {
            is_valid_ = false;
            throw ConfigLifecycleException(
                std::string("Accessor initialization failed: ") + e.what(),
                lifecycle_manager_->GetState());
        }
    }
    
    /**
     * @brief Destructor. Automatically releases the reference to the configuration snapshot.
     */
    ~ConfigRAIIAccessor() {
        is_valid_ = false;
        snapshot_.reset();  // Release the shared_ptr reference.
    }

    // Accessors manage a unique view of a resource, so they should not be copyable.
    ConfigRAIIAccessor(const ConfigRAIIAccessor&) = delete;
    ConfigRAIIAccessor& operator=(const ConfigRAIIAccessor&) = delete;

    // Moving is allowed, transferring ownership of the snapshot view.
    ConfigRAIIAccessor(ConfigRAIIAccessor&& other) noexcept
        : lifecycle_manager_(other.lifecycle_manager_),
          snapshot_(std::move(other.snapshot_)),
          accessor_id_(std::move(other.accessor_id_)),
          access_time_(other.access_time_),
          is_valid_(other.is_valid_) {
        other.is_valid_ = false;
    }
    
    ConfigRAIIAccessor& operator=(ConfigRAIIAccessor&& other) noexcept {
        if (this != &other) {
            lifecycle_manager_ = other.lifecycle_manager_;
            snapshot_ = std::move(other.snapshot_);
            accessor_id_ = std::move(other.accessor_id_);
            access_time_ = other.access_time_;
            is_valid_ = other.is_valid_;
            other.is_valid_ = false;
        }
        return *this;
    }

    /**
     * @brief Retrieves a configuration value from the held snapshot.
     * @param key The configuration key to look up.
     * @param[out] value The output `ConfigValue` (a std::variant).
     * @return True if the key was found, false otherwise.
     */
    bool GetValue(const std::string& key, ConfigValue& value) const {
        if (!is_valid_ || !snapshot_) {
            return false;
        }
        
        return snapshot_->GetValue(key, value);
    }

    /**
     * @brief Retrieves a configuration value with a fallback default.
     * @param key The configuration key to look up.
     * @param default_value The value to return if the key is not found.
     * @return The found `ConfigValue` or the default value.
     */
    ConfigValue GetValue(const std::string& key, const ConfigValue& default_value) const {
        ConfigValue value;
        if (GetValue(key, value)) {
            return value;
        }
        return default_value;
    }

    /**
     * @brief Checks if a key exists in the held snapshot.
     * @param key The configuration key.
     * @return True if the key exists.
     */
    bool HasKey(const std::string& key) const {
        if (!is_valid_ || !snapshot_) {
            return false;
        }
        
        return snapshot_->HasKey(key);
    }

    /**
     * @brief Gets all keys available in the held snapshot.
     * @return A vector of key strings.
     */
    std::vector<std::string> GetAllKeys() const {
        if (!is_valid_ || !snapshot_) {
            return {};
        }
        
        return snapshot_->GetAllKeys();
    }

    /**
     * @brief Gets the ID of this accessor instance.
     * @return The accessor's ID string.
     */
    const std::string& GetAccessorId() const {
        return accessor_id_;
    }

    /**
     * @brief Gets the timestamp when this accessor was created.
     * @return The time_point of creation.
     */
    std::chrono::steady_clock::time_point GetAccessTime() const {
        return access_time_;
    }

    /**
     * @brief Checks if the accessor is currently valid and holds a snapshot.
     * @return True if valid, false otherwise.
     */
    bool IsValid() const {
        return is_valid_ && snapshot_ != nullptr;
    }

    /**
     * @brief Gets the version ID of the snapshot held by this accessor.
     * @return The version ID string.
     */
    std::string GetCurrentVersionId() const {
        if (!is_valid_ || !snapshot_) {
            return "";
        }
        
        return snapshot_->GetMetadata().version_id;
    }
};

/**
 * @brief A template-based utility for type-safe configuration value access.
 *
 * @details This class provides a clean, convenient, and safe API for clients to get
 * values of a specific type (e.g., int, bool, std::string) from a `ConfigRAIIAccessor`.
 * It handles the complexities of converting from the underlying `ConfigValue` variant,
 * providing default values and suppressing exceptions for read operations to simplify client code.
 */
template<typename T>
class SafeConfigAccessor {
public:
    /**
     * @brief Safely retrieves and converts a configuration value.
     * @tparam T The desired type (e.g., int, bool, std::string).
     * @param accessor The RAII accessor holding the configuration snapshot.
     * @param key The configuration key.
     * @param default_value A fallback value to return if the key is not found or conversion fails.
     * @return The converted value or the default value.
     */
    static T GetValue(const ConfigRAIIAccessor& accessor, const std::string& key, const T& default_value = T{}) {
        try {
            ConfigValue config_value;
            if (!accessor.GetValue(key, config_value)) {
                return default_value;
            }
            
            return ConvertConfigValue<T>(config_value, default_value);
            
        } catch (const std::exception& e) {
            // Logging the error in a real implementation is recommended.
            // For safety, we return the default value on any failure.
            return default_value;
        }
    }
    
    /**
     * @brief Converts a given C++ type to its `ConfigValue` variant representation.
     * @tparam U The type of the value to convert.
     * @param value The value to convert.
     * @return The corresponding `ConfigValue`.
     */
    template<typename U>
    static ConfigValue SetValue(const U& value) {
        return ConvertToConfigValue(value);
    }

private:
    /**
     * @brief Internal helper to convert a `ConfigValue` variant to a specific type `U`.
     * Handles common cross-type conversions (e.g., string "true" to bool true).
     */
    template<typename U>
    static U ConvertConfigValue(const ConfigValue& value, const U& default_value) {
        try {
            if constexpr (std::is_same_v<U, bool>) {
                if (std::holds_alternative<bool>(value)) {
                    return std::get<bool>(value);
                } else if (std::holds_alternative<std::string>(value)) {
                    const auto& str = std::get<std::string>(value);
                    // Case-insensitive check for "true"
                    return (str.length() == 4 && (str[0] == 't' || str[0] == 'T') &&
                                                 (str[1] == 'r' || str[1] == 'R') &&
                                                 (str[2] == 'u' || str[2] == 'U') &&
                                                 (str[3] == 'e' || str[3] == 'E')) ||
                           (str == "1");
                }
            } else if constexpr (std::is_integral_v<U> && !std::is_same_v<U, bool>) {
                if (std::holds_alternative<int>(value)) {
                    return static_cast<U>(std::get<int>(value));
                } else if (std::holds_alternative<double>(value)) {
                    return static_cast<U>(std::get<double>(value));
                }
            } else if constexpr (std::is_floating_point_v<U>) {
                if (std::holds_alternative<double>(value)) {
                    return static_cast<U>(std::get<double>(value));
                } else if (std::holds_alternative<int>(value)) {
                    return static_cast<U>(std::get<int>(value));
                }
            } else if constexpr (std::is_same_v<U, std::string>) {
                if (std::holds_alternative<std::string>(value)) {
                    return std::get<std::string>(value);
                } else if (std::holds_alternative<bool>(value)) {
                    return std::get<bool>(value) ? "true" : "false";
                } else if (std::holds_alternative<int>(value)) {
                    return std::to_string(std::get<int>(value));
                } else if (std::holds_alternative<double>(value)) {
                    return std::to_string(std::get<double>(value));
                }
            }
        } catch (const std::exception& e) {
            // Conversion failed, fall through to return default value.
        }
        
        return default_value;
    }
    
    /**
     * @brief Internal helper to convert a C++ type `U` to a `ConfigValue` variant.
     */
    template<typename U>
    static ConfigValue ConvertToConfigValue(const U& value) {
        if constexpr (std::is_same_v<U, bool>) {
            return value;
        } else if constexpr (std::is_integral_v<U> && !std::is_same_v<U, bool>) {
            return static_cast<int>(value);
        } else if constexpr (std::is_floating_point_v<U>) {
            return static_cast<double>(value);
        } else if constexpr (std::is_convertible_v<U, std::string>) {
            return std::string(value);
        } else {
            // If no direct conversion exists, default to converting to string.
            // This requires that the type U is streamable.
            std::ostringstream ss;
            ss << value;
            return ss.str();
        }
    }
};
/**
 * @brief Formats a `ConfigValue` variant into a string representation.
 * @param value The `ConfigValue` to format.
 * @return A string representation suitable for logging or storage.
 */
std::string FormatConfigValue(const ConfigValue& value);

/**
 * @brief Parses a string into a `ConfigValue` variant.
 * Tries to infer the most appropriate type (bool, int, double, or string).
 * @param str The input string to parse.
 * @param[out] value The resulting `ConfigValue`.
 * @return True if parsing was successful, false otherwise.
 */
bool ParseConfigValue(const std::string& str, ConfigValue& value);


}  // namespace sqlcc

#endif  // SQLCC_CONFIG_LIFECYCLE_H_
