#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "protocol.h"
#include "terminal.h"

int client_sock;
bool isAuthen = false;
Message *mess;
char current_usr[255];

/**
 * @brief Checks if the given string is a valid IP address.
 *
 * @param str The string to be checked.
 * @return true if the string is a valid IP address, false otherwise.
 */
bool validIPAddress(const char *str);

/**
 *  @brief authentication menu of application: login, register, exit
 *  @return void
 */
void printAuthenMenu();

/**
 *  @brief main menu of application: download, upload file, etc...
 *  @return void
 */
void printMainMenu();

/**
 * @brief get login information from user: username and password then save it to `msg->payload`
 * @brief also save username to `str`
 * @param str: username
 * @return void
 */
void getLoginInfo(char *str);

/**
 * @brief Performs the login functionality.
 *
 * This function prompts the user for a username, sends it to the server for authentication,
 * and updates the current_user variable with the authenticated username.
 *
 * @param current_user A pointer to a character array to store the authenticated username.
 */
void loginFunc(char *current_user);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ip_address> <port_number>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // check if the IP address is valid
    char *SERV_IP = argv[1];
    if (!validIPAddress(SERV_IP)) {
        fprintf(stderr, "Error: invalid IP address\n");
        return EXIT_FAILURE;
    }

    // check if the port number is valid
    char *endptr;
    errno = 0;
    uint16_t SERV_PORT = strtoul(argv[2], &endptr, 10);
    if (errno != 0 || *endptr != '\0') {
        fprintf(stderr, "Error: %s\n", errno == EINVAL ? "invalid base" : "invalid input");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr; /* server's address information */

    // Step 1: Construct socket
    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("\nError creating socket");
        return EXIT_FAILURE;
    }

    // Step 2: Specify server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERV_IP);

    // Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("\nError connecting to server");
        close(client_sock);
        return EXIT_FAILURE;
    }

    // Step 4: Communicate with server
    char choose;
    mess = (Message *) malloc(sizeof(Message));
    while (true) {
        if (!isAuthen) {
            printAuthenMenu();
            scanf(" %c", &choose);
            while (getchar() != '\n')
                ;
            switch (choose) {
                case '1':
                    loginFunc(current_usr);
                    break;
            }

        } else {
            Message msg;
            msg.type = TYPE_REQUEST_DIRECTORY;
            strcpy(msg.payload, current_usr);
            msg.length = strlen(msg.payload);
            sendMessage(client_sock, msg);

            std::string str_tree = "";
            while (true) {
                receiveMessage(client_sock, &msg);
                if (msg.length > 0) {
                    std::string payload(msg.payload);
                    str_tree += payload;
                } else {
                    break;
                }
            }
            std::system("clear");
            Terminal terminal(str_tree);
            std::string current_path = " ~";
            while (true) {
                std::cout << current_usr << "@cpp_drive:" << current_path << " $ ";
                std::string command;
                std::getline(std::cin, command);
                Command parsedCommand = parseCommand(command);
                switch (parsedCommand.command) {
                    case COMMAND::CLEAR:
                        std::system("clear");
                        break;
                    case COMMAND::LS:
                        terminal.ls();
                        break;
                    case COMMAND::CD:
                        if (parsedCommand.argument[1] == "..") {
                            current_path = " ~";
                            terminal.resetCurrentDirectory();
                            continue;
                        }
                        if (terminal.cd(parsedCommand.argument[1])) {
                            current_path += "/" + parsedCommand.argument[1];
                        }
                        break;
                    case COMMAND::UPLOAD:
                        break;
                    case COMMAND::DOWNLOAD:
                        break;
                    case COMMAND::EXIT:
                        break;
                    case COMMAND::INVALID:
                        break;
                }
            }
        }
    }
    // Step 5: Close socket
    close(client_sock);
    return EXIT_SUCCESS;
}

bool validIPAddress(const char *str) {
    struct in_addr addr;
    return inet_pton(AF_INET, str, &addr) == 1;
}

void printAuthenMenu() {
    printf("\n------------------CPP DRIVE------------------\n");
    printf("\n1 - Login");
    printf("\n2 - Register");
    printf("\n3 - Exit");
    printf("\nChoose: ");
}

void printMainMenu() {
    printf("\n------------------Menu------------------\n");
    printf("\n1 - Upload file");
    printf("\n2 - Download File");
    printf("\n3 - Rename File");
    printf("\n4 - Change directory File");
    printf("\n5 - Open folder");
    printf("\n6 - Create folder");
    printf("\n7 - Logout");
    printf("\nPlease choose: ");
}

void getLoginInfo(char *str) {
    char username[255];
    char password[255];
    printf("Enter username: ");
    scanf("%[^\n]s", username);
    while (getchar() != '\n')
        ;
    printf("Enter password: ");
    scanf("%[^\n]s", password);
    while (getchar() != '\n')
        ;
    sprintf(mess->payload, "LOGIN\nUSER %s\nPASS %s", username, password);
    strcpy(str, username);
}

void loginFunc(char *current_user) {
    char username[255];
    mess->type = TYPE_AUTHENTICATE;
    getLoginInfo(username);
    strcpy(current_user, username);
    mess->length = strlen(mess->payload);
    sendMessage(client_sock, *mess);
    receiveMessage(client_sock, mess);
    if (mess->type != TYPE_ERROR) {
        isAuthen = true;
    } else {
        printf("Login failed\n");
    }
}
