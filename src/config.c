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
 * @file config.c
 * @brief Implementation of the configuration loading functions for the proxy server.
 *
 * This file contains the definition of the functions used to load and initialize
 * the proxy server's configuration from a configuration file.
 */

#include "../includes/config.h"
#include <string.h>

config_t config = {
  .port = 8080,
  .address = "127.0.0.1", 
  .max_client = 10,
  .logger_filename = "proxy.log",
  .rules_filename = "proxy.rules"
};

/**
 * @brief Initializes the configuration from a file.
 * 
 * Reads the configuration file specified by `filename` and loads the proxy
 * server's configuration settings.
 * 
 * @param filename The path to the configuration file.
 * 
 * @return 0 on success, -1 if the file cannot be opened.
 */
int init_config(const char* filename){
  INFO("Loading config from %s\n", filename);

  FILE* file = fopen(filename, "r");
  if (!file) {
    print_error(-1, "Config file not found");
    return -1;
  }

  char line[512];
  int i = 1;
  while(fgets(line, sizeof(line), file)) {
    // avoid comment or empty line
    if(line[0] == '#' || line[0] == '\n') { i++; continue; }

    line[strcspn(line, "\n")] = 0;
    
    char key[250];
    char value[250];

    if (sscanf(line, "%s %s", key, value) == 2){
      if (strcmp(key, "PORT") == 0) {
        config.port = atoi(value);
      } else if (strcmp(key, "ADDRESS") == 0) {
        strncpy(config.address, value, sizeof(config.address));
      } else if (strcmp(key, "MAX_CLIENT") == 0) {
        config.max_client = atoi(value);
      } else if (strcmp(key, "LOGGER_FILENAME") == 0) {
        strncpy(config.logger_filename, value, sizeof(config.logger_filename));
      } else if (strcmp(key, "RULES_FILENAME") == 0) {
        strncpy(config.rules_filename, value, sizeof(config.rules_filename));
      } else {
        WARN("Unknow parameter '%s' at line %d\n", key, i);
      }
    } else {
      WARN("Unvalid line in file configuration: %s\n", line);
    }

    i++;
  }
  
  fclose(file);
  return 0;
}
