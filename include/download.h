#ifndef __DOWNLOAD_H__
#define __DOWNLOAD_H__

#include <string>
/**
 * Client download file or folder from server
 * @param data_sock socket data
 * @param sock_control socket control
 * @param arg file or folder to download
 * @return 0 on success, -1 on error
 */
int handle_download(int data_sock, int sock_control, char *arg);

/**
 * Download file or folder from server
 * @param sock_control socket control
 * @param sock_data socket data
 * @param dir directory to download
 */
void server_download(int sock_control, int sock_data, char *dir);

/**
 * Download list of files from server
 */
int handle_pipe_download(int sockfd, std::string files);
void server_pipe_download(int sockfd, char *output);
#endif
