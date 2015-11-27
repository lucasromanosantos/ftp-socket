#include "header.h"

void operate_client();
int req_ls(char *args);
int req_cd(char *args);
int req_put(char *args);
int req_get(char *args);
Message* wait_data();
int jsend_file(FILE *fp,int len);
int listen_ls();