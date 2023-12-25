#ifndef __FILE_H__
#define __FILE_H__

int renameFile(const char *oldName, const char *newName);
void handleError(const char *message);
void recursiveDelete(const char *path);
int deleteFolder(const char *path);
int deleteFile(const char *filename);
// Function to copy or move a file
// Parameters:
//   sourceFilename: path to the source file
//   destinationFilename: path to the destination file
//   mode: 0 for copy, 1 for move
int copyOrMoveFile(char *sourceFilename, char *destinationFilename, int mode);
int copyDirectory(char *sourcePath, char *destinationPath);
// Function to recursively move files and subdirectories
int moveDirectory(char *sourcePath, char *destinationPath);
void appendLineToSharedFile(const char *username, const char *line);
void processUserFolder(const char *userFolderPath, const char *excludedUsername, const char *line);

void ftserve_rename(int sock_control, char *arg);
void ftserve_delete(int sock_control, char *arg);
void ftserve_mkdir(int sock_control, char *arg);
void ftserve_move(int sock_control, char *arg);
void ftserve_copy(int sock_control, char *arg);
void ftserve_share(int sock_control, char *arg, char *cur_user);

#endif