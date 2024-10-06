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

#include "../includes/utils.h"

void print_error(int result, char* s) {
  if (result < 0) {
    perror(s);
    exit(EXIT_FAILURE);
  }
}

