int print_message(Message *m) {
    int i;
    printf("Mensagem:\n");
    printf("Init: %u | Len: %d | Seq: %d | Type: %d | Msg: %s | Par: %d \n", m->init, m->attr.len, m->attr.seq, m->attr.type, m->data, m->par);
    return 1;
}

void error(const char *msg) {
    puts(msg);
    perror(msg);
    exit(1);
}

Message* create_msg(int length) {
    // Only allocate for m->data, cause the others members of the struct are not pointers, which means they have a constant length,
    // so the compiler will take care of that for us.
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
    if((c = malloc(262)) == NULL)
        error("Unable to allocate memory."); // 1 init + 2 attr + 1 par + 256 = 260 + '\0'
    //unsigned char* c;
    //if((c = malloc(msg_length(m))) == NULL)
    //    error("Unable to allocate memory."); // Allocar com o tamanho CORRETO da mensagem.
    c[0] = m->init;
    unsigned char* p = c + 1;
    memcpy(p,&m->attr,2);
    /* Uncomment this to see the proof that memcpy does its work correctly.
    memcpy(&m->attr,p,2);
    printf("Proof: Len: %d | Seq: %d | Type: %d\n",m->attr.len,m->attr.seq,m->attr.type);
    */
    for(i=0;i<strlen(m->data);i++) // Stop condition without +1, because I dont want the NULL terminator (to get parity)
        c[i+3] = m->data[i];
    pos = strlen(c);
    c[pos] = m->par;
    c[pos+1] = '\0';
    return c;
}

Message* str_to_msg(char* c) {
    Message *m;
    m = create_msg(strlen(c)); // Strlen is wrong used here. We have to get msg->attr.len from the string c to allocate the right memory.
    m->init = c[0];
    puts(c);
    memcpy(&m->attr, c+1, 2);
    puts(c);
    if((m->data = malloc(m->attr.len)) == NULL)
        error("Unable to allocate memory.");
    m->data = memcpy(m->data, c + 3, m->attr.len);
    m->par = c[strlen(c)];
    return m;
}

Message prepare_msg(Attr attr, unsigned char *data) {
    Message *msg;
    int i;
    msg = create_msg(attr.len);
    printf("Criando mensagem... \n");
    msg->init = 0x7E; // 0111 1110
    msg->attr.len = attr.len;
    msg->attr.seq = attr.seq;
    msg->attr.type = attr.type;
    if((msg->data = malloc(sizeof(char) * (attr.len + 1))) == NULL)
        error("Unable to allocate memory.");
    //strcpy(msg->data, data);  // This was throwing an unknown error. Any ideas why?
    for(i=0; i<attr.len; i++)
        msg->data[i] = data[i];
    msg->par = 0;
    print_message(msg);
    return *msg;
}
