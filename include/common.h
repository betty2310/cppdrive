#ifndef __COMMON_H__
#define __COMMON_H__

#define SIZE           1024
#define INVALID_SOCKET -1
#define PORT           9000
#define DEFAULT_PORT   3001
#define ACCOUNTS_FILE  "accounts.txt"
#define APP_STORAGE    "storage/"

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct addrinfo ADDRINFO;

extern char root_dir[SIZE];

#endif   // !__COMMON_H__
