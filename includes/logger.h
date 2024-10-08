#ifndef LOGGER_H
#define LOGGER_H

/**
 * @file logger.h
 * @brief Logger utility for managing log files in the proxy server.
 * 
 * This file provides the logger's function declarations and the log levels used
 * to categorize messages such as INFO, WARN, and ERROR.
 */

/**
 * @brief Enum representing the log levels.
 * 
 * The different log levels used in the logging system to specify the severity of the message.
 */
typedef enum {
  LOG_LEVEL_INFO,  /**< Informational messages */
  LOG_LEVEL_WARN,  /**< Warning messages */
  LOG_LEVEL_ERROR  /**< Error messages */
} LogLevel;

int init_logger(const char* filename);
void close_logger();
void Log(LogLevel level, const char* format, ...);

#endif
