#include "header.h"
#include "utils.h"
#include "message.h"
#include "rawsocket.h"
#include "dir.h"
#include "files.h"

void receive_file(FILE *fp);
void send_ls(char *args);
void operate_server();