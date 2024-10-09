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
 * @file rules.c
 * @brief Implementation of the rules management functions.
 *
 * This file contains the implementation of functions used to load, manage, and
 * free filtering rules from a configuration file.
 */

#include "../includes/rules.h"
#include "../includes/utils.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Global variable holding the filtering rules.
 */
rules_t rules = {
  .rules = NULL,
  .nb_rules = 0
};

/**
 * @brief Loads the filtering rules from a configuration file.
 *
 * This function reads a configuration file that defines filtering rules in categories.
 * Each category can contain banned domains and banned words. The file is expected to 
 * follow a specific format, with categories enclosed in brackets and rules defined 
 * under each category.
 *
 * @param filename The path to the configuration file.
 * @return 0 on success, -1 if the file could not be opened or an error occurred.
 */
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

/**
 * @brief Frees the memory allocated for the rules.
 *
 * This function releases the memory allocated for all categories, including 
 * the banned domains and words within each category, and resets the rules.
 */
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

int is_host_deny(const char* host) {
  for (int i = 0; i < rules.nb_rules; i++) {
    // INFO("nb domain : %d\n",  (int)rules.rules[i].domain_count);
    for (int j = 0; j < rules.rules[i].domain_count; j++) {
      if (host == rules.rules[i].ban_domain_list[j]) return 1;
    }
  }
  return 0;
}
