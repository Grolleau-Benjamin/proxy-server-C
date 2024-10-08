#ifndef CONFIG_H
#define CONFIG_H

#include "utils.h"

/**
 * @file config.h
 * @brief Header file for the proxy server's configuration management.
 * 
 * This file contains the structure definition for the configuration settings and
 * the function declarations for loading the configuration from a file.
 */

/**
 * @brief Structure to hold the configuration settings.
 * 
 * This structure defines the configuration settings such as the port number,
 * server address, maximum number of clients, and filenames for logging and
 * filtering rules.
 */
typedef struct {
    int port;                        /**< The port number on which the proxy server listens. */
    char address[256];               /**< The address (IPv4) on which the proxy server is bound. */
    int max_client;                  /**< The maximum number of clients that can connect simultaneously. */
    char logger_filename[256];       /**< The filename where logs are recorded. */
    char rules_filename[256];        /**< The filename containing the filtering rules. */
} config_t;

/** 
 * @brief Externally declared configuration structure.
 * 
 * This structure holds the global configuration settings that are initialized
 * when the proxy server starts.
 */
extern config_t config;

int init_config(const char* filename);

#endif

