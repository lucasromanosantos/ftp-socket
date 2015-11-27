#include "header.h"

unsigned int get_file_size(FILE *fp);
unsigned char* read_file(FILE *fp,unsigned int size);
void write_file(FILE *fp,unsigned char *c,int size);
int send_file(FILE *fp,int len);
int send_filesize(FILE* fp);
FILE* open_file(char *args);
void receive_file(FILE *fp);
char* ls(char* path, char* args);
void print_ls(char* data);