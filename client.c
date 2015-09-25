void operate_client(int socket) {
    unsigned char *buffer;
    if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL)
        error("Unable to allocate memory.");
    while(1) {
        buffer = fgets(buffer, BUF_SIZE, stdin);
        buffer[strlen(buffer)-1] = '\0'; // Removing the \n
        puts("Sending...");
        Attr attrs = prepare_attr(strlen(buffer),1,TYPE_FILESIZE);
        Message *m;
        m = create_msg(attrs.len + 5);
        *m = prepare_msg(attrs, buffer);
        send_msg(socket, m);
        // Message sent. Waiting for response.
        puts("Waiting for response...");

        wait_response(socket);
        // Send next message.
    }
}
