int print_message(Message *m) {
    int i;
    printf("\tMensagem:\n");
    printf("\tInit: %u | Len: %d | Seq: %d | Type: %d | Msg: '%s' | Par: %d \n", m->init, m->attr.len, m->attr.seq, m->attr.type, m->data, m->par);
    return 1;
}

char get_parity(Message *m) {
    /* This function will get a message and create a byte of parity of it. It will work in this way:
    I will store the xor from all LSB bits in v[7], the next in v[6], and so on, until the xor of the MSB bits
    is stored in v[1]. After that, I will dislocate them and create only one bit with the correct value.
    */
    int count,revcount,i,v[8] = [0,0,0,0,0,0,0,0], mask;
    int len = m->attr.len; // Just for saving words inside the code.
    char res = 0; // Do we need to use a cast?
    // Ill start with the MSB bits.
    for(count = 0,revcount = 7; count < 8; count++,revcount--) {
        printf("Counter: %d - Reverse Counter: %d\n",count,revcount);
        mask = 1;
        for(i=0; i<count; i++)
            mask = mask << 1; // Mask will be a number that changes from 0000 0001, 0000 0010, 0000 0100, ..., 1000 0000
        v[revcount] = ((m->attr.len & mask) ^ (m->attr.seq & mask) ^ (m->attr.type & mask)) // Here we got the xor from attrs.
        for(i=0; i < m->attr.len; i++) // Finishes the parity of the revcount bit by looking every data byte.
            v[revcount] = v[revcount & mask] ^ (m->data[i] & mask);
        // Parity is a bit or a byte? I mean, is it the horizontal parity with all the 8 bits I got?
        // Or is it all the 8 bits? I think its all the 8 bits, so, I will implement that way.
        // Anyway, we should still check it!
        res += v[revcount] * pow(2,count); // This should (not tested) move the bit to the <<
                                           // how many times it has to be moved.
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
