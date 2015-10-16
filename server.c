#include "files.c"
#include "dir.c"

void send_ls_data(char *args) { //
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
        }
        else {
            char tmp[nob + 1];
            attrs = prepare_attr(nob, Seq, TYPE_SHOWSCREEN);
            m = malloc_msg(attrs.len);
            strncpy(tmp, result, nob);
            m = prepare_msg(attrs, tmp);
            send_msg(m);
            nob = 0;

            if(wait_response()) {
                attrs = prepare_attr(0, 0, TYPE_END);
                m = malloc_msg(0);
                m = prepare_msg(attrs, "");
                send_msg(m);
            }
        }
        if(!wait_response())
            break;
        //Seq = (Seq + 1) % 64; Send_msg increment seq counter
    }
    printf("(send_ls) Final sequence: %d \n", Seq);
}

void operate_server() {
    unsigned char *buffer,*buffer2,*addr,par;
    int res = 0;
    Message *m;

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
                send_nack();
                printf("\t(operate_server) Nack sent.");
            } else {
                if (m->attr.type == TYPE_LS) { // client requested LS
                    send_ack();
                    puts("\t(operate_server) Received Ls. Ack sent. Sending ls.");
                    printf("data dentro da mensagem: %s \n", m->data);
                    send_ls_data(m->data);
                    puts("\t(operate_server) Ls sent.");
                } else if (m->attr.type == TYPE_CD) { // client requested CD
                    send_ack();
                    puts("\t(operate_server) Received Cd. Ack sent.");
                    if(!check_cd(m->data)) {
                        puts("\t(operate_server) Cd path was wrong. Error occured.");
                        Attr a;
                        a.len = 0;
                        a.type = TYPE_ERROR;
                        a.seq = Seq;
                        //Seq = (Seq + 1) % 64; Send_msg increment seq counter
                        m = prepare_msg(a,"");
                        send_msg(m);
                    } else {
                        puts("\t(operate_server) Path updated succesfully.");
                    }
                } else if (m->attr.type == TYPE_PUT) { // client request LS
                    send_ack();
                    puts("\t(operate_server) Ready to receive a Put. Ack sent.");
                    strcpy(addr,LocalPath); // Concatenating file name.
                    strcat(addr,m->data); // Concatenating file name.
                    FILE *fp;
                    if((fp = fopen(addr,"w")) == NULL) {
                        puts("Could not create a new file.");
                        exit(1);
                    }
                    receive_file(fp);
                    puts("I should have received a file. Please, check it.");
                } else if (m->attr.type == TYPE_GET) { // client request LS
                    send_ack();
                    puts("\t(operate_server) Ready to receive a Get. Ack sent.");
                }
                send_ack();
                puts("\t(operate_server) Ack sent.");
            }
        }
    }
}
