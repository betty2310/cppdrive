#include "ls.h"

#include <dirent.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>

#include "common.h"
#include "connect.h"

int ftclient_list(int sock_data) {
    size_t num_recvd;     // number of bytes received with recv()
    char buf[SIZE];   // hold a filename received from server

    memset(buf, 0, sizeof(buf));
    while ((num_recvd = recv(sock_data, buf, SIZE, 0)) > 0) {
        printf("%s", buf);
        memset(buf, 0, sizeof(buf));
    }

    return 0;
}

int ftserve_list(int sock_data) {
    struct dirent **output = NULL;
    char curr_dir[SIZE], msgToClient[SIZE];
    memset(curr_dir, 0, SIZE);
    memset(msgToClient, 0, SIZE);

    getcwd(curr_dir, sizeof(curr_dir));
    int n = scandir(curr_dir, &output, NULL, NULL);
    if (n > 0) {
        for (int i = 0; i < n; i++) {
            if (strcmp(output[i]->d_name, ".") != 0 && strcmp(output[i]->d_name, "..") != 0) {
                strcat(msgToClient, output[i]->d_name);
                strcat(msgToClient, "  ");
            }
        }
    }
    strcat(msgToClient, "\n");
    if (send(sock_data, msgToClient, strlen(msgToClient), 0) < 0) {
        perror("error");
    }

    return 0;
}