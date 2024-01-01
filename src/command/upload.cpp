#include "upload.h"

#include <libgen.h>
#include <sys/socket.h>
#include <utils.h>

#include <cstdio>
#include <cstring>

#include "color.h"
#include "common.h"
#include "connect.h"
#include "log.h"
#include "message.h"

char *handle_path(char *dir) {
    char *home = getenv("HOME");
    char *path = (char *) malloc(SIZE);
    strcpy(path, home);
    strcat(path, "/");
    if (dir[0] == '~' || dir[0] == '.') {
        dir += 2;
    }
    strcat(path, dir);
    strcpy(dir, path);
    free(path);
    return dir;
}

void handle_upload(int sock_data, char *dir, int sock_control) {
    dir = handle_path(dir);
    char compress_folder[SIZE];
    int fl = is_folder(dir);

    printf("Uploading file %s to server!\n", dir);
    if (is_folder(dir)) {
        strcpy(compress_folder, dir);
        char *last_past = basename(compress_folder);
        strcat(compress_folder, last_past);
        strcat(compress_folder, ".zip");
        zip(dir, compress_folder);
        strcpy(dir, compress_folder);

        send_message(sock_control, create_status_message(MSG_TYPE_DOWNLOAD_FOLDER, NO));
    } else
        send_message(sock_control, create_status_message(MSG_TYPE_DOWNLOAD_FILE, NO));

    FILE *fp = fopen(dir, "r");
    if (fp == NULL) {
        perror("error opening file\n");
        return;
    }

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

int server_upload(int sock_control, int data_sock, char *arg, char *cur_dir) {
    Message msg;
    recv_message(sock_control, &msg);
    int is_file = msg.type == MSG_TYPE_DOWNLOAD_FILE ? 1 : 0;

    recv_message(sock_control, &msg);
    if (msg.type == MSG_TYPE_ERROR) {
        printf(ANSI_COLOR_RED "%s" ANSI_RESET "\n", msg.payload);
        return -1;
    }

    if (!is_file)
        strcat(arg, ".zip");

    char *last_past = basename(arg);
    char *path = (char *) malloc(SIZE);
    strcpy(path, cur_dir);
    strcat(path, "/");
    strcat(path, last_past);
    printf("%s\n", path);

    FILE *fp = fopen(path, "w");
    if (!fp) {
        perror(ANSI_COLOR_RED "ERROR opening file" ANSI_RESET);
        return -1;
    }

    while (1) {
        recv_message(data_sock, &msg);
        if (msg.type == MSG_TYPE_DOWNLOAD) {
            fwrite(msg.payload, 1, msg.length, fp);
        } else if (msg.type == MSG_TYPE_ERROR) {
            printf(ANSI_COLOR_RED "%s" ANSI_RESET "\n", msg.payload);
            break;
        } else if (msg.type == MSG_TYPE_OK) {
            break;
        }
    }
    char *log_msg = (char *) malloc(SIZE);
    if (!is_file) {
        sprintf(log_msg, "Folder uploaded to %s", path);
    } else {
        sprintf(log_msg, "File uploaded to %s", path);
    }
    server_log('i', log_msg);
    printf("%s\n", log_msg);
    free(log_msg);
    fclose(fp);
    if (!is_file) {
        char *extracted_path = (char *) malloc(SIZE);
        strncpy(extracted_path, path, strlen(path) - 4);
        extracted_path[strlen(path) - 4] = '\0';
        unzip(path, extracted_path);
        remove(path);
    }
    return 0;
}
