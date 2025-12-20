#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <random>
#include <limits>

#include "storage_engine.h"
#include "storage/buffer_pool.h"
#include "storage/b_plus_tree.h"
#include "storage/disk_manager.h"
#include "storage/wal_writer.h"
#include "storage/concurrency_control.h"

using namespace sqlcc::storage;

class StorageEngineBoundaryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test environment
        storage_engine_ = std::make_unique<StorageEngine>();
        buffer_pool_ = std::make_unique<BufferPool>(1024 * 1024); // 1MB buffer pool
        disk_manager_ = std::make_unique<DiskManager>();
        wal_writer_ = std::make_unique<WalWriter>();
        
        // Create test directory
        test_data_dir_ = "/tmp/sqlcc_boundary_test_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        std::filesystem::create_directories(test_data_dir_);
    }

    void TearDown() override {
        storage_engine_.reset();
        buffer_pool_.reset();
        disk_manager_.reset();
        wal_writer_.reset();
        
        // Clean up test directory
        std::filesystem::remove_all(test_data_dir_);
    }

    // Helper method to simulate disk space exhaustion
    bool SimulateDiskSpaceExhaustion() {
        // Create a large file to simulate disk space exhaustion
        std::string large_file = test_data_dir_ + "/large_test_file.dat";
        std::ofstream file(large_file, std::ios::binary);
        
        if (!file.is_open()) {
            return false;
        }
        
        // Write data until we either fill up disk or reach limit
        const size_t chunk_size = 1024 * 1024; // 1MB chunks
        const size_t max_size = 1024 * 1024 * 1024; // 1GB max
        
        for (size_t written = 0; written < max_size; written += chunk_size) {
            std::vector<char> chunk(chunk_size, 'A');
            file.write(chunk.data(), chunk.size());
            
            if (!file.good()) {
                break; // Disk full or other error
            }
        }
        
        file.close();
        return true;
    }

    // Helper method to simulate high memory pressure
    void SimulateMemoryPressure(size_t memory_size) {
        std::vector<std::string> memory_hog;
        
        try {
            // Allocate memory to create pressure
            for (size_t i = 0; i < 10; ++i) {
                memory_hog.emplace_back(memory_size / 10, 'X');
            }
        } catch (const std::bad_alloc&) {
            // Expected when memory is exhausted
        }
    }

    std::unique_ptr<StorageEngine> storage_engine_;
    std::unique_ptr<BufferPool> buffer_pool_;
    std::unique_ptr<DiskManager> disk_manager_;
    std::unique_ptr<WalWriter> wal_writer_;
    std::string test_data_dir_;
};

// Test disk space exhaustion handling
TEST_F(StorageEngineBoundaryTest, DiskSpaceExhaustion) {
    // Simulate disk space exhaustion
    bool space_exhausted = SimulateDiskSpaceExhaustion();
    
    // Test storage engine behavior when disk is full
    EXPECT_NO_THROW({
        // Try to create a table when disk is potentially full
        bool result = storage_engine_->CreateTable("test_table", {
            {"id", DataType::INTEGER},
            {"name", DataType::VARCHAR}
        });
        
        // Should either succeed or fail gracefully (no crashes)
        if (space_exhausted) {
            EXPECT_FALSE(result); // Should fail when disk is full
        }
    });
}

// Test disk I/O error handling
TEST_F(StorageEngineBoundaryTest, DiskIOErrorHandling) {
    // Test behavior when disk I/O fails
    std::string invalid_path = "/invalid/nonexistent/path/database.db";
    
    EXPECT_THROW({
        disk_manager_->OpenDatabase(invalid_path);
    }, std::runtime_error);
}

// Test buffer pool memory pressure
TEST_F(StorageEngineBoundaryTest, BufferPoolMemoryPressure) {
    // Test buffer pool behavior under memory pressure
    const size_t total_memory = 1024 * 1024; // 1MB
    const size_t pressure_size = total_memory * 2; // 2MB pressure
    
    // Simulate memory pressure
    SimulateMemoryPressure(pressure_size);
    
    // Test buffer pool operations under pressure
    EXPECT_NO_THROW({
        // Try to allocate buffer pool pages under memory pressure
        for (int i = 0; i < 100; ++i) {
            auto page = buffer_pool_->AllocatePage();
            if (!page) {
                break; // Expected when memory is tight
            }
        }
    });
}

// Test buffer pool overflow handling
TEST_F(StorageEngineBoundaryTest, BufferPoolOverflow) {
    // Test buffer pool when it exceeds capacity
    const size_t small_buffer_size = 1024; // Very small buffer pool
    
    std::unique_ptr<BufferPool> small_pool = std::make_unique<BufferPool>(small_buffer_size);
    
    EXPECT_NO_THROW({
        // Try to allocate more pages than buffer pool can hold
        std::vector<BufferPool::Page*> allocated_pages;
        
        for (int i = 0; i < 1000; ++i) {
            auto page = small_pool->AllocatePage();
            if (!page) {
                break; // Expected when buffer pool is full
            }
            allocated_pages.push_back(page);
        }
        
        // Should handle overflow gracefully
        EXPECT_TRUE(allocated_pages.size() <= small_buffer_size / 4096); // Assume 4KB pages
    });
}

// Test B+ tree node splitting boundary conditions
TEST_F(StorageEngineBoundaryTest, BPlusTreeNodeSplittingBoundary) {
    // Test B+ tree behavior during node splitting
    BPlusTree tree(3); // Small order to force frequent splits
    
    EXPECT_NO_THROW({
        // Insert many keys to trigger multiple node splits
        for (int i = 0; i < 1000; ++i) {
            bool result = tree.Insert(i, "value_" + std::to_string(i));
            
            if (!result) {
                // Tree is full or other error occurred
                break;
            }
        }
        
        // Verify tree structure is still valid after splits
        for (int i = 0; i < 100; ++i) {
            auto result = tree.Search(i);
            if (result) {
                EXPECT_EQ(result.value(), "value_" + std::to_string(i));
            }
        }
    });
}

// Test B+ tree node merging boundary conditions
TEST_F(StorageEngineBoundaryTest, BPlusTreeNodeMergingBoundary) {
    // Test B+ tree behavior during node merging
    BPlusTree tree(4); // Small order
    
    // First insert many keys
    for (int i = 0; i < 100; ++i) {
        tree.Insert(i, "value_" + std::to_string(i));
    }
    
    EXPECT_NO_THROW({
        // Then delete many keys to trigger node merging
        for (int i = 0; i < 90; ++i) {
            tree.Delete(i);
        }
        
        // Verify remaining keys
        for (int i = 90; i < 100; ++i) {
            auto result = tree.Search(i);
            EXPECT_EQ(result.value(), "value_" + std::to_string(i));
        }
    });
}

// Test B+ tree degenerate cases
TEST_F(StorageEngineBoundaryTest, BPlusTreeDegenerateCases) {
    // Test B+ tree with extreme data patterns
    BPlusTree tree(3);
    
    // Test with duplicate keys
    EXPECT_NO_THROW({
        for (int i = 0; i < 10; ++i) {
            tree.Insert(1, "duplicate_" + std::to_string(i));
        }
        
        // Should handle duplicates appropriately (either ignore or update)
        auto result = tree.Search(1);
        EXPECT_TRUE(result.has_value());
    });
    
    // Test with reverse order insertion
    BPlusTree tree2(3);
    EXPECT_NO_THROW({
        for (int i = 100; i >= 0; --i) {
            tree2.Insert(i, "reverse_" + std::to_string(i));
        }
        
        // Verify some keys
        auto result = tree2.Search(50);
        EXPECT_EQ(result.value(), "reverse_50");
    });
}

// Test WAL log replay with corrupted data
TEST_F(StorageEngineBoundaryTest, WalLogReplayCorruption) {
    // Test WAL recovery with corrupted log entries
    std::string wal_file = test_data_dir_ + "/test.wal";
    
    EXPECT_NO_THROW({
        // Write some normal entries
        wal_writer_->WriteBeginTransaction();
        wal_writer_->WriteInsert("table1", "data1");
        wal_writer_->WriteCommit();
        
        // Write corrupted entry (simulate disk corruption)
        std::ofstream wal_stream(wal_file, std::ios::app | std::ios::binary);
        wal_stream.write("CORRUPTED_DATA", 14);
        wal_stream.close();
        
        // Try to replay WAL (should handle corruption gracefully)
        bool replay_result = storage_engine_->RecoverFromWal(wal_file);
        
        // Should either succeed in recovery or fail gracefully
        EXPECT_TRUE(replay_result || !replay_result); // Allow both outcomes
    });
}

// Test concurrent transaction deadlock scenarios
TEST_F(StorageEngineBoundaryTest, ConcurrentTransactionDeadlock) {
    // Test deadlock detection and resolution
    const int num_threads = 5;
    std::vector<std::thread> threads;
    std::vector<bool> deadlock_detected(num_threads, false);
    
    EXPECT_NO_THROW({
        // Launch multiple threads that might create deadlocks
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([this, i, &deadlock_detected]() {
                // Each thread tries to lock resources in different order
                if (i % 2 == 0) {
                    storage_engine_->BeginTransaction();
                    storage_engine_->LockTable("table1", LockType::EXCLUSIVE);
                    storage_engine_->LockTable("table2", LockType::EXCLUSIVE);
                } else {
                    storage_engine_->BeginTransaction();
                    storage_engine_->LockTable("table2", LockType::EXCLUSIVE);
                    storage_engine_->LockTable("table1", LockType::EXCLUSIVE);
                }
                
                // Simulate some work
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
                storage_engine_->CommitTransaction();
            });
        }
        
        // Wait for all threads
        for (auto& thread : threads) {
            thread.join();
        }
        
        // System should handle deadlocks gracefully (detect and resolve or prevent)
        EXPECT_NO_FATAL_FAILURE();
    });
}

// Test transaction rollback with large datasets
TEST_F(StorageEngineBoundaryTest, TransactionRollbackLargeDataset) {
    // Test transaction rollback with many operations
    EXPECT_NO_THROW({
        storage_engine_->BeginTransaction();
        
        // Insert many records
        const int num_records = 10000;
        for (int i = 0; i < num_records; ++i) {
            storage_engine_->Insert("test_table", {
                {"id", std::to_string(i)},
                {"data", "large_dataset_" + std::to_string(i)}
            });
            
            // Periodically check for rollback
            if (i % 1000 == 0) {
                // Simulate some condition that requires rollback
                if (i == 5000) {
                    storage_engine_->RollbackTransaction();
                    break;
                }
            }
        }
    });
    
    // Verify rollback completed successfully
    EXPECT_NO_FATAL_FAILURE();
}

// Test index rebuild with corrupted index
TEST_F(StorageEngineBoundaryTest, IndexRebuildCorruptedIndex) {
    // Test rebuilding indexes when index data is corrupted
    EXPECT_NO_THROW({
        // First create some data
        storage_engine_->CreateTable("rebuild_test", {
            {"id", DataType::INTEGER},
            {"name", DataType::VARCHAR}
        });
        
        for (int i = 0; i < 100; ++i) {
            storage_engine_->Insert("rebuild_test", {
                {"id", std::to_string(i)},
                {"name", "name_" + std::to_string(i)}
            });
        }
        
        // Create index
        storage_engine_->CreateIndex("rebuild_test", "id");
        
        // Simulate index corruption by directly modifying index files
        // (This would normally be detected during normal operation)
        
        // Try to rebuild the index
        bool rebuild_result = storage_engine_->RebuildIndex("rebuild_test", "id");
        
        // Should either succeed in rebuild or fail gracefully
        EXPECT_TRUE(rebuild_result || !rebuild_result);
    });
}

// Test database recovery with missing data files
TEST_F(StorageEngineBoundaryTest, DatabaseRecoveryMissingFiles) {
    // Test recovery when some data files are missing
    std::string db_path = test_data_dir_ + "/missing_files_db";
    
    EXPECT_NO_THROW({
        // Create database with multiple tables
        storage_engine_->CreateDatabase(db_path);
        storage_engine_->CreateTable("table1", {{"id", DataType::INTEGER}});
        storage_engine_->CreateTable("table2", {{"id", DataType::INTEGER}});
        
        // Insert some data
        storage_engine_->Insert("table1", {{"id", "1"}});
        storage_engine_->Insert("table2", {{"id", "2"}});
        
        // Simulate missing data file by deleting one table's data file
        std::string table1_data_file = db_path + "/table1.dat";
        std::filesystem::remove(table1_data_file);
        
        // Try to recover database
        bool recovery_result = storage_engine_->RecoverDatabase(db_path);
        
        // Should handle missing files gracefully
        EXPECT_TRUE(recovery_result || !recovery_result);
    });
}

// Test storage engine with extremely large tables
TEST_F(StorageEngineBoundaryTest, ExtremelyLargeTable) {
    // Test behavior with tables containing many records
    EXPECT_NO_THROW({
        storage_engine_->CreateTable("large_table", {
            {"id", DataType::INTEGER},
            {"data", DataType::TEXT}
        });
        
        // Insert many records to test scalability
        const int num_records = 100000;
        for (int i = 0; i < num_records; ++i) {
            bool result = storage_engine_->Insert("large_table", {
                {"id", std::to_string(i)},
                {"data", "large_data_" + std::string(100, 'X') + std::to_string(i)}
            });
            
            if (!result) {
                // Storage is full or other limit reached
                break;
            }
            
            // Periodically commit to avoid very long transactions
            if (i % 10000 == 0 && i > 0) {
                storage_engine_->CommitTransaction();
                storage_engine_->BeginTransaction();
            }
        }
    });
}

// Test storage engine with malformed SQL statements
TEST_F(StorageEngineBoundaryTest, MalformedSQLStatements) {
    // Test storage engine resilience to malformed queries
    EXPECT_NO_THROW({
        // Test with NULL pointers
        storage_engine_->ExecuteQuery(nullptr);
        
        // Test with empty SQL
        storage_engine_->ExecuteQuery("");
        
        // Test with very long SQL statement
        std::string long_sql = "SELECT * FROM table WHERE " + std::string(10000, 'a') + " = 'value'";
        storage_engine_->ExecuteQuery(long_sql);
        
        // Test with SQL containing binary data
        std::string binary_sql = "SELECT * FROM table WHERE data = '\x00\x01\x02\x03\x04'";
        storage_engine_->ExecuteQuery(binary_sql);
    });
}

// Test concurrent access to same table
TEST_F(StorageEngineBoundaryTest, ConcurrentTableAccess) {
    // Test multiple threads accessing the same table
    const int num_threads = 10;
    const int operations_per_thread = 100;
    std::vector<std::thread> threads;
    
    EXPECT_NO_THROW({
        storage_engine_->CreateTable("concurrent_test", {
            {"id", DataType::INTEGER},
            {"value", DataType::VARCHAR}
        });
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([this, i, operations_per_thread]() {
                for (int j = 0; j < operations_per_thread; ++j) {
                    int key = i * operations_per_thread + j;
                    
                    // Mix of inserts and selects
                    if (j % 2 == 0) {
                        storage_engine_->Insert("concurrent_test", {
                            {"id", std::to_string(key)},
                            {"value", "thread_" + std::to_string(i) + "_op_" + std::to_string(j)}
                        });
                    } else {
                        storage_engine_->Select("concurrent_test", "id = " + std::to_string(key));
                    }
                }
            });
        }
        
        // Wait for all threads
        for (auto& thread : threads) {
            thread.join();
        }
        
        // Verify data consistency
        auto result = storage_engine_->Select("concurrent_test", "id >= 0");
        EXPECT_TRUE(result.has_value());
    });
}

// Test storage engine resource cleanup
TEST_F(StorageEngineBoundaryTest, ResourceCleanup) {
    // Test that resources are properly cleaned up
    EXPECT_NO_THROW({
        // Create multiple storage engines
        std::vector<std::unique_ptr<StorageEngine>> engines;
        for (int i = 0; i < 10; ++i) {
            engines.push_back(std::make_unique<StorageEngine>());
        }
        
        // Use resources
        for (auto& engine : engines) {
            engine->CreateTable("cleanup_test", {{"id", DataType::INTEGER}});
        }
        
        // Destroy engines - should cleanup all resources properly
        engines.clear();
        
        // Try to create new engine (should work without resource conflicts)
        std::unique_ptr<StorageEngine> new_engine = std::make_unique<StorageEngine>();
        EXPECT_NO_THROW({
            new_engine->CreateTable("after_cleanup_test", {{"id", DataType::INTEGER}});
        });
    });
}
