#include <sys/socket.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "command.h"
#include "common.h"
#include "connect.h"
#include "message.h"

int server_find(int sockfd, char *arg) {
    FILE *fp;
    char buffer[256];
    char *output = (char *) malloc(SIZE);
    if (output == NULL) {
        perror("Malloc failed");
        return -1;
    }
    output[0] = '\0';
    char full_cmd[SIZE];
    snprintf(full_cmd, sizeof(full_cmd), "fd %s", arg);
    // printf("full_cmd: %s\n", full_cmd);
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        perror("Failed to run command");
        free(output);
        return -1;
    }

    size_t currentSize = SIZE;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Check if we need to expand our output buffer
        if (strlen(output) + strlen(buffer) + 1 > currentSize) {
            currentSize *= 2;
            output = (char *) realloc(output, currentSize);
            if (output == NULL) {
                perror("Realloc failed");
                return -1;
            }
        }
        strcat(output, buffer);
    }
    pclose(fp);
    int n = (int) strlen(output);
    int index = 0, readed = 0;
    while (readed < n) {
        output += index;
        send_message(sockfd, create_message(MSG_DATA_FIND, output));
        if (n - readed > PAYLOAD_SIZE) {
            index = PAYLOAD_SIZE;
            readed += PAYLOAD_SIZE;
        } else {
            index = n - readed;
            readed = n;
        }
    }
    send_message(sockfd, create_status_message(MSG_TYPE_OK, NO));
    return 0;
}