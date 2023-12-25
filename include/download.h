#ifndef __DOWNLOAD_H__
#define __DOWNLOAD_H__

int ftclient_get(int data_sock, int sock_control, char *arg);
int recvFile(int sock_control, int sock_data, char *filename);
void ftserve_retr(int sock_control, int sock_data, char *filename);

#endif
