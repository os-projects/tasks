#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <memory.h>
#include <unistd.h>
#include <set>

using std::set;


const size_t MAX_MESSAGE_SIZE(256);
const uint16_t SERVER_PORT(1337);


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

    fd_set master_set;
    FD_ZERO(&master_set);
    FD_SET(listen_sd, &master_set);
    set<int> descriptors = {listen_sd};

    while (true)
    {
        fd_set working_set = master_set;

        timeval timeout;
        timeout.tv_sec  = 3 * 60;
        timeout.tv_usec = 0;

        int maxDescriptor = *descriptors.rbegin();
        int readyCount = select(maxDescriptor + 1, &working_set, NULL, NULL, &timeout);
        if (readyCount == 0)
            continue;

        for (auto i = descriptors.begin(); i != descriptors.end(); ++i)
        {
            if (FD_ISSET(*i, &working_set))
            {
                if (*i == listen_sd)
                {
                    while (true)
                    {
                        int new_sd = accept(listen_sd, NULL, NULL);
                        if (new_sd == -1)
                            break;
                        FD_SET(new_sd, &master_set);
                        descriptors.insert(new_sd);
                    }
                }
                else
                {
                    char buffer[MAX_MESSAGE_SIZE];
                    ssize_t len = recv(*i, buffer, MAX_MESSAGE_SIZE, 0);
                    if (len > 0)
                    {
                        send(*i, buffer, len, 0);
                    }
                    else
                    {
                        close(*i);
                        FD_CLR(*i, &master_set);
                        descriptors.erase(i++);
                        --i;
                    }
                }
            }
        }
    }

    for (auto i = descriptors.begin(); i != descriptors.end(); ++i)
        if (FD_ISSET(*i, &master_set))
            close(*i);

    return EXIT_SUCCESS;
}
