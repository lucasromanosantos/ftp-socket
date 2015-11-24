#include "files.c"
#include "dir.c"

void jreceive_file(FILE *fp);

void jsend_ls(char *args) { //
    char *result = malloc(1024);
    unsigned char *buffer;
    strcpy(result, ls(LocalPath,args));
    size_t nob = strlen(result); // nob = number of bytes
    printf("(send_ls) Size of total nob: %d \n", (int) nob);

    Message *m;
    Attr attrs;
    char *aux;
    int sendIndex = 0; // Indice da mensagem que estamos enviando.
    int createIndex = 0; // Indice da mensagem que estamos criando.
    int count = 0; // Recebi um erro. Quantas mensagens tenho que voltar? GO BACK

    m = malloc_msg(MAX_DATA_LEN);
    if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL) {
        puts("(send_ls) Unable to allocate memory");
        return;
    }

    Seq = 0;
    count = 0;

    // I have to initialize first message
    while(nob > 0) {
        printf("\n ======================== \n ");
        printf("seq: %d \n", Seq);
        if(nob >= MAX_DATA_LEN) {
            char tmp[64];
            attrs = prepare_attr(MAX_DATA_LEN, Seq, TYPE_SHOWSCREEN);
            strncpy(tmp, result, MAX_DATA_LEN);
            m = prepare_msg(attrs, tmp);

            send_msg(m);
            Seq += 1; // Increase. Maybe just inside first if?
            result += MAX_DATA_LEN; // Add MAX_DATA_LEN bytes to result pointer
            nob -= MAX_DATA_LEN;
            if((Seq % 4) == 0 && Seq != 0) {
                if(wait_response()) {
                    printf("\tGot 4th message ACK! \n");
                    //send_msg(m);
                    //Seq += 1; // Increase. Maybe just inside first if?
                    //result += MAX_DATA_LEN; // Add MAX_DATA_LEN bytes to result pointer
                    //nob -= MAX_DATA_LEN;
                } else { // Check if there's ACK from client. If not, try to send the entire window again
                    printf("\tFailed to receive 4th message ACK. Going back: i = 0 \n"); 
                    result -= MAX_DATA_LEN * 4; // Go back (4 + 1) times because we add MAX_DATA_LEN again always
                    nob  += MAX_DATA_LEN * 4;
                    Seq -= 4;
                }
            }

            if(receive(aux,&(m),1) != 0) { // Check if I got a message for 1ms. If I did not, continue sending messages.
                if(m->attr.type == TYPE_NACK) { // The first char has the number (Seq) of the message that had an error.
                    // Got a nack. I have to get which message (m) had an error, and send every message since m.
                    int seq_nack = m->attr.seq;
                    int goback = Seq - seq_nack; //number of times I have to go back
                    result -= MAX_DATA_LEN * goback; // Result pointer go back
                    nob += MAX_DATA_LEN * goback;

                    Seq -= goback;
                    printf("\tGot nack from client. Resending the last %d messages. new seq = %d, seq from nack = %d \n", goback, Seq, m->attr.seq);
                } else if(m->attr.type == TYPE_ACK) {
                    puts("FUCK YOU!");
                }
            }
        } else {
            char tmp[nob + 1];
            attrs = prepare_attr(nob + 1, Seq, TYPE_SHOWSCREEN); // (+1 == TEMPORARY)
            //m = malloc_msg(attrs.len);
            strncpy(tmp, result, nob);
            m = prepare_msg(attrs, tmp);

            send_msg(m);
            if(wait_response()) { // Received an ack.
                nob = 0;
                Seq += 1;
            } else { // Go back 3 messages. nob += 63*4, we did not update nob yet.
                nob += MAX_DATA_LEN * 3;
                result -= MAX_DATA_LEN * 3;
                //int goback = Seq - seq_nack; //number of times I have to go back
                //Seq -= seq_nack;
                Seq -= 3;
            }
/*
            if ((Seq % 4) != 0) { // Get window position - is it the 4th one?
                send_msg(m); // It was not. Send message and increment seq.
                    Seq += 1; // Increase. Maybe just inside first if?
            } else { // It was the 4th message, wait for an ack and return.
                if(wait_response()) {
                    send_msg(m);
                    Seq += 1; // Increase. Maybe just inside first if?
                    printf("\t(last message) Got 4th message ACK! \n");
                } else { // Check if there's ACK from client. If not, try to send the entire window again
                    printf("\t(last message) Failed to receive 4th message ACK. Going back: i = 0 \n"); 
                    result -= MAX_DATA_LEN * 3 + nob; // Go back only 4 times (3 max len + last message) because we DONT add MAX_DATA_LEN as before
                    nob  += MAX_DATA_LEN * 3;
                    Seq -= 4;
                }
            }

            if(receive(aux, &(m), 1) != 0) {
                if(m->attr.type == TYPE_NACK) {
                    int seq_nack = m->attr.seq;
                    result -= nob + (goback - 1) * MAX_DATA_LEN; // Result pointer go back
                    nob += nob + (goback - 1) * MAX_DATA_LEN;

                    Seq -= goback;
                    printf("\t(last message) Got nack from client. Resending the last %d messages. new seq = %d, seq from nack = %d \n", goback, Seq, m->attr.seq);
                }
            }
*/
            send_type(TYPE_END); //Send message TYPE_END
            while(wait_response()) {
                send_type(TYPE_END);
                sleep(1); // temporary. sleep 1 second
            }
            nob = 0;
        }
    }
}


void send_ls(char *args) { //
    char *result = malloc(1024);
    unsigned char *buffer;
    if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL) {
        puts("(send_ls) Unable to allocate memory");
        return;
    }
    strcpy(result, ls(LocalPath,args));
    size_t nob = strlen(result); // nob = number of bytes
    printf("(send_ls) Size of total nob: %d \n", (int) nob);

    Message *m;
    m = malloc_msg(MAX_DATA_LEN);
    Attr attrs;
    while(nob > 0) {
        if(nob >= MAX_DATA_LEN) {
            char tmp[64];
            attrs = prepare_attr(MAX_DATA_LEN, Seq, TYPE_SHOWSCREEN);
            strncpy(tmp, result, MAX_DATA_LEN);
            m = prepare_msg(attrs, tmp);
            send_msg(m);
            result += MAX_DATA_LEN; // add MAX_DATA_LEN bytes to result pointer
            nob -= MAX_DATA_LEN;
            if(!wait_response()) {
                send_type(TYPE_ERROR);
                break;
            }
        } else {
            char tmp[nob + 1];
            attrs = prepare_attr(nob + 1, Seq, TYPE_SHOWSCREEN); // (+1 == TEMPORARY)
            m = malloc_msg(attrs.len);
            strncpy(tmp, result, nob);
            m = prepare_msg(attrs, tmp);
            send_msg(m);
            nob = 0;
            if(wait_response()) {
                send_type(TYPE_END);
            } else {
                send_type(TYPE_ERROR);
            }
        }
    }
    if(wait_response()) {
        puts("Ls completed succesfully. Returning..."); // We should delete this message after LS work properly.
    } else {
        send_type(TYPE_ERROR);
    }
}

void operate_server() {
    unsigned char *buffer,*buffer2,*addr,par;
    int res = 0, length;
    Message *m;
    FILE *fp;

    if((buffer2 = malloc(MIN_LEN)) == NULL)
        error("(operate_server) Error allocating memory.");
    if((buffer = malloc(MAX_MSG_LEN)) == NULL)
        error("(operate_server) Error allocating memory.");
    if((addr = malloc(sizeof(unsigned char) * 1024)) == NULL)
        error("(operate_server) Error allocating memory.");
    m = malloc_msg(0);

    while(1) {
        res = receive(buffer2, &m, STD_TIMEOUT);
        if(res == 1) {
            par = get_parity(m);
            print_message(m);
            if((int)par != (int)m->par) {
                send_type(TYPE_NACK);
                printf("\t(operate_server) Nack sent.");
            } else {
                if (m->attr.type == TYPE_LS) { // client requested LS
                    printf("requisitou ls");
                    send_type(TYPE_ACK);
                    printf("Enviou ack pelo ls \n");
                    if(m->attr.len == 0) {
                        m->data[0] = '\0';
                    }
                    puts("\t(operate_server) Received Ls. Ack sent. Sending ls.");
                    //printf("data dentro da mensagem: %s \n", m->data);
                    printf("Enviando LS com janelas. \n");
                    jsend_ls(m->data);
                    puts("\t(operate_server) Ls was succesfull!\n\n");
                } else if (m->attr.type == TYPE_CD) { // client requested CD
                    puts("\t(operate_server) Received Cd. Ack sent.");
                    if(!check_cd(m->data)) {
                        puts("\t(operate_server) Cd path was wrong. Error occured.");
                        send_type(TYPE_ERROR);
                    } else {
                        puts("\t(operate_server) Path updated succesfully.\n\n");
                        send_type(TYPE_ACK);
                    }
                } else if (m->attr.type == TYPE_PUT) { // client request PUT
                    send_type(TYPE_ACK);
                    puts("\t(operate_server) Ready to receive a Put. Ack sent.");
                    strcpy(addr,LocalPath); // Concatenating file name.
                    strcat(addr,m->data);   // Concatenating file name.
                    if((fp = fopen(addr,"w")) == NULL) {
                        puts("Could not create a new file.");
                        exit(1);
                    }
                    jreceive_file(fp);
                    puts("(operate_server) Put was succesfull!\n\n");
                } else if (m->attr.type == TYPE_GET) { // client request GET
                    send_type(TYPE_ACK);
                    addr = strcpy(addr,LocalPath);
                    strcat(addr,m->data);
                    if((fp = fopen(addr,"r")) == NULL) {
                        puts("(operate_server) File does not exist.");
                    } else {
                        length = send_filesize(fp);
                        if(send_file(fp,length) != 1)
                            puts("(operate_server) Could not send file.");
                        puts("(operate_server) Get was succesfull!\n\n");
                    }
                }
            }
        }
    }
}


void jreceive_file(FILE *fp) {
    int res = 0;
    Message *m;
    unsigned char *buf,par;

    buf = malloc(sizeof(unsigned char) * MAX_DATA_LEN);
    m = malloc_msg(MAX_DATA_LEN);

    res = receive(buf, &m, STD_TIMEOUT);
    par = get_parity(m);
    print_message(m);
    while(((int)par != (int)m->par) || (m->attr.type != TYPE_FILESIZE)) {
        printf("Par: %d, mPar=%d, Type=\n",(int)par,(int)m->par);
        puts("(receive_file) Parity error or message was not the file size.");
        send_type(TYPE_NACK);
        res = receive(buf, &m, STD_TIMEOUT);
        par = get_parity(m);
    }

    unsigned int size,i=0,j;
    memcpy(&size,m->data,4);
    send_type(TYPE_ACK);
/*
    while(i < size) {
        res = receive(buf, &m, STD_TIMEOUT);
        par = get_parity(m);
        if((int)par != (int)m->par) {
            puts("(receive_file) Parity error or message.");
            send_type(TYPE_NACK);
        } else {
            for(j=0; j<m->attr.len;) // Write data received in file.
                j += fwrite(m->data + j,sizeof(unsigned char),m->attr.len-j,fp);
            i += m->attr.len;
            send_type(TYPE_ACK);
        }
    }
*/  
    Seq = 0;
    printf("I = %d, size = %d\n",i,size);
    while(i < size) {
        res = receive(buf, &m, STD_TIMEOUT);
        par = get_parity(m);
        print_message(m);
        if((int)par != (int)m->par) {
            puts("(receive_file) Parity error or message.");
            send_type(TYPE_NACK);
        } else {
            for(j=0; j<m->attr.len;) // Write data received in file.
                j += fwrite(m->data + j,sizeof(unsigned char),m->attr.len-j,fp);
            i += m->attr.len;
            //send_type(TYPE_ACK);
            Seq += 1;
            if((Seq % 3) == 2 || i == size) {
                puts("Sending ack...");
                send_type(TYPE_ACK);
            }
        }
    }




    // Read all messages, created and updated the file, I should receive a TYPE_END.
    res = receive(buf, &m, STD_TIMEOUT);
    par = get_parity(m);
    print_message(m);
    if(((int)par != (int)m->par) || (m->attr.type != TYPE_END)) {
        // This should be a while, sending nack and waiting for the right message.
        puts("(receive_file) Parity error or message wasnt an end.");
        return ;
    }
    fclose(fp);
    return ;
}
