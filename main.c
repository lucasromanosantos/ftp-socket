#include "header.h"
#include "message.c"
#include "rawsocket.c"
#include "server.c"
#include "client.c"

void operate_ls(int socket) {
	send_ls_data(socket);
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
    char buffer[1024]; // Total buffer and argument
    char com[4], *arg; // Command and arguments
    int i = 0;

    fgets(buffer,1024,stdin);
    buffer[strlen(buffer) - 1] = '\0';
    *comm = 0; // Error default

    arg = malloc(sizeof(unsigned char) * 1024);
    arg[0] = '\0';

    printf("buffer: %s \n", buffer);
    while(i < 3 && buffer[i] != ' ' && buffer[i] != '\0') {
        com[i] = buffer[i];
        i++;
    }
    com[i] = '\0';
    printf("printf ls: %s\n", com);

    if(strcmp(com, "ls") == 0) {
        *comm = 1;
    } else if(strcmp(com, "cd") == 0) {
        *comm = 2;
    } else if(strcmp(com, "put") == 0) {
        *comm = 3;
    } else if(strcmp(com, "get") == 0) {
        *comm = 4;
    }

    if(*comm == 1 && buffer[2] != '\n') {
        printf("Entrou if 2\n");
        for(i = 4; buffer[i] != '\n'; i++) {
            if(i == 7) {
                *comm = 0;
                printf("entrou i == 7\n");
                return "";
            }
            arg[i-4] = buffer[i];
        }
        return arg;
    } else if(*comm > 1) {
        // Read path
        for(i=4; buffer[i] != '\0'; i++)
            arg[i-4] = buffer[i];
        return arg;
    }

    printf("com: %s \n", com);
    printf("arg: %s \n", arg);
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
    printf("-- Running on %s mode...\n", argv[1]);

    if(strcmp(argv[1], "client") == 0) {
        operate_client(socket);
    }
    else if(strcmp(argv[1], "server") == 0) {
        operate_server(socket);
    }
    else if(strcmp(argv[1], "alone") == 0) {
        operate_alone(socket);
    }
    else if(strcmp(argv[1], "ls") == 0) {
        operate_ls(socket);
    }
    else if(strcmp(argv[1], "teste") == 0) {
        int *x = malloc(sizeof(int));
        operate_test(socket,x);
    }
    return 1;
}
