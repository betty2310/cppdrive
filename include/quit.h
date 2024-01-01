#ifndef __QUIT_H__
#define __QUIT_H__

/**
 * Quit server
 * @param sockfd socket
 * @param cur_user current user
 */
void server_quit(int sockfd, char* cur_user);

#endif   // !__QUIT_H__
