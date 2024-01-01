#include <arpa/inet.h>
#include <openssl/sha.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>
#include <string>

#include "authenticate.h"
#include "command.h"
#include "common.h"
#include "connect.h"
#include "log.h"
#include "message.h"
#include "status.h"
#include "utils.h"

char root_dir[SIZE];

/**
 * Create cppdrive storage directory
 * @param dir root directory
 */
void create_app_storage(char *dir);

/**
 * Handle client connection
 * @param sockfd socket for control connection
 */
void server_handler(int sockfd);

int main(int argc, char const *argv[]) {
    getcwd(root_dir, sizeof(root_dir));
    int socket, sockfd, pid;
    create_app_storage(root_dir);
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
            server_log('i', "Connection accepted");
            server_handler(sockfd);
            server_log('i', "Connection closed");
            exit(0);
        }
        close(sockfd);
    }
    close(socket);
    return 0;
}

void server_handler(int sockfd) {
    int sock_data;
    char user_dir[SIZE] = APP_STORAGE;
    char *cur_user;
    char *cur_dir = (char *) malloc(sizeof(char) * SIZE);
    char *log_message = (char *) malloc(sizeof(char) * SIZE * 2);

    int logged_in = 0;
    do {
        Message msg;
        recv_message(sockfd, &msg);
        switch (msg.type) {
            case MSG_TYPE_AUTHEN:
                if (server_login(msg, user_dir) == 1) {
                    send_message(sockfd, create_status_message(MSG_TYPE_OK, LOGIN_SUCCESS));
                    logged_in = 1;
                } else {
                    send_message(sockfd, create_status_message(MSG_TYPE_ERROR, LOGIN_FAIL));
                }
                break;
            case MSG_TYPE_REGISTER:
                if (server_register(msg)) {
                    send_message(sockfd, create_status_message(MSG_TYPE_OK, NO));
                } else {
                    send_message(sockfd, create_status_message(MSG_TYPE_ERROR, USERNAME_EXIST));
                }
                break;
            default:
                break;
        }
    } while (!logged_in);

    cur_user = get_username(user_dir);
    sprintf(log_message, "User %s logged in", cur_user);
    server_log('i', log_message);
    strcpy(cur_dir, user_dir);
    load_shared_file(user_dir);
    while (1) {
        Message msg;
        int st = recv_message(sockfd, &msg);

        if (st < 0) {
            break;
        }
        // Open data connection with client
        if ((sock_data = server_start_conn(sockfd)) < 0) {
            sprintf(log_message, "Error: Failed to open data connection with client");
            server_log('e', log_message);
            close(sockfd);
            exit(1);
        }
        sprintf(log_message, "Connect on socket %d", sock_data);
        server_log('i', log_message);

        switch (msg.type) {
            case MSG_TYPE_LS:
                server_list(sock_data);
                break;
            case MSG_TYPE_CD:
                server_cd(sockfd, msg.payload, user_dir, cur_dir);
                break;
            case MSG_TYPE_PWD:
                // handle on client side
                break;
            case MSG_TYPE_DOWNLOAD:
                server_download(sockfd, sock_data, msg.payload);
                break;
            case MSG_TYPE_UPLOAD:
                server_upload(sockfd, sock_data, msg.payload, cur_dir);
                break;
            case MSG_TYPE_BASIC_COMMAND:
                process_command(sockfd, msg.payload, cur_dir);
                break;
            case MSG_TYPE_FIND:
                server_find(sockfd, msg.payload);
                break;
            case MSG_TYPE_SHARE:
                server_share(sockfd, msg.payload, cur_dir);
                break;
            case MSG_TYPE_RELOAD:
                load_shared_file(user_dir);
                break;
            case MSG_TYPE_QUIT:
                server_quit(sockfd, cur_user);
                break;
            default:
                break;
        }
        close(sock_data);
    }
}

void create_app_storage(char *dir) {
    std::string path(dir);
    path += "/";
    std::string storage_path = path + APP_STORAGE;
    std::string accounts_pah = path + ACCOUNTS_FILE;
    create_dir(storage_path.c_str());
    create_file(accounts_pah.c_str());
}
