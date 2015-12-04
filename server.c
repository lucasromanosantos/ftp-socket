#include "server.h"

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
    char tmp[64];
    while(nob > 0) {
        //printf("nob atual: %d \n", (int)nob);
        if(nob > MAX_DATA_LEN) {
            attrs = prepare_attr(MAX_DATA_LEN, Seq, TYPE_SHOWSCREEN);
            strncpy(tmp, result, MAX_DATA_LEN);
            m = prepare_msg(attrs, tmp);
            send_msg(m);
            if((Seq % 3) == 2) { // I sent 4 messages. Were they OK?
                if(!wait_response()) { // Got an nack.
                    Seq -= 3;
                    result -= MAX_DATA_LEN * 4; // 4 cause i add after
                    nob += MAX_DATA_LEN * 4; // cause I subtract one after
                }
            }
            result += MAX_DATA_LEN; // add MAX_DATA_LEN bytes to result pointer
            nob -= MAX_DATA_LEN;
        } else {
            attrs = prepare_attr(nob + 1, Seq, TYPE_SHOWSCREEN); // (+1 == TEMPORARY)
            strncpy(tmp, result, nob);
            tmp[nob] = '\0';
            printf("\nULTIMA STRING SENDO ENVIADA: %s \n", tmp);
            m = prepare_msg(attrs, tmp);
            send_msg(m);

            if(wait_response()) {
                //send_type(TYPE_END);
                nob = 0;
            } else {
                //send_type(TYPE_ERROR);
                Seq -= 3;
                result -= MAX_DATA_LEN * (Seq - m->attr.seq);
                nob += MAX_DATA_LEN * (Seq - m->attr.seq);
            }
        }
    }

    unsigned char s[0];
    attrs = prepare_attr(0,Seq,TYPE_END);
    m = prepare_msg(attrs,s);
    send_msg(m);
    while(!wait_response()) {
        send_msg(m);
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
                    send_ls(m->data);
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
                        printf("Could not create a new file. Error was: %s\n",strerror(errno));
                        exit(1);
                    }
                    receive_file2(fp);
                    puts("(operate_server) Put was succesfull!\n\n");
                } else if (m->attr.type == TYPE_GET) { // client request GET
                    send_type(TYPE_ACK);
                    addr = strcpy(addr,LocalPath);
                    strcat(addr,m->data);
                    if((fp = fopen(addr,"r")) == NULL) {
                        printf("(operate_server) File does not exist. Error was: %s\n",strerror(errno));
                    } else {
                        length = send_filesize(fp);
                        if(send_file2(fp,length) != 1)
                            puts("(operate_server) Could not send file.");
                        puts("(operate_server) Get was succesfull!\n\n");
                    }
                }
            }
        }
    }
}

void receive_file(FILE *fp) {
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

    Seq = 0;
    printf("I = %d, size = %d\n",i,size);
    while(i < size) {
        res = receive(buf, &m, STD_TIMEOUT);
        par = get_parity(m);
        print_message(m);
        printf("I == %d, Size = %d\n",i,size);
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

    puts("Going to read the End Message.");
    Message *m2 = malloc_msg(MAX_DATA_LEN);
    // Read all messages, created and updated the file, I should receive a TYPE_END.
    while((receive(buf, &m2, STD_TIMEOUT)) == 0); // Got a message.
    while(((int)par != (int)m2->par) || (m2->attr.type != TYPE_END)) {
        puts("(receive_file) Parity error or message wasnt an end.");
        print_message(m2);
        send_type(TYPE_NACK);
        while((receive(buf, &m2, STD_TIMEOUT)) == 0);
    }
    /*while((receive(buf, &m2, STD_TIMEOUT)) == 0);
    par = get_parity(m2);
    print_message(m2);
    if(((int)par != (int)m2->par) || (m2->attr.type != TYPE_END)) {
        // This should be a while, sending nack and waiting for the right message.
        puts("(receive_file) Parity error or message wasnt an end.");
        print_message(m2);
        return ;
    }*/
    send_type(TYPE_ACK);
    fclose(fp);
    return ;
}

///////////////////////////////// Versao com repeticao seletiva

void receive_file2(FILE *fp) {
    int res = 0, i, j, size, mp = 0; // mp is message pointer - it counts which message we are sending/receiving.
    Message *m;
    unsigned char *buf,par;

    buf = malloc(sizeof(unsigned char) * MAX_DATA_LEN);
    /*m = malloc(sizeof(Message *) * 3);
    for(i=0; i<3; i++)
        m[i] = malloc_msg(MAX_DATA_LEN);*/
    m = malloc_msg(MAX_DATA_LEN);

    res = receive(buf, &(m), STD_TIMEOUT);
    par = get_parity(m);
    print_message(m);

    while(((int)par != (int)m->par) || (m->attr.type != TYPE_FILESIZE)) {
        printf("Par: %d, mPar=%d, Type=\n",(int)par,(int)m->par);
        puts("(receive_file) Parity error or message was not the file size.");
        send_type(TYPE_NACK);
        res = receive(buf, &m, STD_TIMEOUT);
        par = get_parity(m);
    }

    memcpy(&size,m->data,4);
    send_type(TYPE_ACK);

    i = 0;
    Seq = 0;
    //printf("I = %d, size = %d\n",i,size);
    while(i < size) {
        res = receive(buf, &m, STD_TIMEOUT);
        par = get_parity(m);
        print_message(m);
        printf("I == %d, Size = %d\n",i,size);
        if((int)par != (int)m->par) {
            puts("(receive_file) Parity error or message.");
            send_data_type(TYPE_NACK, m->attr.seq);
        } else {
            for(j=0; j<m->attr.len;) // Write data received in file.
                j += fwrite(m->data + j,sizeof(unsigned char),m->attr.len-j,fp);
            i += m->attr.len;
            //send_type(TYPE_ACK);
            Seq += 1;
            if((Seq % 3) == 2 || i == size) {
                puts("Sending ack...");
                send_data_type(TYPE_ACK, Seq);
            }
        }
    }

    puts("Going to read the End Message.");
    Message *m2 = malloc_msg(MAX_DATA_LEN);
    // Read all messages, created and updated the file, I should receive a TYPE_END.
    while((receive(buf, &m2, STD_TIMEOUT)) == 0); // Got a message.
    while(((int)par != (int)m2->par) || (m2->attr.type != TYPE_END)) {
        puts("(receive_file) Parity error or message wasnt an end.");
        print_message(m2);
        send_type(TYPE_NACK);
        while((receive(buf, &m2, STD_TIMEOUT)) == 0);
    }
    /*while((receive(buf, &m2, STD_TIMEOUT)) == 0);
    par = get_parity(m2);
    print_message(m2);
    if(((int)par != (int)m2->par) || (m2->attr.type != TYPE_END)) {
        // This should be a while, sending nack and waiting for the right message.
        puts("(receive_file) Parity error or message wasnt an end.");
        print_message(m2);
        return ;
    }*/
    send_type(TYPE_ACK);
    fclose(fp);
    return ;
}