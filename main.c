#include "header.h"
#include "message.c"
#include "rawsocket.c"
#include "server.c"
#include "client.c"

void operate_ls(int socket) {
	//send_ls_data(socket);
    printf("This is deprecated.");
}

void operate_alone(int socket) {
    unsigned char *buffer;
    if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL)
        error("(operate_alone) Unable to allocate memory.");
    while(1) {
        buffer = fgets(buffer, BUF_SIZE, stdin);
        buffer[strlen(buffer)-1] = '\0'; // Removing the \n
        Message *m;
        Attr attrs = prepare_attr(strlen(buffer),1,TYPE_FILESIZE);
        m = malloc_msg(attrs.len + 5);
        m = prepare_msg(attrs, buffer);
        buffer = msg_to_str(m);
        m = str_to_msg(buffer);
        print_message(m);
    }
}

unsigned char* operate_test(int socket,int *comm) {
    // Watch out! *comm has to come already allocated.
    char buffer[1024]; // Total buffer and argument
    char com[4], *arg; // Command and arguments
    int i = 0;

    fgets(buffer,1024,stdin);
    buffer[strlen(buffer) - 1] = '\0';

    arg = malloc(sizeof(unsigned char) * 1024);
    arg[0] = '\0';

    //printf("buffer: %s \n", buffer);
    while(i < 3 && buffer[i] != ' ' && buffer[i] != '\0') {
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
    } else {
        *comm = 0;
        printf("%s",com);
        puts(": command not found.");
        return "";
    }

    printf("Command is: %d\n",*comm);

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

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Error: Invalid number of arguments.\nYou should inform if its a client or a server.\n");
        exit(-1);
    }
    int socket;
    socket = ConexaoRawSocket(DEVICE);
    if(socket < 0)
        error("Error opening socket.");
    system("clear");
    printf("-- Running on %s mode...\n", argv[1]);

    register struct passwd *pw;
    register uid_t uid;

    uid = geteuid ();
    pw = getpwuid (uid);
    User = malloc(sizeof(char) * strlen(pw->pw_name) + 1);
    strcpy(User,pw->pw_name);
    Seq = 0;

    if((LocalPath = malloc(sizeof(unsigned char) * 1024)) == NULL)
        error("(main) Error allocating memory.");
    if((RemPath = malloc(sizeof(unsigned char) * 1024)) == NULL)
        error("(main) Error allocating memory.");

    LocalPath[0] = '.';
    LocalPath[1] = '/';
    LocalPath[2] = '\0';
    RemPath[0] = '.';
    RemPath[1] = '/';
    RemPath[2] = '\0';

    if(strcmp(argv[1], "client") == 0) {
        IsClient = 1;
        operate_client(socket);
    }
    else if(strcmp(argv[1], "server") == 0) {
        IsClient = 0;
        operate_server(socket);
    }
    else if(strcmp(argv[1], "alone") == 0) {
        IsClient = 0;
        operate_alone(socket);
    }
    else if(strcmp(argv[1], "ls") == 0) {
        IsClient = 0;
        operate_ls(socket);
    }
    else if(strcmp(argv[1], "teste") == 0) {
        IsClient = 0;
        int *x = malloc(sizeof(int));
        unsigned char *c = malloc(sizeof(unsigned char) * 32);
        c = operate_test(socket,x);
        puts(c);
    }
    return 1;
}
