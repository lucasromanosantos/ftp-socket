unsigned int get_file_size(FILE *fp);
int send_file(int socket,FILE *fp,int len,int *seq);
int send_filesize(int socket,FILE *fp,int *seq);
FILE* get_file();
unsigned char* read_file(FILE *fp,unsigned int size);
void write_file(FILE *fp,unsigned char *c,int size);

unsigned int get_file_size(FILE *fp) {
    int sz = 0;
    fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    rewind(fp);
    return sz;
}

unsigned char* read_file(FILE *fp,unsigned int size) {
	int i,x;
	unsigned char* c = malloc(sizeof(unsigned char) * (size + 1));
	for(i=0; i<size;) {
		i += fread(c+i,1,1,fp);
	}
	return c;
}

void write_file(FILE *fp,unsigned char *c,int size) {
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
	puts("Success?");
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

FILE* get_file() {
	unsigned char *buffer;
    if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL)
        error("(get_file) Unable to allocate memory.");
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

//------------
// Path stuff.

int count_slashes(unsigned char* c, int len) {
    int i,count=0;
    for(i=0; i<len; ++i) {
        if(c[i] == '/') {
            count++;
        }
    }
    return count;
}

void print_matrix(unsigned char** c,int n) {
    int i;
    for(i=0; i<n; i++) {
        printf("\t");
        puts(c[i]);
    }
}

unsigned char** to_matrix(unsigned char *c, int n) {
    int i,j=0,k;
    unsigned char **tmp = malloc(sizeof(unsigned char*) * n);
    for(i=0; i<n; i++) {
        tmp[i] = malloc(sizeof(unsigned char*) * FILE_LEN);
    }
    //       Yes, its j, not i.
    for(i=0,k=0; j<n; i++,k++) {
        if(c[i] == '/') {
            k=-1; // It will be incremented to 0 in the end of the loop.
            j++;
        }
        else {
            tmp[j][k] = c[i];
        }
    }
    return tmp;
}

unsigned char* to_vector(unsigned char **c, int n) {
    int i,size = 1024;
    unsigned char *tmp = malloc(sizeof(unsigned char) * size);
    tmp[0] = '\0';
    for(i=0; i<n; i++) {
        if(strlen(tmp) + strlen(c[i]) > size) { // To prevent overflows.
            size = size * 2;
            tmp = realloc(tmp,sizeof(unsigned char) * size);
        }
        strcat(tmp,c[i]);
        strcat(tmp,"/");
    }
    return tmp;
}

unsigned char* fix_dir(unsigned char *c, int length) {
    int i,j,n = count_slashes(c,length);
    unsigned char **tmp;
    tmp = to_matrix(c,n);
    // Now I have a matrix from the path, with n lines and unknown number of columns.
    for(i=1; i<n; i++) { // If tmp[0] == ../, theres nothing I can do with it.
        if(strcmp(tmp[i],"..") == 0 && strcmp(tmp[i-1],"..") != 0) {
            for(j=i; j<n; j++) {
                tmp[j-1] = tmp[j];
            }
            n--;
            for(j=i; j<n; j++) {
                tmp[j-1] = tmp[j];
            }
            n--;
            i = 1; // So a/b/c/../../ will remove b and c and both ../, not only c.
        }
    }
    c = to_vector(tmp,n);
    return c;
}
/*
int main() {
    unsigned char *addr = malloc(sizeof(char) * 1024);
    int i,n;
    scanf("%d\n",&n);
    for(i=0; i<n; i++) {
        scanf("%s",addr);
        addr = fix_dir(addr,strlen(addr));
        printf("Addr: ");
        puts(addr);
    }
}
*/
