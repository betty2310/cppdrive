#include "cwd.h"

#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "common.h"
#include "connect.h"

// Function to check if a directory is a subdirectory of the base directory
int isSubdirectory(const char *baseDir, const char *dir) {
    size_t baseDirLen = strlen(baseDir);

    if (strncmp(baseDir, dir, baseDirLen) == 0) {
        if (dir[baseDirLen] == '\0' || dir[baseDirLen] == '/')
            return 1;   // It is a subdirectory
    }

    return 0;   // It is not a subdirectory
}

/**
 * Change directory
 * Return -1 on error, 0 on success
 */
int ftpServer_cwd(int sock_control, char *folderName, char *user_dir) {
    char cur_dir[MAX_SIZE];
    char prev_dir[MAX_SIZE];
    getcwd(prev_dir, sizeof(prev_dir));

    if (chdir(folderName) == 0)   // change directory
    {
        getcwd(cur_dir, sizeof(cur_dir));
        if (!isSubdirectory(user_dir, cur_dir)) {
            chdir(prev_dir);
            send_response(sock_control, 551);   // 551 Directory out of user scope
        } else
            send_response(sock_control, 250);   // 250 Directory successfully changed.
    } else {
        send_response(sock_control, 550);   // 550 Requested action not taken
    }
    return 0;
}