#include "client.h"
#include "utils.h"
#include "message.h"
#include "rawsocket.h"
#include "dir.h"
#include "files.h"

void operate_client() { //
    FILE *fp;
    int i,length,*comm;
    unsigned char *args,*buf,*addr;

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
            args = show_interface(comm,args,buf);
            printf("Comm: %d - Args: '%s'\n",*comm,args);
            puts("Yeah");
            if(*comm == 1) {
                print_ls(ls(LocalPath,args));
            } else if(*comm == 2) {
                check_cd(args);
            } else if(*comm == 3) {
                //puts("(operate_client) Sending ls request.");
                while(req_ls(args) == 0);
                //puts("(operate_client) Listening to ls...");
                printf("LS com janelas. \n");
                listen_ls();
                puts("(operate_client) Rls was succesfull!\n\n");
            } else if(*comm == 4) {
                //puts("(operate_client) Sending cd request.");
                while(req_cd(args) == 0);
                puts("(operate_client) Rcd was succesfull!\n\n");
            } else if(*comm == 5) {
                //puts("(operate_client) Sending put request.");
                buf = strcpy(buf,LocalPath);
                buf = strcat(buf,args);
                puts(buf);
                if((fp = fopen(buf,"r")) == NULL) {
                    printf("(operate_client) File does not exist. Error was: %s\n",strerror(errno));
                } else {
                    while(req_put(args) == 0);
                    length = send_filesize(fp);
                    if(send_file2(fp,length) != 1)
                        puts("(operate_client) Could not send file.");
                    puts("(operate_client) Put was succesfull!");
                }
            } else if(*comm == 6) {
                //puts("(operate_client) Sending get request.");
                while(req_get(args) == 0);
                addr = strcpy(addr,LocalPath); // Concatenating file name.
                addr = strcat(addr,args);   // Concatenating file name.
                if((fp = fopen(addr,"w")) == NULL) {
                    printf("(operate_server) Could not create a new file. Error was: %s\n",strerror(errno));
                    exit(1);
                } else {
                    receive_file2(fp);
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
    //printf("(req_ls) argumentos: %s\n", args);
    attrs = prepare_attr(strlen(args),Seq,TYPE_LS);
    m = prepare_msg(attrs,args);
    send_msg(m);
    //puts("(req_ls) Waiting for ls response..."); // Wait for an ACK
    while(!(i = wait_response())) {
        send_msg(m);
    }
    if(i == 1) { // Got an ACK
        // Server will start sending the data.
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
    m = prepare_msg(attrs,args);
    send_msg(m);
    //puts("(req_cd) Waiting for cd response..."); // Wait for an ACK
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
    //printf("(req_put) argumentos: %s\n", args);
    attrs = prepare_attr(strlen(args),Seq,TYPE_PUT);
    m = prepare_msg(attrs,args);
    send_msg(m);
    //puts("(req_put) Waiting for put response..."); // Wait for an ACK
    while(!(i = wait_response()))
        send_msg(m);
    if(i == 1) { // Got an ACK
        // Server will start sending the data.
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
    //printf("(req_get) argumentos: %s\n", args);
    attrs = prepare_attr(strlen(args),Seq,TYPE_GET);
    m = prepare_msg(attrs,args);
    send_msg(m); // Sending get request
    //puts("(req_get) Waiting for get response..."); // Wait for an ACK
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
    int i;
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

int send_file(FILE *fp,int len) {
    int nob = 0,i;
    int size, perc, totalLen, dataSent, completed = 0, valueChange = 1;
    unsigned char *c;
    Message *m;
    Attr a;

    m = malloc_msg(MAX_DATA_LEN);
    if((c = malloc(sizeof(char) * 64)) == NULL)
        error("(send_file) Unable to allocate memory.");

    if(len < 10000) { // small size
        size = 0; // 0 means small size, 1 means medium size, 2 means big file.
        perc = 0;
    } else if(len >= 10000 && len <= 1000000) { // medium size
        size = 1;
        perc = len / 10; // Show on screen every 10% completed.
    } else {
        size = 2;
        perc = len / 100; // Calculate 1% from file_size to show percentage on screen.
    }

    Seq = 0;
    totalLen = len;
    while(len > 0) { // Send n messages until the remaining data is less than 63 bytes (until I need only one message).
        if(len > MAX_DATA_LEN) {
            nob = 0;
            while(nob < MAX_DATA_LEN)
                nob += fread(c + nob,1,MAX_DATA_LEN-nob,fp);
            a = prepare_attr(MAX_DATA_LEN,Seq,TYPE_PUT);
            m = prepare_msg(a,c);
            send_msg(m);
            Seq += 1;
            printf("Len = %d, mlen = %d, mseq = %d\n",len,(int)m->attr.len, (int)m->attr.seq);
/*
            dataSent = totalLen - len;
            if(size > 0) {
                if(dataSent > completed * perc/10 && size == 1) {
                    completed += 10;
                    valueChange = 1;
                } else if(dataSent > completed * perc && size == 2) {
                    completed += 1;
                    valueChange = 1;
                }
                if(valueChange) {
                    system("clear");
                    printf("Sending file...\n+------------+\n| ");
                    for(i=0; i < completed-9; i+=10) {
                        printf("X");
                    }
                    for(i=completed; i<100; i+=10) {
                        printf(" ");
                    }
                    puts(" |\n+------------+\n");
                    printf("%d%% sent.\n\n\n",completed);
                    valueChange = 0;
                }
            }
*/
            if((Seq % 3) == 2) { // I sent 4 messages. Were they ok?
                if(!wait_response()) { // Got an nack.
                    Seq -= 3;
                    fseek(fp, -1 * (MAX_DATA_LEN * 3), SEEK_CUR);
                    len += MAX_DATA_LEN * 3;
                }
            }
            len -= MAX_DATA_LEN;
        } else {
            nob = 0;
            while(nob < len)
                nob += fread(c + nob,1,MAX_DATA_LEN - nob,fp);
            a = prepare_attr(len,Seq,TYPE_PUT);
            c[len] = '\0'; // Not correctly tested, but this might be a bug in other functions! Watch out!!
            m = prepare_msg(a,c);
            send_msg(m);
            printf("Len = %d, mlen = %d, mseq = %d\n",len,(int)m->attr.len, (int)m->attr.seq);
            Seq += 1;
            if(wait_response()) {
                //puts("Got last message ack. 4 Messages were sent succesfully");
                len = 0;
            } else {
                Seq -= 3;
                fseek(fp, -1 * (MAX_DATA_LEN * (Seq - m->attr.seq) + len) , SEEK_CUR);
                len += MAX_DATA_LEN * (Seq - m->attr.seq) + len;
            }

        }
    }
    //puts("Gonna send type_end.");
    unsigned char s[1];
    s[0] = '\0';
    a = prepare_attr(0,Seq,TYPE_END);
    m = prepare_msg(a,s);
    send_msg(m);
    while(!wait_response()) {
        send_msg(m);
    }
    free(c);
    free(m);
    return 1;
}

int listen_ls() {
    Message *m;
    unsigned char *c;
    int size = 0;
    int i=0;
    c = malloc(MAX_DATA_LEN * sizeof(unsigned char) + 1);
    //c = malloc(10000);
    m = malloc_msg(MAX_DATA_LEN);
    m = wait_data(m);
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
            strncat(c, m->data, m->attr.len);
            send_type(TYPE_ACK);
        }
        else {
            //puts("(listen_ls) Can not handle this message.");
        }
        //free(m); // m will be allocated again in wait_data. - Might bug something.
        m = wait_data();
        print_message(m);
    }
    send_type(TYPE_ACK); // Sending an ack to TYPE_END message.
    print_ls(c);
    free(c);
    free(m);
    return 1;
}

//////////////// Repeticao seletiva

int send_file2(FILE *fp,int len) {
    int nob = 0, i, j, window = 3, mp = 0, remaining; // mp is message pointer, it counts which message is in turn.
    int size, perc, totalLen = len, dataSent, completed = 0, valueChange = 1, seqGot, msgWaitAns = 0;
    unsigned char *c;
    Message **m, *aux;
    Attr a;

    if((m = malloc(window * sizeof(Message *))) == NULL) {
        error("(send_file) Unable to allocate memory.");
    }

    for(i = 0; i < window; ++i) {
        m[i] = malloc_msg(MAX_DATA_LEN);
    }

    if((c = malloc(sizeof(char) * 64)) == NULL) {
        error("(send_file) Unable to allocate memory.");
    }

    aux = malloc_msg(MAX_DATA_LEN);
/*
    if(len < 10000) { // small size
        size = 0; // 0 means small size, 1 means medium size, 2 means big file.
        perc = 0;
    } else if(len >= 10000 && len <= 1000000) { // medium size
        size = 1;
        perc = len / 10; // Show on screen every 10% completed.
    } else {
        size = 2;
        perc = len / 100; // Calculate 1% from file_size to show percentage on screen.
    }
*/
    Seq = 0;

    while(len > 0) {
        nob = 0;
        remaining = (len > MAX_DATA_LEN) ? MAX_DATA_LEN : len;
        while(nob < remaining)
            nob += fread(c + nob,1,MAX_DATA_LEN-nob,fp);
        a = prepare_attr(remaining, Seq, TYPE_PUT);
        m[mp] = prepare_msg(a, c);
        send_msg(m[mp]);
        mp = (mp + 1) % window;
        msgWaitAns++;
        Seq += 1;

        while(msgWaitAns >= window) {
            if(!wait_response(aux)) { // Got an nack.
                memcpy(&seqGot,aux->data,4); // Got an nack indicating this message had error.
                for(i = 0; i < window; ++i) {
                    if(m[(mp + i) % window]->attr.seq == seqGot) { // Found the wrong message. Have to send it again.
                        send_msg(m[(mp + i) % window]);
                        break ; // Send message, wait for a response.
                    } else {
                        msgWaitAns--; // This was OK. I can send another one.
                    }
                }
            } else { // Got an ack after sending 3 messages.
                memcpy(&seqGot,aux->data,4); // Got an ack indicating this message and those before it were OK.
                for(i = 0, j = 0; i < window; ++i) {
                    msgWaitAns--;
                    if(seqGot == m[mp+i % window]->attr.seq) { // Look at bottom for proper comments explaining this.
                        break ;
                    }
                    if(i >= window) { // Received a message with a Seq that was not from any message I sent!
                        puts("(Send_file2) Panic!!");
                        return 0;
                    }
                }
            }
        }
    }
    //puts("Gonna send type_end.");
    unsigned char s[1];
    s[0] = '\0';
    a = prepare_attr(0,Seq,TYPE_END);
    m[0] = prepare_msg(a,s);
    send_msg(m[0]);
    while(!wait_response()) {
        send_msg(m[0]);
    }
    free(c);
    free(m); // To do this free, do we have to do a for?
    free(aux);
    return 1;
}


int wait_response_seq(Message *m) { // necessitamos // function that returns 0 if nack or 1 if ack
    unsigned char *buffer;
    time_t seconds = 3,endwait;
    int i, size;

    endwait = time(NULL) + seconds;
    if((buffer = malloc(1024)) == NULL)
        return 0;

    while(time(NULL) < endwait && i != 1)
        i = receive(buffer, &m, STD_TIMEOUT);

    if(i == 1) {
        if(m->attr.type == TYPE_ACK) { // Got ack
            free(buffer);
            return 1;
        } else if(m->attr.type == TYPE_NACK) {
            free(buffer);
            return 0;
        } else if(m->attr.type == TYPE_ERROR) {
            free(buffer);
            return -1;
        } else {
            free(buffer);
            return -2;
        }
    } else {
        puts("(wait_response) Error! Timeout? \n");
        free(buffer);
        return -3;
    }
}


/*
mp = 0.
Prepare message 5 and send it. mp = 1.
Prepare message 6 and send it. mp = 2.
Prepare message 7 and send it. mp = 0.
I sent messages 5, 6 and 7.
Got an ACK from message 7.
My mp was pointing to 0.
So, if I check my messages 5, 6 and 7 (for i=0; i<3; i++) m[mp+i %3], the one which m[]->Seq == seqGot is the one I got an ack.
So, for everyone DIFFERENT, I can know it was OK.

For example, my ACK was from message 7. So, I check if message 5 has the same Seq. No. It means message 5 was ok. I can send another msg (8).
Same to message 6. Its different. So, its ok. I can send another message (9). Message 7 is the right one. So, I can send another one (10).
It means I sent 8, 9 and 10. Go into the loop again.

If I sent 5, 6 and 7 and got ACK from message 6. I check if message 5 has the same Seq. No. It means message 5 was ok. I can send another msg (8).
Message 6 is the right one. So, I can send another one (9). So, I already sent 7, 8 and 9. Go into that loop again.

If I sent 5, 6 and 7 and got ACK from message 5. Message 5 is the right one. So, I can send another one (8).
So, I already sent 6, 7 and 8. Go into that loop again.
*/