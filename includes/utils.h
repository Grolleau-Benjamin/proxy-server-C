#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[38;5;11m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#ifdef DEBUG
  #define INFO(fmt, ...)  fprintf(stdout, "INFO: " fmt, ##__VA_ARGS__)
  #define WARN(fmt, ...)  fprintf(stderr, ANSI_COLOR_YELLOW "WARN: " fmt ANSI_COLOR_RESET, ##__VA_ARGS__)
  #define ERROR(fmt, ...) fprintf(stderr, ANSI_COLOR_RED "ERROR: " fmt ANSI_COLOR_RESET, ##__VA_ARGS__)
#else
  #define INFO(fmt, ...)  do { } while (0)
  #define WARN(fmt, ...)  do { } while (0)
  #define ERROR(fmt, ...) do { } while (0)
#endif

void print_error(int result, char* s);

#endif
