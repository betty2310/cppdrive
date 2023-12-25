#include "find.h"

#include <dirent.h>
#include <limits.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "common.h"
#include "connect.h"

SearchResult searchInDirectory(char *dirPath, char *fileName) {
    SearchResult result = {0, NULL};

    DIR *dir;
    struct dirent *entry;
    if ((dir = opendir(dirPath)) == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            // Ignore "." and ".." directories
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char path[PATH_MAX];
                snprintf(path, sizeof(path), "%s/%s", dirPath, entry->d_name);
                SearchResult subdirResult = searchInDirectory(path, fileName);

                // Merge the results
                result.count += subdirResult.count;
                result.files = (char **) realloc(result.files, result.count * sizeof(char *));
                for (int i = 0; i < subdirResult.count; ++i) {
                    result.files[result.count - subdirResult.count + i] = subdirResult.files[i];
                }

                free(subdirResult.files);
            }
        } else {
            if (strcmp(entry->d_name, fileName) == 0) {
                result.count++;
                result.files = (char **) realloc(result.files, result.count * sizeof(char *));
                result.files[result.count - 1] = (char *) malloc(PATH_MAX);
                snprintf(result.files[result.count - 1], PATH_MAX, "%s/%s", dirPath, entry->d_name);
            }
        }
    }

    closedir(dir);
    return result;
}

/**
 * Send path of the file in the dirPath
 * over data connection
 * Return -1 on error, 0 on success
 */
void ftserve_find(int sock_control, int sock_data, char *filename) {
    char curr_dir[MAX_SIZE - 2];
    memset(curr_dir, 0, MAX_SIZE - 2);
    getcwd(curr_dir, sizeof(curr_dir));
    SearchResult result = searchInDirectory(curr_dir, filename);

    // File found
    if (result.count > 0) {
        send_response(sock_control, 241);
        send_response(sock_control, result.count);
        for (int i = 0; i < result.count; ++i) {
            strcat(result.files[i], "\n");
            if (send(sock_data, result.files[i], strlen(result.files[i]), 0) < 0) {
                perror("error");
                send_response(sock_control, 550);
            }
            free(result.files[i]);   // Free each file path
        }
    }
    // File not found
    else
        send_response(sock_control, 441);
    free(result.files);   // Free the array of file paths
}