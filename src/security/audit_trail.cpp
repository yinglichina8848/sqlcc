/**
 * @file audit_trail.cpp
 * @brief Implementation of audit trail component for SQLCC enterprise features
 */

#include "audit_trail.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <fstream>
#include <map>

namespace sqlcc {

// AuditEntry implementation
AuditEntry::AuditEntry(AuditEventType event_type,
                       const std::string& user_name,
                       const std::string& action,
                       const std::string& details)
    : event_type_(event_type),
      user_name_(user_name),
      action_(action),
      details_(details),
      timestamp_(std::chrono::system_clock::now()) {
}

AuditEntry::AuditEventType AuditEntry::get_event_type() const {
    return event_type_;
}

const std::string& AuditEntry::get_user_name() const {
    return user_name_;
}

const std::string& AuditEntry::get_action() const {
    return action_;
}

const std::string& AuditEntry::get_details() const {
    return details_;
}

std::chrono::system_clock::time_point AuditEntry::get_timestamp() const {
    return timestamp_;
}

std::string AuditEntry::event_type_to_string(AuditEventType type) {
    switch (type) {
        case AuditEventType::LOGIN:
            return "LOGIN";
        case AuditEventType::LOGOUT:
            return "LOGOUT";
        case AuditEventType::QUERY_EXECUTION:
            return "QUERY_EXECUTION";
        case AuditEventType::TABLE_MODIFICATION:
            return "TABLE_MODIFICATION";
        case AuditEventType::USER_MANAGEMENT:
            return "USER_MANAGEMENT";
        case AuditEventType::SECURITY_CHANGE:
            return "SECURITY_CHANGE";
        case AuditEventType::SYSTEM_CONFIG_CHANGE:
            return "SYSTEM_CONFIG_CHANGE";
        case AuditEventType::BACKUP_OPERATION:
            return "BACKUP_OPERATION";
        case AuditEventType::RESTORE_OPERATION:
            return "RESTORE_OPERATION";
        default:
            return "UNKNOWN";
    }
}

// AuditTrail implementation
AuditTrail::AuditTrail()
    : enabled_(true) {
}

AuditTrail::~AuditTrail() {
    shutdown();
}

bool AuditTrail::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    enabled_ = true;
    return load_entries();
}

void AuditTrail::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    enabled_ = false;
    persist_entries();
}

bool AuditTrail::record_event(const AuditEntry& event) {
    if (!enabled_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    audit_entries_.push_back(event);

    // Keep only the most recent entries
    if (audit_entries_.size() > MAX_ENTRIES) {
        audit_entries_.erase(audit_entries_.begin());
    }

    return persist_entry(event);
}

bool AuditTrail::record_event(AuditEntry::AuditEventType event_type,
                              const std::string& user_name,
                              const std::string& action,
                              const std::string& details) {
    AuditEntry event(event_type, user_name, action, details);
    return record_event(event);
}

std::vector<AuditEntry> AuditTrail::get_entries(
    const std::chrono::system_clock::time_point& start_time,
    const std::chrono::system_clock::time_point& end_time) {

    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<AuditEntry> result;

    for (const auto& entry : audit_entries_) {
        if (entry.get_timestamp() >= start_time && entry.get_timestamp() <= end_time) {
            result.push_back(entry);
        }
    }

    return result;
}

std::vector<AuditEntry> AuditTrail::get_entries_for_user(
    const std::string& user_name, size_t limit) {

    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<AuditEntry> result;

    for (const auto& entry : audit_entries_) {
        if (entry.get_user_name() == user_name) {
            result.push_back(entry);
            if (result.size() >= limit) {
                break;
            }
        }
    }

    return result;
}

std::vector<AuditEntry> AuditTrail::get_entries_by_type(
    AuditEntry::AuditEventType event_type, size_t limit) {

    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<AuditEntry> result;

    for (const auto& entry : audit_entries_) {
        if (entry.get_event_type() == event_type) {
            result.push_back(entry);
            if (result.size() >= limit) {
                break;
            }
        }
    }

    return result;
}

std::string AuditTrail::generate_compliance_report(
    const std::chrono::system_clock::time_point& start_time,
    const std::chrono::system_clock::time_point& end_time) {

    auto entries = get_entries(start_time, end_time);

    std::ostringstream report;
    report << "=== SQLCC Audit Trail Compliance Report ===\n";
    report << "Period: " << std::chrono::system_clock::to_time_t(start_time)
           << " to " << std::chrono::system_clock::to_time_t(end_time) << "\n";
    report << "Total Events: " << entries.size() << "\n\n";

    // Count events by type
    std::map<AuditEntry::AuditEventType, size_t> event_counts;
    for (const auto& entry : entries) {
        event_counts[entry.get_event_type()]++;
    }

    report << "Event Summary:\n";
    for (const auto& pair : event_counts) {
        report << "  " << AuditEntry::event_type_to_string(pair.first)
               << ": " << pair.second << "\n";
    }

    report << "\nDetailed Events:\n";
    for (const auto& entry : entries) {
        auto time_t = std::chrono::system_clock::to_time_t(entry.get_timestamp());
        report << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] "
               << entry.get_user_name() << " - "
               << AuditEntry::event_type_to_string(entry.get_event_type()) << " - "
               << entry.get_action();
        if (!entry.get_details().empty()) {
            report << " (" << entry.get_details() << ")";
        }
        report << "\n";
    }

    return report.str();
}

size_t AuditTrail::clear_old_entries(int retention_days) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto cutoff_time = std::chrono::system_clock::now() -
                      std::chrono::hours(24 * retention_days);

    size_t removed_count = 0;
    auto it = std::remove_if(audit_entries_.begin(), audit_entries_.end(),
        [cutoff_time, &removed_count](const AuditEntry& entry) {
            if (entry.get_timestamp() < cutoff_time) {
                removed_count++;
                return true;
            }
            return false;
        });

    audit_entries_.erase(it, audit_entries_.end());
    return removed_count;
}

size_t AuditTrail::get_total_entries() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return audit_entries_.size();
}

bool AuditTrail::is_enabled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return enabled_;
}

void AuditTrail::set_enabled(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    enabled_ = enabled;
}

bool AuditTrail::persist_entry(const AuditEntry& entry) {
    // TODO: Implement persistent storage (file/database)
    // For now, just return true as entries are kept in memory
    return true;
}

bool AuditTrail::load_entries() {
    // TODO: Implement loading from persistent storage
    // For now, just return true
    return true;
}

bool AuditTrail::persist_entries() {
    // TODO: Implement persisting all entries to storage
    // For now, just return true
    return true;
}

} // namespace sqlcc
