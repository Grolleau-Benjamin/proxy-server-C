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

#include "../includes/config.h"
#include <stdio.h>
#include <string.h>

config_t config = {
  .port = 8080,
  .address = "127.0.0.1", 
  .max_client = 10,
  .logger_filename = "proxy.log",
  .rules_filename = "proxy.rules"
};

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
