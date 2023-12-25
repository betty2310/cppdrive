#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/sha.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "authenticate.h"
#include "common.h"
#include "connect.h"
#include "cwd.h"
#include "download.h"
#include "file.h"
#include "find.h"
#include "ls.h"
#include "pwd.h"
#include "utils.h"

char root_dir[MAX_SIZE];

void ftserve_process(int sock_control);

int main() {
    getcwd(root_dir, sizeof(root_dir));
    int ListenSock, CtrlSock, pid;

    if ((ListenSock = socket_create(9000)) < 0) {
        perror("Error creating socket");
        exit(1);
    }

    while (1) {   // wait for client request

        // create new socket for control connection
        if ((CtrlSock = socket_accept(ListenSock)) < 0)
            break;

        // create child process to do actual file transfer
        if ((pid = fork()) < 0) {
            perror("Error forking child process");
        } else if (pid == 0) {
            close(ListenSock);
            ftserve_process(CtrlSock);
            close(CtrlSock);
            exit(0);
        }
        close(CtrlSock);
    }

    close(ListenSock);
    return 0;
}

void ftserve_process(int sock_control) {
    int sock_data;
    char cmd[5];
    char arg[MAX_SIZE];

    char user_dir[MAX_SIZE] = "user/";
    char *cur_user;

    // Send welcome message
    send_response(sock_control, 220);

    // receive Login or Register
    ftserve_recv_cmd(sock_control, cmd, arg, nullptr);

    // Register user
    if (strcmp(cmd, "REG ") == 0) {
        if (ftserve_register(sock_control)) {
            send_response(sock_control, 230);
            // Receive login command
            ftserve_recv_cmd(sock_control, cmd, arg, nullptr);
        } else {
            send_response(sock_control, 430);
            exit(0);
        }
    }
    // Authenticate user
    if (strcmp(cmd, "LGIN") == 0) {
        if (ftserve_login(sock_control, user_dir) == 1) {
            send_response(sock_control, 230);
        } else {
            send_response(sock_control, 430);
            exit(0);
        }
    }

    while (1) {
        // Wait for command
        cur_user = extractUsername(user_dir);
        int rc = ftserve_recv_cmd(sock_control, cmd, arg, cur_user);

        if ((rc < 0) || (rc == 221)) {
            break;
        }
        if (rc == 200) {
            // Open data connection with client
            if ((sock_data = ftserve_start_data_conn(sock_control)) < 0) {
                close(sock_control);
                exit(1);
            }
            // Execute command
            if (strcmp(cmd, "LIST") == 0) {   // Do list
                ftserve_list(sock_data);
            } else if (strcmp(cmd, "CWD ") == 0) {   // change directory
                ftpServer_cwd(sock_control, arg, user_dir);
            } else if (strcmp(cmd, "FIND") == 0) {   // find file
                ftserve_find(sock_control, sock_data, arg);
            } else if (strcmp(cmd, "SHRE") == 0) {   // share file
                ftserve_share(sock_control, arg, cur_user);
            } else if (strcmp(cmd, "RENM") == 0) {   // rename file and folder
                ftserve_rename(sock_control, arg);
            } else if (strcmp(cmd, "DEL ") == 0) {   // rename file and folder
                ftserve_delete(sock_control, arg);
            } else if (strcmp(cmd, "MOV ") == 0) {   // rename file and folder
                ftserve_move(sock_control, arg);
            } else if (strcmp(cmd, "CPY ") == 0) {   // rename file and folder
                ftserve_copy(sock_control, arg);
            } else if (strcmp(cmd, "MKDR") == 0) {   // RETRIEVE: get file
                ftserve_mkdir(sock_control, arg);
            } else if (strcmp(cmd, "PWD ") == 0) {   // print working directory
                ftpServer_pwd(sock_control, sock_data);
            } else if (strcmp(cmd, "RETR") == 0) {   // RETRIEVE: get file
                ftserve_retr(sock_control, sock_data, arg);
            } else if (strcmp(cmd, "STOR") == 0) {   // STOR: send file
                printf("Receving ...\n");
                recvFile(sock_control, sock_data, arg);
            }
            // Close data connection
            close(sock_data);
        }
    }
}