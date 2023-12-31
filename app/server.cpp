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
#include "cd.h"
#include "common.h"
#include "connect.h"
#include "download.h"
#include "file.h"
#include "find.h"
#include "log.h"
#include "ls.h"
#include "message.h"
#include "pwd.h"
#include "quit.h"
#include "status.h"
#include "utils.h"

char root_dir[SIZE];

void ftserve_process(int sockfd);

int main(int argc, char const *argv[]) {
    getcwd(root_dir, sizeof(root_dir));
    int socket, sockfd, pid;

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(0);
    }

    if ((socket = socket_create(atoi(argv[1]))) < 0) {
        printf("Error: Failed to create socket on port %s\n", argv[1]);
        exit(1);
    }

    while (1) {
        if ((sockfd = socket_accept(socket)) < 0)
            break;

        printf("New connection established\n");
        server_log('i', "New connection established");
        if ((pid = fork()) < 0) {
            printf("Error: fork() failed\n");
            server_log('e', "fork() failed");
        } else if (pid == 0) {
            ftserve_process(sockfd);
            exit(0);
        }
        close(sockfd);
    }
    close(socket);
    return 0;
}

void ftserve_process(int sockfd) {
    int sock_data;
    char cmd[5];
    char arg[SIZE];

    char user_dir[SIZE] = "user/";
    char *cur_user;

    Message msg;
    recv_message(sockfd, &msg);

    switch (msg.type) {
        case MSG_TYPE_AUTHEN:
            Message response;
            Status status;
            if (server_login(msg, user_dir) == 1) {
                status = LOGIN_SUCCESS;
                response = create_status_message(MSG_TYPE_OK, status);
                send_message(sockfd, response);
            } else {
                status = LOGIN_FAIL;
                response = create_status_message(MSG_TYPE_ERROR, status);
                send_message(sockfd, response);
                exit(0);
            }
            break;
        case MSG_TYPE_REGISTER:
            if (server_register(sockfd, msg)) {
                status = REGISTER_SUCCESS;
                response = create_status_message(MSG_TYPE_OK, status);
                send_message(sockfd, response);
            } else {
                exit(0);
            }
            break;
        default:
            break;
    }

    cur_user = get_username(user_dir);
    char *log = (char *) malloc(sizeof(char) * 100);
    sprintf(log, "User %s logged in", cur_user);
    server_log('i', log);

    while (1) {
        // TODO: handle relogin case
        Message msg;
        int st = recv_message(sockfd, &msg);

        if (st < 0) {
            break;
        }
        // Open data connection with client
        if ((sock_data = server_start_conn(sockfd)) < 0) {
            close(sockfd);
            exit(1);
        }

        switch (msg.type) {
            case MSG_TYPE_LS:
                server_list(sock_data);
                break;
            case MSG_TYPE_CD:
                server_cd(sockfd, msg.payload, user_dir);
                break;
            case MSG_TYPE_DOWNLOAD:
                server_download(sockfd, sock_data, msg.payload);
                break;
            case MSG_TYPE_QUIT:
                server_quit(sockfd, cur_user);
                break;
            default:
                break;
        }
        if (strcmp(cmd, "FIND") == 0) {   // find file
            ftserve_find(sockfd, sock_data, arg);
        } else if (strcmp(cmd, "SHRE") == 0) {   // share file
            ftserve_share(sockfd, arg, cur_user);
        } else if (strcmp(cmd, "RENM") == 0) {   // rename file and folder
            ftserve_rename(sockfd, arg);
        } else if (strcmp(cmd, "DEL ") == 0) {   // rename file and folder
            ftserve_delete(sockfd, arg);
        } else if (strcmp(cmd, "MOV ") == 0) {   // rename file and folder
            ftserve_move(sockfd, arg);
        } else if (strcmp(cmd, "CPY ") == 0) {   // rename file and folder
            ftserve_copy(sockfd, arg);
        } else if (strcmp(cmd, "MKDR") == 0) {   // RETRIEVE: get file
            ftserve_mkdir(sockfd, arg);
        } else if (strcmp(cmd, "PWD ") == 0) {   // print working directory
            ftpServer_pwd(sockfd, sock_data);
        } else if (strcmp(cmd, "RETR") == 0) {   // RETRIEVE: get file
            server_download(sockfd, sock_data, arg);
        } else if (strcmp(cmd, "STOR") == 0) {   // STOR: send file
            printf("Receving ...\n");
            recvFile(sockfd, sock_data, arg);
        }
        // Close data connection
        close(sock_data);
    }
}