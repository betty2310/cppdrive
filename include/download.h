#ifndef __DOWNLOAD_H__
#define __DOWNLOAD_H__

int ftclient_get(int data_sock, int sock_control, char *arg);
int recvFile(int sock_control, int sock_data, char *filename);

/**
 * Download file or folder from server
 * @param sock_control socket control
 * @param sock_data socket data
 * @param dir directory to download
 */
void server_download(int sock_control, int sock_data, char *dir);

#endif
