#include "header.h"

void flush_buf();
int pot(int base, int exp);
unsigned char get_parity(Message *m);
void error(const char *msg);
size_t strlen2(const char *p);
Attr prepare_attr(int length,int seq,int type);
unsigned char* show_interface(int *comm,char *arg,char *buffer);