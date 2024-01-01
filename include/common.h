#ifndef __COMMON_H__
#define __COMMON_H__

#define SIZE           1024
#define INVALID_SOCKET -1
#define PORT           9000
#define DEFAULT_PORT   3001
#define AUTH_FILE      ".auth"

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct addrinfo ADDRINFO;

extern char root_dir[SIZE];

void read_input(char *user_input, int size);
int isFile(const char *path);

/**
 * Check if path is a folder
 * @param path path to check
 * @return 1 if path is a folder, 0 if not
 */
int is_folder(const char *path);

/**
 * Trim whitespace and line ending
 * characters from a string
 */
void trimstr(char *str, int n);

// Function to lock or unlock a user in the .auth file (0 for unlock, 1 for lock)
void toggle_lock(const char *username, int lockStatus);

/**
 * create user storage
 * @param path path to user storage
 * @return 0 if success, -1 if error
 */
int create_user_storage(const char *path);

/**
 * get username from path
 * @param path path to user storage
 * @return char pointer to username
 */
char *get_username(char *path);
#endif   // !__COMMON_H__
