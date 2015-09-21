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
    printf("-- Running on %s mode...\n", argv[1]);

    if(strcmp(argv[1], "client") == 0) {
        unsigned char *buffer;
        if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL)
            error("Unable to allocate memory.");
        while(1) {
            scanf("%s", buffer);
            puts("Sending...");
            Attr attrs = prepare_attr(strlen(buffer),1,TYPE_FILESIZE);
            Message *m;
            m = create_msg(attrs.len + 5);
            *m = prepare_msg(attrs, buffer);
            send_msg(socket, m);
		    // Message sent. Waiting for response.
		    // Recv_tm is a temp function to the timeout of nack / ack. Since soon we'll not
            // have infinite (-1) timeout anymore, it may become the main one
            int i = 0;
			int waiting = 1;
            puts("Waiting for response...");
			
			time_t seconds = 3;
			time_t endwait;
			endwait = time(NULL) + seconds;
			while (time(NULL) < endwait && i != 1) {
				i = recv_tm(socket, buffer, &m, STD_TIMEOUT);
			}
            // This variable will count a timeout. It will make us know when to stop Waiting
            // for a response from the server.
            time_t start = time(NULL);
            if(i == 1) {
                while(waiting) {
                    print_message(m);
                    if(m->attr.type == TYPE_ACK) {
                        puts("\tGot an ack!");
                        puts("Sending another message...");
                        waiting = 0;
                        break;
                        //received = 1;
                    }
                    else if(m->attr.type == TYPE_NACK) {
                         puts("\tGot an nack!");
                         send_msg(socket,m);
                         break;
                    }
                    else {
                        // Looks like we will not get an answer. Std_timeout is in miliseconds, so /1000.
                        /*if(time(NULL) > start + STD_TIMEOUT/1000) {
                            waiting = 0;
                        }*/
                        puts("Panic!");
                    }
                }
            }
            else if(i == 0) {
                puts("\tError occured. Maybe a timeout. Is the server on?");
            }
            // Send next message.
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
                send_ack(socket);
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
