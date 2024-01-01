#ifndef __COMMAND_H__
#define __COMMAND_H__

/**
 * Run command on server, send result to client over sockfd
 * @param sockfd: socket to send result to
 * @param cmd: command to run
 */
int process_command(int sockfd, char* cmd);

/**
 * List files in current directory
 * @param sockfd: socket to send result to
 * @param arg: argument of command
 * @return 0 if success, -1 if error
 */
int server_find(int sockfd, char* arg);

#endif   // !__COMMAND_H__