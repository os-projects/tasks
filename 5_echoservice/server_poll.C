#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <memory.h>
#include <unistd.h>
#include <vector>
#include <iostream>


using std::vector;


const size_t MAX_MESSAGE_SIZE(256);
const uint16_t SERVER_PORT(1337);


void get_new_connections(int listen_sd, vector<pollfd> &descriptors)
{
    while (true)
    {
        int client_desriptor = accept(listen_sd, NULL, NULL);
        if (client_desriptor < 0)
            return;

        pollfd new_client;
        new_client.fd = client_desriptor;
        new_client.events = POLLIN;
        descriptors.push_back(new_client);
    }
}


int main(int argc, char *argv[])
{
    int listen_sd = socket(AF_INET, SOCK_STREAM, 0);
    int on = 1;
    setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR, (void *)&on, sizeof(on));
    ioctl(listen_sd, FIONBIO, (char *)&on);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(SERVER_PORT);
    bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
    listen(listen_sd, 32);

    pollfd listen_socket;
    listen_socket.fd = listen_sd;
    listen_socket.events = POLLIN;
    vector<pollfd> descriptors = {listen_socket};


    while (true)
    {
        poll(descriptors.data(), descriptors.size(), 3 * 60 * 1000);  // 3 minutes

        for (size_t i = 1; i < descriptors.size(); ++i)
        {
            pollfd desc = descriptors[i];

            if (desc.revents == 0)
                continue;
            
            if (desc.revents != POLLIN)
                goto close_all;

            char buffer[MAX_MESSAGE_SIZE];
            ssize_t len = recv(desc.fd, buffer, MAX_MESSAGE_SIZE, 0);
            if (len <= 0)
            {
                close(desc.fd);
                desc.fd = -1;
            }
            send(desc.fd, buffer, len, 0);   
        }
        
        if (descriptors[0].revents == POLLIN)
            get_new_connections(listen_sd, descriptors);
        
        for (size_t i = 0; i < descriptors.size(); ++i)  // remove closed descriptors
        {
            if (descriptors[i].fd == -1)
            {
                descriptors[i] = descriptors.back();
                descriptors.pop_back();
            }
        }
    }
    
    close_all:
    for (size_t i = 0; i < descriptors.size(); ++i)
        if (descriptors[i].fd != -1)
            close(descriptors[i].fd);
    
    return EXIT_SUCCESS;
}




