#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "authenticate.h"
#include "color.h"
#include "common.h"
#include "connect.h"
#include "download.h"
#include "log.h"
#include "ls.h"
#include "pwd.h"
#include "reply.h"
#include "upload.h"
#include "utils.h"
#include "validate.h"

char root_dir[SIZE];

int main(int argc, char const *argv[]) {
    int sockfd;
    int data_sock;
    char user_input[SIZE];
    char *cur_user;
    struct command cmd;

    if (argc != 3) {
        printf("Usage: %s <ip_adress> <port>\n", argv[0]);
        exit(0);
    }

    if (validate_ip(argv[1]) == INVALID_IP) {
        printf("Error: Invalid ip-address\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd == INVALID_SOCKET) {
        perror("Error: ");
        exit(1);
    }

    SOCKADDR_IN sock_addr;

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(atoi(argv[2]));
    sock_addr.sin_addr.s_addr = inet_addr(argv[1]);

    int connect_status = connect(sockfd, (SOCKADDR *) &sock_addr, sizeof(sock_addr));

    if (connect_status == -1) {
        printf("Error: Connection failed!\n");
        exit(1);
    }

    const char *menu_items[] = {"Login", "Register", "Exit"};

    int choice = process_menu(menu_items, 3);
    switch (choice) {
        case 0:
            print_centered("Login to cppdrive");
            cur_user = handle_login(sockfd);
            break;
        case 1:
            print_centered("Register new account");
            register_acc(sockfd);
            print_centered("Login to cppdrive");
            cur_user = handle_login(sockfd);
            break;
        case 2:
            print_centered("Good bye!");
            exit(0);
    }

    // begin shell
    char *user_dir = (char *) malloc(SIZE);
    strcpy(user_dir, "~/");
    int error = 0;
    while (1) {
        char *prompt = handle_prompt(cur_user, user_dir);
        if (error) {
            printf(ANSI_COLOR_RED "%s" ANSI_RESET, prompt);
        }
        printf(ANSI_COLOR_GREEN "%s" ANSI_RESET, prompt);
        fflush(stdout);
        Message command;
        int fl = cli_read_command(user_input, sizeof(user_input), &cmd, &command);
        if (fl == -1) {
            printf("Invalid command\n");
            // next loop
            continue;
        }
        // Send command to server
        if (send_message(sockfd, command) < 0) {
            close(sockfd);
            exit(1);
        }

        // open data connection
        if ((data_sock = client_start_conn(sockfd)) < 0) {
            perror("Error opening socket for data connection");
            exit(1);
        }

        // execute command
        if (command.type == MSG_TYPE_QUIT) {
            printf("Goodbye.\n");
            break;
        } else if (command.type == MSG_TYPE_LS) {
            handle_list(data_sock);
        } else if (command.type == MSG_TYPE_BASIC_COMMAND) {
            Message response;
            recv_message(sockfd, &response);
            if (response.type == MSG_TYPE_ERROR)
                error = 1;
            else {
                error = 0;
                printf("%s\n", response.payload);
            }
        } else if (command.type == MSG_TYPE_CD) {
            Message response;
            recv_message(sockfd, &response);
            switch (response.type) {
                case MSG_DATA_CD:
                    strcpy(user_dir, response.payload);
                    break;
                case MSG_TYPE_ERROR:
                    printf("%s\n", response.payload);
                    break;
                default:
                    break;
            }
        } else if (command.type == MSG_TYPE_DOWNLOAD) {
            handle_download(data_sock, sockfd, command.payload);
        } else if (command.type == MSG_TYPE_FIND) {
            Message response;
            while (1) {
                recv_message(sockfd, &response);
                if (response.type == MSG_TYPE_ERROR)
                    printf("%s\n", response.payload);
                else if (response.type == MSG_DATA_FIND) {
                    printf("%s", response.payload);
                } else {
                    break;
                }
            }
        } else if (strcmp(cmd.code, "SHRE") == 0) {
            int repl = read_reply(sockfd);
            if (repl == 261)
                printf("261 Shared successfully\n");
            else if (repl == 462)
                printf("462 User not found\n");
            else if (repl == 463)
                printf("463 File/Folder not found\n");
            else if (repl == 464)
                printf("464 Must not share to yourself\n");
            else if (repl == 461)
                printf("461 Syntax error (share <username> <filename>)\n");

        } else if (command.type == MSG_TYPE_PWD) {
            const char *pwd = handle_pwd(cur_user, user_dir);
            printf("%s\n", pwd);
        } else if (command.type == MSG_TYPE_UPLOAD) {
            handle_upload(data_sock, command.payload, sockfd);
        }
        close(data_sock);

    }   // loop back to get more user input

    // Close the socket (control connection)
    close(sockfd);
    return 0;
}