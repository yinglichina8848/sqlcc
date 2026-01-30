/**
 * @file message_processor.h
 * @brief 消息处理器头文件
 */

#ifndef SQLCC_NETWORK_MESSAGE_PROCESSOR_H
#define SQLCC_NETWORK_MESSAGE_PROCESSOR_H

#include <memory>

#include "network/session_manager.h"

namespace sqlcc {
namespace network {

// 消息处理器
class MessageProcessor {
public:
    MessageProcessor(std::shared_ptr<SessionManager> session_manager);

private:
    std::shared_ptr<SessionManager> session_manager_;
};

} // namespace network
} // namespace sqlcc

#endif // SQLCC_NETWORK_MESSAGE_PROCESSOR_H
