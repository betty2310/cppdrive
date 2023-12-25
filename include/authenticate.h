#ifndef __AUTHENTICATE_H__
#define __AUTHENTICATE_H__

#include <cstdio>

void ftclient_login(int sock_control);
void ftclient_register(int sock_control);
void sha256(const char *input, char *output);

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
int ftserve_check_username(char *user);

/**
 * Log in connected client
 */
int ftserve_login(int sock_control, char *user_dir);

/**
 * Log in connected client
 */
int ftserve_register(int sock_control);

#endif
