//#include "header.h"

int print_message(Message *m) {
    int i;
    printf("Mensagem:\n");
    printf("init: %u | tam: %d | seq: %d | type: %d | msg: %s | par: %d \n", m->init, m->attr.tam, m->attr.seq, m->attr.type, m->data, m->par);
    return 1;
}

void error(const char *msg) {
    perror(msg);
    printf("ALALALALA");
    puts("BLABLABLABLBAL");
    exit(1);
}

Message* create_msg(int data_length) {
    // Queria mudar o nome pra create_message, mas jah tem essa funcao, o que fazemos?
    // Soh aloca espaco pro data porque eh a unica parte da struct que eh pointer, ou seja, nao tem tam definido.
    Message *m;
    m->data = malloc(data_length);
    return m;
}

int msg_length(Message *m) {
    // init | attr | data | par | '\0'
    return   1 +  2 + strlen(m->data) + 1 + 1;
}

char* msg_to_str(Message *m) {
    int i,pos;
    unsigned char* c = malloc(262); // 1 init + 2 attr + 1 par + 256 = 260 + '\0'
    //unsigned char* c = malloc(msg_length(m)); // Allocar com o tamanho CORRETO da mensagem.
    c[0] = m->init;
    unsigned char* p = c + 1;
    memcpy(p,&m->attr,2);
    /* Descomente para provar que o MEMCPY faz o trabalho dele corretamente.
    memcpy(&m->attr,p,2);
    printf("Prova: Tam: %d | Seq: %d | Type: %d\n",m->attr.tam,m->attr.seq,m->attr.type);
    */
    for(i=0;i<strlen(m->data);i++) // condicao de parada nao tem +1 porque nao quero o '\0', tem o parity ainda.
        c[i+3] = m->data[i];
    pos = strlen(c);
    c[pos] = '0'; // preenche no lugar do '\0'
    c[pos+1] = '\0';
    return c;
}

Message* str_to_msg(char* c) {
    Message m,*n;
    m.init = c[0];
    m.attr.tam = ((c[1] & MASK_TAM) >> 2) & 0x3F;
    m.attr.seq = (c[1] & MASK_SEQ_MSB) * 32;
    m.attr.seq = (m.attr.seq + (c[2] & MASK_SEQ_LSB));
    m.attr.type = (c[2] & MASK_TYPE);
    m.data = memcpy(m.data, c + 3, m.attr.tam);
    m.par = 0; //(c[strlen(c)]);
    // n = create_msg(strlen(m->data)+1);
    *n = m; // Tem que alocar n antes??
    return n;
}

Message prepare_msg(Attr attr, unsigned char *data) {
    Message *msg;
    printf("Criando mensagem... \n");
    msg->init = 0x7E;
    msg->attr.tam = attr.tam;
    msg->attr.seq = attr.seq;
    msg->attr.type = attr.type;
    msg->data = malloc(sizeof(char) * BUF_SIZE);
    strcpy(msg->data, data);
    msg->par = 0;
    print_message(msg); //temp
    return *msg;
}

int send_msg(int socket, Message *m) { // nome temp
    int i,cont = 0;
    ssize_t n;
    size_t length = MAX_LEN * 8;
    Message *p = m;
    char *s;
    while(length > 0) {
        s = msg_to_str(p);
        printf("String : ");
        puts(s);
        n = send(socket, s, length, 0);
        printf("%d bits enviados... \n", (int)n);
        if(n <= 0) break; // erro
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
    rv = poll(ufds, 1, -1); // -1 = timout infinito (p/ testar)
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
if(strlen(buf) > 6) le tam
....
if(strlen(buf) > 6) le seq
...
le tipo
...
if(strlen(buf) > tam) le dados
...
if(strlen(buf) > 8) le paridade
*/