#ifndef __COMMON_H__
#define __COMMON_H__

#define SIZE           1024
#define INVALID_SOCKET -1
#define INVALID_IP     -1
#define PORT           9000
#define DEFAULT_PORT   3001
#define AUTH_FILE      ".auth"

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct addrinfo ADDRINFO;

struct command {
    char arg[SIZE];
    char code[5];
};

extern char root_dir[SIZE];

/*Validating IP Address*/
int validate_ip(const char *ip);
void read_input(char *user_input, int size);
int isFile(const char *path);
int isDirectory(const char *path);

/**
 * Trim whitespace and line ending
 * characters from a string
 */
void trimstr(char *str, int n);

/**
 * Function to split arg
 * eg input="name1 name2" into str1="name1",str2="name2"
 * return 0 when success, return -1 on error
 */
int splitString(char *input, char **str1, char **str2);

// Function to lock or unlock a user in the .auth file (0 for unlock, 1 for lock)
void toggleUserLock(const char *username, int lockStatus);

// Function to create a directory
int createDirectory(const char *path);

// Function to extract the username from userpath
char *extractUsername(char *path);
#endif   // !__COMMON_H__
