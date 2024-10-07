#ifndef RULES_H
#define RULES_H

#include "utils.h"
#include <stdlib.h>

/**
* Rules format: 

# Some comments

[categorie]
BAN_DOMAIN some_domain.com
BAN_WORD some_word

*/

#define MAX_STRING_LEN 255

typedef struct {
  char name[255];
  char** ban_domain_list;
  char** ban_word_list;
  size_t domain_count;
  size_t word_count;
} categories;

typedef struct {
  categories* rules;
  size_t nb_rules;
} rules_t;

extern rules_t rules;

int init_rules(const char* filename);
void free_rules();

#endif
