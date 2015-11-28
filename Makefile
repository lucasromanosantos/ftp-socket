CC = gcc
#CFLAGS = -O2 -I.
#CFLAGS = -O2
CFLAGS = 
DEPS = header.h utils.h message.h rawsocket.h dir.h files.h client.h server.h
OBJ = main.o utils.o message.o rawsocket.o dir.o files.o client.o server.o

all: main

%.o: %.c $(DEPS)
#%.o: %.c %.h header.h
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.o *.~ *.swp core
