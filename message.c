#include "utils.c"

int print_message(Message *m) {
    printf("Msg-> Init: %u | Len: %d | Seq: %d | Type: %d | Msg: '%s' | Par: %d \n",
        m->init, m->attr.len, m->attr.seq, m->attr.type, m->data, m->par);
    return 1;
}

Message* malloc_msg(int length) {
    Message *m;
    if((m = malloc(1 + 2 + length + 1 + 1)) == NULL)
        error("(malloc_msg) Unable to allocate memory.");
    return m;
}

int msg_length(Message *m) {
    // init | attr | data | par | '\0'
    return   1 +  2 + strlen(m->data) + 1 + 1;
}

char* msg_to_str(Message *m) {
    int i,pos;
    unsigned char *c;
    if((c = malloc(msg_length(m))) == NULL)
        error("(msg_to_str) Unable to allocate memory."); // Allocar com o tamanho CORRETO da mensagem.
    //else
    //    puts("Memory allocated succesfully.");
    c[0] = m->init;
    memcpy(c+1,&m->attr,2);
    memcpy(c+3,m->data,m->attr.len);
    c[m->attr.len + 3] = m->par;
    c[m->attr.len + 4] = '\0';
    return c;
}

Message* str_to_msg(char* c) {
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
    m->par = c[strlen2(c)-1];
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
    strcpy(m->data, data);
    m->par = get_parity(m);
    return m;
}

void send_ack(int socket) {
    Message *m;
    m = malloc_msg(0); // No data in this message, so, its length is 0.
    Attr attr = prepare_attr(0,0,TYPE_ACK);
    m = prepare_msg(attr,"0");
    send_msg(socket, m);
    free(m);
    return ;
}

void send_nack(int socket) {
    Message *m;
    m = malloc_msg(0); // No data in this message, so, its length is 0.
    Attr attr = prepare_attr(0,0,TYPE_NACK);
    m = prepare_msg(attr,"");
    send_msg(socket, m);
    free(m);
    return ;
}
int send_msg(int socket, Message *m) {
    int i,cont = 0;
    ssize_t n;
    size_t length = msg_length(m) * 8;
    char *s = msg_to_str(m);
    printf("\t(send_msg) Enviando: ");
    print_message(m);
    // Actually send the message.
    while(length > 0) {
        n = send(socket, s, length, 0);
        printf("\t(send_msg) %d bits enviados... \n", (int)n);
        //if(n <= 0) break; // Error
        if(n < 0) {
            printf("\t(send_msg) Did not operate well. Error was: %s\n",strerror(errno));
        }
        s += n;
        length -= n;
    }
    //free(s);
    return (n <= 0) ? - 1 : 0;
}

int receive(int socket, unsigned char *data, Message **m) {
    int retorno,rv;
    struct pollfd ufds[1];
    ufds[0].fd = socket;
    ufds[0].events = POLLIN; // check for just normal data
    rv = poll(ufds, 1, -1); // -1 = Infinite timeout (for testing)
    if(rv == -1)
        error("(receive) Erro no poll");
    else if (rv == 0) {
        printf("(receive) Timeout! No data received");
        return 0; // Fail
    }
    else {
        // Read the message. If the first byte isnt the init (0111 1110), discard the message.
        int tmp_recv;
        if(ufds[0].revents & POLLIN) {
            tmp_recv = recv(socket, data, MAX_LEN, 0);
            if(data[0] != 126) // 126 = 0111 1110
                return 0; // Fail
            *m = str_to_msg(data);
            printf("(receive)");
            print_message(*m);
            return 1; // Success
        }
    }
}

int recv_tm(int socket, unsigned char *data, Message **m, int timeout) {
    int retorno,rv = 0;
    struct pollfd ufds[1];
    ufds[0].fd = socket;
    ufds[0].events = POLLIN; // check for just normal data
    //rv = poll(ufds, 1, timeout);
	rv = poll(ufds, 1, -1);
    time_t start = time(NULL);
    while(time(NULL) < start + 3 && rv <= 0) {
        rv = poll(ufds,1,-1);
    }
    if(rv == -1)
        error("(recv_tm) Erro no poll");
    else if (rv == 0) {
        puts("\t(recv_tm) Timeout! No data received! Is the server working?");
        return 0; // Fail
    }
    else {
        // Read the message. If the first byte isnt the init (0111 1110), discard the message.
        int tmp_recv;
        if(ufds[0].revents & POLLIN) {
            tmp_recv = recv(socket, data, MAX_LEN, 0);
            //puts(data);
            //printf("(recv_tm) Init is: %d\n",(int)(data[0]));
            if(data[0] != 126) {// 126 = 0111 1110
		      return 0; // Fail
            }

            // Impressão da mensagem recebida:
            //printf("\t(recv_tm) Mensagem recebida: '");
            Attr a;
            memcpy(&a,data+1,2);
            int i;
            /*printf("\n(recv_tm) After init: %d%d - A.len = %d\n",(int)data[1],(int)data[2],a.len);
            for(i=0; i<a.len; i++)
                printf("%c",data[i+3]);
            printf("'\n");*/

            *m = str_to_msg(data);
            return 1; // Success
        }
    }
}

int wait_response(int socket) { // necessitamos // function that returns 0 if nack or 1 if ack
    unsigned char *buffer;
    time_t seconds = 3,endwait;
    int i;
    Message *m; // check

    endwait = time(NULL) + seconds;
    if((buffer = malloc(1024)) == NULL)
        return 0;
    m = malloc_msg(0);

    while(time(NULL) < endwait && i != 1)
        i = recv_tm(socket, buffer, &m, STD_TIMEOUT);

    if(i == 1) {
        if(m->attr.type == TYPE_ACK) { // got ack
            puts("(wait_response) Got an ack! \n");
            free(buffer);
            free(m);
            return 1;
        } else if(m->attr.type == TYPE_NACK) {
            puts("(wait_response) Got a nack! \n");
            free(buffer);
            free(m);
            return 0;
        } else if(m->attr.type == TYPE_ERROR) {
            puts("(wait_response) Got an error! \n");
            free(buffer);
            free(m);
            return -1;
        } else {
            puts("(wait_response) Panic!!\n");
            free(buffer);
            free(m);
            return -2;
        }
    }
    else {
        puts("(wait_response) Error! Timeout? \n");
        free(buffer);
        free(m);
        return 0;
    }
}


/*DATA
||||||||
char* c = data;
c++;

recebeu algo
if(strlen(buf) > 8) le init
else espera_mais_dado
strlen(buf) -= 8;

--
if(strlen(buf) > 6) le len
....
if(strlen(buf) > 6) le seq
...
le tipo
...
if(strlen(buf) > len) le dados
...
if(strlen(buf) > 8) le paridade
*/
