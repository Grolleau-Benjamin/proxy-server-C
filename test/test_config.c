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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../includes/config.h"
#include "../includes/utils.h"

void create_test_config_file(const char* filename) {
    FILE* file = fopen(filename, "w");
    assert(file != NULL);
    
    fprintf(file, "# Test configuration file\n");
    fprintf(file, "PORT 9090\n");
    fprintf(file, "ADDRESS 192.168.1.100\n");
    fprintf(file, "MAX_CLIENT 20\n");
    fprintf(file, "LOGGER_FILENAME test_log.log\n");
    fprintf(file, "RULES_FILENAME test_rules.rules\n");
    
    fclose(file);
}

void test_init_config() {
    INFO("Testing init_config...\n");

    const char* config_filename = "test_config.cfg";
    create_test_config_file(config_filename);

    int result = init_config(config_filename);
    assert(result == 0);
    INFO("\tsuccess: Configuration file has been loaded successfully\n");

    assert(config.port == 9090);
    INFO("\tsuccess: Port has been set correctly\n");

    assert(strcmp(config.address, "192.168.1.100") == 0);
    INFO("\tsuccess: Address has been set correctly\n");

    assert(config.max_client == 20);
    INFO("\tsuccess: Max clients has been set correctly\n");

    assert(strcmp(config.logger_filename, "test_log.log") == 0);
    INFO("\tsuccess: Logger filename has been set correctly\n");

    assert(strcmp(config.rules_filename, "test_rules.rules") == 0);
    INFO("\tsuccess: Rules filename has been set correctly\n");

    remove(config_filename);
}

int main() {
    INFO("Running config.c tests...\n");

    test_init_config();

    INFO("Cleaning up after config tests...\n");
    remove("test_log.log");
    remove("test_rules.rules");

    return 0;
}
