#include "utils.c"

void send_ack(int socket) {
    Message *m;
    m = create_msg(0); // No data in this message, so, its length is 0.
    Attr attr = prepare_attr(0,0,TYPE_ACK);
    *m = prepare_msg(attr,"");
	send_msg(socket, m);
    print_message(m);
    return ;
}

int send_msg(int socket, Message *m) {
    int i,cont = 0;
    ssize_t n;
    size_t length = msg_length(m) * 8;
    Message *p = m;
    char *s = msg_to_str(p);
    // Actually send the message.
    while(length > 0) {
        n = send(socket, s, length, 0);
        printf("\t%d bits enviados... \n", (int)n);
        if(n <= 0) break; // Error
        s += n;
        length -= n;
    }
    return (n <= 0) ? - 1 : 0;
}

int receive(int socket, unsigned char *data, Message *m) {
    int retorno,rv;
    struct pollfd ufds[1];
    ufds[0].fd = socket;
    ufds[0].events = POLLIN; // check for just normal data
    rv = poll(ufds, 1, -1); // -1 = Infinite timeout (for testing)
    if(rv == -1)
        error("Erro no poll");
    else if (rv == 0) {
        printf("Timeout! No data received");
        return 0; // Fail
    }
    else {
        // Read the message. If the first byte isnt the init (0111 1110), discard the message.
        int tmp_recv;
        if(ufds[0].revents & POLLIN) {
            tmp_recv = recv(socket, data, MAX_LEN, 0);
            if(data[0] != 126) // 126 = 0111 1110
                return 0; // Fail
            m = str_to_msg(data);
            print_message(m);
            return 1; // Success
        }
    }
}

int recv_tm(int socket, unsigned char *data, Message **m, int timeout) {
    int retorno,rv = 0;
    struct pollfd ufds[1];
    ufds[0].fd = socket;
    ufds[0].events = POLLIN; // check for just normal data
    rv = poll(ufds, 1, timeout); // -1 = Infinite timeout (for testing)
    time_t start = time(NULL);
    while(time(NULL) < start + 3 && rv <= 0) {
        rv = poll(ufds,1,-1);
    }
    if(rv == -1)
        error("Erro no poll");
    else if (rv == 0) {
        puts("\tTimeout! No data received! Is the server working?");
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
            //print_message(*m);
            return 1; // Success
        }
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
