#include <arpa/inet.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <idl/idl.capnp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <sys/sendfile.h>
#include <string>
#include <vector>

#include "chrono"

struct PutRecordRequest {
    std::string topic;
    size_t sizeOfRecords;
};

struct GetRecordRequest {
    std::string topic;
    std::optional<std::chrono::nanoseconds> start;
    std::optional<std::chrono::nanoseconds> end;
};

struct GetRecordResponse {
    size_t numberOfRecords;
};

struct IRequestHandler {
    virtual void process(const PutRecordRequest& request) = 0;
    virtual void process(const GetRecordRequest& request) = 0;
    virtual void consumeData(char* data, size_t size) = 0;
    virtual size_t consumeMessage(int fd) = 0;
};

struct PrintRequestHandler : public IRequestHandler {
    int out_fd = 1;
    size_t sizeOfRecordsComing;

    void process(const PutRecordRequest& request) {
        //would normally set out_fd here based on request
        sizeOfRecordsComing = request.sizeOfRecords;
        std::cout << request.topic << std::endl;
    };

    void process(const GetRecordRequest& request) {
        std::cout << request.topic << std::endl;
    };

    void consumeData(char* data, size_t size){
        //assume data is being written
        write(out_fd, data, size);
    }

    size_t consumeMessage(int fd) {
        auto ret = sendfile(out_fd, fd, 0, sizeOfRecordsComing);
        sizeOfRecordsComing =- ret;
        return ret;
    }
};

struct IRequestBuilder {
    enum Mode {
        API,
        Consuming,
        Producing,
    };

   protected:
    IRequestHandler* handler;
    Mode mode = Mode::API;

   public:
    virtual void read(int socketfb) = 0;
    void setHandler(IRequestHandler* handler) { this->handler = handler; }
    Mode getMode() { return mode; }

    virtual ~IRequestBuilder() {}
};


struct CapProtoRequestBuilder : public IRequestBuilder {
    capnp::word recv_buf[65536];
    size_t bytesRead = 0;
    bool errored = false;
    size_t sizeOfRecordsComing;

    kj::ArrayPtr<const capnp::word> BufPtr() {
        auto begin = &recv_buf;
        auto end = begin + bytesRead;
        std::cout << end - begin << std::endl;
        return kj::ArrayPtr<const capnp::word>((const capnp::word*)begin,
                                               (const capnp::word*)end);
    }

    size_t expectedLeft() {
        return capnp::expectedSizeInWordsFromPrefix(BufPtr());
        // *
            //    sizeof(capnp::word);
    }

    void read(int socket) override {
        if (mode == Mode::API) {
            auto expected = expectedLeft();
            if (expected > bytesRead) {
                auto bytesLeft = expected - bytesRead;
                auto ret = recv(socket, recv_buf + bytesRead, bytesLeft * sizeof(::capnp::word), 0);
                if (ret == -1) {
                    errored = true;
                    return;
                }
                bytesRead += ret / sizeof(capnp::word);
            }

            expected = expectedLeft();

            if (expected <= bytesRead) {
                processMessage(expected);

                char temp[65536];
                memcpy(temp, recv_buf, 65536);
                memset(recv_buf, 0, 65536);
                memcpy(recv_buf, temp + bytesRead, bytesRead - expected);
                bytesRead -= expected;
            }
        } else if (mode == Mode::Consuming) {
            auto ret = handler->consumeMessage(socket);
            if (ret == -1) {
                errored = true;
                return;
            }
            bytesRead += ret;
        }
    }

    void processMessage(int size) {
        capnp::FlatArrayMessageReader reader(BufPtr());
        auto message = reader.getRoot<Cap::Message>();
        switch (message.getPayload().which()) {
            case Cap::Message::Payload::MESSAGE_PUT: {
                PutRecordRequest r;
                r.topic =
                    message.getPayload().getMessagePut().getTopic().cStr();
                // r.sizeOfRecords =  message.getPayload().getMessagePut().getSizeOfRecordsComming();
                handler->process(r);
                // handler->consumeData(recv_buf+size, bytesRead-size);
                mode = Mode::Producing;
            } break;
            case Cap::Message::Payload::MESSAGE_GET: {
                GetRecordRequest r;
                r.topic =
                    message.getPayload().getMessageGet().getTopic().cStr();
                handler->process(r);
                mode = Mode::Consuming;
            } break;
        }
    }
};

#include <fcntl.h>

/** Returns true on success, or false if there was an error */
bool SetSocketBlockingEnabled(int fd, bool blocking) {
    if (fd < 0) return false;

#ifdef _WIN32
    unsigned long mode = blocking ? 0 : 1;
    return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
#endif
}

#define MAX_EVENTS 10

#include <csignal>

int listen_sock = 0;  // server socket fd

int main(int argc, char* argv[]) {
    signal(SIGINT, [](int) {
        printf("KILL");
        std::cout << "KILL" << std::endl;
        if (listen_sock != 0) close(listen_sock);
    });

    struct sockaddr_in server_addr;    // server info struct
    server_addr.sin_family = AF_INET;  // TCP/IP
    server_addr.sin_addr.s_addr =
        INADDR_ANY;                      // server addr--permit all connection
    server_addr.sin_port = htons(8000);  // server port

    /* create socket fd with IPv4 and TCP protocal*/
    if ((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return 1;
    }

    /* bind socket with server addr */
    if (bind(listen_sock, (struct sockaddr*)&server_addr,
             sizeof(struct sockaddr)) < 0) {
        perror("bind error");
        return 1;
    }

    /* listen connection request with a queue length of 20 */
    if (listen(listen_sock, 20) < 0) {
        perror("listen error");
        return 1;
    }

    printf("listen success.\n");

    CapProtoRequestBuilder requestReader;
    PrintRequestHandler handler;

    requestReader.setHandler(&handler);

    struct epoll_event ev, events[MAX_EVENTS];
    int conn_sock, nfds, epollfd;

    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == listen_sock) {
                struct sockaddr_in client_addr;
                socklen_t length = sizeof(client_addr);
                // block on accept until positive fd or error
                conn_sock = accept(listen_sock, (struct sockaddr*)&client_addr,
                                   &length);
                if (conn_sock == -1) {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                SetSocketBlockingEnabled(conn_sock, false);

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
                    perror("epoll_ctl: conn_sock");
                    exit(EXIT_FAILURE);
                }
            } else {
                auto fd = events[n].data.fd;
                requestReader.read(fd);
            }
        }
    }

    printf("closed. \n");
    close(listen_sock);
    return 0;
}