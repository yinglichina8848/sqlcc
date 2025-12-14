#!/bin/bash

# Basic SQL Test - Robust client-server testing with proper error handling
echo "=== Robust SQLCC Client-Server Test ==="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Function to check if port is in use
check_port() {
    if lsof -Pi :$1 -sTCP:LISTEN -t >/dev/null; then
        return 0  # Port is in use
    else
        return 1  # Port is free
    fi
}

# Function to kill process on port
kill_port_process() {
    local port=$1
    local pid=$(lsof -ti:$port)
    if [ ! -z "$pid" ]; then
        print_warning "Killing existing process on port $port (PID: $pid)"
        kill -9 $pid 2>/dev/null
        sleep 2
    fi
}

# Function to wait for server startup
wait_for_server() {
    local port=$1
    local timeout=10
    local count=0

    print_status "Waiting for server to start on port $port..."
    while ! check_port $port; do
        if [ $count -ge $timeout ]; then
            print_error "Server failed to start within $timeout seconds"
            return 1
        fi
        sleep 1
        count=$((count + 1))
        echo -n "."
    done
    echo ""
    print_success "Server is now listening on port $port"
    return 0
}

# Check if server binary exists
if [ ! -f "./bazel-bin/sqlcc" ]; then
    print_error "Server binary not found: ./bazel-bin/sqlcc"
    print_error "Please build the server first with: bazel build //:sqlcc"
    exit 1
fi

# Kill any existing server on port 18647
kill_port_process 18647

# Start server
print_status "Starting SQLCC server on port 18647..."
./bazel-bin/sqlcc -p 18647 &
SERVER_PID=$!

# Trap to ensure server is killed on exit
trap "print_warning 'Cleaning up...'; kill -9 $SERVER_PID 2>/dev/null; exit" INT TERM EXIT

# Wait for server to start
if ! wait_for_server 18647; then
    print_error "Failed to start server"
    exit 1
fi

# Create a simple SQL client script with better error handling and debug output
cat > simple_sql_client.cpp << 'EOF'
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <thread>
#include <chrono>

// Message header structure matching the server's protocol
#pragma pack(push, 1)
struct MessageHeader {
    uint32_t magic;      // 0x53514C43 ('SQLC')
    uint32_t length;     // Body length
    uint16_t type;       // Message type
    uint16_t flags;      // Flags
    uint32_t sequence_id; // Sequence ID
};
#pragma pack(pop)

const char* messageTypeToString(uint16_t type) {
    switch (type) {
        case 0: return "CONNECT";
        case 1: return "CONN_ACK";
        case 2: return "AUTH";
        case 3: return "AUTH_ACK";
        case 4: return "QUERY";
        case 5: return "QUERY_RESULT";
        case 6: return "ERROR";
        case 7: return "CLOSE";
        case 8: return "KEY_EXCHANGE";
        case 9: return "KEY_EXCHANGE_ACK";
        default: return "UNKNOWN";
    }
}

bool sendMessage(int sock, const MessageHeader& header, const std::string& body = "") {
    std::cout << "[CLIENT] Sending " << messageTypeToString(header.type)
              << " message (seq=" << header.sequence_id
              << ", len=" << header.length << ")" << std::endl;

    // Send header
    if (send(sock, &header, sizeof(MessageHeader), 0) != sizeof(MessageHeader)) {
        std::cerr << "[CLIENT] Failed to send message header" << std::endl;
        return false;
    }

    // Send body if any
    if (!body.empty()) {
        if (send(sock, body.c_str(), body.length(), 0) != (ssize_t)body.length()) {
            std::cerr << "[CLIENT] Failed to send message body" << std::endl;
            return false;
        }
        std::cout << "[CLIENT] Sent body: '" << body << "'" << std::endl;
    }

    return true;
}

bool receiveMessage(int sock, MessageHeader& header, std::string& body) {
    // Set a timeout for receive operations
    struct timeval tv;
    tv.tv_sec = 5;  // 5 second timeout
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Receive header
    ssize_t header_bytes = recv(sock, &header, sizeof(MessageHeader), MSG_WAITALL);
    if (header_bytes != sizeof(MessageHeader)) {
        if (header_bytes == 0) {
            std::cerr << "[CLIENT] Connection closed by server" << std::endl;
        } else if (header_bytes < 0) {
            std::cerr << "[CLIENT] Failed to receive message header: " << strerror(errno) << std::endl;
        } else {
            std::cerr << "[CLIENT] Incomplete message header (got " << header_bytes << " bytes, expected " << sizeof(MessageHeader) << ")" << std::endl;
        }
        return false;
    }

    std::cout << "[CLIENT] Received " << messageTypeToString(header.type)
              << " message (seq=" << header.sequence_id
              << ", len=" << header.length << ")" << std::endl;

    // Validate magic number
    if (header.magic != 0x53514C43) {
        std::cerr << "[CLIENT] Invalid magic number: " << std::hex << header.magic << std::dec << std::endl;
        return false;
    }

    // Receive body if any
    if (header.length > 0) {
        char* buffer = new char[header.length + 1];
        ssize_t body_bytes = recv(sock, buffer, header.length, MSG_WAITALL);
        if (body_bytes != (ssize_t)header.length) {
            if (body_bytes < 0) {
                std::cerr << "[CLIENT] Failed to receive message body: " << strerror(errno) << std::endl;
            } else {
                std::cerr << "[CLIENT] Incomplete message body (expected "
                          << header.length << ", got " << body_bytes << ")" << std::endl;
            }
            delete[] buffer;
            return false;
        }
        buffer[header.length] = '\0';
        body = buffer;
        delete[] buffer;
        std::cout << "[CLIENT] Received body: '" << body << "'" << std::endl;
    }

    return true;
}

int main() {
    std::cout << "[CLIENT] Starting SQLCC test client..." << std::endl;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "[CLIENT] Socket creation failed: " << strerror(errno) << std::endl;
        return 1;
    }

    std::cout << "[CLIENT] Socket created successfully" << std::endl;

    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(18647);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    std::cout << "[CLIENT] Connecting to 127.0.0.1:18647..." << std::endl;
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "[CLIENT] Connection failed: " << strerror(errno) << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "[CLIENT] Connected to server successfully" << std::endl;

    MessageHeader header;
    std::string body;

    try {
        // Step 1: Send CONNECT message
        std::cout << "[CLIENT] Step 1: Sending CONNECT message..." << std::endl;
        MessageHeader connect_header = {0x53514C43, 0, 0, 0x02, 1}; // CONNECT=0, disable auth
        if (!sendMessage(sock, connect_header)) {
            throw std::runtime_error("Failed to send CONNECT message");
        }

        // Wait a bit for server to process
        std::cout << "[CLIENT] Waiting for server response..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Receive CONN_ACK
        if (!receiveMessage(sock, header, body)) {
            throw std::runtime_error("Failed to receive CONN_ACK");
        }
        if (header.type != 1) { // CONN_ACK
            throw std::runtime_error("Expected CONN_ACK, got " + std::string(messageTypeToString(header.type)));
        }
        std::cout << "[CLIENT] ✓ Connection acknowledged" << std::endl;

        // Step 2: Send AUTH message (empty credentials since auth disabled)
        std::cout << "[CLIENT] Step 2: Sending AUTH message..." << std::endl;
        uint32_t username_len = 0;
        uint32_t password_len = 0;
        std::string auth_body;
        auth_body.append(reinterpret_cast<char*>(&username_len), sizeof(uint32_t));
        auth_body.append(reinterpret_cast<char*>(&password_len), sizeof(uint32_t));

        MessageHeader auth_header = {0x53514C43, (uint32_t)auth_body.length(), 2, 0, 2}; // AUTH
        if (!sendMessage(sock, auth_header, auth_body)) {
            throw std::runtime_error("Failed to send AUTH message");
        }

        // Receive AUTH_ACK
        if (!receiveMessage(sock, header, body)) {
            throw std::runtime_error("Failed to receive AUTH_ACK");
        }
        if (header.type != 3) { // AUTH_ACK
            throw std::runtime_error("Expected AUTH_ACK, got " + std::string(messageTypeToString(header.type)));
        }
        if (header.flags != 0) {
            throw std::runtime_error("Authentication failed (flags=" + std::to_string(header.flags) + ")");
        }
        std::cout << "[CLIENT] ✓ Authentication successful" << std::endl;

        // Step 3: Send QUERY message
        std::cout << "[CLIENT] Step 3: Sending QUERY message..." << std::endl;
        std::string sql = "SELECT 1;";
        MessageHeader query_header = {0x53514C43, (uint32_t)sql.length(), 3, 0, 3}; // QUERY
        if (!sendMessage(sock, query_header, sql)) {
            throw std::runtime_error("Failed to send QUERY message");
        }

        // Receive QUERY_RESULT
        if (!receiveMessage(sock, header, body)) {
            throw std::runtime_error("Failed to receive QUERY_RESULT");
        }
        if (header.type != 5) { // QUERY_RESULT
            if (header.type == 6) { // ERROR
                throw std::runtime_error("Query failed: " + body);
            } else {
                throw std::runtime_error("Expected QUERY_RESULT, got " + std::string(messageTypeToString(header.type)));
            }
        }
        std::cout << "[CLIENT] ✓ Query executed successfully" << std::endl;
        std::cout << "[CLIENT] Result: " << body << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "[CLIENT] Error: " << e.what() << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "[CLIENT] Test completed successfully!" << std::endl;
    close(sock);
    return 0;
}
EOF

# Compile the client with verbose output
print_status "Compiling enhanced SQL client..."
if ! g++ -o simple_sql_client simple_sql_client.cpp -std=c++17 2>&1; then
    print_error "Failed to compile client"
    kill -9 $SERVER_PID 2>/dev/null
    exit 1
fi
print_success "Client compiled successfully"

# Run client
print_status "Running SQL client..."
if ./simple_sql_client; then
    print_success "Client executed successfully"
    CLIENT_SUCCESS=true
else
    print_error "Client execution failed"
    CLIENT_SUCCESS=false
fi

# Stop server gracefully first
print_status "Stopping server gracefully..."
kill $SERVER_PID 2>/dev/null
sleep 2

# Force kill if still running
if kill -0 $SERVER_PID 2>/dev/null; then
    print_warning "Server still running, force killing..."
    kill -9 $SERVER_PID 2>/dev/null
    sleep 1
fi

# Verify port is freed
if check_port 18647; then
    print_warning "Port 18647 still in use after server shutdown"
else
    print_success "Port 18647 freed successfully"
fi

# Cleanup
rm -f simple_sql_client simple_sql_client.cpp

# Final result
echo ""
if [ "$CLIENT_SUCCESS" = true ]; then
    print_success "=== SQLCC Client-Server Test PASSED ==="
    exit 0
else
    print_error "=== SQLCC Client-Server Test FAILED ==="
    exit 1
fi
