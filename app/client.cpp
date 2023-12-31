#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "authenticate.h"
#include "common.h"
#include "connect.h"
#include "download.h"
#include "log.h"
#include "ls.h"
#include "reply.h"
#include "upload.h"
#include "utils.h"
#include "validate.h"

char root_dir[SIZE];

int main(int argc, char const *argv[]) {
    int sockfd;
    int data_sock;
    char user_input[SIZE];
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
            handle_login(sockfd);
            break;
        case 1:
            print_centered("Register new account");
            register_acc(sockfd);
            print_centered("Login to cppdrive");
            handle_login(sockfd);
            break;
        case 2:
            print_centered("Good bye!");
            exit(0);
    }

    // begin shell
    while (1) {
        printf("ftp@cppdrive ");
        fflush(stdout);
        Message command;
        int cmd_stt = ftclient_read_command(user_input, sizeof(user_input), &cmd, &command);
        if (cmd_stt == -1) {
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
        if ((data_sock = ftclient_open_conn(sockfd)) < 0) {
            perror("Error opening socket for data connection");
            exit(1);
        }

        // execute command
        if (command.type == MSG_TYPE_LS) {
            list(data_sock);
        } else if (strcmp(cmd.code, "CWD ") == 0) {
            int repl = read_reply(sockfd);
            if (repl == 250)
                print_reply(250);
            else if (repl == 551)
                print_reply(551);
            else
                printf("%s is not a directory\n", cmd.arg);
        } else if (strcmp(cmd.code, "FIND") == 0) {
            int repl = read_reply(sockfd);
            // File found
            if (repl == 241) {
                int nums = read_reply(sockfd);
                for (int i = 0; i < nums; ++i) list(data_sock);   // ham nay in mess tu server
            } else if (repl == 441)
                printf("441 File not found!\n");
        } else if (strcmp(cmd.code, "RENM") == 0) {
            int repl = read_reply(sockfd);
            if (repl == 251)
                printf("251 Rename successfully\n");
            else if (repl == 451)
                printf("451 Rename failure\n");
            else if (repl == 452)
                printf("452 Syntax error (renm <oldfilename> <newfilename>)\n");
        } else if (strcmp(cmd.code, "DEL ") == 0) {
            int repl = read_reply(sockfd);
            if (repl == 252)
                printf("252 Delete successfully\n");
            else if (repl == 453)
                printf("451 Delete failure\n");
        } else if (strcmp(cmd.code, "MOV ") == 0) {
            int repl = read_reply(sockfd);
            if (repl == 253)
                printf("253 Moved successfully\n");
            else if (repl == 454)
                printf("454 Move failure\n");
            else if (repl == 455)
                printf("455 Syntax error (mov <filepath> <newfilepath>)\n");
        } else if (strcmp(cmd.code, "CPY ") == 0) {
            int repl = read_reply(sockfd);
            if (repl == 253)
                printf("253 Copied successfully\n");
            else if (repl == 454)
                printf("454 Copy failure\n");
            else if (repl == 455)
                printf("455 Syntax error (cpy <filepath> <newfilepath>)\n");
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
        } else if (strcmp(cmd.code, "MKDR") == 0) {
            int repl = read_reply(sockfd);
            if (repl == 254)
                printf("254 Mkdir successfully\n");
            else if (repl == 456)
                printf("451 Mkdir failure\n");
        } else if (strcmp(cmd.code, "PWD ") == 0) {
            if (read_reply(sockfd) == 212) {
                list(data_sock);   // ham nay in mess tu server
            }
        } else if (strcmp(cmd.code, "RETR") == 0) {
            ftclient_get(data_sock, sockfd, cmd.arg);
        } else if (strcmp(cmd.code, "STOR") == 0) {
            printf("Uploading ...\n");
            upload(data_sock, cmd.arg, sockfd);
            printf("xong\n");
        }
        close(data_sock);

    }   // loop back to get more user input

    // Close the socket (control connection)
    close(sockfd);
    return 0;
}