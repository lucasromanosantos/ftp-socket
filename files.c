unsigned int get_file_size(FILE *fp);
int send_file(int socket,FILE *fp,int len,int *seq);
int send_filesize(int socket,FILE *fp,int *seq);
FILE* open_file();
unsigned char* read_file(FILE *fp,unsigned int size);
void write_file(FILE *fp,unsigned char *c,int size);
void receive_file(int socket,FILE *fp);

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
/*
int main() {
	FILE* fp;
	if(!(fp = fopen("./main2.c","r"))) {
		printf("Couldnt open file.\n");
		exit(1);
	}
	unsigned int size = get_file_size(fp);
	unsigned char* c = read_file(fp,size);
	fclose(fp);
	if(!(fp = fopen("copy.c","w"))) {
		printf("Couldnt open file.\n");
		exit(1);
	}
	write_file(fp,c,size);
	fclose(fp);
	return 0;
}
*/
int send_file(int socket,FILE *fp,int len,int *seq) {
	int nob = 0,i;
	unsigned char *c;
	Message *m;
	Attr a;

	m = malloc_msg(MAX_DATA_LEN);
    if((c = malloc(sizeof(char) * 64)) == NULL)
        error("(send_file) Unable to allocate memory.");

	while(len > MAX_DATA_LEN) { // Send n messages until the remaining data is less than 63 bytes (until I need only one message).
		len -= MAX_DATA_LEN;
		nob = 0;
		while(nob < MAX_DATA_LEN)
			nob += fread(c + nob,1,MAX_DATA_LEN-nob,fp);
		a = prepare_attr(MAX_DATA_LEN,*seq,TYPE_PUT);
		m = prepare_msg(a,c);
		send_msg(socket,m);
		while(!wait_response) { // If we enter this while we got an nack, so, resend the message.
			send_msg(socket,m);
		}
		(*seq)++;
	}
	while(nob < len)
		nob += fread(c + nob,1,MAX_DATA_LEN - nob,fp);
	a = prepare_attr(len,*seq,TYPE_PUT);
	m = prepare_msg(a,c);
	send_msg(socket,m);
	while(!wait_response) {
		send_msg(socket,m);
	}
	(*seq)++;
	unsigned char s[0];
	a = prepare_attr(0,*seq,TYPE_END);
	m = prepare_msg(a,s);
	send_msg(socket,m);
	while(!wait_response) {
		send_msg(socket,m);
	}
	free(c);
	free(m);
	return 1;
}

int send_filesize(int socket,FILE* fp,int *seq) {
	unsigned int length = get_file_size(fp);
	Message *m;
	Attr a;
	m = malloc_msg(sizeof(unsigned int));
	a = prepare_attr(sizeof(unsigned int),*seq,TYPE_FILESIZE);
	unsigned char s[5];
	memcpy(s,&length,sizeof(unsigned int));
	m = prepare_msg(a,s);
	send_msg(socket,m);
	while(!wait_response) {
		send_msg(socket,m);
	}
	(*seq)++;
	free(m);
	return length;
}

FILE* open_file() {
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

void receive_file(int socket,FILE *fp) {
	int res = 0;
	Message *m;
	unsigned char *buf,par;

	buf = malloc(sizeof(unsigned char) * MAX_DATA_LEN);
	m = malloc_msg(MAX_DATA_LEN);

    res = receive(socket, buf, &m, STD_TIMEOUT);
	par = get_parity(m);
	if(((int)par != (int)m->par) || (m->attr.type != TYPE_FILESIZE)) {
		// This should be a while, sending nack and waiting for the right message.
		puts("(receive_file) Parity error or message wasnt the file size.");
		return ;
	}
	int size,i=0,j;
	memcpy(&size,m->data,4);
	while(i < size) {
	    res = receive(socket, buf, &m, STD_TIMEOUT);
		par = get_parity(m);
		if((int)par != (int)m->par) {
			puts("(receive_file) Parity error or message.");
			send_nack(socket);
		} else {
			for(j=0; j<m->attr.len;) // Write data received in file.
				j += fwrite(m->data + j,sizeof(unsigned char),m->attr.len-j,fp);
			i += m->attr.len;
			send_ack(socket);
		}
	}
	// Read all messages, created and updated the file, I should receive a TYPE_END.
	res = receive(socket, buf, &m, STD_TIMEOUT);
	par = get_parity(m);
	if(((int)par != (int)m->par) || (m->attr.type != TYPE_END)) {
		// This should be a while, sending nack and waiting for the right message.
		puts("(receive_file) Parity error or message wasnt an end.");
		return ;
	}
	fclose(fp);
	return ;
}