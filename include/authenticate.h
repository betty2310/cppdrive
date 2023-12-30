#ifndef __AUTHENTICATE_H__
#define __AUTHENTICATE_H__

#include <cstdio>

#include "message.h"

void sha256(const char *input, char *output);

/**
 * Login to cppdrive
 * @param sock_control socket for control connection
 */
void handle_login(int sock_control);

/**
 * Register new account
 * @param sockfd socket for control connection
 */
void register_acc(int sockfd);

// Function to remove duplicates from the file
void removeDuplicates(FILE *file);
// Function to clean up the file
void cleanUpFile(const char *filename);

/**
 * Authenticate a user's credentials
 * Return 1 if authenticated, 0 if not
 */
int ftserve_check_user(char *user, char *pass, char *user_dir);

/**
 * Check if db has existing username
 * Return 1 if authenticated, 0 if not
 */
int check_username(char *user);

/**
 * server handle login
 * @param msg message to receive
 * @param user_dir user directory
 * @return 1 if authenticated, 0 if not
 */
int server_login(Message msg, char *user_dir);

/**
 * server handle register
 * @param sockfd socket for control connection
 * @param msg message to receive
 * @return 1 if success, 0 if not
 */
int server_register(int sockfd, Message msg);

#endif
