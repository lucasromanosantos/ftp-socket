void send_string(int socket);

void operate_client(int socket) {
    FILE *fp;
    int i,length,*comm;
    //int *seq;
    unsigned char *args;
    //char *data;
    //seq = malloc(sizeof(int));
    comm = malloc(sizeof(int));
    args = malloc(sizeof(unsigned char) * 1024);
    while(1) {
        *comm = 0;
        while(*comm <= 0 || *comm >= 7) {
            //i = load_interface();
            //args[0] = '\0';
            args = show_interface(comm,args);
            printf("Comm: %d - Args: '%s'\n",*comm,args);
            //*seq = 0;
            if(*comm == 1) {
                // Deve executar ls local. Nao implementado.
                print_ls(ls(LocalPath,args));
            } else if(*comm == 2) {
                // Deve executar cd local. Nao implementado.
                /*puts("(operate_client) Sending cd request.");
                req_cd(socket,args);*/
            } else if(*comm == 3) {
                puts("(operate_client) Sending ls request.");
                while(req_ls(socket) == 0);
                    //printf("(operate_client) Not able to send a LS request.");
                puts("(operate_client) Listening to ls...");
                listen_ls(socket);
            } else if(*comm == 4) {
                puts("(operate_client) Sending cd request.");
                req_cd(socket,args);
            } else if(*comm == 5) {
                puts("(operate_client) Sending put request.");
                fp = open_file();
                //length = send_filesize(socket,fp,seq);
                //if(send_file(socket,fp,length,seq) != 1)
                length = send_filesize(socket,fp,&Seq); // Maybe I should change the parameters in the function, not here.
                if(send_file(socket,fp,length,&Seq) != 1) // Maybe I should change the parameters in the function, not here.
                    puts("(operate_client) Could not send file.");
            } else if(*comm == 6) {
                puts("(operate_client) Sending get request.");
                //open_file(socket);
                send_string(socket);
            } else if(*comm == 7) {
                char *s;
                s = ls("./", " ");
                printf("imprimindo: ... \n");
                printf("%s \n", s);
            } else {
                puts("(operate_client) Invalid option. Please, type another number.");
                args = show_interface(comm,args);
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

int req_cd(int socket,char *args) {
    Message *m;
    Attr attrs;
    int i,len = strlen(args);
    if(len > 63) {
        puts("Path too long. Try again with 63 or less bytes.");
        return -1;
    }
    m = malloc_msg(len+1); // Data is empty
    attrs = prepare_attr(0,len,TYPE_LS);
    m = prepare_msg(attrs,args);
    send_msg(socket,m);
    puts("(req_cd) Waiting for ls response..."); // Wait for an ACK
    i = 0;
    while((i = wait_response(socket)) == 0)
        send_msg(socket,m);
    if(i == 1) { // Got an ACK
        // Server will update the path.
        puts("(req_cd) Got an ack.");
        free(m);
        return 1;
    } else if(i == 0) { // Got an NACK
        puts("(req_cd) Got a nack.");
        free(m);
        return 0;
    } else if(i == -1) { // Got an ERROR
        puts("(req_cd) Error requesting CD.");
        free(m);
        return 0;
    } else { // Panic!
        puts("(req_cd) Panic!!");
        free(m);
        exit(1);
    }
}

Message* wait_data(int socket,Message* m) {
    unsigned char *buffer;
    time_t seconds = 3;
    time_t endwait;
    int i;
    Message *m2;
    if((buffer = malloc(1024)) == NULL)
        return 0;
    buffer[0] = '\0';
    m2 = malloc_msg(63);
    //m = realloc(m,68);
    endwait = time(NULL) + seconds;

    while(time(NULL) < endwait && i != 1 && buffer[0] != 126) {
    //while(time(NULL) < endwait && i != 1) {
        i = recv_tm(socket, buffer, &m2, STD_TIMEOUT);
    }
    if(i == 1) {
        free(buffer);
        return m2;
    }
    else {
        puts("(wait_data) Error! Timeout? \n");
        m2->attr = prepare_attr(0,0,TYPE_ERROR);
        free(buffer);
        return m2;
    }
}


int listen_ls(int socket) {
    Message *m;
    unsigned char *c;
    int size = 0;
    int i=0;
    //c = malloc(size); This might be the problem. We changed size to 0. Dont know why, but..
    c = malloc(MAX_DATA_LEN * sizeof(unsigned char) + 1);
    m = malloc_msg(MAX_DATA_LEN);
    m = wait_data(socket, m);
    print_message(m);
    //strcpy(c, ""); // we have to initialize c or the first char will be garbage
    c[0] = '\0'; // Better way to initialize an empty string.
    while (m->attr.type != TYPE_END) {
        if(m->attr.type == TYPE_ERROR) {
            puts("(listen_ls) Problem receiving message.");
            send_nack(socket);
        } else if (m->attr.type == TYPE_SHOWSCREEN) {
            printf("recebeu 1 msg\n");
            size += (int) m->attr.len;
            printf("(listen_ls) Size of message type showscreen: %d \n", size);
            c = realloc(c,sizeof(char) * size);
            strcat(c, m->data);
            send_ack(socket);
        }
        else {
            //puts("(listen_ls) Can not handle this message.");
        }
        free(m); // m will be allocated again in wait_data. - Might bug something.
        m = wait_data(socket,m);
    }
    printf("\n========= LS ======== (atualmente com ' de separador)\n");
    //puts(c);
    //printf("\n");
    for(i=0; i<size; i++) {
        if(c[i] == '\'') {
            printf("   ");
        }
        else
            printf("%c",c[i]);
    }
    printf("\n");
    free(c);
    free(m);
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
