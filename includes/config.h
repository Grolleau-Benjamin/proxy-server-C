#ifndef CONFIG_H
#define CONFIG_H

#include "utils.h"

typedef struct {
    int port;
    char address[256];
    int max_client;
    char logger_filename[256];
    char rules_filename[256];
} config_t;
extern config_t config;

int init_config(const char* filename);

#endif

