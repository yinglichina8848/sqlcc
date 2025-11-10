# CMake generated Testfile for 
# Source directory: /home/liying/sqlcc/tests
# Build directory: /home/liying/sqlcc/build-coverage/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(storage_engine_test "/home/liying/sqlcc/build-coverage/bin/storage_engine_test")
set_tests_properties(storage_engine_test PROPERTIES  LABELS "unit_tests" TIMEOUT "300" _BACKTRACE_TRIPLES "/home/liying/sqlcc/tests/CMakeLists.txt;46;add_test;/home/liying/sqlcc/tests/CMakeLists.txt;77;add_sqlcc_test;/home/liying/sqlcc/tests/CMakeLists.txt;0;")
add_test(exception_test "/home/liying/sqlcc/build-coverage/bin/exception_test")
set_tests_properties(exception_test PROPERTIES  LABELS "unit_tests" TIMEOUT "300" _BACKTRACE_TRIPLES "/home/liying/sqlcc/tests/CMakeLists.txt;46;add_test;/home/liying/sqlcc/tests/CMakeLists.txt;77;add_sqlcc_test;/home/liying/sqlcc/tests/CMakeLists.txt;0;")
add_test(buffer_pool_test "/home/liying/sqlcc/build-coverage/bin/buffer_pool_test")
set_tests_properties(buffer_pool_test PROPERTIES  LABELS "unit_tests" TIMEOUT "300" _BACKTRACE_TRIPLES "/home/liying/sqlcc/tests/CMakeLists.txt;46;add_test;/home/liying/sqlcc/tests/CMakeLists.txt;77;add_sqlcc_test;/home/liying/sqlcc/tests/CMakeLists.txt;0;")
add_test(page_test "/home/liying/sqlcc/build-coverage/bin/page_test")
set_tests_properties(page_test PROPERTIES  LABELS "unit_tests" TIMEOUT "300" _BACKTRACE_TRIPLES "/home/liying/sqlcc/tests/CMakeLists.txt;46;add_test;/home/liying/sqlcc/tests/CMakeLists.txt;77;add_sqlcc_test;/home/liying/sqlcc/tests/CMakeLists.txt;0;")
add_test(storage_engine_newpage_test "/home/liying/sqlcc/build-coverage/bin/storage_engine_newpage_test")
set_tests_properties(storage_engine_newpage_test PROPERTIES  LABELS "unit_tests" TIMEOUT "300" _BACKTRACE_TRIPLES "/home/liying/sqlcc/tests/CMakeLists.txt;46;add_test;/home/liying/sqlcc/tests/CMakeLists.txt;77;add_sqlcc_test;/home/liying/sqlcc/tests/CMakeLists.txt;0;")
add_test(disk_manager_test "/home/liying/sqlcc/build-coverage/bin/disk_manager_test")
set_tests_properties(disk_manager_test PROPERTIES  LABELS "unit_tests" TIMEOUT "300" _BACKTRACE_TRIPLES "/home/liying/sqlcc/tests/CMakeLists.txt;46;add_test;/home/liying/sqlcc/tests/CMakeLists.txt;77;add_sqlcc_test;/home/liying/sqlcc/tests/CMakeLists.txt;0;")
add_test(config_manager_test "/home/liying/sqlcc/build-coverage/bin/config_manager_test")
set_tests_properties(config_manager_test PROPERTIES  LABELS "unit_tests" TIMEOUT "300" _BACKTRACE_TRIPLES "/home/liying/sqlcc/tests/CMakeLists.txt;66;add_test;/home/liying/sqlcc/tests/CMakeLists.txt;86;add_sqlcc_enhanced_test;/home/liying/sqlcc/tests/CMakeLists.txt;0;")
add_test(config_manager_enhanced_test "/home/liying/sqlcc/build-coverage/bin/config_manager_enhanced_test")
set_tests_properties(config_manager_enhanced_test PROPERTIES  LABELS "unit_tests" TIMEOUT "300" _BACKTRACE_TRIPLES "/home/liying/sqlcc/tests/CMakeLists.txt;66;add_test;/home/liying/sqlcc/tests/CMakeLists.txt;86;add_sqlcc_enhanced_test;/home/liying/sqlcc/tests/CMakeLists.txt;0;")
add_test(buffer_pool_enhanced_test "/home/liying/sqlcc/build-coverage/bin/buffer_pool_enhanced_test")
set_tests_properties(buffer_pool_enhanced_test PROPERTIES  LABELS "unit_tests" TIMEOUT "300" _BACKTRACE_TRIPLES "/home/liying/sqlcc/tests/CMakeLists.txt;66;add_test;/home/liying/sqlcc/tests/CMakeLists.txt;86;add_sqlcc_enhanced_test;/home/liying/sqlcc/tests/CMakeLists.txt;0;")
add_test(page_enhanced_test "/home/liying/sqlcc/build-coverage/bin/page_enhanced_test")
set_tests_properties(page_enhanced_test PROPERTIES  LABELS "unit_tests" TIMEOUT "300" _BACKTRACE_TRIPLES "/home/liying/sqlcc/tests/CMakeLists.txt;66;add_test;/home/liying/sqlcc/tests/CMakeLists.txt;86;add_sqlcc_enhanced_test;/home/liying/sqlcc/tests/CMakeLists.txt;0;")
add_test(disk_manager_enhanced_test "/home/liying/sqlcc/build-coverage/bin/disk_manager_enhanced_test")
set_tests_properties(disk_manager_enhanced_test PROPERTIES  LABELS "unit_tests" TIMEOUT "300" _BACKTRACE_TRIPLES "/home/liying/sqlcc/tests/CMakeLists.txt;66;add_test;/home/liying/sqlcc/tests/CMakeLists.txt;86;add_sqlcc_enhanced_test;/home/liying/sqlcc/tests/CMakeLists.txt;0;")
add_test(storage_engine_enhanced_test "/home/liying/sqlcc/build-coverage/bin/storage_engine_enhanced_test")
set_tests_properties(storage_engine_enhanced_test PROPERTIES  LABELS "unit_tests" TIMEOUT "300" _BACKTRACE_TRIPLES "/home/liying/sqlcc/tests/CMakeLists.txt;66;add_test;/home/liying/sqlcc/tests/CMakeLists.txt;86;add_sqlcc_enhanced_test;/home/liying/sqlcc/tests/CMakeLists.txt;0;")
subdirs("performance")
