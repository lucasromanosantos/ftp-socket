#include "utils.c"

int send_msg(int socket, Message *m) {
    int i,cont = 0;
    ssize_t n;
    size_t length = msg_length(m) * 8;
    Message *p = m;
    char *s = msg_to_str(p);
    while(length > 0) {
        n = send(socket, s, length, 0);
        printf("%d bits enviados... \n", (int)n);
        if(n <= 0) break; // Error
        s += n;
        length -= n;
    }
    return (n <= 0) ? - 1 : 0;
}

Message* receive(int socket, unsigned char *data) {
    int retorno,rv;
    Message *m;
    struct pollfd ufds[1];
    ufds[0].fd = socket;
    ufds[0].events = POLLIN; // check for just normal data
    rv = poll(ufds, 1, -1); // -1 = Infinite timeout (for testing)
    if(rv == -1)
        error("Erro no poll");
    else if (rv == 0)
        printf("Timeout! No data received");
    else {
        int tmp_recv;
        if(ufds[0].revents & POLLIN) {
            tmp_recv = recv(socket, data, MAX_LEN, 0);
            if(data[0] != 126) // 126 = 0111 1110
                return ;
            puts(data);
            m = str_to_msg(data);
            print_message(m);
            return m;
        }
    }
}
/*
DATA
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