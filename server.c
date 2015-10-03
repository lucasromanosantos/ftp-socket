void send_ls(int socket) {
    char *result = malloc(1024); // temp size.. realloc maybe??
    unsigned char *buffer;
    if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL) {
        puts("(send_ls) Unable to allocate memory");
        return;
    }
    get_files(".", result);
    size_t nob = strlen(result); // nob = number of bytes
    printf("(send_ls) Size of total nob: %d \n", (int) nob);
    int seq = 0;

    Message *m; // check allocation / realloc???
    Attr attrs;
    while(nob > 0) {
        if(nob >= 63) {
            char tmp[64];
            attrs = prepare_attr(63, seq, TYPE_LS);
            m = malloc_msg(attrs.len);
            strncpy(tmp, result, 63);
            m = prepare_msg(attrs, tmp);
            send_msg(socket, m);
            result += 63; // add 63 bytes to result pointer
            nob -= 63;
        }
        else {
            char tmp[nob + 1];
            attrs = prepare_attr(nob, seq, TYPE_LS);
            m = malloc_msg(attrs.len); // ou nob
            strncpy(tmp, result, nob);
            m = prepare_msg(attrs, tmp);
            send_msg(socket, m);
            //result -= nob;
            nob = 0;

            if(wait_response(socket)) {
                attrs = prepare_attr(0, 0, TYPE_END);
                m = malloc_msg(0);
                m = prepare_msg(attrs, "");
                send_msg(socket, m);
            }
        }
        if(!wait_response(socket)) // fast test
            break;
        seq += 1;
    }
    printf("(send_ls) Final sequence: %d \n", seq);
}

void operate_server(int socket) {
    unsigned char *buffer2;
    if((buffer2 = malloc(MIN_LEN)) == NULL)
        error("(operate_server) Error allocating memory."); // alocar menos para ack/nack , não? e dps q receber free()

    unsigned char *buffer;
    if((buffer = malloc(MAX_MSG_LEN)) == NULL)
        error("(operate_server) Error allocating memory.");
    Message *m;
    m = malloc_msg(0);
    unsigned char par;
    int res = 0;
    while(1) {
        res = recv_tm(socket, buffer2, &m, STD_TIMEOUT);
        if(res == 1) {
            par = get_parity(m);
            //printf("Paridade calculada: %d \n", (int) par);
            //printf("Paridade mensagem: %d \n", (int) m->par);
            if((int) par != (int) m->par) {
                //printf("\tError in parity! Please resend the message!\n\tSending nack...\n");
                //send_nack(socket);
                printf("\t(operate_server) Nack sent.");
            }
            else {
                if (m->attr.type == TYPE_LS) { // client request LS
                    send_ack(socket);
                    puts("\t(operate_server) Ack sent.");
                    send_ls(socket);
                    puts("\t(operate_server) Ls sent.");
                }
                //puts("Sending acknowledge...");
                send_ack(socket); // Now with "socket" parameter we missed!
                puts("\t(operate_server) Ack sent.");
            }
        }
    }
}
