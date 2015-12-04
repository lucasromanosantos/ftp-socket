#include "header.h"

int print_message(Message *m);
Message* malloc_msg(int length);
int msg_length(Message *m);
char* msg_to_str(Message *m);
Message* str_to_msg(char* c);
Message* prepare_msg(Attr attr, unsigned char *data);
void send_type(int type);
void send_data_type(int type, int seq);
int send_msg(Message *m);
int receive(unsigned char *data, Message **m, int timeout);
int wait_response();
