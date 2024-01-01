#include <libgen.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utils.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "color.h"
#include "command.h"
#include "common.h"
#include "connect.h"
#include "log.h"
#include "message.h"
#include "validate.h"

int handle_download(int data_sock, int sock_control, char *arg) {
    Message msg;
    recv_message(sock_control, &msg);
    int is_file = msg.type == MSG_TYPE_DOWNLOAD_FILE ? 1 : 0;
    std::string str_log_message = is_file ? "Downloading file " : "Downloading folder ";
    std::string str_arg(arg);
    str_log_message += str_arg;
    log_message('i', str_log_message.c_str());

    recv_message(sock_control, &msg);
    if (msg.type == MSG_TYPE_ERROR) {
        printf(ANSI_COLOR_RED "%s\n" ANSI_RESET, msg.payload);
        std::string str_payload(msg.payload);
        log_message('e', str_payload.c_str());
        return -1;
    }

    if (!is_file)
        strcat(arg, ".zip");

    char *home = getenv("HOME");
    char *path = (char *) malloc(SIZE);
    strcpy(path, home);
    strcat(path, "/Downloads/");

    char *last_past = basename(arg);
    strcat(path, last_past);
    std::string str_path(path);

    FILE *fp = fopen(path, "w");
    if (!fp) {
        perror("ERROR opening file: ");
        str_log_message = "ERROR opening file: ";
        str_log_message += str_path;
        log_message('e', str_log_message.c_str());
        return -1;
    }

    while (1) {
        recv_message(data_sock, &msg);
        if (msg.type == MSG_TYPE_DOWNLOAD) {
            fwrite(msg.payload, 1, msg.length, fp);
        } else if (msg.type == MSG_TYPE_ERROR) {
            printf(ANSI_COLOR_RED "%s\n" ANSI_RESET, msg.payload);
            std::string str_payload(msg.payload);
            log_message('e', str_payload.c_str());
            break;
        } else if (msg.type == MSG_TYPE_OK) {
            break;
        }
    }
    printf("File downloaded to %s\n", path);
    str_log_message = "File downloaded to ";
    str_log_message += str_path;
    log_message('i', str_log_message.c_str());
    fclose(fp);
    free(path);
    return 0;
}

int handle_pipe_download(int sockfd, std::string files) {
    std::vector<std::string> tokens = split(files, '\n');

    printf("Downloading %d files...\n", (int) tokens.size());
    std::string str_log_message = "Downloading ";
    str_log_message += std::to_string(tokens.size());
    str_log_message += " files";
    log_message('i', str_log_message.c_str());
    for (const auto &l : tokens) {
        handle_download(sockfd, sockfd, (char *) l.c_str());
    }
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

    std::string str_dir(dir);
    FILE *fp = fopen(dir, "r");

    if (!fp) {
        send_message(sock_control, create_status_message(MSG_TYPE_ERROR, FILE_NOT_FOUND));
        std::string str_log_message = str_dir + " not found";
        server_log('e', str_log_message.c_str());
    } else {
        send_message(sock_control, create_status_message(MSG_TYPE_OK, NO));
        std::string str_log_message = "Sending file " + str_dir;
        server_log('i', str_log_message.c_str());
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
        str_log_message = "File " + str_dir + " sent";
        server_log('i', str_log_message.c_str());
        fclose(fp);
        if (fl) {
            remove(compress_folder);
        }
    }
}

void server_pipe_download(int sockfd, char *output) {
    std::vector<std::string> tokens = split(output, '\n');

    std::string str_log_message = "Downloading ";
    str_log_message += std::to_string(tokens.size());
    str_log_message += " files";
    server_log('i', str_log_message.c_str());
    // Print the lines or process them as needed
    for (const auto &l : tokens) {
        server_download(sockfd, sockfd, (char *) l.c_str());
    }
    str_log_message = "Downloaded ";
    str_log_message += std::to_string(tokens.size());
    str_log_message += " files";
    server_log('i', str_log_message.c_str());
}
