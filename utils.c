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

void flush_buf() {
    // Removes a remaining \n from stdin (in case we do a scanf("%d"))
    char c;
    while((c = getchar()) != '\n');
    return ;
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
    flush_buf();
    return i;
}

unsigned char* show_interface(int *comm) {
    // Watch out! *comm has to come already allocated.
    char buffer[1024]; // Total buffer and argument
    char com[6], *arg; // Command and arguments
    int i = 0;

    if(CLIENT)
        printf("%s@client:%s$ ",USER,ADDR);
    else
        printf("%s@server:%s$ ",USER,ADDR);

    fgets(buffer,1024,stdin);
    buffer[strlen(buffer) - 1] = '\0';

    arg = malloc(sizeof(unsigned char) * 1024);
    arg[0] = '\0';

    //printf("buffer: %s \n", buffer);
    while(i < 5 && buffer[i] != ' ' && buffer[i] != '\0') {
        com[i] = buffer[i];
        i++;
    }
    com[i] = '\0';
    //printf("printf ls: %s\n", com);

    if(strcmp(com, "ls") == 0) {
        *comm = 1;
    } else if(strcmp(com, "cd") == 0) {
        *comm = 2;
    } else if(strcmp(com, "put") == 0) {
        *comm = 3;
    } else if(strcmp(com, "get") == 0) {
        *comm = 4;
    } else if(strcmp(com, "exit") == 0) {
        exit(0);
    } else if(strcmp(com, "clear") == 0) {
        system("clear");
        *comm = 0;
        return "";
    } else if(strcmp(com, "help") == 0) {
        *comm = 0;
        puts("Available commands:");
        puts("\tclear");
        puts("\tls (options: -l, -a, -la)");
        puts("\tcd <path>");
        puts("\tput <path>");
        puts("\tget <path>");
        puts("\texit");
        return "";
    } else {
        *comm = 0;
        printf("%s",com);
        puts(": command not found.");
        return "";
    }

    if(*comm == 1 && buffer[2] != '\0') {
        // We got an LS. And it has some parameters! Time to check them.
        for(i = 4; buffer[i] != '\0'; i++) {
        // We initialize i as 4 because we want to ignore the - (ls -la, or ls -l).
            if(i == 7) {
                // Ls can only have 2 arguments (max), so, if it has 3, its an unknown command.
                *comm = 0;
                puts("Ls can't have more than 2 arguments.");
                return "";
            }
            arg[i-4] = buffer[i];
        }
        return arg;
    } else {
        // Cd, put and get have a path as parameter. Time to read it!
        int x = (*comm == 2) ? 3 : 4;
        // This inline if is to dont read a space in put and get (they have 1 more digit than cd).
        for(i=x; buffer[i] != '\0'; i++)
            arg[i-x] = buffer[i];
        return arg;
    }

    return arg;
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