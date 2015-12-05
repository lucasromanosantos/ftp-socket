#include "header.h"

unsigned int get_file_size(FILE *fp);
char* to_vector(char **c, int n);
char** to_matrix(char *c, int n);
void print_matrix(char** c,int n);
int count_slashes(char* c, int len);
int check_cd(char* c);
unsigned char* read_file(FILE *fp,unsigned int size);
void write_file(FILE *fp,unsigned char *c,int size);
int send_file(FILE *fp,int len);
int send_filesize(FILE* fp);
FILE* open_file(char *args);
void receive_file(FILE *fp);
char* ls(char* path, char* args);
void print_ls(char* data);