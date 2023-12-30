#include "common.h"

#include <sys/stat.h>
#include <unistd.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

/**
 * Read input from command line
 */
void read_input(char *user_input, int size) {
    memset(user_input, 0, size);
    int n = read(STDIN_FILENO, user_input, size);
    user_input[n] = '\0';

    /* Remove trailing return and newline characters */
    if (user_input[n - 1] == '\n')
        user_input[n - 1] = '\0';
    if (user_input[n - 1] == '\r')
        user_input[n - 1] = '\0';
}

// Function to check if a path corresponds to a file

int isFile(const char *path) {
    struct stat pathStat;
    if (stat(path, &pathStat) == 0) {
        return S_ISREG(pathStat.st_mode);
    }
    return 0;   // Return 0 for error or if the path is not a regular file
}

// Function to check if a path corresponds to a directory (folder)
int isDirectory(const char *path) {
    struct stat pathStat;
    if (stat(path, &pathStat) == 0) {
        return S_ISDIR(pathStat.st_mode);
    }
    return 0;   // Return 0 for error or if the path is not a directory
}

void trimstr(char *str, int n) {
    int i;
    for (i = 0; i < n; i++) {
        if (isspace(str[i]))
            str[i] = 0;
        if (str[i] == '\n')
            str[i] = 0;
    }
}

int splitString(char *input, char **str1, char **str2) {
    if (input == NULL || strlen(input) == 0) {
        // Return error for empty string
        return -1;
    }

    // Find the first occurrence of a space in the string
    char *spacePos = strchr(input, ' ');

    if (spacePos == NULL || *(spacePos + 1) == '\0') {
        // Return error if there is no space or there is no character after space
        return -1;
    }

    // Calculate the length of the first substring
    size_t len1 = spacePos - input;

    // Allocate memory for the first substring and copy it
    *str1 = (char *) malloc(len1 + 1);
    if (*str1 == NULL) {
        // Return error if memory allocation fails
        return -1;
    }
    strncpy(*str1, input, len1);
    (*str1)[len1] = '\0';   // Null-terminate the string

    // Find the first non-space character after the space
    char *nonSpacePos = spacePos + 1;
    while (*nonSpacePos != '\0' && *nonSpacePos == ' ') {
        nonSpacePos++;
    }

    if (*nonSpacePos == '\0') {
        // Return error if there are no characters after the space
        free(*str1);
        return -1;
    }

    // Calculate the length of the second substring
    size_t len2 = strlen(nonSpacePos);

    // Allocate memory for the second substring and copy it
    *str2 = (char *) malloc(len2 + 1);
    if (*str2 == NULL) {
        // Return error if memory allocation fails
        free(*str1);
        return -1;
    }
    strcpy(*str2, nonSpacePos);

    return 0;   // Success
}

void toggleUserLock(const char *username, int lockStatus) {
    // Open the .auth file in read mode
    FILE *file = fopen(AUTH_FILE, "r");

    // Check if the file opened successfully
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    // Temporary variables to store data read from the file
    char currentUsername[100];
    char password[100];
    int isLocked;

    // Create a temporary file to store updated information
    FILE *tempFile = fopen("temp.auth", "w");

    // Check if the temporary file opened successfully
    if (tempFile == NULL) {
        printf("Error creating temporary file!\n");
        fclose(file);
        return;
    }

    // Read lines from the .auth file
    while (fscanf(file, "%s %s %d", currentUsername, password, &isLocked) == 3) {
        // Check if the current line corresponds to the given username
        if (strcmp(currentUsername, username) == 0) {
            // Update the lock status
            fprintf(tempFile, "%s %s %d\n", currentUsername, password, lockStatus);
        } else {
            // Copy the line as is to the temporary file
            fprintf(tempFile, "%s %s %d\n", currentUsername, password, isLocked);
        }
    }

    // Close both files
    fclose(file);
    fclose(tempFile);

    // Replace the original .auth file with the temporary file
    remove(".auth");
    rename("temp.auth", ".auth");

    printf("User '%s' has been %s.\n", username, lockStatus == 1 ? "locked" : "unlocked");
}

int create_user_storage(const char *path) {
    int status = 0;
    status = mkdir(path, 0755);

    if (status == 0) {
        printf("Directory %s created successfully.\n", path);
        return 0;
    } else {
        perror("Error creating directory");
        return -1;
    }
}

// Function to extract the username from userpath
char *extractUsername(char *path) {
    char *lastSlash = strrchr(path, '/');

    if (lastSlash != NULL) {
        // Return the substring after the last '/'
        return lastSlash + 1;
    }

    // Return the original path if no '/'
    return path;
}
