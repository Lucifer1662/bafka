#include <arpa/inet.h>

#include <idl/idl.capnp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <sys/sendfile.h>
#include <string>
#include <vector>
#include "chrono"
#include <errno.h>
#include <unordered_map>
#include <algorithm>
#include <filesystem>
#include <unistd.h>

#include <csignal>
#include <stdlib.h>
#include <algorithm>
#include <bafka/topic.h>
#include <bafka/message.h>
#include <bafka/apiHandler.h>
#include <bafka/capnproto_api.h>
#include <bafka/printRequestHandler.h>

#include <fcntl.h>

/** Returns true on success, or false if there was an error */
bool SetSocketBlockingEnabled(int fd, bool blocking)
{
    if (fd < 0)
        return false;

#ifdef _WIN32
    unsigned long mode = blocking ? 0 : 1;
    return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return false;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
#endif
}

#define MAX_EVENTS 10

int listen_sock = 0; // server socket fd

int main(int argc, char *argv[])
{
    signal(SIGINT, [](int)
           {
        printf("KILL");
        std::cout << "KILL" << std::endl;
        if (listen_sock != 0) close(listen_sock); });

    struct sockaddr_in server_addr;   // server info struct
    server_addr.sin_family = AF_INET; // TCP/IP
    server_addr.sin_addr.s_addr =
        INADDR_ANY;                     // server addr--permit all connection
    server_addr.sin_port = htons(3006); // server port

    /* create socket fd with IPv4 and TCP protocal*/
    if ((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        return 1;
    }

    /* bind socket with server addr */
    if (bind(listen_sock, (struct sockaddr *)&server_addr,
             sizeof(struct sockaddr)) < 0)
    {
        perror("bind error");
        return 1;
    }

    /* listen connection request with a queue length of 20 */
    if (listen(listen_sock, 20) < 0)
    {
        perror("listen error");
        return 1;
    }

    printf("listen success.\n");

    std::unordered_map<int, std::unique_ptr<IRequestBuilder>> builders;

    struct epoll_event ev, events[MAX_EVENTS];
    int conn_sock, nfds, epollfd;

    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1)
    {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    for (;;)
    {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == listen_sock)
            {
                struct sockaddr_in client_addr;
                socklen_t length = sizeof(client_addr);
                // block on accept until positive fd or error
                conn_sock = accept(listen_sock, (struct sockaddr *)&client_addr,
                                   &length);
                if (conn_sock == -1)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                SetSocketBlockingEnabled(conn_sock, false);

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1)
                {
                    perror("epoll_ctl: conn_sock");
                    exit(EXIT_FAILURE);
                }

                auto builder = std::make_unique<CapProtoRequestBuilder>();
                builder->setHandler(new PrintRequestHandler());
                builders[conn_sock] = std::move(builder);
            }
            else
            {
                auto fd = events[n].data.fd;
                builders[fd]->read(fd);
            }
        }
    }

    printf("closed. \n");
    close(listen_sock);
    return 0;
}