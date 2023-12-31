#ifndef __UPLOAD_H__
#define __UPLOAD_H__

void handle_upload(int data_sock, char *filename, int sock_control);
int server_upload(int sock_control, int sock_data, char *filename, char *user_dir);

#endif   // !
