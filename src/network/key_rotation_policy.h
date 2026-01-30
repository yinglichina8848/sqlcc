/**
 * @file key_rotation_policy.h
 * @brief 密钥轮换策略头文件
 */

#ifndef SQLCC_NETWORK_KEY_ROTATION_POLICY_H
#define SQLCC_NETWORK_KEY_ROTATION_POLICY_H

namespace sqlcc {
namespace network {

// 密钥轮换策略
class KeyRotationPolicy {
public:
    explicit KeyRotationPolicy(size_t interval_messages = 1000)
        : interval_(interval_messages) {}
    bool ShouldRotate(size_t messages_sent) const { return interval_ > 0 && (messages_sent % interval_) == 0; }
private:
    size_t interval_;
};

} // namespace network
} // namespace sqlcc

#endif // SQLCC_NETWORK_KEY_ROTATION_POLICY_H
