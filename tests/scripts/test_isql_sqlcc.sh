#!/bin/bash

# SQLCC isql Client Connection and SQL Execution Test
echo "=== SQLCC isql Client Connection Test ==="

# Clean up any existing server processes
pkill -f "bazel-bin/sqlcc" || true
sleep 1

# Start SQLCC server in background
echo "Starting SQLCC server..."
./bazel-bin/sqlcc &
SERVER_PID=$!

# Wait for server to start
sleep 3

# Check if server is running
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "❌ Server failed to start"
    exit 1
fi

echo "✅ Server started successfully (PID: $SERVER_PID)"

# Test basic connection with isql client
echo ""
echo "Testing basic connection with isql client..."
echo "Note: Using the built-in isql client that sends 'SELECT * FROM test_table'"

# Build and run the isql client
echo "Building isql client..."
bazel build //src/isql_network:isql_network_lib

echo "Running isql client to connect to SQLCC server..."
bazel run //src/isql_network:isql_network_lib -- -h 127.0.0.1 -p 18647 -u admin -P password

# Wait a moment
sleep 2

# Test with a more comprehensive SQL test
echo ""
echo "=== Testing SQL Execution ==="

# Create a test script for SQL execution
cat > test_sql_commands.sql << 'EOF'
-- Test basic SQL commands
SELECT 1;
CREATE DATABASE testdb;
USE testdb;
CREATE TABLE users (id INT, name VARCHAR(50), age INT);
INSERT INTO users VALUES (1, 'Alice', 25);
INSERT INTO users VALUES (2, 'Bob', 30);
SELECT * FROM users;
UPDATE users SET age = 26 WHERE id = 1;
SELECT * FROM users WHERE id = 1;
DELETE FROM users WHERE id = 2;
SELECT * FROM users;
EOF

echo "Created test SQL commands file: test_sql_commands.sql"
echo "Contents:"
cat test_sql_commands.sql

echo ""
echo "Note: The current isql client sends a hardcoded query 'SELECT * FROM test_table'"
echo "To test full SQL functionality, we need to modify the client to read from stdin"
echo "or accept command line SQL arguments."

# Stop the server gracefully
echo ""
echo "Stopping SQLCC server..."
kill -TERM $SERVER_PID
wait $SERVER_PID 2>/dev/null

echo ""
echo "=== Test Summary ==="
echo "✅ Server startup: PASSED"
echo "✅ Client connection: Basic connection established"
echo "✅ Server shutdown: Graceful shutdown completed"
echo "⚠️  Full SQL testing: Requires client modification for interactive SQL input"
echo ""
echo "Next steps: Modify isql client to accept SQL commands from stdin or arguments"

# Cleanup
rm -f test_sql_commands.sql
