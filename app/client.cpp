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

#define BUFF_SIZE 1024

bool is_valid_ipv4_address(const char *str) {
    struct in_addr addr;
    return inet_pton(AF_INET, str, &addr) == 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ip_address> <port_number>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // check if the IP address is valid
    char *SERV_IP = argv[1];
    if (!is_valid_ipv4_address(SERV_IP)) {
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

    char buff[BUFF_SIZE + 1];
    int client_sock;
    struct sockaddr_in server_addr; /* server's address information */
    int msg_len, bytes_sent, bytes_received;

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
    while (true) {
        // Send message
        printf("\nInsert string to send:");
        if (fgets(buff, BUFF_SIZE, stdin) == NULL) {
            perror("\nError reading input");
            break;
        }
        msg_len = strlen(buff);
        if (msg_len == 0)
            break;

        bytes_sent = send(client_sock, buff, msg_len, 0);
        if (bytes_sent == -1) {
            perror("\nError sending data");
            break;
        }

        // Receive echo reply
        bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
        if (bytes_received == -1) {
            perror("\nError receiving data");
            break;
        } else if (bytes_received == 0) {
            printf("Connection closed.\n");
            break;
        }

        buff[bytes_received] = '\0';
        int status = 100;
        printf("Reply from server: %s\n", buff);
        if (strcmp(buff, "enter password: ") != 0) {
            status = atoi(buff);
        }
        if (status == 1) {
            printf("Login successful. Access granted.\n");
            break;
        } else if (status == -1) {
            printf("Invalid username or password. Please try again.\n");
        } else if (status == 0) {
            printf("Account is blocked. Please contact the administrator.\n");
            break;
        }
    }

    // Step 5: Close socket
    close(client_sock);
    return EXIT_SUCCESS;
}