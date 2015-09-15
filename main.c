#include "header.h"
#include "message.c"
#include "rawsocket.c"

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Error: Invalid number of arguments.\nYou should inform if its a client or a server.\n");
        exit(-1);
    }
    int socket;
    socket = ConexaoRawSocket(DEVICE);
    if(socket < 0)
        error("Error opening socket.");
    printf("Running on %s mode...\n", argv[1]);

    if(strcmp(argv[1], "client") == 0) {
        unsigned char *buffer = malloc(sizeof(char) * BUF_SIZE + 1);
        while(1) {
            scanf("%s", buffer);
            puts(buffer);
            Message m;
            Attr attrs;
            attrs.tam = 63;
            attrs.seq = 1;
            attrs.type = TYPE_FILESIZE;
            m = prepare_msg(attrs, buffer);
            send_msg(socket, &m);
        }
    }
    else if(strcmp(argv[1], "server") == 0) {
        unsigned char *buffer = malloc(MAX_MSG_LEN);
        Message *m;
        while(1) {
            m = receive(socket, buffer);
        }
    }
    return 1;
}
