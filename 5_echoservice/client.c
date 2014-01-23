#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

const size_t MAXCOUNT = 1024;

int main(int argc, char* argv[])
{
    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr)); 
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    saddr.sin_family = AF_INET;
    inet_pton(AF_INET,"127.0.0.1", &saddr.sin_addr);
    saddr.sin_port = htons(1337);
    connect(sfd,(struct sockaddr*) &saddr, sizeof(saddr));
    
    while (1) {
        char msg[MAXCOUNT];
        memset(msg, 0, MAXCOUNT);
        char blanmsg[MAXCOUNT];
        memset(blanmsg, 0, MAXCOUNT);
        fgets(msg,MAXCOUNT, stdin);
        send(sfd,msg,strlen(msg), 0);
        recv(sfd,blanmsg,sizeof(blanmsg), 0);
        printf("%s", blanmsg);
        fflush(stdout);
    }
}

