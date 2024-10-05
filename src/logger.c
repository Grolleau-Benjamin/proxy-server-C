#include "../includes/logger.h"
#include "../includes/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

static FILE* log_file = NULL;

int init_logger(const char* filename) {
  log_file = fopen(filename, "a");
  if (log_file == NULL) {
    ERROR("Error while openning the log file\n");
    return -1;
  } 

  INFO("Log file is open\n");
  return 0;
}

void close_logger() {
  if (log_file != NULL) {
    fclose(log_file);
    log_file = NULL;
    INFO("Log file is close");
  }
}

static void get_timestamp(char* buffer, size_t size) {
  time_t now = time(NULL);
  struct tm* tstruct = localtime(&now);
  strftime(buffer, size, "%Y-%m-%d %X", tstruct);
}

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
