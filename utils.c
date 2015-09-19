int print_message(Message *m) {
    int i;
    printf("\tMensagem:\n");
    printf("\tInit: %u | Len: %d | Seq: %d | Type: %d | Msg: '%s' | Par: %d \n", m->init, m->attr.len, m->attr.seq, m->attr.type, m->data, m->par);
    return 1;
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
    m->par = c[strlen(c)];
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
    m->par = 0;
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
