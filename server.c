void operate_server(int socket) {
    unsigned char *buffer;
    if((buffer = malloc(MAX_MSG_LEN)) == NULL)
        error("Error allocating memory.");
    Message *m;
    unsigned char par;
    int res = 0;
    while(1) {
        res = receive(socket, buffer, &m);
        if(res == 1) {
            printf("Parity received: %d\n",m->par);
            par = get_parity(m);
            if(par != m->par) {
                printf("\tError in parity! Please resend the message!\nSending nack...\n");
                send_nack(socket);
                printf("\tNack sent.");
            }
            else {
                puts("Sending acknowledge...");
                send_ack(socket); // now with "socket" parameter we missed!
                puts("Ack sent.");
            }
        }
    }
}
