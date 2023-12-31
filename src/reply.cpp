#include "reply.h"

#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "common.h"

void print_reply(int rc) {
    switch (rc) {
        case 220:
            printf("220 Welcome, FTP server ready.\n");
            break;
        case 221:
            printf("221 Goodbye!\n");
            break;
        case 212:
            printf("212 Directory status!\n");
            break;
        case 226:
            printf("226 Closing data connection. Requested file action successful.\n");
            break;
        case 250:
            printf("250 Directory successfully changed.\n");
            break;
        case 550:
            printf("550 Requested action not taken. File unavailable.\n");
            break;
        case 551:
            printf("551 Directory out of user scope.\n");
            break;
    }
}

/**
 * Receive a response from server
 * Returns -1 on error, return code on success
 */
int read_reply(int sock_control) {
    int retcode = 0;
    if (recv(sock_control, &retcode, sizeof(retcode), 0) < 0) {
        perror("client: error reading message from server\n");
        return -1;
    }
    return retcode;
}

/**
 * Input: cmd struct with an a code and an arg
 * Concats code + arg into a string and sends to server
 */
int ftclient_send_cmd(struct command *cmd, int sock_control) {
    char buffer[SIZE + 5];
    int rc;

    sprintf(buffer, "%s %s", cmd->code, cmd->arg);

    // Send command string to server
    rc = send(sock_control, buffer, (int) strlen(buffer), 0);
    if (rc < 0) {
        perror("Error sending command to server");
        return -1;
    }
    return 0;
}

/**
 * Parse command in cstruct
 */
int ftclient_read_command(char *user_input, int size, struct command *cstruct, Message *msg) {
    memset(cstruct->code, 0, sizeof(cstruct->code));
    memset(cstruct->arg, 0, sizeof(cstruct->arg));

    // wait for user to enter a command
    read_input(user_input, size);

    if (strcmp(user_input, "ls ") == 0 || strcmp(user_input, "ls") == 0) {
        msg->type = MSG_TYPE_LS;
        strcpy(cstruct->code, "LIST");
        memset(user_input, 0, SIZE);
        strcpy(user_input, cstruct->code);
    } else if (strncmp(user_input, "cd ", 3) == 0) {
        strcpy(cstruct->code, "CWD ");
        strcpy(cstruct->arg, user_input + 3);

        memset(user_input, 0, SIZE);
        sprintf(user_input, "%s %s", cstruct->code, cstruct->arg);
    } else if (strncmp(user_input, "find ", 5) == 0) {
        strcpy(cstruct->code, "FIND");
        strcpy(cstruct->arg, user_input + 5);

        memset(user_input, 0, SIZE);
        sprintf(user_input, "%s %s", cstruct->code, cstruct->arg);
    } else if (strncmp(user_input, "renm ", 5) == 0) {
        strcpy(cstruct->code, "RENM");
        strcpy(cstruct->arg, user_input + 5);

        memset(user_input, 0, SIZE);
        sprintf(user_input, "%s %s", cstruct->code, cstruct->arg);
    } else if (strncmp(user_input, "del ", 4) == 0) {
        strcpy(cstruct->code, "DEL ");
        strcpy(cstruct->arg, user_input + 4);

        memset(user_input, 0, SIZE);
        sprintf(user_input, "%s %s", cstruct->code, cstruct->arg);
    } else if (strncmp(user_input, "mov ", 4) == 0) {
        strcpy(cstruct->code, "MOV ");
        strcpy(cstruct->arg, user_input + 4);

        memset(user_input, 0, SIZE);
        sprintf(user_input, "%s %s", cstruct->code, cstruct->arg);
    } else if (strncmp(user_input, "cpy ", 4) == 0) {
        strcpy(cstruct->code, "CPY ");
        strcpy(cstruct->arg, user_input + 4);

        memset(user_input, 0, SIZE);
        sprintf(user_input, "%s %s", cstruct->code, cstruct->arg);
    } else if (strncmp(user_input, "share ", 6) == 0) {
        strcpy(cstruct->code, "SHRE");
        strcpy(cstruct->arg, user_input + 6);

        memset(user_input, 0, SIZE);
        sprintf(user_input, "%s %s", cstruct->code, cstruct->arg);
    } else if (strncmp(user_input, "mkdir ", 6) == 0) {
        strcpy(cstruct->code, "MKDR");
        strcpy(cstruct->arg, user_input + 6);

        memset(user_input, 0, SIZE);
        sprintf(user_input, "%s %s", cstruct->code, cstruct->arg);
    } else if (strcmp(user_input, "pwd") == 0 || strcmp(user_input, "pwd ") == 0) {
        strcpy(cstruct->code, "PWD ");
        memset(user_input, 0, SIZE);
        strcpy(user_input, cstruct->code);
    } else if (strncmp(user_input, "get ", 4) == 0) {   // RETRIEVE
        strcpy(cstruct->code, "RETR");
        strcpy(cstruct->arg, user_input + 4);

        memset(user_input, 0, SIZE);
        sprintf(user_input, "%s %s", cstruct->code, cstruct->arg);
    } else if (strncmp(user_input, "put ", 4) == 0) {
        strcpy(cstruct->code, "STOR");   // STORE
        strcpy(cstruct->arg, user_input + 4);

        memset(user_input, 0, SIZE);
        sprintf(user_input, "%s %s", cstruct->code, cstruct->arg);
    } else if (strcmp(user_input, "quit") == 0) {
        strcpy(cstruct->code, "QUIT");
        memset(user_input, 0, SIZE);
        strcpy(user_input, cstruct->code);
    } else {   // invalid
        return -1;
    }

    return 0;
}