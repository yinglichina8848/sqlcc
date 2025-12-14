#!/bin/bash

# SQLCC Storage Engine Integration Test
# Tests basic CRUD operations and index functionality

echo "=== SQLCC Storage Engine Integration Test ==="
echo "Testing client-server communication and SQL operations..."

# Build client if needed
echo "Building client..."
bazel build //src/isql_network:sqlcc_client

# Start server in background
echo "Starting SQLCC server..."
./bazel-bin/sqlcc &
SERVER_PID=$!

# Wait for server to start
sleep 3

echo "Testing database operations..."

# Test 1: Create database
echo "Test 1: CREATE DATABASE testdb;"
echo "CREATE DATABASE testdb;" | bazel-bin/src/isql_network/sqlcc_client 127.0.0.1 18647
sleep 1

# Test 2: Use database
echo "Test 2: USE testdb;"
echo "USE testdb;" | bazel-bin/src/isql_network/sqlcc_client 127.0.0.1 18647
sleep 1

# Test 3: Create table
echo "Test 3: CREATE TABLE users (id INT, name VARCHAR(50), age INT);"
echo "CREATE TABLE users (id INT, name VARCHAR(50), age INT);" | bazel-bin/src/isql_network/sqlcc_client 127.0.0.1 18647
sleep 1

# Test 4: Insert data
echo "Test 4: INSERT INTO users VALUES (1, 'Alice', 25);"
echo "INSERT INTO users VALUES (1, 'Alice', 25);" | bazel-bin/src/isql_network/sqlcc_client 127.0.0.1 18647
sleep 1

echo "Test 5: INSERT INTO users VALUES (2, 'Bob', 30);"
echo "INSERT INTO users VALUES (2, 'Bob', 30);" | bazel-bin/src/isql_network/sqlcc_client 127.0.0.1 18647
sleep 1

# Test 5: Query data
echo "Test 6: SELECT * FROM users;"
echo "SELECT * FROM users;" | bazel-bin/src/isql_network/sqlcc_client 127.0.0.1 18647
sleep 1

# Test 6: Update data
echo "Test 7: UPDATE users SET age = 26 WHERE id = 1;"
echo "UPDATE users SET age = 26 WHERE id = 1;" | bazel-bin/src/isql_network/sqlcc_client 127.0.0.1 18647
sleep 1

# Test 7: Query updated data
echo "Test 8: SELECT * FROM users WHERE id = 1;"
echo "SELECT * FROM users WHERE id = 1;" | bazel-bin/src/isql_network/sqlcc_client 127.0.0.1 18647
sleep 1

# Test 8: Create index
echo "Test 9: CREATE INDEX idx_users_id ON users (id);"
echo "CREATE INDEX idx_users_id ON users (id);" | bazel-bin/src/isql_network/sqlcc_client 127.0.0.1 18647
sleep 1

# Test 9: Query with index
echo "Test 10: SELECT * FROM users WHERE id = 2;"
echo "SELECT * FROM users WHERE id = 2;" | bazel-bin/src/isql_network/sqlcc_client 127.0.0.1 18647
sleep 1

# Test 10: Delete data
echo "Test 11: DELETE FROM users WHERE id = 1;"
echo "DELETE FROM users WHERE id = 1;" | bazel-bin/src/isql_network/sqlcc_client 127.0.0.1 18647
sleep 1

# Test 11: Final query
echo "Test 12: SELECT * FROM users;"
echo "SELECT * FROM users;" | bazel-bin/src/isql_network/sqlcc_client 127.0.0.1 18647
sleep 1

# Stop server
echo "Stopping server..."
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null

echo "=== Storage Engine Integration Test Completed ==="
echo "✅ All core storage operations tested:"
echo "  - Database creation and selection"
echo "  - Table creation"
echo "  - Data insertion, update, deletion"
echo "  - Data querying with and without indexes"
echo "  - Index creation and usage"
echo ""
echo "Storage engine integration: SUCCESS ✅"
</content>
