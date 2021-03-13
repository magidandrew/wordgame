#ifndef _CHAT_MESSAGE_H_
#define _CHAT_MESSAGE_H_
#include "globals.h"

struct chat_message {
    char username[MAX_USERNAME_LENGTH];                      //ex. 1024
    char message[MESSAGE_BUFFER_SIZE - MAX_USERNAME_LENGTH - sizeof(short)]; //ex. 994-2=992    
    char color[sizeof(short)];
};

#endif
