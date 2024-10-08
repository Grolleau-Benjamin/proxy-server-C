/*
 * MIT License
 * 
 * Copyright (c) 2024
 * Benjamin Grolleau
 * Alexis Carle
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction.
 * 
 * See the LICENSE file for the full license text.
 */

/**
 * @file logger.c
 * @brief Implementation of the logger utility for the proxy server.
 * 
 * This file provides the implementation for logging messages to a log file with different log levels
 * (INFO, WARN, ERROR), as well as initializing and closing the logger.
 */

#include "../includes/logger.h"
#include "../includes/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

static FILE* log_file = NULL;

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
int init_logger(const char* filename) {
  log_file = fopen(filename, "a");
  if (log_file == NULL) {
    ERROR("Error while openning the log file\n");
    return -1;
  } 

  INFO("Log file is open\n");
  return 0;
}

/**
 * @brief Closes the logger.
 * 
 * This function closes the log file if it is open. It should be called at the end
 * of the application to properly close the logging system.
 */
void close_logger() {
  if (log_file != NULL) {
    fclose(log_file);
    log_file = NULL;
    INFO("Log file is close");
  }
}

/**
 * @brief Initializes the logger with the specified file.
 * 
 * This function opens the specified log file for appending log messages.
 * If the file cannot be opened, an error is logged and the function returns -1.
 * 
 * @param filename The path to the log file.
 * 
 * @return 0 on success, -1 if the file could not be opened.
 */
static void get_timestamp(char* buffer, size_t size) {
  time_t now = time(NULL);
  struct tm* tstruct = localtime(&now);
  strftime(buffer, size, "%Y-%m-%d %X", tstruct);
}

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
void Log(LogLevel level, const char* format, ...) {
  if (log_file == NULL) {
    ERROR("Logger havn't been initialize, call init_logger() before.\n");
    return;
  }

  char timestamp[20];
  get_timestamp(timestamp, sizeof(timestamp));

  const char* level_str = "";
  switch(level) {
    case LOG_LEVEL_INFO:
      level_str = "INFO";
      break;
    case LOG_LEVEL_WARN:
      level_str = "WARN";
      break;
    case LOG_LEVEL_ERROR:
      level_str = "ERROR";
      break;
  }

  fprintf(log_file, "[%s] [%s] ", timestamp, level_str);

  // https://www.ibm.com/docs/nl/zos/2.4.0?topic=functions-vfprintf-format-print-data-stream#d151044e205
  va_list args;
  va_start(args, format);
  vfprintf(log_file, format, args);
  va_end(args);

  fprintf(log_file, "\n");
  fflush(log_file);
}
