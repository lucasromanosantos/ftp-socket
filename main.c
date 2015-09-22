#include "header.h"
#include "message.c"
#include "rawsocket.c"

// About parity:
// It should be a vertical parity, between the 8 bits of each byte.
// It should consider every byte: attr.length, attr.seq, attr.type, and all the data.
// Maybe use a for. We will have to get each one separed by bits. How will we do this?
// Maybe a union?

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
            printf("\tParity test: %d-%c\n", (int)get_parity(m), get_parity(m));
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
        unsigned char par;
        int res = 0;
        while(1) {
            res = receive(socket, buffer, &m);
            if(res == 1) {
                printf("Parity received: %d\n",m->par);
                par = get_parity(m);
                if(par != m->par) {
                    printf("\tError in parity! Please resend the message!\nSending nack...\n");
                    send_nack(socket);
                    printf("\tNack sent.");
                }
                else {
                    puts("Sending acknowledge...");
                    send_ack(socket); // now with "socket" parameter we missed!
                    puts("Ack sent.");
                }
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
    else if(strcmp(argv[1], "ls") == 0) {
        DIR *dp;
        struct dirent *ep;
        char *c = malloc(100);
        scanf("%s",c);
        dp = opendir("./");
        if (dp != NULL) {
                while (ep = readdir (dp))
                    puts (ep->d_name);
                (void) closedir (dp);
        }
        else
          perror ("Couldn't open the directory");

        return 0;
    }
    return 1;
}
