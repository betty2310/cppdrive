#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "status.h"

#define PAYLOAD_SIZE 1024

typedef enum {
    MSG_TYPE_OK,
    MSG_TYPE_ERROR,
    MSG_TYPE_AUTHEN,
    MSG_TYPE_REGISTER,
    MSG_TYPE_LS,
} MessageType;

struct Message_ {
    MessageType type;
    int length;
    char payload[PAYLOAD_SIZE];
};

typedef struct Message_ Message;

/**
 * Message constructor
 * @param type message type
 * @param payload message payload
 * @return message
 */
Message create_message(MessageType type, char* payload);

/**
 * Create message with status
 * @param type message type
 * @param status message status
 * @return message
 */
Message create_status_message(MessageType type, Status status);

/**
 * copy message from temp to mess
 * @param mess pointer to message
 * @param temp message to copy
 * @return 0 if success, -1 if error
 */
int messsagecpy(Message* mess, Message temp);

void print_message(Message mess);

#endif   // !
