#ifndef __COMMAND_H__
#define __COMMAND_H__

/**
 * Run command on server, send result to client over sockfd
 * @param sockfd: socket to send result to
 * @param cmd: command to run
 */
int process_command(int sockfd, char* cmd);

#endif   // !__COMMAND_H__