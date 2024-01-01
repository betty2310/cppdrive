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

/**
 * share file to another user
 * @param sockfd: socket to send result to
 * @param arg: argument of command
 * @param user_dir: current directory of user
 * @return 0 if success, -1 if error
 */
int server_share(int sockfd, char* arg, char* user_dir);

/**
 * Load shared file to shared folder of each user
 */
int load_shared_file(char* user_dir);

#endif   // !__COMMAND_H__