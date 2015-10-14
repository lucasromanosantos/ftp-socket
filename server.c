#include "files.c"
#include "dir.c"

void send_ls_data(int socket,char *args) {
    char *result = malloc(1024); // temp size.. realloc maybe??
    unsigned char *buffer;
    if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL) {
        puts("(send_ls) Unable to allocate memory");
        return;
    }
    //get_files(".", result);
    ls(LocalPath,args);
    size_t nob = strlen(result); // nob = number of bytes
    printf("(send_ls) Size of total nob: %d \n", (int) nob);
    int seq = 1;

    Message *m; // check allocation / realloc???
    m = malloc_msg(MAX_DATA_LEN);
    Attr attrs;
    while(nob > 0) {
        if(nob >= MAX_DATA_LEN) {
            char tmp[64];
            attrs = prepare_attr(MAX_DATA_LEN, seq, TYPE_SHOWSCREEN);
            strncpy(tmp, result, MAX_DATA_LEN);
            m = prepare_msg(attrs, tmp);
            send_msg(socket, m);
            result += MAX_DATA_LEN; // add MAX_DATA_LEN bytes to result pointer
            nob -= MAX_DATA_LEN;
        }
        else {
            char tmp[nob + 1];
            attrs = prepare_attr(nob, seq, TYPE_SHOWSCREEN);
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
    unsigned char *buffer,*buffer2,*addr,par;
    int res = 0;
    Message *m;

    if((buffer2 = malloc(MIN_LEN)) == NULL)
        error("(operate_server) Error allocating memory."); // alocar menos para ack/nack , não? e dps q receber free()
    if((buffer = malloc(MAX_MSG_LEN)) == NULL)
        error("(operate_server) Error allocating memory.");
    if((addr = malloc(sizeof(unsigned char) * 1024)) == NULL)
        error("(operate_server) Error allocating memory.");
    m = malloc_msg(0);

    while(1) {
        res = recv_tm(socket, buffer2, &m, STD_TIMEOUT);
        if(res == 1) {
            par = get_parity(m);
            if((int)par != (int)m->par) {
                send_nack(socket);
                printf("\t(operate_server) Nack sent.");
            } else {
                if (m->attr.type == TYPE_LS) { // client request LS
                    send_ack(socket);
                    puts("\t(operate_server) Received Ls. Ack sent. Sending ls.");
                    send_ls_data(socket,m->data);
                    puts("\t(operate_server) Ls sent.");
                } else if (m->attr.type == TYPE_CD) { // client request LS
                    send_ack(socket);
                    puts("\t(operate_server) Received Cd. Ack sent.");
                    if(!check_cd(m->data)) {
                        puts("\t(operate_server) Cd path was wrong. Error occured.");
                        Attr a;
                        a.len = 0;
                        a.type = TYPE_ERROR;
                        a.seq = Seq;
                        Seq++;
                        m = prepare_msg(a,"");
                        send_msg(socket,m);
                    } else {
                        puts("\t(operate_server) Path updated succesfully.");
                    }
                } else if (m->attr.type == TYPE_PUT) { // client request LS
                    send_ack(socket);
                    puts("\t(operate_server) Ready to receive a Put. Ack sent.");
                    strcpy(addr,LocalPath); // Concatenating file name.
                    strcat(addr,m->data); // Concatenating file name.
                    FILE *fp;
                    if((fp = fopen(addr,"w")) == NULL) {
                        puts("Could not create a new file.");
                        exit(1);
                    }
                    receive_file(socket,fp);
                    puts("I should have received a file. Please, check it.");
                } else if (m->attr.type == TYPE_GET) { // client request LS
                    send_ack(socket);
                    puts("\t(operate_server) Ready to receive a Get. Ack sent.");
                }
                //puts("Sending acknowledge...");
                send_ack(socket); // Now with "socket" parameter we missed!
                puts("\t(operate_server) Ack sent.");
            }
        }
    }
}
