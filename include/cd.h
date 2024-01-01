#ifndef __CD_H__
#define __CD_H__

// Function to check if a directory is a subdirectory of the base directory
int is_subdir(const char *baseDir, const char *dir);
/**
 * Change directory
 * Return -1 on error, 0 on success
 */
int server_cd(int sock_control, char *folderName, char *user_dir, char *cur_user_dir);

#endif