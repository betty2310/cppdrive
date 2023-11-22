#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "account.h"

#define BACKLOG  20 /* Number of allowed connections */
#define FILENAME "../account.txt"

Account account_list = NULL;

/* Receive and echo message to client */
void echo(int);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./server <port_number>\n");
        return 0;
    }
    char *endstr;
    // check if the port number is valid
    errno = 0;
    uint16_t PORT = strtoul(argv[1], &endstr, 10);

    if (errno != 0 || *endstr != '\0') {
        printf("Error: %s\n", errno == EINVAL ? "invalid base" : "invalid input");
        return 0;
    }
    account_list = read_account(FILENAME);

    int listenfd, connfd;
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    socklen_t sin_size;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { /* calls socket() */
        perror("\nError: ");
        return 0;
    }

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */

    if (bind(listenfd, (struct sockaddr *) &server, sizeof(server)) == -1) {
        perror("\nError: ");
        return 0;
    }

    if (listen(listenfd, BACKLOG) == -1) {
        perror("\nError: ");
        return 0;
    }

    sin_size = sizeof(struct sockaddr_in);

    while (true) {
        if ((connfd = accept(listenfd, (struct sockaddr *) &client, &sin_size)) == -1) {
            perror("\nError: ");
        }

        printf("You got a connection from %s\n",
               inet_ntoa(client.sin_addr)); /* prints client's IP */

        echo(connfd);

        close(connfd);
    }

    close(listenfd);
    return 0;
}

void echo(int connfd) {
    int bytes_sent, bytes_received;
    char buff[MAX_CHARS + 1];
    char username[MAX_CHARS + 1];
    char password[MAX_CHARS + 1];
    int status = USERNAME_REQUIRED;
    while (true) {
        bytes_received = recv(connfd, buff, MAX_CHARS, 0);   // blocking
        if (bytes_received == -1) {
            perror("\nError: ");
            break;
        } else if (bytes_received == 0) {
            printf("Connection closed.\n");
            break;
        }
        buff[bytes_received - 1] = '\0';
        printf("Received from client: %s\n", buff);
        switch (status) {
            case USERNAME_REQUIRED: {
                if (strcmp(buff, "") == 0) {
                    strcpy(buff, "goodbye!");
                    memset(username, '\0', sizeof(username));
                } else {
                    strcpy(username, buff);
                    strcpy(buff, "enter password: ");
                    status = PASSWORD_REQUIRED;
                }
                break;
            }
            case PASSWORD_REQUIRED: {
                strcpy(password, buff);
                status = process_login(account_list, username, password);
                switch (status) {
                    case USERNAME_REQUIRED: {
                        strcpy(buff, "-1");
                        memset(username, '\0', sizeof(username));
                        memset(password, '\0', sizeof(password));
                        break;
                    }
                    case VALID_CREDENTIALS: {
                        strcpy(buff, "1");
                        break;
                    }
                    case WRONG_PASSWORD: {
                        strcpy(buff, "-1");
                        status = USERNAME_REQUIRED;
                        memset(username, '\0', sizeof(username));
                        memset(password, '\0', sizeof(password));
                        break;
                    }
                    case ACCOUNT_NOT_ACTIVE: {
                        strcpy(buff, "0");
                        status = USERNAME_REQUIRED;
                        memset(username, '\0', sizeof(username));
                        memset(password, '\0', sizeof(password));
                        break;
                    }
                }
                break;
            }
        }

        bytes_sent = send(connfd, buff, strlen(buff), 0); /* reply to client with proper messages */
        if (bytes_sent == -1) {
            perror("\nError: ");
            break;
        }
    }
    close(connfd);
}