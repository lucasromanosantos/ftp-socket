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
        unsigned char *buffer;
        if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL)
            error("Unable to allocate memory.");
        while(1) {
            scanf("%s", buffer);
            Attr attrs = prepare_attr(strlen(buffer),1,TYPE_FILESIZE);
            Message *m;
            m = create_msg(attrs.len + 5);
            *m = prepare_msg(attrs, buffer);
            send_msg(socket, m);
        }
    }
    else if(strcmp(argv[1], "server") == 0) {
        unsigned char *buffer;
        if((buffer = malloc(MAX_MSG_LEN)) == NULL)
            error("Error allocating memory.");
        Message *m;
        int res = 0;
        while(1) {
            res = receive(socket, buffer, m);
            if(res == 1) {
                puts("Sending acknowledge...");
                send_ack();
                puts("Ack sent.");
            }
        }
    }
    else if(strcmp(argv[1], "alone") == 0) {
        unsigned char *buffer;
        if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL)
            error("Unable to allocate memory.");
        while(1) {
            scanf("%s", buffer);
            Message *m;
            Attr attrs = prepare_attr(strlen(buffer),1,TYPE_FILESIZE);
            m = create_msg(attrs.len + 5);
            *m = prepare_msg(attrs, buffer);
            buffer = msg_to_str(m);
            m = str_to_msg(buffer);
            print_message(m);
        }
    }
    return 1;
}
