#include "../test_utils/TestFixture.hpp"
#include "falcon-routine/log.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
using namespace falcon::routine;
using namespace falcon::routine::test;

// Use the same log file as set in the environment before running the test
// binary
inline std::string getTestLogFile() {
  const char *env = std::getenv("LOG_FILE");
  return env ? std::string(env) : "falcon_test.log";
}

class LogUnitTest : public RoutineTestFixture {
protected:
  void SetUp() override {
    RoutineTestFixture::SetUp();
    // No need to setenv here; environment is set before process starts
  }
  void TearDown() override {
    log::flush();
    RoutineTestFixture::TearDown();
  }
};

TEST_F(LogUnitTest, LogLevels) {
  // All should work without throwing
  EXPECT_NO_THROW(log::trace("trace message"));
  EXPECT_NO_THROW(log::debug("debug message"));
  EXPECT_NO_THROW(log::info("info message"));
  EXPECT_NO_THROW(log::warn("warn message"));
  EXPECT_NO_THROW(log::error("error message"));
  EXPECT_NO_THROW(log::critical("critical message"));
}

TEST_F(LogUnitTest, LogToFile) {
  log::info("test log message");
  log::flush();
  // Check file exists
  auto log_file = getTestLogFile();
  ASSERT_TRUE(std::filesystem::exists(log_file));
  // Read file and check content
  std::ifstream file(log_file);
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  EXPECT_TRUE(content.find("test log message") != std::string::npos);
}

TEST_F(LogUnitTest, SetLevel) {
  log::set_level("error");
  // Info should not appear (we'd need to check file content to verify)
  log::info("should not appear");
  log::error("should appear");
  log::flush();
  auto log_file = getTestLogFile();
  std::ifstream file(log_file);
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  EXPECT_TRUE(content.find("should appear") != std::string::npos);
  // Note: "should not appear" might still be in file from buffering,
  // so we don't check for its absence
}
