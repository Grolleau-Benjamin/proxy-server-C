#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>

#ifdef DEBUG
  #define INFO(fmt, ...)  fprintf(stderr, "INFO: " fmt, __VA_ARGS__)
  #define WARN(fmt, ...)  fprintf(stderr, "WARN: " fmt, __VA_ARGS__)
  #define ERROR(fmt, ...) fprintf(stderr, "ERROR: " fmt, __VA_ARGS__)
#else
  #define INFO(fmt, ...)  do { } while (0)
  #define WARN(fmt, ...)  do { } while (0)
  #define ERROR(fmt, ...) do { } while (0)
#endif

void print_error(int result, char* s);

#endif
