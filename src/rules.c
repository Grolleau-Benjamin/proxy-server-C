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

#include "../includes/rules.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

rules_t rules = {
  .rules = NULL,
  .nb_rules = 0
};

int init_rules(const char* filename) {
  INFO("Loading rules config from %s\n", filename);

  FILE* file = fopen(filename, "r");
  if (!file) {
    print_error(-1, "Rules file not found");
    return -1;
  }

  char line[512];
  categories* current_categorie = NULL;
  
  while(fgets(line, sizeof(line), file)) {
    // avoid comment or empty line
    if (line[0] == '#' || line[0] == '\n') continue;

    line[strcspn(line, "\r\n")] = 0;

    if (line[0] == '[' || line[strlen(line) - 1] == ']') {
      rules.rules = realloc(rules.rules, sizeof(categories) * (rules.nb_rules + 1));
      current_categorie = &rules.rules[rules.nb_rules];
      
      strncpy(current_categorie->name, line + 1, strlen(line) - 2);
      current_categorie->name[strlen(line) -2] = '\0';

      current_categorie->ban_domain_list = NULL;
      current_categorie->domain_count = 0;
      current_categorie->ban_word_list = NULL;
      current_categorie->word_count = 0;

      rules.nb_rules ++;
      
      INFO("Charging new categorie: %s\n", current_categorie->name);
    } else if (current_categorie) {
      if (strncmp(line, "BAN_DOMAIN", 10) == 0) {
        char* domain = line + 11;
        current_categorie->ban_domain_list = realloc(
              current_categorie->ban_domain_list, 
              sizeof(char*) * (current_categorie->domain_count + 1)
        );
        current_categorie->ban_domain_list[current_categorie->domain_count] = strdup(domain);
        current_categorie->domain_count ++;

        INFO("\tDomain ban: %s\n", domain);
      } else if(strncmp(line, "BAN_WORD", 8) == 0) {
        char* word = line + 9;
        current_categorie->ban_word_list = realloc(
              current_categorie->ban_word_list, 
              sizeof(char*) * (current_categorie->word_count + 1)
        );
        current_categorie->ban_word_list[current_categorie->word_count] = strdup(word);
        current_categorie->word_count ++;

        INFO("\tWord ban: %s\n", word);
      }
    }
  }
  
  fclose(file);
  return 0;
}

void free_rules() {
  for (size_t i = 0; i < rules.nb_rules; i++) {
    categories* cat = &rules.rules[i];

    for (size_t j = 0; j < cat->domain_count; j++) free(cat->ban_domain_list[j]);
    free(cat->ban_domain_list);
    
    for (size_t j = 0; j < cat->word_count; j++) free(cat->ban_word_list[j]);
    free(cat->ban_word_list);
  }
  free(rules.rules);
  rules.rules = NULL;
  rules.nb_rules = 0;
}
