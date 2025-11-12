# Database Principles Final Project: AI-Driven Micro Database System Development

> 🎯 **Database Development Guide for Sophomore Students** - Build your own database system with AI assistance from scratch!

## 📦 Current Version: v0.3.9

### 🆕 v0.3.9 New Features (Deadlock Fix and Concurrent Performance Optimization)
- **Deadlock Fix**: Fixed deadlock issues in BufferPool, implemented lock release mechanism before disk I/O
- **Concurrent Performance Optimization**: 8-thread concurrent throughput reaches 2044.99 ops/sec, significantly improving system response speed
- **Code Robustness Enhancement**: Added dedicated deadlock test files to comprehensively verify fix effectiveness
- **Documentation Update**: Improved version information, performance test results and code statistics
- **Test File Organization**: Standardized test file structure, moved temporary test files to tests directory

## 📊 Code Scale Statistics

### Core Code Statistics (v0.3.9)
| Metric | Number | Description |
|--------|--------|-------------|
| **Source Code Lines** | 4,712 lines | Total core C++ code lines |
| **Source Files** | 56 | Total .cc and .h files |
| **Class Count** | 11 | Total core class definitions |
| **Main Modules** | 5 | Storage engine, buffer pool, disk management, configuration management, logging system |
| **Line Coverage** | 83.3% | Overall code line coverage |
| **Function Coverage** | 90.7% | Overall function coverage |

### 🧪 Test File Structure
- **Temporary Test Files**: Organized under `tests/temporary/` directory
  - `tests/temporary/test_simple.cc` - BufferPool basic functionality quick verification test
  - `tests/temporary/test_page_id_fix.cc` - Page ID allocation logic fix verification test
  - `tests/temporary/test_sync_functionality.cc` - Disk synchronization functionality verification test
  - `tests/temporary/test_deadlock_fix_simple.cc` - Deadlock fix verification test
- Detailed documentation: [TEMPORARY_TEST_FILES.md](docs/TEMPORARY_TEST_FILES.md)

### Core Performance Metrics
| Test Type | Throughput | Latency | Scaling |
|-----------|------------|---------|----------|
| **8-thread Concurrent** | 2,044.99 ops/sec | 3.628ms/op | Baseline |
| **4-thread Concurrent** | 1,015.23 ops/sec | 3.596ms/op | Linear scaling |
| **2-thread Concurrent** | 535.33 ops/sec | 3.526ms/op | Linear scaling |
| **1-thread Baseline** | 261.57 ops/sec | 3.629ms/op | Baseline |

## 🚀 Quick Start

### Clone the project
```bash
git clone https://gitee.com/yourusername/sqlcc.git
cd sqlcc
```

### Compile and Test
```bash
# Clean compilation
make clean

# Compile project
make -j$(nproc)

# Run unit tests
make test

# Generate coverage report
make coverage

# Run performance tests
make perf_test
```

## 📚 Related Resources

- **API Documentation**: `docs/doxygen/html/index.html`
- **Coverage Report**: `coverage/index.html`
- **Performance Test Report**: `build/perf_results.json`
- **Change Log**: `ChangeLog.md`
- **Release Notes**: `RELEASE_NOTES_v0.3.9.md`

## 🤝 Contribution Guide

1. Fork the project
2. Create feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
5. Create Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

Thanks to ByteDance Trae AI for providing a powerful AI-assisted programming environment, making database system development more efficient and interesting!

---

**🎯 Remember: Code with AI, don't be coded by AI!**
