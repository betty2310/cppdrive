#include "pwd.h"

#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "common.h"
#include "connect.h"

void ftpServer_pwd(int sock_control, int sock_data) {
    char curr_dir[MAX_SIZE - 2], msgToClient[MAX_SIZE];
    memset(curr_dir, 0, MAX_SIZE - 2);
    memset(msgToClient, 0, MAX_SIZE);

    getcwd(curr_dir, sizeof(curr_dir));
    sprintf(msgToClient, "%s\n", curr_dir);
    if (send(sock_data, msgToClient, strlen(msgToClient), 0) < 0) {
        perror("error");
        send_response(sock_control, 550);
    }
    send_response(sock_control, 212);
}