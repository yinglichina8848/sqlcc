#ifndef SQLCC_NETWORK_H
#define SQLCC_NETWORK_H

#include <cstdint>

// Message types for network protocol
enum MessageType {
    CONNECT = 1,
    QUERY = 2,
    RESPONSE = 3,
    ERROR = 4,
    AUTHENTICATE = 5,
    CLOSE = 6,
    KEY_EXCHANGE = 7,
    KEY_EXCHANGE_ACK = 8
};

// Expected magic number for message validation
const uint32_t EXPECTED_MAGIC = 0x534C4343; // "SLCC" in ASCII

namespace sqlcc {
namespace network {

} // namespace network
} // namespace sqlcc

#endif // SQLCC_NETWORK_H
