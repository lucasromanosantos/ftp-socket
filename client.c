void send_file(int socket);

void operate_client(int socket) {
    unsigned char *buffer;
    if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL)
        error("Unable to allocate memory.");
    while(1) {
        /* buffer = fgets(buffer, BUF_SIZE, stdin);
        buffer[strlen(buffer)-1] = '\0'; // Removing the \n
        puts("Sending...");
        Attr attrs = prepare_attr(strlen(buffer),1,TYPE_FILESIZE);
        Message *m;
        m = malloc_msg(attrs.len + 5);
        m = prepare_msg(attrs, buffer);
        send_msg(socket, m);
        // Message sent. Waiting for response.
        puts("Waiting for response...");
        wait_response(socket); */

        int i = 0;
        while(i <= 0 || i >= 5) {
            i = load_interface();
            if(i == 1) {
                while(req_ls(socket) == 0)
                    req_ls(socket);
                listen_ls(socket);
            } else if(i == 2) {
                //req_cd();
            } else if(i == 3) {
                send_file(socket);
            } else if(i == 4) {
                //get_file();
            } else {
                puts("Invalid option. Please, type another number.");
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
    send_msg(socket, m);
    puts("Waiting for ls response..."); // Wait for an ACK
    i = wait_response(socket);

    int seq; // message sequence
    if(i == 1) { // Got an ACK
        // Server will start sending the data.
        //puts("Got an ack.");
        puts("Got an ack!! Now receiving data\n");
        return 1;
    } else if(i == 0) { // Got an NACK
        //puts("Got an nack.");
        return 0;
    } else { // Panic!
        puts("Panic in req_ls!!");
        exit(1);
    }
}

Message* wait_data(int socket) {
    unsigned char *buffer;
    time_t seconds = 3;
    time_t endwait;
    int i;
    Message *m;
    if((buffer = malloc(1024)) == NULL)
        return 0;
    m = malloc_msg(63);
    endwait = time(NULL) + seconds;

    while(time(NULL) < endwait && i != 1)
        i = recv_tm(socket, buffer, &m, STD_TIMEOUT);
    if(i == 1) {
        return m;
    }
    else {
        puts("Error! Timeout? \n");
        m->attr = prepare_attr(0,0,TYPE_ERROR);
        return m;
    }
}

int listen_ls(int socket) {
    Message *m;
    unsigned char *c;
    int size = 0;
    //c = malloc(sizeof(char) * size); // have to start with min length
    c = malloc(MAX_LEN);
    m = malloc_msg(63); // Maximum length
    m = wait_data(socket);
    while (m->attr.type != TYPE_END) {
        if(m->attr.type == TYPE_ERROR) {
            puts("Problem receiving message.");
            send_nack(socket);
        } else if (m->attr.type == TYPE_SHOWSCREEN) {
            size += m->attr.len;
            c = realloc(c,sizeof(char) * size);
            //c[strlen(c)] = strcpy(m->data); ??
            strcat(c, m->data);
            send_ack(socket);
        }
        m = wait_data(socket);
    }
    puts(c);
}

void send_file(int socket) {
    char* buffer = malloc(sizeof(char) * 1024);
    buffer = fgets(buffer, BUF_SIZE, stdin);
    buffer[strlen(buffer)-1] = '\0'; // Removing the \n
    Message *m;
    Attr attrs = prepare_attr(strlen(buffer),0,TYPE_FILESIZE);
    /*if(attrs.len == 0)
        attrs.len = 1;*/
    m = malloc_msg(attrs.len + 5);
    m = prepare_msg(attrs, buffer);
    send_msg(socket,m);
}
