void operate_client(int socket) {
    unsigned char *buffer;
    if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL)
        error("Unable to allocate memory.");
    while(1) {
        buffer = fgets(buffer, BUF_SIZE, stdin);
        buffer[strlen(buffer)-1] = '\0'; // Removing the \n
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
