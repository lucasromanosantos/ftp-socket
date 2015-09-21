#include <math.h>

int print_message(Message *m) {
    int i;
    printf("\tMensagem:\n");
    printf("\tInit: %u | Len: %d | Seq: %d | Type: %d | Msg: '%s' | Par: %d \n", m->init, m->attr.len, m->attr.seq, m->attr.type, m->data, m->par);
    return 1;
}

int pot(int base, int exp) {
    if(exp < 0)
        return 0;
    if(exp == 0)
        return 1;
    if(exp == 1)
        return base;
    return base * pot(base, exp-1);
}

/* Expected Parity:
Init does not count.
Length, sequency and type: (Using values 1, 1 and 9)
0000 0100
0001 1001
0010 1001 // Character (in ASCII) from the message
---------
0011 0100

What I am doing with my function:
  0000 01
  0000 01
     1001
0010 1001
---------
0010 0001
*/

unsigned char get_parity(Message *m) {
    int i;
    unsigned char res = 0,c[2];
    memcpy(c,&m->attr,2); // c will have m->attr data so we can look to this struct as 2 chars.
    res = c[0] ^ c[1];
    //printf("\tConverting: %d-%d - Lenght:%da\n",(int)c[0],(int)c[1],(int)m->attr.len);
    print_message(m);
    for(i=0; i < (int)m->attr.len; i++) {
        res = res ^ m->data[i];
    }
    return res;
}

void error(const char *msg) {
    puts(msg);
    perror(msg);
    exit(1);
}

Message* create_msg(int length) {
    Message *m;
    if((m = malloc(1 + 2 + length + 1 + 1)) == NULL)
        error("Unable to allocate memory.");
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
        error("Unable to allocate memory."); // Allocar com o tamanho CORRETO da mensagem.
    c[0] = m->init;
    memcpy(c+1,&m->attr,2);
    memcpy(c+3,m->data,m->attr.len);
    c[m->attr.len + 3] = m->par;
    c[m->attr.len + 4] = '\0';
    return c;
}

Message* str_to_msg(char* c) {
    Message *m;
    m = create_msg(strlen(c)-5); // Strlen is wrong used here. We have to get msg->attr.len from the string c to allocate the right memory.
    // Ok, maybe strlen is not thaaat wrong. In fact, its probably correct. Someone should revise it.
    m->init = c[0];
    memcpy(&m->attr, c+1, 2);
    if((m->data = malloc(m->attr.len)) == NULL)
        error("Unable to allocate memory.");
    m->data = memcpy(m->data, c + 3, m->attr.len);
    m->par = c[strlen(c)-1];
    return m;
}

Message prepare_msg(Attr attr, unsigned char *data) {
    Message *m;
    m = create_msg(attr.len);
    printf("\tCriando mensagem... \n");
    m->init = 0x7E; // 0111 1110
    m->attr.len = attr.len;
    m->attr.seq = attr.seq;
    m->attr.type = attr.type;
    if((m->data = malloc(sizeof(char) * (attr.len + 1))) == NULL)
        error("Unable to allocate memory.");
    strcpy(m->data, data);  // This was throwing an unknown error. Any ideas why?
    //m->par = 0;
    m->par = get_parity(m);
    print_message(m);
    return *m;
}

Attr prepare_attr(int length,int seq,int type) {
    Attr a;
    a.len = length;
    a.seq = seq;
    a.type = type;
    return a;
}
