#ifndef __LS_H__
#define __LS_H__

/**
 * @brief List files and directories in current working directory
 * @param sockfd Socket to receive data
 * @return 0 if success, -1 if error
 */
int handle_list(int sockfd);

/**
 * @brief Server handle list files and directories in current working directory
 * @param sockfd Socket to send data
 * @return 0 if success, -1 if error
 */
int server_list(int sockfd);

#endif   // !__LS_H__
