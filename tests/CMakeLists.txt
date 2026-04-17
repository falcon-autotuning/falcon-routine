find_package(GTest REQUIRED)
include(GoogleTest)

set(DLL_SEARCH_PATHS "${CMAKE_BINARY_DIR}}/tests" "${CMAKE_BINARY_DIR}")
string(REPLACE ";" "\\;" TEST_PATH_ENV "${DLL_SEARCH_PATHS}")
set(TEST_PATH_ENV "${TEST_PATH_ENV};$ENV{PATH}")

# Test utilities
add_library(falcon-routine-test-utils INTERFACE)

target_link_libraries(falcon-routine-test-utils
  INTERFACE
    falcon-routine
    GTest::gtest
)

target_include_directories(falcon-routine-test-utils
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/test_utils
)

set_target_properties(falcon-routine-test-utils PROPERTIES
  LINKER_LANGUAGE CXX
)

# Unit tests
add_executable(falcon_routine_unit_tests
  unit/test_database.cpp
  unit/test_log.cpp
)

target_link_libraries(falcon_routine_unit_tests
  PRIVATE
    falcon-routine
    falcon-routine-test-utils
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME falcon_routine_unit_tests COMMAND falcon_routine_unit_tests)
if(WIN32)
  set_tests_properties(falcon_routine_unit_tests
    PROPERTIES ENVIRONMENT "PATH=${TEST_PATH_ENV}")
endif()


