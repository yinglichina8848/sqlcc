// SQLCC Storage Engine Precompiled Header File
// Generated automatically - DO NOT EDIT

#pragma once

// Core project headers for storage engine
#include "include/storage_engine.h"
#include "storage/index_manager.h"
#include "storage/buffer_pool_sharded.h"
#include "storage/b_plus_tree.h"
#include "storage/disk_manager.h"
#include "storage/wal_writer.h"
#include "storage/checkpoint.h"

// Exception and utility headers
#include "exception.h"
#include "utils/logger.h"
#include "utils/config_manager.h"

// Standard library headers commonly used in storage engine
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <functional>
