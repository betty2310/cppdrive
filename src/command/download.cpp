#include "download.h"

#include <sys/socket.h>
#include <utils.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "color.h"
#include "common.h"
#include "connect.h"
#include "log.h"
#include "message.h"
#include "reply.h"

int handle_download(int data_sock, int sock_control, char *arg) {
    Message msg;
    recv_message(sock_control, &msg);
    int is_file = msg.type == MSG_TYPE_DOWNLOAD_FILE ? 1 : 0;

    recv_message(sock_control, &msg);
    if (msg.type == MSG_TYPE_ERROR) {
        printf(ANSI_COLOR_RED "%s\n" ANSI_RESET, msg.payload);
        return -1;
    }

    if (!is_file)
        strcat(arg, ".zip");

    char *home = getenv("HOME");
    char *path = (char *) malloc(SIZE);
    strcpy(path, home);
    strcat(path, "/Downloads/");
    strcat(path, arg);

    FILE *fp = fopen(path, "w");
    if (!fp) {
        perror("error opening file\n");
        return -1;
    }

    while (1) {
        recv_message(data_sock, &msg);
        if (msg.type == MSG_TYPE_DOWNLOAD) {
            fwrite(msg.payload, 1, msg.length, fp);
        } else if (msg.type == MSG_TYPE_ERROR) {
            printf(ANSI_COLOR_RED "%s\n" ANSI_RESET, msg.payload);
            break;
        } else if (msg.type == MSG_TYPE_OK) {
            break;
        }
    }
    printf("File downloaded to %s\n", path);
    fclose(fp);
    free(path);
    return 0;
}

void server_download(int sock_control, int sock_data, char *dir) {
    char compress_folder[SIZE];
    int fl = is_folder(dir);
    if (is_folder(dir)) {
        // need to compress folder before sending
        strcpy(compress_folder, dir);
        // use zip
        strcat(compress_folder, ".zip");
        zip(dir, compress_folder);
        strcpy(dir, compress_folder);

        send_message(sock_control, create_status_message(MSG_TYPE_DOWNLOAD_FOLDER, NO));
    } else
        send_message(sock_control, create_status_message(MSG_TYPE_DOWNLOAD_FILE, NO));

    FILE *fp = fopen(dir, "r");

    if (!fp) {
        send_message(sock_control, create_status_message(MSG_TYPE_ERROR, FILE_NOT_FOUND));
        server_log('e', "File not found");
    } else {
        send_message(sock_control, create_status_message(MSG_TYPE_OK, NO));
        server_log('i', "Sending file");
        size_t byte_read;

        Message data;
        do {
            byte_read = fread(data.payload, 1, SIZE, fp);
            data.type = MSG_TYPE_DOWNLOAD;
            data.length = byte_read;
            if (send_message(sock_data, data) < 0)
                perror("error sending file\n");
            memset(data.payload, 0, SIZE);
        } while (byte_read > 0);

        send_message(sock_data, create_status_message(MSG_TYPE_OK, NO));
        server_log('i', "File sent");
        fclose(fp);
        if (fl) {
            remove(compress_folder);
        }
    }
}
