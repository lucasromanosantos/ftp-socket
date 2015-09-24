#include "header.h"
#include "message.c"
#include "rawsocket.c"
#include "server.c"
#include "client.c"

void operate_ls(int socket) {
	char *result = malloc(1024);
	get_files(".", result);
	printf("test strlen: %d \n \n", (int)strlen(result));
	printf("LIST OF FILES: %s \n", result);
}

void operate_alone(int socket) {
    unsigned char *buffer;
    if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL)
        error("Unable to allocate memory.");
    while(1) {
        buffer = fgets(buffer, BUF_SIZE, stdin);
        buffer[strlen(buffer)-1] = '\0'; // Removing the \n
        Message *m;
        Attr attrs = prepare_attr(strlen(buffer),1,TYPE_FILESIZE);
        m = create_msg(attrs.len + 5);
        *m = prepare_msg(attrs, buffer);
        buffer = msg_to_str(m);
        m = str_to_msg(buffer);
        print_message(m);
    }
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
    return 1;
}
