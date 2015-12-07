#include "message.h"
#include "utils.h"

int receive(unsigned char *data, Message **m, int timeout) {
    int rv = 0;
    struct pollfd ufds[1];
    ufds[0].fd = Socket;
    ufds[0].events = POLLIN; // check for just normal data
    //rv = poll(ufds, 1, timeout);
    //rv = poll(ufds, 1, -1);
    rv = poll(ufds, 1, 500);
    //time_t start = time(NULL);
    //while(time(NULL) < start + 3 && rv <= 0) {
    //    rv = poll(ufds,1,-1);     TESTE
    //}

    if(rv == -1)
        error("(recv_tm) Erro no poll");
    else if (rv == 0) {
        //puts("\t(recv_tm) Timeout! No data received! Is the server working?");
        return 0; // Fail
    }
    else { // Read the message. If the first byte isnt the init (0111 1110), discard the message.
        int tmp_recv;
        if(ufds[0].revents & POLLIN) {
            tmp_recv = recv(Socket, data, MAX_LEN, 0);
            if(tmp_recv == -1) {
                printf("(receive) Error in recv! Error was: %s\n",strerror(errno));
            }
            if(data[0] != 126) {// 126 = 0111 1110
              return 0; // Fail
            }
            Attr a;
            memcpy(&a,data+1,2);

            *m = str_to_msg(data);/*
            char par = get_parity(*m);
            if(par != (*m)->par) {
                send_type(TYPE_NACK);
                return 0;
            }
            if(Seq + 1 != (*m)->attr.seq) { isto acho que n vai ter
                send_type(TYPE_NACK);
                printf("seq fucked");
                return 0;
            }
            Seq = (Seq + 1) % 64;
            printf("(receive) Got this message(%d):",a.len);
            print_message(*m); */
            return 1; // Success
        }
    }
    return 0;
}

int wait_response() { // necessitamos // function that returns 0 if nack or 1 if ack
    unsigned char *buffer;
    time_t seconds = 3,endwait;
    int i = 0;
    Message *m; // check

    endwait = time(NULL) + seconds;
    if((buffer = malloc(1024)) == NULL)
        return 0;
    m = malloc_msg(0);

    while(time(NULL) < endwait && i != 1)
        i = receive(buffer, &m, STD_TIMEOUT);
    printf("Got this message in wait_response:");
    print_message(m);
    if(i == 1) {
        if(m->attr.type == TYPE_ACK) { // got ack
            //puts("(wait_response) Got an ack! \n");
            //Seq = (Seq + 1) % 64;
            free(buffer);
            free(m);
            return 1;
        } else if(m->attr.type == TYPE_NACK) {
            //puts("(wait_response) Got a nack! \n");
            //Seq = (Seq + 1) % 64;
            free(buffer);
            free(m);
            return 0;
        } else if(m->attr.type == TYPE_ERROR) {
            //puts("(wait_response) Got an error! \n");
            //Seq = (Seq + 1) % 64;
            free(buffer);
            free(m);
            return -1;
        } else {
            //puts("(wait_response) Panic!!\n");
            free(buffer);
            free(m);
            return -2;
        }
    } else {
        puts("(wait_response) Error! Timeout? \n");
        free(buffer);
        free(m);
        return 0;
    }
}

int print_message(Message *m) {
    printf("Msg-> Init: %u | Len: %d | Seq: %d | Type: %d | Msg: '",m->init, m->attr.len, m->attr.seq, m->attr.type);
    int i;
    for(i=0; i<(int)m->attr.len; ++i) {
        printf("%c",m->data[i]);
    }
    printf("' | Par: %d \n", m->par);
    return 1;
}

Message* malloc_msg(int length) {
    Message *m;
    if((m = malloc(1 + 2 + length + 1 + 1)) == NULL)
        error("(malloc_msg) Unable to allocate memory.");
    m->data = malloc(length);
    return m;
}

int msg_length(Message *m) {
    // init | attr | data | par | '\0'
    //return   1 +  2 + strlen(m->data) + 1 + 1; // I changed this 26/10. It might bug something.
    return 1 + 2 + m->attr.len + 1 + 1;
}

unsigned char* msg_to_str(Message *m) {
    unsigned char *c;
    if((c = malloc(msg_length(m))) == NULL) // put +1 for \0
        error("(msg_to_str) Unable to allocate memory."); // Allocar com o tamanho CORRETO da mensagem.
    c[0] = m->init;
    memcpy(c+1,&m->attr,2);
    memcpy(c+3,m->data,m->attr.len);
    c[m->attr.len + 3] = m->par;
    c[m->attr.len + 4] = '\0';
    return c;
}

Message* str_to_msg(unsigned char* c) {
    Message *m;
    Attr a;
    memcpy(&a,c+1,2);
    m = malloc_msg(a.len + 5); // a.len only contains the length of the data. So, we have to add space for init byte,
                               // len, seq and type (2 bytes), parity byte and NULL byte, which equals to a.len + 5.
    m->init = c[0];
    //memcpy(&m->attr,c+1 2);
    m->attr = a;
    if((m->data = malloc(m->attr.len)) == NULL)
        error("(str_to_msg) Unable to allocate memory.");
    m->data = memcpy(m->data, c + 3, m->attr.len);
    m->data[m->attr.len] = '\0';
    m->par = c[3 + (int)m->attr.len];
    return m;
}

Message* prepare_msg(Attr attr, unsigned char *data) {
    Message *m;
    m = malloc_msg(attr.len);
    m->init = 0x7E; // 0111 1110
    m->attr.len = attr.len;
    m->attr.seq = attr.seq;
    m->attr.type = attr.type;
    if((m->data = malloc(sizeof(char) * (attr.len))) == NULL)
        error("(prepare_msg) Unable to allocate memory.");
    //strcpy(m->data, data);
    memcpy(m->data,data,(int)attr.len);
    m->par = get_parity(m);
    return m;
}

void send_type(int type) {
/* Send an empty message with attr->type set to our parameter.
 * We can use it to send acks, nacks, error and end messages.
 */
    unsigned char *empty = malloc(sizeof(unsigned char));
    empty[0] = '\0';
    Message *m;
    m = malloc_msg(0); // No data in this message, so, its length is 0.
    Attr attr = prepare_attr(0,Seq,type);
    m = prepare_msg(attr,empty);
    send_msg(m);
    free(m);
    return ;
}

void send_data_type(int type, int seq) {
/* Send an empty message with attr->type set to our parameter.
 * We can use it to send acks, nacks, error and end messages.
 */
    Message *m;
    unsigned char *s = malloc((sizeof(int)+1) * sizeof(unsigned char));
    memcpy(s,&seq,sizeof(int)); // Got an nack indicating this message had error.
    m = malloc_msg(sizeof(int));
    Attr attr = prepare_attr(sizeof(int),Seq,type);
    m = prepare_msg(attr,s);
    memcpy(&seq,s,sizeof(int)); // Got an nack indicating this message had error.
    printf("Seq that will be sent: %d\n\n",seq);
    send_msg(m);
    print_message(m);
    free(m);
    return ;
}

int send_msg(Message *m) {
    int i;
    ssize_t n;
    size_t length = msg_length(m) * 8;
    unsigned char *aux, *s = msg_to_str(m);
    aux = s;
    //printf("\t(send_msg) Enviando (%d bytes): ",(int)length);
    //print_message(m);
    // Actually send the message.
    while(length > 0) {
        n = send(Socket, s, length, 0);
        //printf("\t(send_msg) %d bits enviados... \n", (int)n);
        //if(n <= 0) break; // Error
        //Seq = (Seq + 1) % 64;
        if(n < 0) {
            print_message(m);
            for(i=0; i<m->attr.len; i++) {
                printf("%c",s[i]);
            }
            puts("");
            printf("\t(send_msg) Did not operate well. Error was: %s\n",strerror(errno));
            return 0;
        }
        s += n;
        length -= n;
    }
    free(aux);
    //free(s);
    //if(n <= 0)
       //Seq = (Seq + 1) % 64;
    return (n <= 0) ? - 1 : 0;
}

int wait_response_seq(Message **m) { // Function that returns 0 if nack or 1 if ack and edits m->data to get the wrong/correct seq.
    unsigned char *buffer;
    time_t seconds = 3,endwait;
    int i = 0;

    endwait = time(NULL) + seconds;
    if((buffer = malloc(1024)) == NULL)
        return 0;

    while(time(NULL) < endwait && i != 1)
        i = receive(buffer, m, STD_TIMEOUT);

    printf("Look what I got! ");
    print_message(*m);

    if(i == 1) {
        if((*m)->attr.type == TYPE_ACK) { // Got ack
            free(buffer);
            return 1;
        } else if((*m)->attr.type == TYPE_NACK) {
            free(buffer);
            return 0;
        } else if((*m)->attr.type == TYPE_ERROR) {
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