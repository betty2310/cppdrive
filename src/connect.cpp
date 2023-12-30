#include "connect.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "common.h"

/**
 * Create listening socket on remote host
 * Returns -1 on error, socket fd on success
 */
int socket_create(int port) {
    int sockfd;
    SOCKADDR_IN sock_addr;

    // create new socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket() error");
        return -1;
    }

    // set local address info
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind
    int flag = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag));
    if (bind(sockfd, (SOCKADDR *) &sock_addr, sizeof(sock_addr)) < 0) {
        close(sockfd);
        perror("bind() error");
        return -1;
    }

    // begin listening for incoming TCP requests
    if (listen(sockfd, 5) < 0) {
        close(sockfd);
        perror("listen() error");
        return -1;
    }
    return sockfd;
}

/**
 * Create new socket for incoming client connection request
 * Returns -1 on error, or fd of newly created socket
 */
int socket_accept(int sock_listen) {
    int sockfd;
    SOCKADDR_IN client_addr;
    socklen_t len = sizeof(client_addr);   // Cast len to socklen_t*

    // Wait for incoming request, store client info in client_addr
    sockfd = accept(sock_listen, (SOCKADDR *) &client_addr, &len);   // Cast &len to socklen_t*

    if (sockfd < 0) {
        perror("accept() error");
        return -1;
    }
    return sockfd;
}

/**
 * Open data connection
 */
int ftclient_open_conn(int sock_con) {
    int sock_listen = socket_create(DEFAULT_PORT);

    // send an ACK on control conn
    int ack = 1;
    if ((send(sock_con, &ack, sizeof(ack), 0)) < 0) {
        printf("client: ack write error :%d\n", errno);
        exit(1);
    }

    int sock_conn = socket_accept(sock_listen);
    close(sock_listen);
    return sock_conn;
}

int send_response(int sockfd, int rc) {
    int conv = rc;
    if (send(sockfd, &conv, sizeof(conv), 0) < 0) {
        perror("error sending...\n");
        return -1;
    }
    return 0;
}

/**
 * Receive data on sockfd
 * Returns -1 on error, number of bytes received
 * on success
 */
int recv_data(int sockfd, char *buf, int bufsize) {
    memset(buf, 0, bufsize);
    int num_bytes = recv(sockfd, buf, bufsize, 0);
    if (num_bytes < 0) {
        return -1;
    }
    return num_bytes;
}

/**
 * Connect to remote host at given port
 * Returns:	socket fd on success, -1 on error
 */
int socket_connect(int port, char *host) {
    int sockfd;
    SOCKADDR_IN dest_addr;

    // create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("error creating socket");
        return -1;
    }

    // create server address
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = inet_addr(host);

    // Connect on socket
    if (connect(sockfd, (struct sockaddr *) &dest_addr, sizeof(dest_addr)) < 0) {
        perror("error connecting to server");
        return -1;
    }
    return sockfd;
}

/**
 * Open data connection to client
 * Returns: socket for data connection
 * or -1 on error
 */
int ftserve_start_data_conn(int sock_control) {
    char buf[1024];
    int wait, sock_data;

    // Wait for go-ahead on control conn
    if (recv(sock_control, &wait, sizeof wait, 0) < 0) {
        perror("Error while waiting");
        return -1;
    }

    // Get client address
    SOCKADDR_IN client_addr;
    socklen_t len = sizeof(client_addr);   // Cast len to socklen_t
    getpeername(sock_control, reinterpret_cast<struct sockaddr *>(&client_addr),
                &len);   // Cast len to socklen_t

    inet_ntop(AF_INET, &client_addr.sin_addr, buf, sizeof(buf));

    // Initiate data connection with client
    if ((sock_data = socket_connect(DEFAULT_PORT, buf)) < 0)
        return -1;

    return sock_data;
}

/**
 * Wait for command from client and
 * send response
 * Returns response code
 */
int ftserve_recv_cmd(int sock_control, char *cmd, char *arg, char *cur_user) {
    int rc = 200;
    char user_input[SIZE];

    memset(user_input, 0, SIZE);
    memset(cmd, 0, 5);
    memset(arg, 0, SIZE);

    // Wait to receive command
    if ((recv_data(sock_control, user_input, sizeof(user_input))) == -1) {
        perror("recv error\n");
        return -1;
    }

    strncpy(cmd, user_input, 4);
    strcpy(arg, user_input + 5);

    if (strcmp(cmd, "QUIT") == 0) {
        chdir(root_dir);
        toggleUserLock(cur_user, 0);
        rc = 221;
    } else if ((strcmp(cmd, "USER") == 0) || (strcmp(cmd, "PASS") == 0) ||
               (strcmp(cmd, "LIST") == 0) || (strcmp(cmd, "RETR") == 0) ||
               (strcmp(cmd, "CWD ") == 0) || (strcmp(cmd, "PWD ") == 0) ||
               (strcmp(cmd, "LGIN") == 0) || (strcmp(cmd, "REG ") == 0) ||
               (strcmp(cmd, "STOR") == 0) || (strcmp(cmd, "FIND") == 0) ||
               (strcmp(cmd, "RENM") == 0) || (strcmp(cmd, "DEL ") == 0) ||
               (strcmp(cmd, "MOV ") == 0) || (strcmp(cmd, "CPY ") == 0) ||
               (strcmp(cmd, "MKDR") == 0) || (strcmp(cmd, "SHRE") == 0)) {
        rc = 200;
    } else {   // invalid command
        rc = 502;
    }

    send_response(sock_control, rc);
    return rc;
}