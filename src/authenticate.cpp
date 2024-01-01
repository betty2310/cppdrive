#include "authenticate.h"

#include <openssl/evp.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "common.h"
#include "connect.h"
#include "message.h"

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

char *get_acc_from_cli() {
    char *payload = (char *) malloc(PAYLOAD_SIZE);

    char user[SIZE];
    memset(user, 0, SIZE);

    printf("Enter username: ");
    fflush(stdout);
    read_input(user, SIZE);
    strcat(payload, user);

    fflush(stdout);
    char *pass = getpass("Enter password: ");
    strcat(payload, " ");
    strcat(payload, pass);

    return payload;
}

char *handle_login(int sockfd) {
    Message msg;

    char *payload = (char *) malloc(PAYLOAD_SIZE);
    memset(payload, 0, PAYLOAD_SIZE);
    payload = get_acc_from_cli();

    msg = create_message(MSG_TYPE_AUTHEN, payload);

    send_message(sockfd, msg);

    Message response;
    recv_message(sockfd, &response);
    switch (response.type) {
        case MSG_TYPE_ERROR:
            printf("%s\n", response.payload);
            exit(0);
        case MSG_TYPE_OK:
            printf("%s\n", response.payload);
            break;
        default:
            perror("error reading message from server");
            exit(1);
            break;
    }
    return strtok(payload, " ");
}

void register_acc(int sockfd) {
    Message msg;
    char *payload = (char *) malloc(PAYLOAD_SIZE);
    memset(payload, 0, PAYLOAD_SIZE);
    payload = get_acc_from_cli();
    msg = create_message(MSG_TYPE_REGISTER, payload);

    send_message(sockfd, msg);

    Message response;
    recv_message(sockfd, &response);

    switch (response.type) {
        case MSG_TYPE_ERROR:
            printf("%s\n", response.payload);
            exit(0);
        case MSG_TYPE_OK:
            printf("%s\n", response.payload);
            break;
        default:
            perror("error reading message from server");
            exit(1);
            break;
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

        // printf("%d %d %d\n", strcmp(user, username), strcmp(outputBuffer, password), isLock);
        if ((strcmp(user, username) == 0) &&
            (strcmp(outputBuffer, password) == 0 && (isLock == 0))) {
            auth = 1;
            // Lock user to prevent concurrent login
            toggle_lock(user, 1);

            // Change dir to user root dir
            strcat(user_dir, username);
            chdir(user_dir);

            // Save user root dir to a global variable for future use
            getcwd(curDir, sizeof(curDir));
            strcpy(user_dir, curDir);
            break;
        }
    }
    free(line);
    fclose(fd);
    return auth;
}

/**
 * Check if username exist
 * @return 1 if authenticated, 0 if not
 */
int check_username(char *user) {
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

int server_login(Message msg, char *user_dir) {
    char buf[SIZE];
    char user[SIZE];
    char pass[SIZE];
    memset(user, 0, SIZE);
    memset(pass, 0, SIZE);
    memset(buf, 0, SIZE);

    strcpy(buf, msg.payload);
    strcpy(user, strtok(buf, " "));
    strcpy(pass, strtok(NULL, " "));

    return (ftserve_check_user(user, pass, user_dir));
}

int server_register(int sock_control, Message msg) {
    char buf[SIZE];
    char user[SIZE];
    char pass[SIZE];
    char user_storage[SIZE] = "user/";
    memset(user, 0, SIZE);
    memset(pass, 0, SIZE);
    memset(buf, 0, SIZE);

    strcpy(buf, msg.payload);
    strcpy(user, strtok(buf, " "));
    strcpy(pass, strtok(NULL, " "));

    Status status;
    while (check_username(user)) {
        status = USERNAME_EXIST;
        Message response = create_status_message(MSG_TYPE_ERROR, status);
        send_message(sock_control, response);
        // Wait to receive username
        if ((recv_data(sock_control, buf, sizeof(buf))) == -1) {
            perror("recv error\n");
            exit(1);
        }
        memset(user, 0, SIZE);
        memset(pass, 0, SIZE);
        memset(buf, 0, SIZE);

        strcpy(buf, msg.payload);
        strcpy(user, strtok(buf, " "));
        strcpy(pass, strtok(NULL, " "));
    }

    FILE *fp;

    fp = fopen(AUTH_FILE, "a");
    if (fp == NULL) {
        perror("AUTH_FILE not found");
        exit(1);
    }
    fprintf(fp, "%s ", user);
    char hash_pass[65];
    sha256(pass, hash_pass);
    fprintf(fp, "%s 0\n", hash_pass);

    // Create storage directory for new user
    strcat(user_storage, user);
    create_user_storage(user_storage);
    strcat(user_storage, "/share");
    mkdir(user_storage, 0777);
    strcat(user_storage, "/.share");
    FILE *share = fopen(user_storage, "w");
    fclose(share);

    fclose(fp);
    return 1;
}