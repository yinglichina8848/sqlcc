#include "crud_performance_test.h"

int main() {
  sqlcc::test::CRUDPerformanceTest test;
  test.RunAllTests();
  return 0;
}