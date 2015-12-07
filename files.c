#include "files.h"
#include "utils.h"
#include "message.h"

unsigned int get_file_size(FILE *fp) {
    int sz = 0;
    fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    rewind(fp);
    return sz;
}

unsigned char* read_file(FILE *fp,unsigned int size) {
	// Size is the size of the file (return value from get_file_size(fp)).
	int i;
	unsigned char* c = malloc(sizeof(unsigned char) * (size + 1));
	for(i=0; i<size;) {
		i += fread(c+i,sizeof(unsigned char),size - i,fp);
	}
	return c;
}

void write_file(FILE *fp,unsigned char *c,int size) {
	// Size is the size of the file (return value from get_file_size(fp)).
	int i;
	for(i=0; i<size; i++)
		i += fwrite(c,sizeof(unsigned char),size-i,fp);
	return ;
}

int send_filesize(FILE* fp) {
	unsigned int length = get_file_size(fp);
	Message *m;
	Attr a;
	a = prepare_attr(sizeof(unsigned int),Seq,TYPE_FILESIZE);
	unsigned char *s = malloc(5 * sizeof(unsigned char));
	s[4] = '\0';
	//printf("The length is going to be %u",length);
	//s = memcpy(s,&length,4);
	memcpy(s,&length,4);
	m = prepare_msg(a,s);
	send_msg(m);
	while(!wait_response()) {
		send_msg(m);
	}
	memcpy(&length,m->data,4); // Deletar depois!
	//printf("M%densagem enviada com sucesso. Tam = %u\n",sizeof(unsigned int),length);
	//Seq = (Seq + 1) % 64; Send_msg increment seq counter
	free(m);
	return length;
}

FILE* open_file(char *args) {
	char *buffer;
    if((buffer = malloc(sizeof(char) * BUF_SIZE + 1)) == NULL)
        error("(open_file) Unable to allocate memory.");
    puts("What is the file path?");
    buffer = fgets(buffer, BUF_SIZE, stdin);
    buffer[strlen(buffer)-1] = '\0'; // Removing the \n
    // I have the file name, now I have to open it and return.
    FILE *fp;
    while((fp = fopen(buffer,"r")) == NULL) {
    	printf("Error opening file: %s",strerror(errno));
	    puts("Please, enter another file path.");
	    buffer = fgets(buffer, BUF_SIZE, stdin);
	    buffer[strlen(buffer)-1] = '\0';
    }
    free(buffer);
    return fp;
}

void receive_file2(FILE *fp) {
    int i, j, size; // mp is message pointer - it counts which message we are sending/receiving.
    Message *m;
    unsigned char *buf,par;

    buf = malloc(sizeof(unsigned char) * MAX_DATA_LEN);
    /*m = malloc(sizeof(Message *) * 3);
    for(i=0; i<3; i++)
        m[i] = malloc_msg(MAX_DATA_LEN);*/
    m = malloc_msg(MAX_DATA_LEN);

    while((receive(buf, &m, STD_TIMEOUT)) == 0); // Got a message.
    par = get_parity(m);
    print_message(m);

    while(((int)par != (int)m->par) || (m->attr.type != TYPE_FILESIZE)) {
        printf("Par: %d, mPar=%d, Type=\n",(int)par,(int)m->par);
        puts("(receive_file) Parity error or message was not the file size.");
        send_type(TYPE_NACK);
        while((receive(buf, &m, STD_TIMEOUT)) == 0); // Got a message.
        par = get_parity(m);
    }

    memcpy(&size,m->data,4);
    send_type(TYPE_ACK);

    i = 0;
    Seq = 0;
    //printf("I = %d, size = %d\n",i,size);
    while(i < size) {
        while((receive(buf, &(m), STD_TIMEOUT)) == 0); // Got a message.
        par = get_parity(m);
        //print_message(m);
        printf("I == %d, Size = %d\n",i,size);
        if((int)par != (int)m->par) {
            puts("(receive_file) Parity error or message.");
            send_data_type(TYPE_NACK, m->attr.seq);
        } else {
            for(j=0; j<m->attr.len;) // Write data received in file.
                j += fwrite(m->data + j,sizeof(unsigned char),m->attr.len-j,fp);
            i += m->attr.len;
            //send_type(TYPE_ACK);
            //Seq += 1;
        	Seq = (Seq + 1) % 63;
            if((Seq % 3) == 2 || i == size) {
                puts("Sending ack...");
                send_data_type(TYPE_ACK, Seq);
            }
        }
    }

    puts("Going to read the End Message.");
    Message *m2 = malloc_msg(MAX_DATA_LEN);
    // Read all messages, created and updated the file, I should receive a TYPE_END.
    while((receive(buf, &m2, STD_TIMEOUT)) == 0); // Got a message.
    par = get_parity(m2);
    while(((int)par != (int)m2->par) || (m2->attr.type != TYPE_END)) {
        puts("(receive_file) Parity error or message wasnt an end.");
        print_message(m2);
        send_type(TYPE_NACK);
        while((receive(buf, &m2, STD_TIMEOUT)) == 0);
        par = get_parity(m2);
    }
    /*while((receive(buf, &m2, STD_TIMEOUT)) == 0);
    par = get_parity(m2);
    print_message(m2);
    if(((int)par != (int)m2->par) || (m2->attr.type != TYPE_END)) {
        // This should be a while, sending nack and waiting for the right message.
        puts("(receive_file) Parity error or message wasnt an end.");
        print_message(m2);
        return ;
    }*/
    send_type(TYPE_ACK);
    fclose(fp);
    printf("Size = %d\n",size);
    return ;
}

int send_file2(FILE *fp,int len) {
    int nob = 0, i, window = 3, mc = 0, remaining; // mc is message pointer, it counts which message is in turn.
    int size, perc, totalLen = len, dataSent, completed = 0, valueChange = 1, seqGot, msgWaitAns = 0;
    unsigned char *c;
    Message **m, *aux;
    Attr a;

    if((m = malloc(window * sizeof(Message *))) == NULL) {
        error("(send_file) Unable to allocate memory.");
    }

    for(i = 0; i < window; ++i) {
        m[i] = malloc_msg(MAX_DATA_LEN);
    }

    if((c = malloc(sizeof(char) * 64)) == NULL) {
        error("(send_file) Unable to allocate memory.");
    }

    aux = malloc_msg(MAX_DATA_LEN);

    if(len < 10000) { // small size
        size = 0; // 0 means small size, 1 means medium size, 2 means big file.
        perc = 0;
    } else if(len >= 10000 && len <= 1000000) { // medium size
        size = 1;
        perc = len / 10; // Show on screen every 10% completed.
    } else {
        size = 2;
        perc = len / 100; // Calculate 1% from file_size to show percentage on screen.
    }

    Seq = 0;
    totalLen = len;
    printf("TotalLen = %d, Len = %d\n",totalLen,len);

    while(len > 0) {
        nob = 0;
        remaining = (len > MAX_DATA_LEN) ? MAX_DATA_LEN : len;
        while(nob < remaining)
            nob += fread(c + nob,1,MAX_DATA_LEN-nob,fp);
        a = prepare_attr(remaining, Seq, TYPE_PUT);
        m[mc] = prepare_msg(a, c);
        send_msg(m[mc]);
        printf("Sending -> ");
        print_message(m[mc]);
        mc = (mc + 1) % window;
        msgWaitAns++;
        //Seq += 1;
        Seq = (Seq + 1) % 63;
        len -= remaining;

        //print_progress(&dataSent, totalLen, len, size, &completed, &valueChange, perc);

        while(msgWaitAns >= window) { // I sent 3 messages. I am waiting for an ack to tell me they were alright.
            if(!wait_response_seq(&aux)) { // Got an nack.
                memcpy(&seqGot,aux->data,4); // Got an nack indicating this message had error.
                puts("Some problem occured.");
                for(i = 0; i < window; ++i) {
                    if(m[(mc + i) % window]->attr.seq == seqGot) { // Found the wrong message. Have to send it again.
                        send_msg(m[(mc + i) % window]);
                        printf("Sending again --> ");
                        print_message(m[(mc + i) % window]);
                        break ; // Send message, wait for a response.
                    } else {
                        msgWaitAns--; // This was OK. I can send another one.
                    }
                }
            } else { // Got an ack after sending 3 messages.
                memcpy(&seqGot,aux->data,4); // Got an ack indicating this message and those before it were OK.
                for(i = 0; i < window; ++i) {
                    msgWaitAns--;
                    if(seqGot == m[(mc+i) % window]->attr.seq) { // Look at bottom for proper comments explaining this.
                        break ;
                    }
                    if(i >= window) { // Received a message with a Seq that was not from any message I sent!
                        puts("(Send_file2) Panic!!");
                        return 0;
                    }
                }
                printf("Aux was: ");
                print_message(aux);
                printf("Got an ack to sequency %d, msgWaitAns = %d\n",seqGot,msgWaitAns);
            }
        }
    }
    printf("My length became 0 and WaitAns = %d.\n",msgWaitAns);
    while(msgWaitAns >= 0) { // I sent 3 messages. I am waiting for an ack to tell me they were alright.
        if(!wait_response_seq(&aux)) { // Got an nack.
        	puts("A!!");
            memcpy(&seqGot,aux->data,4); // Got an nack indicating this message had error.
            puts("B!!");
            for(i = 0; i < window; ++i) {
                if(m[(mc + i) % window]->attr.seq == seqGot) { // Found the wrong message. Have to send it again.
                    send_msg(m[(mc + i) % window]);
                    break ; // Send message, wait for a response.
                } else {
                    msgWaitAns--; // This was OK. I can send another one.
                }
            }
        } else { // Got an ack after sending 3 messages.
        	puts("A!!");
            memcpy(&seqGot,aux->data,4); // Got an ack indicating this message and those before it were OK.
            puts("B!!");
            for(i = 0; i < window; ++i) {
                msgWaitAns--;
                if(seqGot == m[(mc+i) % window]->attr.seq) { // Look at bottom for proper comments explaining this.
                    break ;
                }
                if(i >= window) { // Received a message with a Seq that was not from any message I sent!
                    puts("(Send_file2) Panic!!");
                    return 0;
                }
            }
        }
        printf("msgWaitAns = %d\n",msgWaitAns);
    }

    //puts("Gonna send type_end.");
    unsigned char s[1];
    s[0] = '\0';
    a = prepare_attr(0,Seq,TYPE_END);
    m[0] = prepare_msg(a,s);
    send_msg(m[0]);
    print_message(m[0]);
    while(!wait_response()) {
        send_msg(m[0]);
        print_message(m[0]);
    }
    free(c);
    for(i=0; i<3; i++) {
        free(m[i]);
    }
    free(m); // To do this free, do we have to do a for?
    free(aux);
    printf("TotalLen = %d, Len = %d\n",totalLen,len);
    return 1;
}

/*
mp = 0.
Prepare message 5 and send it. mp = 1.
Prepare message 6 and send it. mp = 2.
Prepare message 7 and send it. mp = 0.
I sent messages 5, 6 and 7.
Got an ACK from message 7.
My mp was pointing to 0.
So, if I check my messages 5, 6 and 7 (for i=0; i<3; i++) m[mp+i %3], the one which m[]->Seq == seqGot is the one I got an ack.
So, for everyone DIFFERENT, I can know it was OK.

For example, my ACK was from message 7. So, I check if message 5 has the same Seq. No. It means message 5 was ok. I can send another msg (8).
Same to message 6. Its different. So, its ok. I can send another message (9). Message 7 is the right one. So, I can send another one (10).
It means I sent 8, 9 and 10. Go into the loop again.

If I sent 5, 6 and 7 and got ACK from message 6. I check if message 5 has the same Seq. No. It means message 5 was ok. I can send another msg (8).
Message 6 is the right one. So, I can send another one (9). So, I already sent 7, 8 and 9. Go into that loop again.

If I sent 5, 6 and 7 and got ACK from message 5. Message 5 is the right one. So, I can send another one (8).
So, I already sent 6, 7 and 8. Go into that loop again.
*/