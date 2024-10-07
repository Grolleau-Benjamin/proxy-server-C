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

/**
 * @brief Initializes the logger with the specified file.
 * 
 * This function opens a log file for appending log messages. It should be called
 * at the start of the application to initialize the logging system.
 * 
 * @param filename The path to the log file.
 * 
 * @return 0 on success, -1 if the file could not be opened.
 */
int init_logger(const char* filename);

/**
 * @brief Closes the logger.
 * 
 * This function closes the log file if it is open. It should be called at the end
 * of the application to properly close the logging system.
 */
void close_logger();

/**
 * @brief Logs a message with the specified log level.
 * 
 * This function logs a formatted message to the log file. It adds a timestamp and
 * the log level to each message.
 * 
 * @param level The log level (INFO, WARN, ERROR).
 * @param format The format string for the message (similar to printf).
 * @param ... Additional arguments for the format string.
 */
void Log(LogLevel level, const char* format, ...);

#endif
