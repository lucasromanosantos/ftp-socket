unsigned int get_file_size(FILE *fp);
int send_file(int socket,FILE *fp,int len,int *seq);
int send_filesize(int socket,FILE *fp,int *seq);
FILE* get_file();

unsigned int get_file_size(FILE *fp) {
    int sz = 0;
    //fp = fopen("./abla","r");
    fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    return sz;
}

int send_file(int socket,FILE *fp,int len,int *seq) {
	int nob = 0,i;
	unsigned char *c;
	Message *m;
	Attr a;

	m = malloc_msg(MAX_DATA_LEN);
    if((c = malloc(sizeof(char) * 64)) == NULL)
        error("Unable to allocate memory.");

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

FILE* get_file() {
	unsigned char *buffer;
    if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL)
        error("Unable to allocate memory.");
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