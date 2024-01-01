#ifndef __COMMON_H__
#define __COMMON_H__

#define SIZE          1024
#define DEFAULT_PORT  3001
#define MAX_CLIENTS   10
#define ACCOUNTS_FILE "accounts.txt"
#define APP_STORAGE   "storage/"

extern const char* process;
extern char root_dir[SIZE];

extern std::string public_key;
extern std::string private_key;
extern std::string public_client_key;
extern std::string public_server_key;

#endif   // !__COMMON_H__
