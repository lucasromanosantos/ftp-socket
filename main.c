#include "header.h"
#include "message.c"
#include "rawsocket.c"
#include "server.c"
#include "client.c"

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Error: Invalid number of arguments.\nYou should inform if its a client or a server.\n");
        exit(-1);
    }
    int socket;
    //socket = ConexaoRawSocket(DEVICE);
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    ///////////////////////////
    if(socket < 0)
        error("Error opening socket.");
    system("clear");
    printf("-- Running on %s mode...\n", argv[1]);

    register struct passwd *pw;
    register uid_t uid;

    uid = geteuid ();
    pw = getpwuid (uid);
    User = malloc(sizeof(char) * strlen(pw->pw_name) + 1);
    strcpy(User,pw->pw_name);
    Seq = 0;

    if((LocalPath = malloc(sizeof(unsigned char) * 1024)) == NULL)
        error("(main) Error allocating memory.");
    if((RemPath = malloc(sizeof(unsigned char) * 1024)) == NULL)
        error("(main) Error allocating memory.");

    LocalPath[0] = '.';
    LocalPath[1] = '/';
    LocalPath[2] = '\0';
    RemPath[0] = '.';
    RemPath[1] = '/';
    RemPath[2] = '\0';

    if(strcmp(argv[1], "client") == 0) {
        IsClient = 1;
        operate_client(socket);
    }
    else if(strcmp(argv[1], "server") == 0) {
        IsClient = 0;
        operate_server(socket);
    }
    return 1;
}
