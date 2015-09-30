#include <math.h>

void flush_buf() {
    // Removes a remaining \n (in case we do a scanf("%d"))
    char c;
    while((c = getchar()) != '\n');
    return ;
}

int print_message(Message *m) {
    printf("\tMsg-> Init: %u | Len: %d | Seq: %d | Type: %d | Msg: '%s' | Par: %d \n", m->init, m->attr.len, m->attr.seq, m->attr.type, m->data, m->par);
    return 1;
}

int get_files(char *path, char *c) {
	DIR *dp;
	struct dirent *ep;
	dp = opendir(path);
	if(dp != NULL) {
		c = strcpy(c, ""); // Starting the buffer with something to use strcat.
		while(ep = readdir(dp)) {
            strcat(c, ep->d_name);
            strcat(c, "\'");
		}
		(void) closedir(dp);
		return 1;
	}
	else {
		puts("Error! Could not open the directory");
		return 0;
	}
}

int pot(int base, int exp) {
    if(exp < 0)
        return 0;
    if(exp == 0)
        return 1;
    if(exp == 1)
        return base;
    return base * pot(base, exp-1);
}

unsigned char get_parity(Message *m) {
    int i;
    unsigned char res = 0,c[2];
    memcpy(c,&m->attr,2); // c will have m->attr data so we can look to this struct as 2 chars.
    res = c[0] ^ c[1];
    for(i=0; i < (int)m->attr.len; i++) {
        res = res ^ m->data[i];
    }
    return res;
}

void error(const char *msg) {
    puts(msg);
    perror(msg);
    exit(1);
}

size_t strlen2(const char *p) {
    size_t result = 2; // two first bytes of nack are 0000 0000
    while(p[result] != '\0') ++result;
    return result;
}

Attr prepare_attr(int length,int seq,int type) {
    Attr a;
    a.len = length;
    a.seq = seq;
    a.type = type;
    return a;
}

int load_interface() {
    int i;
    printf("What would you like to do ?\n");
    printf("\t1- Remote ls.\n");
    printf("\t2- Remote cd.\n");
    printf("\t3- Send file.\n");
    printf("\t4- Get file.\n");
    scanf("%d", &i);
    return i;
}

/* Expected Parity:
Init does not count.
Length, sequency and type: (Using values 1, 1 and 9)
0000 0100 - len + 2 bits seq
0001 1001 - 4 bits seq + type
0010 1001 // Character (in ASCII) from the message
---------
0011 0100

What I am doing with my function:
  0000 01
  0000 01
     1001
0010 1001
---------
0010 0001
*/
