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
int ftclient_open_conn(int sock_con);
int send_response(int sockfd, int rc);

int recv_data(int sockfd, char *buf, int bufsize);
int socket_connect(int port, char *host);
int ftserve_start_data_conn(int sock_control);
int ftserve_recv_cmd(int sock_control, char *cmd, char *arg, char *cur_user);

#endif