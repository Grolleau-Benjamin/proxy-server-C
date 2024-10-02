#include <stdio.h>

int main() {
  printf("Test de server\n");

  #ifdef DEBUG
    printf("Debug is ON\n");
  #endif
  
  return 0;
}
