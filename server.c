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
    if(Log == 1)
        printf("(send_ls) Size of total nob: %d \n", (int) nob);

    Message *m;
    m = malloc_msg(MAX_DATA_LEN);
    Attr attrs;
    unsigned char tmp[64];
    while(nob > 0) {
        if(nob > MAX_DATA_LEN) {
            attrs = prepare_attr(MAX_DATA_LEN, Seq, TYPE_SHOWSCREEN);
            strncpy((char*)tmp, result, MAX_DATA_LEN);
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
            strncpy((char*)tmp, result, nob);
            tmp[nob] = '\0';
            printf("\nULTIMA STRING SENDO ENVIADA: %s \n", tmp);
            m = prepare_msg(attrs, tmp);
            send_msg(m);

            if(wait_response()) {
                nob = 0;
            } else {
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
    char *addr;
    unsigned char *buffer2, par;
    int i, res = 0, length;
    Message *m;
    FILE *fp;

    if((buffer2 = malloc(MIN_LEN)) == NULL)
        error("(operate_server) Error allocating memory.");
    if((addr = malloc(sizeof(char) * 1024)) == NULL)
        error("(operate_server) Error allocating memory.");
    m = malloc_msg(0);

    while(1) {
        for(i = 0; i < MIN_LEN; i++)
            buffer2[i] = '\0';
        for(i = 0; i < 1024; i++)
            addr[i] = '\0';
        res = receive(buffer2, &m, STD_TIMEOUT);
        if(res == 1) {
            par = get_parity(m);
            if(Log == 1)
                print_message(m);
            if((int)par != (int)m->par) {
                send_type(TYPE_NACK);
                printf("\t(operate_server) Nack sent. (%d vs %d) ",(int)par, (int)m->par);
            } else {
                if (m->attr.type == TYPE_LS) { // client requested LS
                    send_type(TYPE_ACK);
                    if(m->attr.len == 0) {
                        m->data[0] = '\0';
                    }
                    puts("\t(operate_server) Received Ls request. Ack sent. Sending ls.");
                    send_ls((char*)m->data);
                    puts("\t(operate_server) Ls was succesfull!");
                } else if (m->attr.type == TYPE_CD) { // client requested CD
                    puts("\t(operate_server) Received Cd. Ack sent.");
                    if(!check_cd((char*)m->data)) {
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
                    strcat(addr,(char*)m->data);   // Concatenating file name.
                    if((fp = fopen(addr,"w")) == NULL) {
                        printf("Could not create a new file. Error was: %s\n",strerror(errno));
                        exit(1);
                    }
                    receive_file(fp);
                    puts("(operate_server) Put was succesfull!\n\n");
                } else if (m->attr.type == TYPE_GET) { // client request GET
                    send_type(TYPE_ACK);
                    strcpy(addr,LocalPath);
                    strcat(addr,(char*)m->data);
                    if((fp = fopen(addr,"r")) == NULL) {
                        printf("(operate_server) File does not exist. Error was: %s\n",strerror(errno));
                    } else {
                        length = send_filesize(fp);
                        if(send_file(fp,length) != 1)
                            puts("(operate_server) Could not send file.");
                        else
                            puts("(operate_server) Get was succesfull!\n\n");
                    }
                }
            }
        }
    }
}