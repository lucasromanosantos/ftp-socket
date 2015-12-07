#include "header.h"

unsigned int get_file_size(FILE *fp);
unsigned char* read_file(FILE *fp, unsigned int size);
void write_file(FILE *fp, unsigned char *c, int size);
int send_filesize(FILE* fp);
FILE* open_file(char *args);
void receive_file2(FILE *fp);
int send_file2(FILE *fp,int len);