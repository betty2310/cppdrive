#ifndef __CONNECT_H__
#define __CONNECT_H__

#include "message.h"

/**
 * Send message over socket
 * @param sockfd socket to send message
 * @param msg message to send
 * @return 0 if success, -1 if error
 */
int send_message(int sockfd, Message msg);

/**
 * Receive message over socket
 * @param sockfd socket to receive message
 * @param msg message to receive
 * @return 0 if success, -1 if error
 */
int recv_message(int sockfd, Message *msg);

int socket_create(int port);
int socket_accept(int sock_listen);
int client_start_conn(int sock_con);

int recv_data(int sockfd, char *buf, int bufsize);
int socket_connect(int port, char *host);
int server_start_conn(int sock_control);

#endif