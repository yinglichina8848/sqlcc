#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <iostream>
#include <fstream>

#include "core/database_manager.h"
#include "sql/parser/parser.h"
#include "sql/executor/executor.h"
#include "storage/storage_engine.h"

using namespace sqlcc;
using namespace std::chrono;

class LargeScaleCRUDTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test database
        db_manager = std::make_unique<DatabaseManager>();
        db_manager->Initialize("test_large_scale_crud.db");
        
        // Create test tables
        CreateTestTables();
        
        // Initialize random number generator
        rng.seed(42);  // Fixed seed for reproducible tests
    }
    
    void TearDown() override {
        // Clean up test database
        db_manager.reset();
    }
    
    void CreateTestTables() {
        // Create large table for performance testing
        std::string create_table_sql = 
            "CREATE TABLE large_test ("
            "id INT PRIMARY KEY,"
            "name VARCHAR(100),"
            "email VARCHAR(100),"
            "age INT,"
            "salary FLOAT,"
            "department VARCHAR(50),"
            "address TEXT,"
            "description TEXT,"
            "created_at TIMESTAMP,"
            "updated_at TIMESTAMP"
            ")";
        
        auto result = db_manager->Execute(create_table_sql);
        ASSERT_TRUE(result->IsSuccess());
        
        // Create index on some columns for query performance
        std::vector<std::string> create_index_sqls = {
            "CREATE INDEX idx_large_test_name ON large_test(name)",
            "CREATE INDEX idx_large_test_age ON large_test(age)",
            "CREATE INDEX idx_large_test_department ON large_test(department)",
            "CREATE INDEX idx_large_test_salary ON large_test(salary)",
            "CREATE INDEX idx_large_test_created_at ON large_test(created_at)"
        };
        
        for (const auto& sql : create_index_sqls) {
            auto index_result = db_manager->Execute(sql);
            // Some indexes might fail depending on implementation, which is OK
        }
    }
    
    std::string GenerateRandomName(int id) {
        std::string first_names[] = {"John", "Jane", "Bob", "Alice", "Charlie", "David", "Eve", "Frank", "Grace", "Henry"};
        std::string last_names[] = {"Smith", "Johnson", "Brown", "Davis", "Wilson", "Miller", "Garcia", "Rodriguez", "Lee", "Clark"};
        
        return first_names[id % 10] + " " + last_names[(id / 10) % 10] + std::to_string(id);
    }
    
    std::string GenerateRandomEmail(int id) {
        std::string domains[] = {"example.com", "test.org", "sample.net", "demo.co", "mock.io"};
        return "user" + std::to_string(id) + "@" + domains[id % 5];
    }
    
    int GenerateRandomAge(int id) {
        // Generate age between 20 and 65
        std::uniform_int_distribution<int> dist(20, 65);
        return dist(rng);
    }
    
    float GenerateRandomSalary(int id) {
        // Generate salary between 30000 and 120000
        std::uniform_real_distribution<float> dist(30000.0f, 120000.0f);
        return dist(rng);
    }
    
    std::string GenerateRandomDepartment(int id) {
        std::string departments[] = {"Engineering", "Marketing", "Sales", "HR", "Finance", "Operations"};
        return departments[id % 6];
    }
    
    std::string GenerateRandomAddress(int id) {
        std::string streets[] = {"Main St", "Oak Ave", "Park Rd", "Elm Dr", "Pine Ln", "Maple Ct"};
        std::string cities[] = {"New York", "Los Angeles", "Chicago", "Houston", "Phoenix", "Philadelphia"};
        
        return std::to_string(100 + (id % 900)) + " " + streets[id % 6] + ", " + cities[id % 6] + ", " + std::to_string(10000 + (id % 90000));
    }
    
    std::string GenerateRandomDescription(int id) {
        std::string descriptions[] = {
            "Experienced professional with excellent skills",
            "Detail-oriented individual with strong work ethic",
            "Creative problem solver with innovative ideas",
            "Team player with excellent communication skills",
            "Results-driven professional with proven track record"
        };
        
        return descriptions[id % 5];
    }
    
    std::string GenerateCurrentTimestamp() {
        auto now = system_clock::now();
        auto time_t = system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&time_t);
        
        char buffer[20];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
        return std::string(buffer);
    }
    
    std::unique_ptr<DatabaseManager> db_manager;
    std::mt19937 rng;
    const int LARGE_DATA_SIZE = 100000;  // 10万条数据
};

TEST_F(LargeScaleCRUDTest, BulkInsertPerformance) {
    std::cout << "=== Bulk Insert Performance Test ===" << std::endl;
    
    // Measure time for bulk insert
    auto start_time = high_resolution_clock::now();
    
    // Use batch insert for better performance
    const int BATCH_SIZE = 1000;
    const int BATCH_COUNT = LARGE_DATA_SIZE / BATCH_SIZE;
    
    for (int batch = 0; batch < BATCH_COUNT; batch++) {
        std::string sql = "INSERT INTO large_test VALUES ";
        std::vector<std::string> value_strings;
        
        for (int i = 0; i < BATCH_SIZE; i++) {
            int id = batch * BATCH_SIZE + i + 1;
            std::string timestamp = GenerateCurrentTimestamp();
            
            std::string value_str = "(" +
                std::to_string(id) + ", '" +
                GenerateRandomName(id) + "', '" +
                GenerateRandomEmail(id) + "', " +
                std::to_string(GenerateRandomAge(id)) + ", " +
                std::to_string(GenerateRandomSalary(id)) + ", '" +
                GenerateRandomDepartment(id) + "', '" +
                GenerateRandomAddress(id) + "', '" +
                GenerateRandomDescription(id) + "', '" +
                timestamp + "', '" +
                timestamp + "')";
            
            value_strings.push_back(value_str);
        }
        
        sql += std::accumulate(value_strings.begin() + 1, value_strings.end(), value_strings[0],
                              [](const std::string& a, const std::string& b) {
                                  return a + ", " + b;
                              });
        
        auto result = db_manager->Execute(sql);
        ASSERT_TRUE(result->IsSuccess());
    }
    
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);
    
    std::cout << "Inserted " << LARGE_DATA_SIZE << " records in " 
              << duration.count() << " ms" << std::endl;
    std::cout << "Average: " << (float)duration.count() / LARGE_DATA_SIZE 
              << " ms per record" << std::endl;
    std::cout << "Rate: " << (float)LARGE_DATA_SIZE * 1000 / duration.count() 
              << " records per second" << std::endl;
    
    // Verify count
    auto count_result = db_manager->Execute("SELECT COUNT(*) FROM large_test");
    ASSERT_TRUE(count_result->IsSuccess());
    ASSERT_EQ(count_result->GetRows()[0].GetInt(0), LARGE_DATA_SIZE);
}

TEST_F(LargeScaleCRUDTest, PointQueryPerformance) {
    std::cout << "\n=== Point Query Performance Test ===" << std::endl;
    
    // First, insert data if not already present
    auto count_result = db_manager->Execute("SELECT COUNT(*) FROM large_test");
    ASSERT_TRUE(count_result->IsSuccess());
    
    if (count_result->GetRows()[0].GetInt(0) < LARGE_DATA_SIZE) {
        std::cout << "Data not present, running bulk insert first..." << std::endl;
        BulkInsertPerformance();
    }
    
    // Measure time for point queries
    const int QUERIES_TO_RUN = 1000;
    std::vector<int> query_ids;
    
    // Generate random IDs to query
    std::uniform_int_distribution<int> id_dist(1, LARGE_DATA_SIZE);
    for (int i = 0; i < QUERIES_TO_RUN; i++) {
        query_ids.push_back(id_dist(rng));
    }
    
    auto start_time = high_resolution_clock::now();
    
    for (int id : query_ids) {
        std::string sql = "SELECT name, email, age, salary, department FROM large_test WHERE id = " + std::to_string(id);
        auto result = db_manager->Execute(sql);
        ASSERT_TRUE(result->IsSuccess());
        ASSERT_EQ(result->GetRows().size(), 1);
    }
    
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);
    
    std::cout << "Executed " << QUERIES_TO_RUN << " point queries in " 
              << duration.count() << " ms" << std::endl;
    std::cout << "Average: " << (float)duration.count() / QUERIES_TO_RUN 
              << " ms per query" << std::endl;
    std::cout << "Rate: " << (float)QUERIES_TO_RUN * 1000 / duration.count() 
              << " queries per second" << std::endl;
}

TEST_F(LargeScaleCRUDTest, RangeQueryPerformance) {
    std::cout << "\n=== Range Query Performance Test ===" << std::endl;
    
    // First, insert data if not already present
    auto count_result = db_manager->Execute("SELECT COUNT(*) FROM large_test");
    ASSERT_TRUE(count_result->IsSuccess());
    
    if (count_result->GetRows()[0].GetInt(0) < LARGE_DATA_SIZE) {
        std::cout << "Data not present, running bulk insert first..." << std::endl;
        BulkInsertPerformance();
    }
    
    // Measure time for range queries
    struct RangeQuery {
        int min_age;
        int max_age;
        std::string department;
    };
    
    std::vector<RangeQuery> queries = {
        {20, 30, "Engineering"},
        {30, 40, "Marketing"},
        {40, 50, "Sales"},
        {25, 35, "HR"},
        {35, 45, "Finance"},
        {30, 55, "Operations"},
        {20, 65, "Engineering"},  // Wide range
        {40, 50, ""},             // No department filter
        {0, 0, ""}                 // No filters (should return all)
    };
    
    auto start_time = high_resolution_clock::now();
    
    for (const auto& query : queries) {
        std::string sql = "SELECT COUNT(*) FROM large_test WHERE age >= " + 
                         std::to_string(query.min_age) + " AND age <= " + 
                         std::to_string(query.max_age);
        
        if (!query.department.empty()) {
            sql += " AND department = '" + query.department + "'";
        }
        
        auto result = db_manager->Execute(sql);
        ASSERT_TRUE(result->IsSuccess());
        int count = result->GetRows()[0].GetInt(0);
        
        std::cout << "Query: age " << query.min_age << "-" << query.max_age;
        if (!query.department.empty()) {
            std::cout << ", department '" << query.department << "'";
        }
        std::cout << " -> " << count << " records" << std::endl;
    }
    
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);
    
    std::cout << "Executed " << queries.size() << " range queries in " 
              << duration.count() << " ms" << std::endl;
    std::cout << "Average: " << (float)duration.count() / queries.size() 
              << " ms per query" << std::endl;
}

TEST_F(LargeScaleCRUDTest, UpdatePerformance) {
    std::cout << "\n=== Update Performance Test ===" << std::endl;
    
    // First, insert data if not already present
    auto count_result = db_manager->Execute("SELECT COUNT(*) FROM large_test");
    ASSERT_TRUE(count_result->IsSuccess());
    
    if (count_result->GetRows()[0].GetInt(0) < LARGE_DATA_SIZE) {
        std::cout << "Data not present, running bulk insert first..." << std::endl;
        BulkInsertPerformance();
    }
    
    // Measure time for update operations
    const int UPDATES_TO_RUN = 1000;
    std::uniform_int_distribution<int> id_dist(1, LARGE_DATA_SIZE);
    std::uniform_real_distribution<float> salary_dist(30000.0f, 120000.0f);
    
    auto start_time = high_resolution_clock::now();
    
    for (int i = 0; i < UPDATES_TO_RUN; i++) {
        int id = id_dist(rng);
        float new_salary = salary_dist(rng);
        std::string new_timestamp = GenerateCurrentTimestamp();
        
        std::string sql = "UPDATE large_test SET salary = " + std::to_string(new_salary) +
                         ", updated_at = '" + new_timestamp + "' WHERE id = " + std::to_string(id);
        
        auto result = db_manager->Execute(sql);
        ASSERT_TRUE(result->IsSuccess());
    }
    
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);
    
    std::cout << "Executed " << UPDATES_TO_RUN << " update operations in " 
              << duration.count() << " ms" << std::endl;
    std::cout << "Average: " << (float)duration.count() / UPDATES_TO_RUN 
              << " ms per update" << std::endl;
    std::cout << "Rate: " << (float)UPDATES_TO_RUN * 1000 / duration.count() 
              << " updates per second" << std::endl;
}

TEST_F(LargeScaleCRUDTest, DeletePerformance) {
    std::cout << "\n=== Delete Performance Test ===" << std::endl;
    
    // First, insert data if not already present
    auto count_result = db_manager->Execute("SELECT COUNT(*) FROM large_test");
    ASSERT_TRUE(count_result->IsSuccess());
    
    if (count_result->GetRows()[0].GetInt(0) < LARGE_DATA_SIZE) {
        std::cout << "Data not present, running bulk insert first..." << std::endl;
        BulkInsertPerformance();
    }
    
    // Measure time for delete operations
    const int DELETES_TO_RUN = 1000;
    std::uniform_int_distribution<int> id_dist(1, LARGE_DATA_SIZE);
    
    auto start_time = high_resolution_clock::now();
    
    for (int i = 0; i < DELETES_TO_RUN; i++) {
        int id = id_dist(rng);
        
        std::string sql = "DELETE FROM large_test WHERE id = " + std::to_string(id);
        
        auto result = db_manager->Execute(sql);
        // Some IDs might have already been deleted, which is OK
    }
    
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);
    
    std::cout << "Executed " << DELETES_TO_RUN << " delete operations in " 
              << duration.count() << " ms" << std::endl;
    std::cout << "Average: " << (float)duration.count() / DELETES_TO_RUN 
              << " ms per delete" << std::endl;
    std::cout << "Rate: " << (float)DELETES_TO_RUN * 1000 / duration.count() 
              << " deletes per second" << std::endl;
    
    // Verify count after deletes
    auto final_count_result = db_manager->Execute("SELECT COUNT(*) FROM large_test");
    ASSERT_TRUE(final_count_result->IsSuccess());
    std::cout << "Final record count: " << final_count_result->GetRows()[0].GetInt(0) << std::endl;
}

TEST_F(LargeScaleCRUDTest, ComplexQueryPerformance) {
    std::cout << "\n=== Complex Query Performance Test ===" << std::endl;
    
    // First, insert data if not already present
    auto count_result = db_manager->Execute("SELECT COUNT(*) FROM large_test");
    ASSERT_TRUE(count_result->IsSuccess());
    
    if (count_result->GetRows()[0].GetInt(0) < LARGE_DATA_SIZE) {
        std::cout << "Data not present, running bulk insert first..." << std::endl;
        BulkInsertPerformance();
    }
    
    // Measure time for complex queries
    std::vector<std::string> queries = {
        "SELECT department, COUNT(*) as count FROM large_test GROUP BY department ORDER BY count DESC",
        "SELECT department, AVG(salary) as avg_salary, MAX(salary) as max_salary, MIN(salary) as min_salary FROM large_test GROUP BY department",
        "SELECT age, COUNT(*) as count FROM large_test GROUP BY age ORDER BY age",
        "SELECT department, age, COUNT(*) as count FROM large_test GROUP BY department, age ORDER BY department, age",
        "SELECT * FROM large_test WHERE salary > (SELECT AVG(salary) FROM large_test)",
        "SELECT department, COUNT(*) as count FROM large_test WHERE age BETWEEN 30 AND 40 GROUP BY department",
        "SELECT SUBSTR(name, 1, INSTR(name, ' ') - 1) as first_name, COUNT(*) as count FROM large_test GROUP BY first_name ORDER BY count DESC LIMIT 10",
        "SELECT * FROM large_test WHERE name LIKE '%John%' OR email LIKE '%john%' ORDER BY salary DESC LIMIT 100",
        "SELECT department, SUM(salary) as total_salary FROM large_test GROUP BY department HAVING SUM(salary) > 1000000",
        "SELECT * FROM large_test WHERE created_at >= DATE_SUB(NOW(), INTERVAL 1 DAY) ORDER BY created_at DESC"
    };
    
    auto start_time = high_resolution_clock::now();
    
    for (const auto& sql : queries) {
        auto query_start = high_resolution_clock::now();
        auto result = db_manager->Execute(sql);
        auto query_end = high_resolution_clock::now();
        
        ASSERT_TRUE(result->IsSuccess());
        int count = result->GetRows().size();
        auto query_duration = duration_cast<milliseconds>(query_end - query_start);
        
        std::cout << "Query returned " << count << " rows in " 
                  << query_duration.count() << " ms" << std::endl;
        std::cout << "SQL: " << sql.substr(0, 80);
        if (sql.length() > 80) {
            std::cout << "...";
        }
        std::cout << std::endl;
    }
    
    auto end_time = high_resolution_clock::now();
    auto total_duration = duration_cast<milliseconds>(end_time - start_time);
    
    std::cout << "Executed " << queries.size() << " complex queries in " 
              << total_duration.count() << " ms" << std::endl;
    std::cout << "Average: " << (float)total_duration.count() / queries.size() 
              << " ms per query" << std::endl;
}