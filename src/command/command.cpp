#include "command.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "connect.h"
#include "log.h"
#include "message.h"
#define INITIAL_SIZE 1024

/**
 * wrapper output of command to a string
 */
char *get_cmd_output(char *cmd) {
    FILE *fp;
    char buffer[256];
    char *output = (char *) malloc(INITIAL_SIZE);
    if (output == NULL) {
        perror("Malloc failed");
        return NULL;
    }
    output[0] = '\0';
    char full_cmd[1024];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);   // Capture stderr as well
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        perror("Failed to run command");
        free(output);
        return NULL;
    }

    size_t currentSize = INITIAL_SIZE;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Check if we need to expand our output buffer
        if (strlen(output) + strlen(buffer) + 1 > currentSize) {
            currentSize *= 2;
            output = (char *) realloc(output, currentSize);
            if (output == NULL) {
                perror("Realloc failed");
                return NULL;
            }
        }
        strcat(output, buffer);
    }

    pclose(fp);
    return output;
}
int process_command(int sockfd, char *base_cmd) {
    char cmd[256];
    char *cmd_output = get_cmd_output(base_cmd);
    // we want slient mode
    snprintf(cmd, sizeof(cmd), "%s > /dev/null 2>&1", base_cmd);
    int res = system(cmd);
    char *msg_log = (char *) malloc(SIZE);
    if (res == -1) {
        sprintf(msg_log, "Error: %s", base_cmd);
        server_log('e', msg_log);
        char *errorString = strerror(errno);
        send_message(sockfd, create_message(MSG_TYPE_ERROR, errorString));
        return -1;
    }
    send_message(sockfd, create_message(MSG_TYPE_OK, cmd_output));
    sprintf(msg_log, "Success: %s", base_cmd);
    server_log('i', msg_log);
    free(msg_log);
    free(cmd_output);
    return 0;
}
