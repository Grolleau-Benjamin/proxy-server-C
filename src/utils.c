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
 * @file utils.c
 * @brief Implementation of utility functions for the proxy server.
 * 
 * This file provides the definition of utility functions used to handle errors
 * and log messages in the proxy server.
 */

#include "../includes/utils.h"

/**
 * @brief Prints an error message and exits the program if the result is negative.
 * 
 * This function checks the result of a system call or other operation, and if the result
 * is negative, it prints the error message (associated with the string `s`) using `perror()`
 * and terminates the program with `EXIT_FAILURE`.
 * 
 * @param result The result of an operation to check (usually the return value of a function).
 * @param s A string describing the error (used in `perror()`).
 */
void print_error(int result, char* s) {
  if (result < 0) {
    perror(s);
    exit(EXIT_FAILURE);
  }
}
