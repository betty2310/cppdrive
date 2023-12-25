#ifndef __REPLY_H__
#define __REPLY_H__

void print_reply(int rc);

int read_reply(int sock_control);
int ftclient_send_cmd(struct command *cmd, int sock_control);
int ftclient_read_command(char *user_input, int size, struct command *cstruct);
#endif