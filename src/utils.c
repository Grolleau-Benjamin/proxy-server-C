#include "../includes/utils.h"

void print_error(int result, char* s) {
  if (result < 0) {
    perror(s);
    exit(EXIT_FAILURE);
  }
}

