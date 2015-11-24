void send_string();

void operate_client() { //
    FILE *fp;
    int i,length,*comm;
    unsigned char *args,*buf,*addr;

    if((comm = malloc(sizeof(int))) == NULL)
        error("(operate_client) Error allocating memory.");
    if((buf = malloc(sizeof(unsigned char) * 1024)) == NULL)
        error("(operate_client) Error allocating memory.");
    if((args = malloc(sizeof(unsigned char) * 1024)) == NULL)
        error("(operate_client) Error allocating memory.");
    if((addr = malloc(sizeof(unsigned char) * 1024)) == NULL)
        error("(operate_client) Error allocating memory.");

    while(1) {
        *comm = 0;
        while(*comm <= 0 || *comm >= 7) {
            args = show_interface(comm,args,buf);
            //printf("Comm: %d - Args: '%s'\n",*comm,args);
            if(*comm == 1) {
                print_ls(ls(LocalPath,args));
            } else if(*comm == 2) {
                check_cd(args);
            } else if(*comm == 3) {
                //puts("(operate_client) Sending ls request.");
                while(req_ls(args) == 0);
                //puts("(operate_client) Listening to ls...");
                printf("LS com janelas. \n");
                jlisten_ls();
                puts("(operate_client) Rls was succesfull!\n\n");
            } else if(*comm == 4) {
                //puts("(operate_client) Sending cd request.");
                while(req_cd(args) == 0);
                puts("(operate_client) Rcd was succesfull!\n\n");
            } else if(*comm == 5) {
                //puts("(operate_client) Sending put request.");
                buf = strcpy(buf,LocalPath);
                buf = strcat(buf,args);
                if((fp = fopen(buf,"r")) == NULL) {
                    puts("(operate_client) File does not exist.");
                } else {
                    while(req_put(args) == 0);
                    length = send_filesize(fp);
                    if(jsend_file(fp,length) != 1)
                        puts("(operate_client) Could not send file.");
                    puts("(operate_client) Put was succesfull!\n\n");
                }
            } else if(*comm == 6) {
                //puts("(operate_client) Sending get request.");
                while(req_get(args) == 0);
                addr = strcpy(addr,LocalPath); // Concatenating file name.
                addr = strcat(addr,args);   // Concatenating file name.
                if((fp = fopen(addr,"w")) == NULL) {
                    puts("(operate_server) Could not create a new file.");
                    exit(1);
                } else {
                    receive_file(fp);
                    puts("(operate_client) Get was succesfull!\n\n");
                }
            } else {
                if(*comm != 7)
                    puts("(operate_client) Invalid option. Please, type another number.");
                args = show_interface(comm,args,buf);
            }
        }
    }
}

int req_ls(char *args) {
    Message *m;
    Attr attrs;
    int i;
    m = malloc_msg(0); // Data is empty
    //printf("(req_ls) argumentos: %s\n", args);
    attrs = prepare_attr(strlen(args),Seq,TYPE_LS);
    m = prepare_msg(attrs,args);
    send_msg(m);
    //puts("(req_ls) Waiting for ls response..."); // Wait for an ACK
    while(!(i = wait_response())) {
        send_msg(m);
    }
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

int req_cd(char *args) {
    Message *m;
    Attr attrs;
    int i,len = strlen(args);
    if(len > 63) {
        puts("Path too long. Try again with 63 or less bytes.");
        return -1;
    }
    m = malloc_msg(len+1); // Data is empty
    attrs = prepare_attr(len,Seq,TYPE_CD);
    m = prepare_msg(attrs,args);
    send_msg(m);
    //puts("(req_cd) Waiting for cd response..."); // Wait for an ACK
    i = 0;
    while((i = wait_response()) == 0)
        send_msg(m);
    if(i == 1) { // Got an ACK
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

int req_put(char *args) {
    Message *m;
    Attr attrs;
    int i,len = strlen(args);
    if(len > 63) {
        puts("Path too long. Try again with 63 or less bytes.");
        return -1;
    }
    m = malloc_msg(len); // Data is empty
    //printf("(req_put) argumentos: %s\n", args);
    attrs = prepare_attr(strlen(args),Seq,TYPE_PUT);
    m = prepare_msg(attrs,args);
    send_msg(m);
    //puts("(req_put) Waiting for put response..."); // Wait for an ACK
    while(!(i = wait_response()))
        send_msg(m);
    if(i == 1) { // Got an ACK
        // Server will start sending the data.
        puts("(req_put) Got an ack.");
        free(m);
        return 1;
    } else if(i == 0) { // Got an NACK
        puts("(req_put) Got a nack.");
        free(m);
        return 0;
    } else { // Panic!
        puts("(req_put) Panic!!");
        free(m);
        exit(1);
    }
}

int req_get(char *args) {
    Message *m;
    Attr attrs;
    int i,len = strlen(args);
    if(len > 63) {
        puts("Path too long. Try again with 63 or less bytes.");
        return -1;
    }
    m = malloc_msg(len);
    //printf("(req_get) argumentos: %s\n", args);
    attrs = prepare_attr(strlen(args),Seq,TYPE_GET);
    m = prepare_msg(attrs,args);
    send_msg(m); // Sending get request
    //puts("(req_get) Waiting for get response..."); // Wait for an ACK
    while(!(i = wait_response())) // Waiting an ACK.
        send_msg(m);
    if(i == 1) { // Got an ACK - Server will send file_size
        puts("(req_get) Got an ack.");
        free(m);
        return 1;
    } else if(i == 0) { // Got an NACK
        puts("(req_get) Got a nack.");
        free(m);
        return 0;
    } else { // Panic!
        puts("(req_get) Panic!!");
        free(m);
        exit(1);
    }
}


Message* wait_data() {
    unsigned char *buffer;
    time_t seconds = 3;
    time_t endwait;
    int i;
    if((buffer = malloc(1024)) == NULL)
        return 0;
    buffer[0] = '\0';
    Message *m;
    m = malloc_msg(MAX_DATA_LEN);
    endwait = time(NULL) + seconds;

    while(time(NULL) < endwait && i != 1 && buffer[0] != 126) {
        i = receive(buffer, &m, STD_TIMEOUT);
    }
    if(i == 1) {
        free(buffer);
        //Seq = (Seq + 1) % 64;
        return m;
    }
    else {
        puts("(wait_data) Error! Timeout? \n");
        m->attr = prepare_attr(0,0,TYPE_ERROR);
        free(buffer);
        return m;
    }
}


int jlisten_ls() {
    Message *m;
    unsigned char *c;
    int size = 0;
    Seq = 0;
    c = malloc(10000); // !!!!!!

    m = malloc_msg(MAX_DATA_LEN);
    m = wait_data();
    printf(" VER VER VER Primeira mensagem::: \n"); // temp
    print_message(m);
    Seq += 1;
    c[0] = '\0'; // Initializing an empty string.

    while (m->attr.type != TYPE_END) {
        printf("Seq atual: %d \n", Seq);
        if(m->attr.type == TYPE_ERROR) {
            puts("\t(listen_ls) Problem receiving message. Type_nack sent");
            send_type(TYPE_NACK);
            Seq -= 1;
        } else if (m->attr.type == TYPE_SHOWSCREEN) {
           // if(m->attr.seq >= Seq) {
                size += (int) m->attr.len;
                //printf("(listen_ls) Size of message type showscreen: %d \n", size);
                c = realloc(c,sizeof(char) * (size + 1));
                strncat(c, m->data, m->attr.len);

                if ( (Seq % 4) == 0 && Seq != 0) {
                    printf("\t Sending ACK after 4 messages (full window) \n");
                    send_type(TYPE_ACK);
                }
           // } else {
           //     printf("=> Got message with old sequence. Probaly because of previous nack\n");
           // }
        }
        else {
            printf("(listen_ls) Can not handle this message.");
        }
        free(m); // m will be allocated again in wait_data. - Might bug something.
        m = wait_data();
        print_message(m);
        Seq += 1;
    }
    printf("\tGot type_end. Sending ack");
    send_type(TYPE_ACK); // Sending an ack to TYPE_END message.
    print_ls(c);
    free(c);
    free(m);
    return 1;
}

int jsend_file(FILE *fp,int len) {
    int nob = 0,i;
    unsigned char *c;
    Message *m;
    Attr a;

    m = malloc_msg(MAX_DATA_LEN);
    if((c = malloc(sizeof(char) * 64)) == NULL)
        error("(send_file) Unable to allocate memory.");

/*
    while(len > MAX_DATA_LEN) { // Send n messages until the remaining data is less than 63 bytes (until I need only one message).
        len -= MAX_DATA_LEN;
        nob = 0;
        while(nob < MAX_DATA_LEN)
            nob += fread(c + nob,1,MAX_DATA_LEN-nob,fp);
        a = prepare_attr(MAX_DATA_LEN,Seq,TYPE_PUT);
        m = prepare_msg(a,c);
        send_msg(m);
        //Seq = (Seq + 1) % 64; Send_msg increment seq counter
    }
    nob = 0;
    while(nob < len)
        nob += fread(c + nob,1,MAX_DATA_LEN - nob,fp);
    a = prepare_attr(len,Seq,TYPE_PUT);
    c[len] = '\0'; // Not correctly tested, but this might be a bug in other functions! Watch out!!
    m = prepare_msg(a,c);
    send_msg(m);
    while(!wait_response) {
        send_msg(m);
    }
*/
    Seq = 0;
    while(len > 0) { // Send n messages until the remaining data is less than 63 bytes (until I need only one message).
        if(len > MAX_DATA_LEN) {
            len -= MAX_DATA_LEN;
            nob = 0;
            while(nob < MAX_DATA_LEN)
                nob += fread(c + nob,1,MAX_DATA_LEN-nob,fp);
            a = prepare_attr(MAX_DATA_LEN,Seq,TYPE_PUT);
            m = prepare_msg(a,c);
            send_msg(m);
            if((Seq % 3) == 2) { // I sent 4 messages. Were they ok?
                if(wait_response()) { // Yes! I got an ack.
                    Seq += 1;
                    puts("Got ack. 4 Messages were sent succesfully");
                } else {
                    Seq -= 3;
                    fseek(fp, -1 * (MAX_DATA_LEN * 3), SEEK_CUR);
                    len += MAX_DATA_LEN * 3;
                }
            }
            len -= MAX_DATA_LEN;
        } else {
            nob = 0;
            while(nob < len)
                nob += fread(c + nob,1,MAX_DATA_LEN - nob,fp);
            a = prepare_attr(len,Seq,TYPE_PUT);
            c[len] = '\0'; // Not correctly tested, but this might be a bug in other functions! Watch out!!
            m = prepare_msg(a,c);
            send_msg(m);
           
            if(wait_response()) {
                puts("Got last message ack. 4 Messages were sent succesfully");
            } else {
                Seq -= 3;
                fseek(fp, -1 * (MAX_DATA_LEN * (Seq - m->attr.seq) + len) , SEEK_CUR);
                len += MAX_DATA_LEN * (Seq - m->attr.seq) + len;
            }
           
        }
    }

    //Seq = (Seq + 1) % 64; Send_msg increment seq counter
    unsigned char s[0];
    a = prepare_attr(0,Seq,TYPE_END);
    m = prepare_msg(a,s);
    send_msg(m);
    while(!wait_response) {
        send_msg(m);
    }
    //Seq = (Seq + 1) % 64; Send_msg increment seq counter
    free(c);
    free(m);
    return 1;
}

int listen_ls() {
    Message *m;
    unsigned char *c;
    int size = 0;
    int i=0;
    c = malloc(MAX_DATA_LEN * sizeof(unsigned char) + 1);
    //c = malloc(10000);
    m = malloc_msg(MAX_DATA_LEN);
    m = wait_data(m);
    print_message(m);
    c[0] = '\0'; // Initializing an empty string.
    while (m->attr.type != TYPE_END) {
        if(m->attr.type == TYPE_ERROR) {
            puts("(listen_ls) Problem receiving message.");
            send_type(TYPE_NACK);
        } else if (m->attr.type == TYPE_SHOWSCREEN) {
            size += (int) m->attr.len;
            printf("(listen_ls) Size of message type showscreen: %d \n", size);
            c = realloc(c,sizeof(char) * (size + 1));
            strncat(c, m->data, m->attr.len);
            send_type(TYPE_ACK);
        }
        else {
            //puts("(listen_ls) Can not handle this message.");
        }
        //free(m); // m will be allocated again in wait_data. - Might bug something.
        m = wait_data();
        print_message(m);
    }
    send_type(TYPE_ACK); // Sending an ack to TYPE_END message.
    print_ls(c);
    free(c);
    free(m);
    return 1;
}


/*
void send_string() {
    unsigned char *buffer;
    if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL)
        error("(send_string) Unable to allocate memory.");
    buffer = fgets(buffer, BUF_SIZE, stdin);
    buffer[strlen(buffer)-1] = '\0'; // Removing the \n
    Message *m;
    Attr attrs = prepare_attr(strlen(buffer),Seq,TYPE_FILESIZE);
    m = malloc_msg(attrs.len + 5);
    m = prepare_msg(attrs, buffer);
    send_msg(m);
    free(buffer);
    free(m);
    return ;
}
*/
