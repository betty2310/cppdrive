#ifndef __CWD_H__
#define __CWD_H__

// Function to check if a directory is a subdirectory of the base directory
int isSubdirectory(const char *baseDir, const char *dir);
/**
 * Change directory
 * Return -1 on error, 0 on success
 */
int ftpServer_cwd(int sock_control, char *folderName, char *user_dir);

#endif