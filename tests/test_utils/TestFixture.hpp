#pragma once
#include <cstdlib>
#include <falcon-database/DatabaseConnection.hpp>
#include <gtest/gtest.h>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace falcon::routine::test {

/**
 * @brief Cross-platform environment variable setter
 */
inline int set_env(const std::string &name, const std::string &value) {
#ifdef _WIN32
  std::string env_entry = name + "=" + value;
  return _putenv(env_entry.c_str());
#else
  return setenv(name.c_str(), value.c_str(), 1); // overwrite=1
#endif
}

/**
 * @brief Cross-platform environment variable unsetter
 */
inline int unset_env(const std::string &name) {
#ifdef _WIN32
  std::string env_entry = name + "=";
  return _putenv(env_entry.c_str());
#else
  return unsetenv(name.c_str());
#endif
}

/**
 * @brief Base test fixture with environment setup
 */
class RoutineTestFixture : public ::testing::Test {
protected:
  void SetUp() override { setupEnvironment(); }

  void TearDown() override {
    // Clean up environment variables to avoid pollution
    unset_env("FALCON_DATABASE_URL");
    unset_env("NATS_URL");
  }

  void setupEnvironment() {
    // Database URL from TEST_DATABASE_URL environment variable
    const char *test_db_url = std::getenv("TEST_DATABASE_URL");
    if (test_db_url == nullptr) {
      // Default for tests - use 127.0.0.1 to force TCP connection
      db_url_ = "postgresql://falcon_test:falcon_test_password@127.0.0.1:5433/"
                "falcon_test";
    } else {
      db_url_ = test_db_url;
    }

    // Set FALCON_DATABASE_URL for database connections that use env var
    set_env("FALCON_DATABASE_URL", db_url_);

    // NATS URL from environment or default
    const char *test_nats_url = std::getenv("TEST_NATS_URL");
    if (test_nats_url == nullptr) {
      nats_url_ = "nats://localhost:4222";
    } else {
      nats_url_ = test_nats_url;
    }

    // Set NATS_URL for Hub connections
    set_env("NATS_URL", nats_url_);

    // Set log level to debug for tests
    set_env("LOG_LEVEL", "debug");
  }

  [[nodiscard]] std::string getDatabaseUrl() const { return db_url_; }
  [[nodiscard]] std::string getNatsUrl() const { return nats_url_; }

  std::string db_url_;
  std::string nats_url_;
};

/**
 * @brief Test fixture with database connection
 */
class DatabaseTestFixture : public RoutineTestFixture {
protected:
  void SetUp() override {
    RoutineTestFixture::SetUp();

    // Create database connection with explicit URL
    // This ensures tests don't accidentally use production database
    db_ = std::make_unique<falcon::database::AdminDatabaseConnection>(
        getDatabaseUrl());
    db_->initialize_schema();
    db_->clear_all();
  }

  void TearDown() override {
    if (db_) {
      db_->clear_all();
    }
    RoutineTestFixture::TearDown();
  }

  std::unique_ptr<falcon::database::AdminDatabaseConnection> db_;
};

} // namespace falcon::routine::test
