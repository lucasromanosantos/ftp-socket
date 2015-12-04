#include "header.h"

void operate_client();
int req_ls(char *args);
int req_cd(char *args);
int req_put(char *args);
int req_get(char *args);
Message* wait_data();
int send_file(FILE *fp,int len);
int send_file2(FILE *fp,int len);
int listen_ls();