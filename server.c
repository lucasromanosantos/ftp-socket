void send_ls(int socket) {
    char *result = malloc(1024); // temp size.. realloc maybe??
    unsigned char *buffer;
    if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL) {
        puts("unable to allocate memory");
        return;
    }
    get_files(".", result);
    size_t nob = strlen(result); // nob = number of bytes
    puts(result);
    printf("size of total nob: %d \n", (int) nob);
    int seq = 0;

    Message *m; // check allocation / realloc???
    Attr attrs;
    while(nob > 0) {
        if(nob >= 63) {
            char tmp[64];
            attrs = prepare_attr(63, seq, TYPE_LS);
            m = create_msg(attrs.len);
            strncpy(tmp, result, 63);
            printf("test string recortada: %s \n", tmp);
            *m = prepare_msg(attrs, tmp);
            send_msg(socket, m);
            result += 63; // add 63 bytes to result pointer
            nob -= 63;
        }
        else {
            char tmp[nob + 1];
            attrs = prepare_attr(nob, seq, TYPE_LS);
            m = create_msg(attrs.len); // ou nob
            strncpy(tmp, result, nob);
            printf("test string recortada: %s \n", tmp);
            *m = prepare_msg(attrs, tmp);
            send_msg(socket, m);
            //result -= nob;
            nob = 0;
        }
        if(!wait_response(socket)) // fast test 
            break;
        seq += 1;
    }
    printf("final sequence: %d \n", seq);
}

void operate_server(int socket) {
    unsigned char *buffer;
    if((buffer = malloc(MAX_MSG_LEN)) == NULL)
        error("Error allocating memory.");
    Message *m;
    unsigned char par;
    int res = 0;
    while(1) {
        res = receive(socket, buffer, &m);
        if(res == 1) {
            if (m->attr.type == TYPE_LS) { // client request LS
                send_ls(socket);
            }

            printf("Parity received: %d\n",m->par);
            par = get_parity(m);
            if(par != m->par) {
                printf("\tError in parity! Please resend the message!\nSending nack...\n");
                send_nack(socket);
                printf("\tNack sent.");
            }
            else {
                puts("Sending acknowledge...");
                send_ack(socket); // Now with "socket" parameter we missed!
                puts("Ack sent.");
            }
        }
    }
}