#include <sys/socket.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "command.h"
#include "common.h"
#include "connect.h"
#include "message.h"
#include "utils.h"

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
    std::string cmd(full_cmd);
    std::string left_cmd, right_cmd;

    size_t pos = cmd.find('|');
    if (pos != std::string::npos) {
        // '|' found, split the string
        left_cmd = cmd.substr(0, pos);
        left_cmd += " --type f";
        right_cmd = cmd.substr(pos + 1);

        left_cmd.erase(left_cmd.find_last_not_of(" \n\r\t") + 1);
        right_cmd.erase(0, right_cmd.find_first_not_of(" \n\r\t"));
    } else {
        left_cmd = cmd;
    }

    fp = popen(left_cmd.c_str(), "r");
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

    if (!right_cmd.empty()) {
        send_message(sockfd, create_status_message(MSG_TYPE_PIPE, NO));
        server_pipe_download(sockfd, output);
    } else {
        send_message(sockfd, create_status_message(MSG_TYPE_OK, NO));
    }
    return 0;
}
