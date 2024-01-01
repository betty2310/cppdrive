#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <string>

std::string get_log_level(char level) {
    switch (level) {
        case 'i':
            return "INFO";
        case 'w':
            return "WARNING";
        case 'e':
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

void log_message(char level, const char* message) {
    FILE* fp = fopen(LOG_FILE, "a");
    if (fp == NULL) {
        printf("Error: Failed to open file %s\n", LOG_FILE);
        exit(1);
    }
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_now);
    fprintf(fp, "[%s] [%s] %s\n", time_str, get_log_level(level).c_str(), message);
    fclose(fp);
}

void server_log(char level, const char* message) {
    FILE* fp = fopen(SERVER_LOG_FILE, "a");
    if (fp == NULL) {
        printf("Error: Failed to open file %s\n", SERVER_LOG_FILE);
        exit(1);
    }
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_now);
    fprintf(fp, "[%s] [%s] %s\n", time_str, get_log_level(level).c_str(), message);
    fclose(fp);
}