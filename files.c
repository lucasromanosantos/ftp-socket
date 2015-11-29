#include "files.h"
#include "utils.h"
#include "message.h"

unsigned int get_file_size(FILE *fp) {
    int sz = 0;
    fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    rewind(fp);
    return sz;
}

unsigned char* read_file(FILE *fp,unsigned int size) {
	// Size is the size of the file (return value from get_file_size(fp)).
	int i,x;
	unsigned char* c = malloc(sizeof(unsigned char) * (size + 1));
	for(i=0; i<size;) {
		i += fread(c+i,sizeof(unsigned char),size - i,fp);
	}
	return c;
}

void write_file(FILE *fp,unsigned char *c,int size) {
	// Size is the size of the file (return value from get_file_size(fp)).
	int i;
	for(i=0; i<size; i++)
		i += fwrite(c,sizeof(unsigned char),size-i,fp);
	return ;
}

int send_filesize(FILE* fp) {
	unsigned int length = get_file_size(fp);
	Message *m;
	Attr a;
	a = prepare_attr(sizeof(unsigned int),Seq,TYPE_FILESIZE);
	unsigned char *s = malloc(5 * sizeof(unsigned char));
	s[4] = '\0';
	//printf("The length is going to be %u",length);
	//s = memcpy(s,&length,4);
	memcpy(s,&length,4);
	m = prepare_msg(a,s);
	send_msg(m);
	while(!wait_response) {
		send_msg(m);
	}
	memcpy(&length,m->data,4); // Deletar depois!
	//printf("M%densagem enviada com sucesso. Tam = %u\n",sizeof(unsigned int),length);
	//Seq = (Seq + 1) % 64; Send_msg increment seq counter
	free(m);
	return length;
}

FILE* open_file(char *args) {
	unsigned char *buffer;
    if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL)
        error("(open_file) Unable to allocate memory.");
    puts("What is the file path?");
    buffer = fgets(buffer, BUF_SIZE, stdin);
    buffer[strlen(buffer)-1] = '\0'; // Removing the \n
    // I have the file name, now I have to open it and return.
    FILE *fp;
    while((fp = fopen(buffer,"r")) == NULL) {
    	printf("Error opening file: %s",strerror(errno));
	    puts("Please, enter another file path.");
	    buffer = fgets(buffer, BUF_SIZE, stdin);
	    buffer[strlen(buffer)-1] = '\0';
    }
    free(buffer);
    return fp;
}