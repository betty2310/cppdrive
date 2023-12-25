#include "download.h"

#include <sys/socket.h>
#include <utils.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "common.h"
#include "connect.h"
#include "reply.h"

int ftclient_get(int data_sock, int sock_control, char *arg) {
    char data[MAX_SIZE];
    int size, stt = 0;
    int isReceiveFile = read_reply(sock_control);

    recv(sock_control, &stt, sizeof(stt), 0);
    if (stt == 550) {
        print_reply(stt);
        return -1;
    }

    char folderName[MAX_SIZE];
    strcpy(folderName, arg);
    if (!isReceiveFile)
        strcat(arg, ".zip");
    FILE *fd = fopen(arg, "w");

    while ((size = recv(data_sock, data, MAX_SIZE, 0)) > 0) fwrite(data, 1, size, fd);

    if (size < 0)
        perror("error\n");

    fclose(fd);
    int rep = read_reply(sock_control);
    print_reply(rep);
    if (!isReceiveFile) {
        unzipFolder(arg, folderName);
        remove(arg);
    }
    return 0;
}

int recvFile(int sock_control, int sock_data, char *filename) {
    char data[MAX_SIZE];
    int size, stt = 0;
    int isReceiveFile = read_reply(sock_control);

    recv(sock_control, &stt, sizeof(stt), 0);
    // printf("%d\n", stt);
    if (stt == 550) {
        printf("can't not open file!\n");
        return -1;
    }
    if (strcmp(filename, ".shared") == 0) {
        printf("User should not upload .shared!\n");
        return -1;
    }

    char folderName[MAX_SIZE];
    strcpy(folderName, filename);
    if (!isReceiveFile)
        strcat(filename, ".zip");
    FILE *fd = fopen(filename, "w");

    while ((size = recv(sock_data, data, MAX_SIZE, 0)) > 0) {
        fwrite(data, 1, size, fd);
    }

    if (size < 0) {
        perror("error\n");
    }
    fclose(fd);
    if (!isReceiveFile) {
        unzipFolder(filename, folderName);
        remove(filename);
    }
    return 0;
}

/**
 * Send file specified in filename over data connection, sending
 * control message over control connection
 * Handles case of null or invalid filename
 */
void ftserve_retr(int sock_control, int sock_data, char *filename) {
    FILE *fd = NULL;
    char data[MAX_SIZE];
    memset(data, 0, MAX_SIZE);
    size_t num_read;

    char tempZip[MAX_SIZE];
    int isDir = isDirectory(filename);

    if (isDir) {
        strcpy(tempZip, filename);
        strcat(tempZip, ".zip");
        zipFolder(filename, tempZip);
        strcpy(filename, tempZip);
        // tell client that we're sending a folder
        send_response(sock_control, 0);
    } else
        send_response(sock_control, 1);

    fd = fopen(filename, "r");

    if (!fd) {
        // send error code (550 Requested action not taken)
        send_response(sock_control, 550);
    } else {
        // send okay (150 File status okay)
        send_response(sock_control, 150);

        do {
            num_read = fread(data, 1, MAX_SIZE, fd);
            // send block
            if (send(sock_data, data, num_read, 0) < 0)
                perror("error sending file\n");

        } while (num_read > 0);

        // send message: 226: closing conn, file transfer successful
        send_response(sock_control, 226);

        fclose(fd);
        if (isDir)
            remove(filename);
    }
}