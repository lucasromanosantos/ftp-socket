

void send_string(int socket);

void operate_client(int socket) {
    FILE *fp;
    int i,length,*seq;
    seq = malloc(sizeof(int));
    while(1) {
        i = 0;
        while(i <= 0 || i >= 6) {
            i = load_interface();
            *seq = 0;
            if(i == 1) {
                flush_buf();
                while(req_ls(socket) == 0);
                listen_ls(socket);
            } else if(i == 2) {
                flush_buf();
                //req_cd(socket);
            } else if(i == 3) {
                flush_buf();
                fp = open_file();
                length = send_filesize(socket,fp,seq);
                if(send_file(socket,fp,length,seq) != 1)
                    puts("(operate_client) Could not send file.");
            } else if(i == 4) {
                flush_buf();
                //open_file(socket);
                send_string(socket);
            } else if(i == 5) {
                char *s;
                s = ls("./", " ");
                printf("imprimindo: ... \n");
                printf("%s \n", s);
            }
            else {
                puts("(operate_client) Invalid option. Please, type another number.");
                i = load_interface();
            }
        }
    }
}

int req_ls(int socket) {
    Message *m;
    Attr attrs;
    int i;
    m = malloc_msg(0); // Data is empty
    attrs = prepare_attr(0,0,TYPE_LS);
    m = prepare_msg(attrs, "");
    send_msg(socket,m);
    puts("(req_ls) Waiting for ls response..."); // Wait for an ACK
    //i = wait_response(socket);
    while(!(i = wait_response(socket)))
        send_msg(socket,m);
    if(i == 1) { // Got an ACK
        // Server will start sending the data.
        puts("(req_ls) Got an ack.");
        free(m);
        return 1;
    } else if(i == 0) { // Got an NACK
        puts("(req_ls) Got a nack.");
        free(m);
        return 0;
    } else { // Panic!
        puts("(req_ls) Panic!!");
        free(m);
        exit(1);
    }
}

Message* wait_data(int socket,Message* m) {
    unsigned char *buffer;
    time_t seconds = 3;
    time_t endwait;
    int i;
    //Message *m;
    if((buffer = malloc(1024)) == NULL)
        return 0;
    //m = malloc_msg(63);
    m = realloc(m,68);
    endwait = time(NULL) + seconds;

    while(time(NULL) < endwait && i != 1) {
        i = recv_tm(socket, buffer, &m, STD_TIMEOUT);
    }
    if(i == 1) {
        free(buffer);
        return m;
    }
    else {
        puts("(wait_data) Error! Timeout? \n");
        m->attr = prepare_attr(0,0,TYPE_ERROR);
        free(buffer);
        return m;
    }
}


int listen_ls(int socket) {
    Message *m;
    unsigned char *c;
    int size = 0;
    int i=0;
    c = malloc(size);
    m = malloc_msg(MAX_DATA_LEN);
    m = wait_data(socket, m);
    strcpy(c, ""); // we have to initialize c or the first char will be garbage
    while (m->attr.type != TYPE_END) {
        if(m->attr.type == TYPE_ERROR) {
            puts("(listen_ls) Problem receiving message.");
            send_nack(socket);
        } else if (m->attr.type == TYPE_SHOWSCREEN) {
            size += (int) m->attr.len;
            printf("(listen_ls) Size of message type showscreen: %d \n", size);
            c = realloc(c,sizeof(char) * size);
            strcat(c, m->data);
            send_ack(socket);
        }
        else {
            puts("(listen_ls) Can not handle this message.");
        }
        m = wait_data(socket,m);
    }
    printf("\n========= LS ======== (atualmente com ' de separador)\n");
    puts(c);
    printf("\n");
    free(c);
    //free(m);
    return 1;
}

void send_string(int socket) {
    unsigned char *buffer;
    if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL)
        error("(send_string) Unable to allocate memory.");
    buffer = fgets(buffer, BUF_SIZE, stdin);
    buffer[strlen(buffer)-1] = '\0'; // Removing the \n
    Message *m;
    Attr attrs = prepare_attr(strlen(buffer),1,TYPE_FILESIZE);
    m = malloc_msg(attrs.len + 5);
    m = prepare_msg(attrs, buffer);
    send_msg(socket,m);
    free(buffer);
    free(m);
    return ;
}
