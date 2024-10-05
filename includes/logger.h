#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

typedef enum {
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERROR
} LogLevel;

int init_logger(const char* filename);
void close_logger();
void Log(LogLevel level, const char* format, ...);

#endif

