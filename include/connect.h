#ifndef __CONNECT_H__
#define __CONNECT_H__

int socket_create(int port);
int socket_accept(int sock_listen);
int ftclient_open_conn(int sock_con);
int send_response(int sockfd, int rc);

int recv_data(int sockfd, char *buf, int bufsize);
int socket_connect(int port, char *host);
int ftserve_start_data_conn(int sock_control);
int ftserve_recv_cmd(int sock_control, char *cmd, char *arg, char *cur_user);

#endif