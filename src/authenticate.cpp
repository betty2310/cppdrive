#include "authenticate.h"

#include <openssl/evp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "common.h"
#include "connect.h"
#include "reply.h"

void sha256(const char *input, char *output) {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;

    md = EVP_get_digestbyname("sha256");

    if (!md) {
        printf("Unknown message digest %s\n", "sha256");
        exit(1);
    }

    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, input, strlen(input));
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    EVP_MD_CTX_free(mdctx);

    // Encode hash in hexadecimal
    for (unsigned int i = 0; i < md_len; i++) {
        sprintf(output + 2 * i, "%02x", md_value[i]);
    }
    output[2 * md_len] = '\0';
}

/**
 * Get login details from user and
 * send to server for authentication
 */
void ftclient_login(int sock_control) {
    struct command cmd;
    char user[SIZE];
    memset(user, 0, SIZE);

    // Send LOGIN command to server
    strcpy(cmd.code, "LGIN");
    strcpy(cmd.arg, "");
    ftclient_send_cmd(&cmd, sock_control);

    // Wait for go-ahead
    int wait;
    recv(sock_control, &wait, sizeof(wait), 0);

    // Get username from user
    printf("Name: ");
    fflush(stdout);
    read_input(user, SIZE);

    // Send USER command to server
    strcpy(cmd.code, "USER");
    strcpy(cmd.arg, user);
    ftclient_send_cmd(&cmd, sock_control);

    // Wait for go-ahead to send password
    recv(sock_control, &wait, sizeof(wait), 0);

    // Get password from user
    fflush(stdout);
    char *pass = getpass("Password: ");

    // Send PASS command to server
    strcpy(cmd.code, "PASS");
    strcpy(cmd.arg, pass);
    ftclient_send_cmd(&cmd, sock_control);

    // wait for response
    int retcode = read_reply(sock_control);
    switch (retcode) {
        case 430:
            printf("430 Invalid username/password or user in use.\n");
            exit(0);
        case 230:
            printf("230 Successful login.\n");
            break;
        default:
            perror("error reading message from server");
            exit(1);
            break;
    }
}

/**
 * Register new user
 */
void ftclient_register(int sock_control) {
    struct command cmd;
    char user[SIZE];
    memset(user, 0, SIZE);

    // Send REG command to server
    strcpy(cmd.code, "REG");
    ftclient_send_cmd(&cmd, sock_control);

    // Wait for go-ahead
    int wait;
    recv(sock_control, &wait, sizeof(wait), 0);

    // Send USER command to server
    int rep;
    do {
        // Get username from user
        printf("Name: ");
        fflush(stdout);
        read_input(user, SIZE);
        strcpy(cmd.code, "USER");
        strcpy(cmd.arg, user);
        ftclient_send_cmd(&cmd, sock_control);
        rep = read_reply(sock_control);
        if (rep == 431)
            printf("431 Username already exist.\n");
    } while (rep == 431);

    // Get password from user

    fflush(stdout);
    char *pass = getpass("Password: ");

    // Send PASS command to server
    strcpy(cmd.code, "PASS");
    strcpy(cmd.arg, pass);
    ftclient_send_cmd(&cmd, sock_control);

    // wait for response
    int retcode = read_reply(sock_control);
    switch (retcode) {
        case 431:
            printf("431 Username already exist.\n");
            exit(0);
        case 230:
            printf("230 Successfully registered.\n");
            break;
        default:
            perror("error reading message from server");
            exit(1);
            break;
    }
}

// Function to remove duplicates from the file
void removeDuplicates(FILE *file) {
    char line[256];
    char uniqueLines[10000][256];   // Assuming a maximum of 10000 lines and each line is not more
                                    // than 256 characters

    int numLines = 0;
    fseek(file, 0, SEEK_SET);   // Move the file pointer to the beginning

    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        strtok(line, "\n");

        int isDuplicate = 0;

        // Check if the line is a duplicate
        for (int i = 0; i < numLines; ++i) {
            if (strcmp(line, uniqueLines[i]) == 0) {
                isDuplicate = 1;
                break;
            }
        }

        // If not a duplicate, add it to the unique lines array
        if (!isDuplicate) {
            strcpy(uniqueLines[numLines], line);
            numLines++;
        }
    }

    // Move the file pointer to the beginning and truncate the file
    freopen(NULL, "w", file);

    // Write the unique lines back to the file
    for (int i = 0; i < numLines; ++i) {
        if (strcmp(uniqueLines[i], "\n") == 0)
            continue;
        fprintf(file, "%s\n", uniqueLines[i]);
    }
}

// Function to clean up the file
void cleanUpFile(const char *filename) {
    FILE *file = fopen(filename, "r+");

    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char line[256];
    FILE *tempFile = tmpfile();

    // Copy valid lines to a temporary file
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        strtok(line, "\n");
        char dir[SIZE] = "";
        strcat(dir, root_dir);
        strcat(dir, line);

        if (isFile(dir)) {
            fprintf(tempFile, "%s\n", line);
        }
    }

    // Move the file pointer to the beginning and truncate the original file
    freopen(NULL, "w", file);

    // Copy the content from the temporary file back to the original file
    fseek(tempFile, 0, SEEK_SET);
    while (fgets(line, sizeof(line), tempFile)) {
        fprintf(file, "%s\n", line);
    }

    // Close the files
    fclose(file);
    fclose(tempFile);

    // Remove duplicates from the cleaned file
    file = fopen(filename, "r+");
    if (file != NULL) {
        removeDuplicates(file);
        fclose(file);
    }
}

/**
 * Authenticate a user's credentials
 * Return 1 if authenticated, 0 if not
 */
int ftserve_check_user(char *user, char *pass, char *user_dir) {
    char username[SIZE];
    char password[SIZE];
    char curDir[SIZE];
    char shared[SIZE] = "";
    int isLock;
    char *pch;
    char buf[SIZE];
    char *line = NULL;
    size_t num_read;
    size_t len = 0;
    FILE *fd;
    int auth = 0;

    fd = fopen(AUTH_FILE, "r");
    if (fd == NULL) {
        perror("file not found");
        exit(1);
    }

    while ((num_read = getline(&line, &len, fd)) != (size_t) -1) {
        memset(buf, 0, SIZE);
        strcpy(buf, line);

        pch = strtok(buf, " ");
        strcpy(username, pch);

        if (pch != NULL) {
            pch = strtok(NULL, " ");
            strcpy(password, pch);
            pch = strtok(NULL, " ");
            trimstr(pch, (int) strlen(pch));
            isLock = atoi(pch);
        }

        // remove end of line and whitespace
        trimstr(password, (int) strlen(password));

        char outputBuffer[65];
        sha256(pass, outputBuffer);

        if ((strcmp(user, username) == 0) &&
            (strcmp(outputBuffer, password) == 0 && (isLock == 0))) {
            auth = 1;
            // Lock user to prevent concurrent login
            toggleUserLock(user, 1);

            // Change dir to user root dir
            strcat(user_dir, username);
            chdir(user_dir);

            // Save user root dir to a global variable for future use
            getcwd(curDir, sizeof(curDir));
            strcpy(user_dir, curDir);

            // Clean up user's .shared file
            strcat(shared, root_dir);
            strcat(shared, "/user/");
            strcat(shared, user);
            strcat(shared, "/.shared");
            cleanUpFile(shared);
            break;
        }
    }
    free(line);
    fclose(fd);
    return auth;
}

/**
 * Check if db has existing username
 * Return 1 if authenticated, 0 if not
 */
int ftserve_check_username(char *user) {
    char username[SIZE];
    char *pch;
    char buf[SIZE];
    char *line = NULL;
    size_t num_read;
    size_t len = 0;
    FILE *fd;
    int check = 0;

    fd = fopen(".auth", "r");
    if (fd == NULL) {
        perror("file not found");
        exit(1);
    }

    while ((num_read = getline(&line, &len, fd)) != (size_t) -1) {
        memset(buf, 0, SIZE);
        strcpy(buf, line);

        pch = strtok(buf, " ");
        strcpy(username, pch);

        if (strcmp(user, username) == 0) {
            check = 1;
            break;
        }
    }
    free(line);
    fclose(fd);
    return check;
}

/**
 * Log in connected client
 */
int ftserve_login(int sock_control, char *user_dir) {
    char buf[SIZE];
    char user[SIZE];
    char pass[SIZE];
    memset(user, 0, SIZE);
    memset(pass, 0, SIZE);
    memset(buf, 0, SIZE);

    // Wait to receive username
    if ((recv_data(sock_control, buf, sizeof(buf))) == -1) {
        perror("recv error\n");
        exit(1);
    }

    strcpy(user, buf + 5);   // 'USER ' has 5 char

    // tell client we're ready for password
    send_response(sock_control, 331);

    // Wait to receive password
    memset(buf, 0, SIZE);
    if ((recv_data(sock_control, buf, sizeof(buf))) == -1) {
        perror("recv error\n");
        exit(1);
    }

    strcpy(pass, buf + 5);   // 'PASS ' has 5 char

    return (ftserve_check_user(user, pass, user_dir));
}

/**
 * Log in connected client
 */
int ftserve_register(int sock_control) {
    char buf[SIZE];
    char user[SIZE];
    char pass[SIZE];
    char userDir[SIZE] = "user/";
    memset(user, 0, SIZE);
    memset(pass, 0, SIZE);
    memset(buf, 0, SIZE);

    // Wait to receive username
    if ((recv_data(sock_control, buf, sizeof(buf))) == -1) {
        perror("recv error\n");
        exit(1);
    }

    strcpy(user, buf + 5);   // 'USER ' has 5 char

    while (ftserve_check_username(user)) {
        // tell client username already exist
        send_response(sock_control, 431);
        // Wait to receive username
        if ((recv_data(sock_control, buf, sizeof(buf))) == -1) {
            perror("recv error\n");
            exit(1);
        }
        strcpy(user, buf + 5);   // 'USER ' has 5 char
    }

    // tell client we're ready for password
    send_response(sock_control, 331);

    // Wait to receive password
    memset(buf, 0, SIZE);
    if ((recv_data(sock_control, buf, sizeof(buf))) == -1) {
        perror("recv error\n");
        exit(1);
    }

    strcpy(pass, buf + 5);   // 'PASS ' has 5 char}

    FILE *fptr;

    fptr = fopen(AUTH_FILE, "a");
    if (fptr == NULL) {
        perror("file not found");
        exit(1);
    }
    fprintf(fptr, "%s ", user);
    char outputBuffer[65];
    sha256(pass, outputBuffer);
    fprintf(fptr, "%s 0\n", outputBuffer);

    // Make new user folder
    strcat(userDir, user);
    createDirectory(userDir);
    strcat(userDir, "/.shared");
    FILE *shared;
    shared = fopen(userDir, "w");
    fclose(shared);

    fclose(fptr);
    return 1;
}