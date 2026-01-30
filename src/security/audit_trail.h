/**
 * @file audit_trail.h
 * @brief Enterprise audit trail component for SQLCC
 *
 * This header defines the audit trail functionality for enterprise
 * features, including logging, compliance reporting, and audit
 * management.
 */

#ifndef SQLCC_SECURITY_AUDIT_TRAIL_H_
#define SQLCC_SECURITY_AUDIT_TRAIL_H_

#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <map>

namespace sqlcc {

/**
 * @class AuditEntry
 * @brief Represents a single audit log entry
 */
class AuditEntry {
public:
    /**
     * @enum AuditEventType
     * @brief Types of audit events
     */
    enum class AuditEventType {
        LOGIN,
        LOGOUT,
        QUERY_EXECUTION,
        TABLE_MODIFICATION,
        USER_MANAGEMENT,
        SECURITY_CHANGE,
        SYSTEM_CONFIG_CHANGE,
        BACKUP_OPERATION,
        RESTORE_OPERATION,
        UNKNOWN
    };

    /**
     * @brief Constructor for audit entry
     * @param event_type Type of audit event
     * @param user_name Name of the user performing the action
     * @param action Description of the action
     * @param details Additional details about the action
     */
    AuditEntry(AuditEventType event_type,
               const std::string& user_name,
               const std::string& action,
               const std::string& details = "");

    // Getters
    AuditEventType get_event_type() const;
    const std::string& get_user_name() const;
    const std::string& get_action() const;
    const std::string& get_details() const;
    std::chrono::system_clock::time_point get_timestamp() const;

    /**
     * @brief Convert audit event type to string
     * @param type Audit event type
     * @return String representation
     */
    static std::string event_type_to_string(AuditEventType type);

private:
    AuditEventType event_type_;
    std::string user_name_;
    std::string action_;
    std::string details_;
    std::chrono::system_clock::time_point timestamp_;
};

/**
 * @class AuditTrail
 * @brief Main audit trail management class
 *
 * Provides functionality for recording, retrieving, and managing
 * audit logs for enterprise compliance and security monitoring.
 */
class AuditTrail {
public:
    /**
     * @brief Constructor
     */
    AuditTrail();

    /**
     * @brief Destructor
     */
    ~AuditTrail();

    /**
     * @brief Initialize the audit trail system
     * @return true if initialization successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Shutdown the audit trail system
     */
    void shutdown();

    /**
     * @brief Record an audit event
     * @param event The audit entry to record
     * @return true if recording successful, false otherwise
     */
    bool record_event(const AuditEntry& event);

    /**
     * @brief Record an audit event with parameters
     * @param event_type Type of audit event
     * @param user_name Name of the user
     * @param action Description of the action
     * @param details Additional details
     * @return true if recording successful, false otherwise
     */
    bool record_event(AuditEntry::AuditEventType event_type,
                     const std::string& user_name,
                     const std::string& action,
                     const std::string& details = "");

    /**
     * @brief Retrieve audit entries within a time range
     * @param start_time Start time for the query
     * @param end_time End time for the query
     * @return Vector of audit entries
     */
    std::vector<AuditEntry> get_entries(
        const std::chrono::system_clock::time_point& start_time,
        const std::chrono::system_clock::time_point& end_time);

    /**
     * @brief Retrieve audit entries for a specific user
     * @param user_name Name of the user
     * @param limit Maximum number of entries to return
     * @return Vector of audit entries
     */
    std::vector<AuditEntry> get_entries_for_user(
        const std::string& user_name, size_t limit = 100);

    /**
     * @brief Retrieve audit entries by event type
     * @param event_type Type of audit event
     * @param limit Maximum number of entries to return
     * @return Vector of audit entries
     */
    std::vector<AuditEntry> get_entries_by_type(
        AuditEntry::AuditEventType event_type, size_t limit = 100);

    /**
     * @brief Generate compliance report
     * @param start_time Start time for the report
     * @param end_time End time for the report
     * @return Compliance report as string
     */
    std::string generate_compliance_report(
        const std::chrono::system_clock::time_point& start_time,
        const std::chrono::system_clock::time_point& end_time);

    /**
     * @brief Clear old audit entries
     * @param retention_days Number of days to retain entries
     * @return Number of entries cleared
     */
    size_t clear_old_entries(int retention_days);

    /**
     * @brief Get total number of audit entries
     * @return Total count
     */
    size_t get_total_entries() const;

    /**
     * @brief Check if audit trail is enabled
     * @return true if enabled, false otherwise
     */
    bool is_enabled() const;

    /**
     * @brief Enable or disable audit trail
     * @param enabled true to enable, false to disable
     */
    void set_enabled(bool enabled);

private:
    /**
     * @brief Internal storage for audit entries
     */
    std::vector<AuditEntry> audit_entries_;

    /**
     * @brief Mutex for thread safety
     */
    mutable std::mutex mutex_;

    /**
     * @brief Whether audit trail is enabled
     */
    bool enabled_;

    /**
     * @brief Maximum number of entries to keep in memory
     */
    static constexpr size_t MAX_ENTRIES = 10000;

    /**
     * @brief Persist audit entry to storage
     * @param entry The entry to persist
     * @return true if successful, false otherwise
     */
    bool persist_entry(const AuditEntry& entry);

    /**
     * @brief Load audit entries from storage
     * @return true if successful, false otherwise
     */
    bool load_entries();

    /**
     * @brief Persist all audit entries to storage
     * @return true if successful, false otherwise
     */
    bool persist_entries();
};

} // namespace sqlcc

#endif // SQLCC_SECURITY_AUDIT_TRAIL_H_
