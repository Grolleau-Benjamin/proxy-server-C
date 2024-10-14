#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>

/**
 * @file utils.h
 * @brief Utility macros and functions for the proxy server.
 * 
 * This file contains macros for logging information, warnings, and errors,
 * along with the declaration of utility functions used throughout the proxy server.
 */

/**
 * @brief ANSI color code for red text.
 */
#define ANSI_COLOR_RED     "\x1b[31m"

/**
 * @brief ANSI color code for yellow text.
 */
#define ANSI_COLOR_YELLOW  "\x1b[38;5;11m"

/**
 * @brief ANSI color code to reset text formatting.
 */
#define ANSI_COLOR_RESET   "\x1b[0m"

#ifdef DEBUG
  /**
   * @brief Macro to log information messages to stdout.
   * 
   * This macro only logs messages if DEBUG mode is enabled.
   * 
   * @param fmt The format string (as used in printf).
   * @param ... The arguments to format.
   */
  #define INFO(fmt, ...)  fprintf(stdout, "INFO: " fmt, ##__VA_ARGS__)

  /**
   * @brief Macro to log warning messages to stderr with yellow text.
   * 
   * This macro only logs messages if DEBUG mode is enabled.
   * 
   * @param fmt The format string (as used in printf).
   * @param ... The arguments to format.
   */
  #define WARN(fmt, ...)  fprintf(stderr, ANSI_COLOR_YELLOW "WARN: " fmt ANSI_COLOR_RESET, ##__VA_ARGS__)

  /**
   * @brief Macro to log error messages to stderr with red text.
   * 
   * This macro only logs messages if DEBUG mode is enabled.
   * 
   * @param fmt The format string (as used in printf).
   * @param ... The arguments to format.
   */
  #define ERROR(fmt, ...) fprintf(stderr, ANSI_COLOR_RED "ERROR: " fmt ANSI_COLOR_RESET, ##__VA_ARGS__)

#else
  #define INFO(fmt, ...)  do { } while (0)
  #define WARN(fmt, ...)  do { } while (0)
  #define ERROR(fmt, ...) do { } while (0)
#endif

#endif
