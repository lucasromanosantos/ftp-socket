#include "client.h"
#include "utils.h"
#include "message.h"
#include "rawsocket.h"
#include "dir.h"
#include "files.h"

void operate_client() {
    FILE *fp;
    int i, length, *comm;
    char *args, *addr, *buf;

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
            for(i = 0; i < 1024; i++) {
                buf[i] = addr[i] = '\0';
            }
            args = show_interface(comm,args,buf);
            if(Log == 1)
                printf("Comm: %d - Args: '%s'\n",*comm,args);
            if(*comm == 1) {
                print_ls(ls(LocalPath,args));
            } else if(*comm == 2) {
                check_cd(args);
            } else if(*comm == 3) {
                puts("(operate_client) Sending ls request.");
                while(req_ls(args) == 0);
                listen_ls();
                puts("(operate_client) Rls was succesfull!\n\n");
            } else if(*comm == 4) {
                puts("(operate_client) Sending cd request.");
                while(req_cd(args) == 0);
                puts("(operate_client) Rcd was succesfull!\n\n");
            } else if(*comm == 5) {
                puts("(operate_client) Sending put request.");
                buf = strcpy(buf,LocalPath);
                buf = strcat(buf,args);
                puts(buf);
                if((fp = fopen(buf,"r")) == NULL) {
                    printf("(operate_client) File does not exist. Error was: %s\n",strerror(errno));
                } else {
                    while(req_put(args) == 0);
                    length = send_filesize(fp);
                    if(send_file(fp,length) != 1)
                        puts("(operate_client) Could not send file.");
                    puts("(operate_client) Put was succesfull!");
                }
            } else if(*comm == 6) {
                puts("(operate_client) Sending get request.");
                while(req_get(args) == 0);
                addr = strcpy(addr,LocalPath); // Concatenating file name.
                addr = strcat(addr,args);   // Concatenating file name.
                if((fp = fopen(addr,"w")) == NULL) {
                    printf("(operate_server) Could not create a new file. Error was: %s\n",strerror(errno));
                    exit(1);
                } else {
                    receive_file(fp);
                    puts("(operate_client) Get was succesfull!");
                }
            } else {
                if(*comm != 7)
                    puts("(operate_client) Invalid option. Please, type another number.");
            }
        }
    }
}

int req_ls(char *args) {
    Message *m;
    Attr attrs;
    int i;
    m = malloc_msg(0); // Data is empty
    if(Log == 1)
        printf("(req_ls) argumentos: %s\n", args);
    attrs = prepare_attr(strlen(args),Seq,TYPE_LS);
    m = prepare_msg(attrs,(unsigned char*)args);
    send_msg(m);
    while(!(i = wait_response())) {
        send_msg(m);
    }
    if(i == 1) { // Got an ACK, server will start sending data.
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
    m = prepare_msg(attrs,(unsigned char*)args);
    send_msg(m);
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
    attrs = prepare_attr(strlen(args),Seq,TYPE_PUT);
    m = prepare_msg(attrs,(unsigned char*)args);
    send_msg(m);
    while(!(i = wait_response()))
        send_msg(m);
    if(i == 1) { // Got an ACK - server will start sending the data.
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
    attrs = prepare_attr(strlen(args),Seq,TYPE_GET);
    m = prepare_msg(attrs,(unsigned char*)args);
    send_msg(m); // Sending get request
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
    int i = 0;
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

int listen_ls() {
    Message *m;
    char *c;
    int size = 0;

    c = malloc(MAX_DATA_LEN * sizeof(char) + 1);
    m = malloc_msg(MAX_DATA_LEN);
    m = wait_data(m);
    if(Log == 1)
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
            strncat(c, (char*)m->data, m->attr.len);
            send_type(TYPE_ACK);
        }
        m = wait_data();
        if(Log == 1)
            print_message(m);
    }
    send_type(TYPE_ACK); // Sending an ack to TYPE_END message.
    print_ls(c);
    free(c);
    free(m);
    return 1;
}
