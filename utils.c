#include <math.h>

void flush_buf();
int get_files(char *path, char *c);
int pot(int base, int exp);
unsigned char get_parity(Message *m);
void error(const char *msg);
size_t strlen2(const char *p);
Attr prepare_attr(int length,int seq,int type);
int load_interface();
int count_slash(unsigned char* c, int len);
unsigned char* return_dir(unsigned char *c, int length);
//void send_string(int socket);
char* ls_la(char* param);

void flush_buf() {
    // Removes a remaining \n from stdin (in case we do a scanf("%d"))
    char c;
    while((c = getchar()) != '\n');
    return ;
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
		puts("(get_files) Error! Could not open the directory");
		return 0;
	}
}

char* ls(char* path, char* args) { // generic ls
    //Receives a path and arguments as param and returns a char* with the data requested
    //it has \n separator for every file
    DIR *dir;
    struct dirent *file;
    char *res;
    char this[1024]; // Aux variable

    res = malloc(sizeof(char));
    strcpy(res, "");

    dir = opendir(path);
    if(dir == NULL) {
        printf("(ls_la) Error opening directory: %s\n", strerror(errno));
        return strerror(errno);
    }

    //if((dir[strlen(dir) -1]) != '/') { // To correct directory name //BEFORE OR AFTER
    //    strcat(dir, "/");
    //}

    if(args == "" || args == "a") {       // No arguments or -a
        int total_length = 1;
        strcpy(this, "");            // Start the buffer
        while(file = readdir(dir)) {
            strcpy(this, "");
            if (args == "") {
                if(file->d_name[0] != '.' && 
                    file->d_name[strlen(file->d_name) - 1] != '~') { 
                    // Eliminating hidden files and files ending in "~"
                    strcat(this, file->d_name);
                    strcat(this, "\n");
                }
            }
            else {
                strcat(this, file->d_name);
                strcat(this, "\n");
            }
            total_length += strlen(this);
            printf("total length: %d\n", total_length);
            if((res = realloc(res,total_length)) == NULL) {
                printf("(ls_la) Unable to allocate memory.");
                exit(-1);
            }
            strcat(res,this);
        }
    }

    else if(args == "l" || args == "la" || args == "al") {
        char *fileName, aux[64], timebuf[64];
        struct stat fileStat;
        struct tm lt;
        struct passwd *pwd;
        struct group *grp;

        fileName = malloc(sizeof(char) * 1024); // max size of filename

        int total_length = 1;

        while((file = readdir(dir)) != NULL) {
            if ((args == "l" && (file->d_name[0] != '.' && file->d_name[strlen(file->d_name) - 1] != '~'))
                    || args == "la" || args == "al") { // Eliminating hidden files and files ending in "~")
                strcpy(fileName, path);
                strcat(fileName,file->d_name);
                printf("(ls) filename = %s \n", fileName);
                if(stat(fileName, &fileStat) != 0) {
                    printf("(ls) Erro na syscall stat!\n");
                    return;
                }
                strcpy(this, ""); // reseting

                // Permissions
                strcat(this,(S_ISDIR(fileStat.st_mode)) ? "d" : "-");
                strcat(this,(fileStat.st_mode & S_IRUSR) ? "r" : "-");
                strcat(this,(fileStat.st_mode & S_IWUSR) ? "w" : "-");
                strcat(this,(fileStat.st_mode & S_IXUSR) ? "x" : "-");
                strcat(this,(fileStat.st_mode & S_IRGRP) ? "r" : "-");
                strcat(this,(fileStat.st_mode & S_IWGRP) ? "w" : "-");
                strcat(this,(fileStat.st_mode & S_IXGRP) ? "x" : "-");
                strcat(this,(fileStat.st_mode & S_IROTH) ? "r" : "-");
                strcat(this,(fileStat.st_mode & S_IWOTH) ? "w" : "-");
                strcat(this,(fileStat.st_mode & S_IXOTH) ? "x | " : "-\'");

                // Number of Hardlinks
                sprintf(aux, "%d", (int)fileStat.st_nlink);
                strcat(this,aux);
                strcat(this,"\'");

                // File Owner
                pwd = getpwuid(fileStat.st_uid);
                if(pwd != 0) {
                    strcat(this,pwd->pw_name);
                    strcat(this,"\'");
                }

                // File Group
                grp = getgrgid(fileStat.st_gid);
                if(grp != 0) {
                    strcat(this,grp->gr_name);
                    strcat(this,"\'");
                }

                // File Size
                sprintf(aux, "%d",(int)fileStat.st_size);
                strcat(this,aux);
                strcat(this,"\'");

                // Modified Date
                time_t t = fileStat.st_mtime;
                localtime_r(&t, &lt);
                strftime(timebuf, sizeof(timebuf), "%c", &lt);
                strcpy(aux,timebuf+4);
                strcat(this,aux);
                strcat(this,"\'");

                // File Name
                strcat(this,file->d_name);
                strcat(this,"\n");

                // Concatenate into response
                total_length += strlen(this);
                printf("total length: %d\n", total_length);
                if((res = realloc(res,total_length)) == NULL) {
                    printf("(ls_la) Unable to allocate memory.");
                    exit(-1);
                }
                strcat(res,this);
            }
        }
    }

    else {
        printf("Invalid arguments \n");
        return;
    }

    closedir(dir);
    return res;
}

char* ls_la(char* param) {
    // Receives a path as param and returns a char* with all the ls -la data.
    // Ps: It has a \n separator for every file.
    char *fileName,aux[64],timebuf[64],this[1024],*res;
    int totalLength = 1;
    DIR *dir;
    struct dirent *file;
    struct stat fileStat;
    struct tm lt;
    struct passwd *pwd;
    struct group *grp;

    fileName = malloc(sizeof(char) * 1024);
    res = malloc(sizeof(char));

    if((dir = opendir(param)) == NULL) {
        printf("(ls_la) Error opening directory: %s\n",strerror(errno));
        return strerror(errno);
    }

    if((param[strlen(param) -1]) != '/') { // To correct directory name
        strcat(param,"/");
    }

    while((file = readdir(dir)) != NULL)
    {
        strcpy(fileName,param);
        strcat(fileName,file->d_name);
        if(stat(fileName, &fileStat) != 0)
            printf("(ls_la) Erro na syscall stat!\n");
        this[0] = '\0';
        // Permissions
        strcat(this,(S_ISDIR(fileStat.st_mode)) ? "d" : "-");
        strcat(this,(fileStat.st_mode & S_IRUSR) ? "r" : "-");
        strcat(this,(fileStat.st_mode & S_IWUSR) ? "w" : "-");
        strcat(this,(fileStat.st_mode & S_IXUSR) ? "x" : "-");
        strcat(this,(fileStat.st_mode & S_IRGRP) ? "r" : "-");
        strcat(this,(fileStat.st_mode & S_IWGRP) ? "w" : "-");
        strcat(this,(fileStat.st_mode & S_IXGRP) ? "x" : "-");
        strcat(this,(fileStat.st_mode & S_IROTH) ? "r" : "-");
        strcat(this,(fileStat.st_mode & S_IWOTH) ? "w" : "-");
        strcat(this,(fileStat.st_mode & S_IXOTH) ? "x | " : "- | ");

        // Number of Hardlinks
        sprintf(aux, "%d", (int)fileStat.st_nlink);
        strcat(this,aux);
        strcat(this," | ");

        // File Owner
        pwd = getpwuid(fileStat.st_uid);
        if(pwd != 0) {
            strcat(this,pwd->pw_name);
            strcat(this," | ");
        }

        // File Group
        grp = getgrgid(fileStat.st_gid);
        if(grp != 0) {
            strcat(this,grp->gr_name);
            strcat(this," | ");
        }

        // File Size
        sprintf(aux, "%d",(int)fileStat.st_size);
        strcat(this,aux);
        strcat(this," | ");

        // Modified Date
        time_t t = fileStat.st_mtime;
        localtime_r(&t, &lt);
        strftime(timebuf, sizeof(timebuf), "%c", &lt);
        strcpy(aux,timebuf+4);
        strcat(this,aux);
        strcat(this," | ");

        // File Name
        strcat(this,file->d_name);
        strcat(this,"\n");

        // Concatenate into response
        totalLength += strlen(this);
        if((res = realloc(res,totalLength)) == NULL) {
            printf("(ls_la) Unable to allocate memory.");
            exit(-1);
        }
        strcat(res,this);
    }
    closedir(dir);
    return res;
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
    size_t result = 2; // Two first bytes of nack are 0000 0000
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
    printf("\t5- ls -la (temp test).\n");
    scanf("%d", &i);
    return i;
}

int count_slash(unsigned char* c, int len) {
    int i,count=0;
    for(i=0; i<len; ++i) {
        if(c[i] == '/') {
            count++;
        }
    }
    return count;
}

unsigned char* return_dir(unsigned char *c, int length) {
    // We should call this if the client send a "cd .."
    int count = count_slash(c,length);
    if(count == 0)
        return c = strcat(c,"../");
    // If we got here, we have a dir in cd. Ex: ./bin/
    int i;
    unsigned char *aux = malloc(sizeof(char) * 3);
    count = 0;
    for(i=length-2; c[i] != '/'; i--) { // Checking if the dir is something like: ./../
        count++;
        if(c[i] != '.') {
            break;
        }
    }
    if(count == 2) {
        return c = strcat(c,"../");
    }
    // If it was "./bin/", we go back one directory -> "./"
    // Length -2 because c[length] == '\0' and c[length-1] == '/' (.../bin'/')
    for(i=length-2; c[i]!='/'; i--) {
        c[i] = '\0';
    }
    return c;
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
