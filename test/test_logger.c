#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../includes/logger.h"
#include "../includes/utils.h"

void test_init_logger() {
    INFO("Testing init_logger...\n");

    const char* log_filename = "test_log.txt";
    
    int result = init_logger(log_filename);
    assert(result == 0);
    INFO("\tsuccess: Logger has been initialized with file: test_log.txt\n");

    FILE* file = fopen(log_filename, "r");
    assert(file != NULL);
    INFO("\tsuccess: Log file was successfully created\n");
    fclose(file);

    close_logger();
}

void test_log_messages() {
    INFO("Testing log messages...\n");

    const char* log_filename = "test_log.txt";
    init_logger(log_filename);

    Log(LOG_LEVEL_INFO, "This is an info message");
    Log(LOG_LEVEL_WARN, "This is a warning message");
    Log(LOG_LEVEL_ERROR, "This is an error message");

    FILE* file = fopen(log_filename, "r");
    assert(file != NULL);

    char buffer[512];
    int info_found = 0, warn_found = 0, error_found = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        if (strstr(buffer, "INFO")) {
            info_found = 1;
        } else if (strstr(buffer, "WARN")) {
            warn_found = 1;
        } else if (strstr(buffer, "ERROR")) {
            error_found = 1;
        }
    }

    fclose(file);

    assert(info_found);
    INFO("\tsuccess: INFO message found in log\n");

    assert(warn_found);
    INFO("\tsuccess: WARN message found in log\n");

    assert(error_found);
    INFO("\tsuccess: ERROR message found in log\n");

    close_logger();
}

void test_logger_not_initialized() {
    INFO("Testing logger without initialization...\n");

    Log(LOG_LEVEL_INFO, "This message should not be logged");

    FILE* file = fopen("non_existent_log.txt", "r");
    assert(file == NULL);
    INFO("\tsuccess: No log file created without logger initialization\n");
}

int main() {
    INFO("Running logger tests...\n");

    test_init_logger();
    test_log_messages();
    test_logger_not_initialized();

    INFO("Cleaning up: removing test log file...\n");
    remove("test_log.txt");

    return 0;
}
