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
#include "ls.h"
#include "reply.h"
#include "upload.h"
#include "utils.h"

char root_dir[SIZE];

int main(int argc, char const *argv[]) {
    int sockfd;
    int data_sock, retcode;
    char user_input[SIZE];
    struct command cmd;

    if (argc != 3) {
        printf("Usage: %s <ip_adress> <port>\n", argv[0]);
        exit(0);
    }

    int ip_valid = validate_ip(argv[1]);
    if (ip_valid == INVALID_IP) {
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
    printf("You chose: %d\n", choice);
    // Register
    char hasAcc;
    printf("Do you have an account? (Y/N) ");
    scanf("%c", &hasAcc);
    if ((hasAcc == 'n') || (hasAcc == 'N'))
        ftclient_register(sockfd);

    /* Get name and password and send to server */
    printf("Please login!\n");
    ftclient_login(sockfd);

    while (1) {   // loop until user types quit

        // Get a command from user
        int cmd_stt = ftclient_read_command(user_input, sizeof(user_input), &cmd);
        if (cmd_stt == -1) {
            printf("Invalid command\n");
            continue;   // loop back for another command
        } else if (cmd_stt == 0) {
            // Send command to server
            if (send(sockfd, user_input, strlen(user_input), 0) < 0) {
                close(sockfd);
                exit(1);
            }

            retcode = read_reply(sockfd);
            if (retcode == 221) {
                /* If command was quit, just exit */
                print_reply(221);
                break;
            }

            if (retcode == 502) {
                // If invalid command, show error message
                printf("%d Invalid command.\n", retcode);
            } else {
                // Command is valid (RC = 200), process command

                // open data connection
                if ((data_sock = ftclient_open_conn(sockfd)) < 0) {
                    perror("Error opening socket for data connection");
                    exit(1);
                }

                // execute command
                if (strcmp(cmd.code, "LIST") == 0) {
                    ftclient_list(data_sock);
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
                        for (int i = 0; i < nums; ++i)
                            ftclient_list(data_sock);   // ham nay in mess tu server
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
                        ftclient_list(data_sock);   // ham nay in mess tu server
                    }
                } else if (strcmp(cmd.code, "RETR") == 0) {
                    ftclient_get(data_sock, sockfd, cmd.arg);
                } else if (strcmp(cmd.code, "STOR") == 0) {
                    printf("Uploading ...\n");
                    upload(data_sock, cmd.arg, sockfd);
                    printf("xong\n");
                }
                close(data_sock);
            }
        }

    }   // loop back to get more user input

    // Close the socket (control connection)
    close(sockfd);
    return 0;
}