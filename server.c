#include "files.c"
#include "dir.c"

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
            if((int)par != (int)m->par) {
                send_type(TYPE_NACK);
                printf("\t(operate_server) Nack sent.");
            } else {
                if (m->attr.type == TYPE_LS) { // client requested LS
                    send_type(TYPE_ACK);
                    if(m->attr.len == 0) {
                        m->data[0] = '\0';
                    }
                    puts("\t(operate_server) Received Ls. Ack sent. Sending ls.");
                    //printf("data dentro da mensagem: %s \n", m->data);
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
                        puts("Could not create a new file.");
                        exit(1);
                    }
                    receive_file(fp);
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
