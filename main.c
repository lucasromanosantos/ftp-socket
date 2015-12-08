#include "header.h"
#include "message.h"
#include "dir.h"
#include "files.h"
#include "client.h"
#include "server.h"
#include "utils.h"
#include "rawsocket.h"

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Error: Invalid number of arguments.\nYou should inform if its a client or a server.\n");
        exit(-1);
    }
    Socket = ConexaoRawSocket(DEVICE);
    if(Socket < 0)
        error("Error opening socket.");
    system("clear");
    printf("-- Running on %s mode...\n", argv[1]);

    register struct passwd *pw;
    register uid_t uid;

    if((LocalPath = malloc(sizeof(unsigned char) * 1024)) == NULL)
        error("(main) Error allocating memory.");
    if((RemPath = malloc(sizeof(unsigned char) * 1024)) == NULL)
        error("(main) Error allocating memory.");

    Log = 1;
    Log = 0;

    uid = geteuid ();
    pw = getpwuid (uid);
    User = malloc(sizeof(char) * strlen(pw->pw_name) + 1);
    strcpy(User,pw->pw_name);
    Seq = 0;
    LocalPath[0] = '.';
    LocalPath[1] = '/';
    LocalPath[2] = '\0';
    RemPath[0] = '.';
    RemPath[1] = '/';
    RemPath[2] = '\0';

    if(strcmp(argv[1], "client") == 0) {
        IsClient = 1;
        operate_client();
    }
    else if(strcmp(argv[1], "server") == 0) {
        IsClient = 0;
        operate_server();
    }
    return 1;
}
