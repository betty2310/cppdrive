#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>

#include "authenticate.h"
#include "color.h"
#include "common.h"
#include "connect.h"
#include "download.h"
#include "log.h"
#include "ls.h"
#include "pwd.h"
#include "upload.h"
#include "utils.h"
#include "validate.h"

char root_dir[SIZE];

/**
 * Read command from user and store in Message struct
 * @param user_input user input
 * @param size size of user input
 * @param msg Message struct
 * @return 0 if success, -1 if error
 */
int cli_read_command(char *user_input, int size, Message *msg);

int main(int argc, char const *argv[]) {
    int sockfd;
    int data_sock;
    char user_input[SIZE];
    char *cur_user;

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
        int fl = cli_read_command(user_input, sizeof(user_input), &command);
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
            if (response.type == MSG_TYPE_ERROR) {
                printf(ANSI_COLOR_YELLOW "%s" ANSI_RESET "\n", response.payload);
                continue;
            }
            while (1) {
                recv_message(sockfd, &response);
                if (response.type == MSG_TYPE_ERROR)
                    printf("%s\n", response.payload);
                else if (response.type == MSG_DATA_CMD) {
                    printf("%s", response.payload);
                } else {
                    break;
                }
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
            std::string files;
            while (1) {
                recv_message(sockfd, &response);
                if (response.type == MSG_TYPE_ERROR)
                    printf("%s\n", response.payload);
                else if (response.type == MSG_DATA_FIND) {
                    printf("%s", response.payload);
                    std::string str(response.payload);
                    files += str;
                } else {
                    break;
                }
            }
            recv_message(sockfd, &response);
            if (response.type == MSG_TYPE_PIPE) {
                handle_pipe_download(sockfd, files);
            }
            files = "";
        } else if (command.type == MSG_TYPE_SHARE) {
            Message response;
            recv_message(sockfd, &response);
            if (response.type == MSG_TYPE_ERROR) {
                printf("%s\n", response.payload);
                continue;
            } else {
                recv_message(sockfd, &response);
                if (response.type == MSG_TYPE_ERROR) {
                    printf("%s\n", response.payload);
                    continue;
                } else {
                    printf("Shared sucessfully!\n");
                }
            }

        } else if (command.type == MSG_TYPE_PWD) {
            const char *pwd = handle_pwd(cur_user, user_dir);
            printf("%s\n", pwd);
        } else if (command.type == MSG_TYPE_UPLOAD) {
            handle_upload(data_sock, command.payload, sockfd);
        } else if (command.type == MSG_TYPE_RELOAD) {
        }
        close(data_sock);

    }   // loop back to get more user input

    // Close the socket (control connection)
    close(sockfd);
    return 0;
}

int cli_read_command(char *user_input, int size, Message *msg) {
    // wait for user to enter a command
    read_input(user_input, size);

    if (strcmp(user_input, "ls ") == 0 || strcmp(user_input, "ls") == 0) {
        msg->type = MSG_TYPE_LS;
    } else if (strncmp(user_input, "cd ", 3) == 0) {
        msg->type = MSG_TYPE_CD;
        strcpy(msg->payload, user_input + 3);
    } else if (strncmp(user_input, "find ", 5) == 0) {
        std::string cmd(user_input);
        std::string left_cmd, right_cmd;
        size_t pos = cmd.find('|');
        if (pos != std::string::npos) {
            // '|' found, split the string
            left_cmd = cmd.substr(0, pos);
            right_cmd = cmd.substr(pos + 1);

            left_cmd.erase(left_cmd.find_last_not_of(" \n\r\t") + 1);
            right_cmd.erase(0, right_cmd.find_first_not_of(" \n\r\t"));
            if (!(right_cmd == "dl" || right_cmd == "download")) {
                printf("%s not supported!\n", right_cmd.c_str());
                return -1;
            } else {
                msg->type = MSG_TYPE_FIND;
                strcpy(msg->payload, user_input + 5);
            }
        } else {
            msg->type = MSG_TYPE_FIND;
            strcpy(msg->payload, user_input + 5);
        }

    } else if (strncmp(user_input, "rm", 2) == 0) {
        msg->type = MSG_TYPE_BASIC_COMMAND;
        strcpy(msg->payload, user_input);
    } else if (strncmp(user_input, "mv", 2) == 0) {
        msg->type = MSG_TYPE_BASIC_COMMAND;
        strcpy(msg->payload, user_input);
    } else if (strncmp(user_input, "cp", 2) == 0) {
        msg->type = MSG_TYPE_BASIC_COMMAND;
        strcpy(msg->payload, user_input);
    } else if (strncmp(user_input, "share ", 6) == 0) {
        msg->type = MSG_TYPE_SHARE;
        strcpy(msg->payload, user_input + 6);
    } else if (strncmp(user_input, "mkdir", 5) == 0) {
        msg->type = MSG_TYPE_BASIC_COMMAND;
        strcpy(msg->payload, user_input);
    } else if (strncmp(user_input, "touch", 5) == 0) {
        msg->type = MSG_TYPE_BASIC_COMMAND;
        strcpy(msg->payload, user_input);
    } else if (strncmp(user_input, "cat", 3) == 0) {
        msg->type = MSG_TYPE_BASIC_COMMAND;
        strcpy(msg->payload, user_input);
    } else if (strncmp(user_input, "echo", 4) == 0) {
        msg->type = MSG_TYPE_BASIC_COMMAND;
        strcpy(msg->payload, user_input);
    } else if (strcmp(user_input, "pwd") == 0 || strcmp(user_input, "pwd ") == 0) {
        msg->type = MSG_TYPE_PWD;
    } else if (strncmp(user_input, "up ", 3) == 0 || strncmp(user_input, "upload ", 7) == 0) {
        msg->type = MSG_TYPE_UPLOAD;
        if (strncmp(user_input, "up ", 3) == 0) {
            strcpy(msg->payload, user_input + 3);
        } else {
            strcpy(msg->payload, user_input + 7);
        }
    } else if (strncmp(user_input, "dl ", 3) == 0 || strncmp(user_input, "download ", 9) == 0) {
        msg->type = MSG_TYPE_DOWNLOAD;
        if (strncmp(user_input, "dl ", 3) == 0) {
            strcpy(msg->payload, user_input + 3);
        } else {
            strcpy(msg->payload, user_input + 9);
        }
    } else if (strcmp(user_input, "quit") == 0 || strcmp(user_input, "quit ") == 0 ||
               strcmp(user_input, "exit") == 0 || strcmp(user_input, "exit ") == 0) {
        msg->type = MSG_TYPE_QUIT;
    } else if (strcmp(user_input, "clear") == 0) {
        system("clear");
    } else if (strcmp(user_input, "reload") == 0) {
        msg->type = MSG_TYPE_RELOAD;
    } else {
        return -1;
    }

    return 0;
}