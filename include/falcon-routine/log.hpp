#pragma once

#include "falcon-routine/export.h"
#include <memory>
#include <string>

// Forward declare spdlog logger
namespace spdlog {
class logger;
}

/**
 * @brief Logging utilities using spdlog
 *
 * Lazy-initialized logger. Reads configuration from environment:
 *   LOG_LEVEL - debug, info, warn, error (default: info)
 *   LOG_FILE  - path to log file (default: logs to stdout)
 *   LOG_PATTERN - custom log pattern (optional)
 *
 * Example:
 *   export LOG_LEVEL=debug
 *   export LOG_FILE=/var/log/falcon-routine.log
 */
namespace falcon::routine::log {

/**
 * @brief Log at trace level (very verbose)
 */
void FALCON_ROUTINE_API trace(const std::string &msg);

/**
 * @brief Log at debug level
 */
void FALCON_ROUTINE_API debug(const std::string &msg);

/**
 * @brief Log at info level
 */
void FALCON_ROUTINE_API info(const std::string &msg);

/**
 * @brief Log at warning level
 */
void FALCON_ROUTINE_API warn(const std::string &msg);

/**
 * @brief Log at error level
 */
void FALCON_ROUTINE_API error(const std::string &msg);

/**
 * @brief Log at critical level
 */
void FALCON_ROUTINE_API critical(const std::string &msg);

/**
 * @brief Set log level programmatically
 * @param level One of: trace, debug, info, warn, error, critical, off
 */
void FALCON_ROUTINE_API set_level(const std::string &level);

/**
 * @brief Flush logs to file/output
 */
void FALCON_ROUTINE_API flush();

/**
 * @brief Get the underlying spdlog logger (advanced use)
 */
std::shared_ptr<spdlog::logger> FALCON_ROUTINE_API get_logger();

} // namespace falcon::routine::log
