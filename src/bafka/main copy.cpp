#include <arpa/inet.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <idl/idl.capnp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <optional>
#include <string>

struct PutRecordRequest {
    std::string topic;
};

struct GetRecordRequest {
    std::string topic;
    std::optional<std::chrono::nanoseconds> start;
    std::optional<std::chrono::nanoseconds> end;
};

struct IRequestHandler {
    virtual void process(const PutRecordRequest& request) = 0;
    virtual void process(const GetRecordRequest& request) = 0;
};

struct PrintRequestHandler : public IRequestHandler {
    void process(const PutRecordRequest& request) {
        std::cout << request.topic << std::endl;
    };

    void process(const GetRecordRequest& request) {
        std::cout << request.topic << std::endl;
    };
};

struct IRequestBuilder {
   protected:
    IRequestHandler* handler;

   public:
    virtual void read(int socketfb) = 0;
    void setHandler(IRequestHandler* handler) { this->handler = handler; }

    virtual ~IRequestBuilder() {}
};

struct CapProtoRequestBuilder : public IRequestBuilder {
    char recv_buf[65536];
    size_t bytesRead = 0;
    bool errored = false;

    kj::ArrayPtr<const capnp::word> BufPtr() {
        auto begin = &recv_buf;
        auto end = begin + bytesRead;
        return kj::ArrayPtr<const capnp::word>((const capnp::word*)begin,
                                               (const capnp::word*)end);
    }

    size_t expectedLeft() {
        return capnp::expectedSizeInWordsFromPrefix(BufPtr()) *
               sizeof(capnp::word);
    }

    void read(int socket) override {
        auto expected = expectedLeft();
        if (expected > bytesRead) {
            auto bytesLeft = expected - bytesRead;
            auto ret = recv(socket, recv_buf + bytesRead, bytesLeft, 0);
            if (ret == -1) {
                errored = true;
                return;
            }
            bytesRead += ret;
        }

        expected = expectedLeft();

        if (expected <= bytesRead) {
            processMessage();

            char temp[65536];
            memcpy(temp, recv_buf, 65536);
            memset(recv_buf, 0, 65536);
            memcpy(recv_buf, temp + bytesRead, bytesRead - expected);
            bytesRead -= expected;
        }
    }

    void processMessage() {
        capnp::FlatArrayMessageReader reader(BufPtr());
        auto message = reader.getRoot<Cap::Message>();
        std::cout << (message.getPayload().hasMessageGet() ? "Get" : "NoGet")
                  << std::endl;
        std::cout << (message.getPayload().hasMessagePut() ? "Put" : "NoPut")
                  << std::endl;
        switch (message.getPayload().which()) {
            case Cap::Message::Payload::MESSAGE_PUT: {
                PutRecordRequest r;
                r.topic =
                    message.getPayload().getMessagePut().getTopic().cStr();
                handler->process(r);
            } break;
            case Cap::Message::Payload::MESSAGE_GET: {
                GetRecordRequest r;
                r.topic =
                    message.getPayload().getMessageGet().getTopic().cStr();
                handler->process(r);
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

int main(int argc, char* argv[]) {
    int server_sockfd;                 // server socket fd
    struct sockaddr_in server_addr;    // server info struct
    server_addr.sin_family = AF_INET;  // TCP/IP
    server_addr.sin_addr.s_addr =
        INADDR_ANY;                      // server addr--permit all connection
    server_addr.sin_port = htons(8000);  // server port

    /* create socket fd with IPv4 and TCP protocal*/
    if ((server_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return 1;
    }

    /* bind socket with server addr */
    if (bind(server_sockfd, (struct sockaddr*)&server_addr,
             sizeof(struct sockaddr)) < 0) {
        perror("bind error");
        return 1;
    }

    /* listen connection request with a queue length of 20 */
    if (listen(server_sockfd, 20) < 0) {
        perror("listen error");
        return 1;
    }

    printf("listen success.\n");

    // char recv_buf[65536];
    // memset(recv_buf, '\0', sizeof(recv_buf));

    CapProtoRequestBuilder requestReader;
    PrintRequestHandler handler;

    requestReader.setHandler(&handler);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
        // block on accept until positive fd or error
        int conn =
            accept(server_sockfd, (struct sockaddr*)&client_addr, &length);
        if (conn < 0) {
            perror("connect");
            return -1;
        }

        // SetSocketBlockingEnabled(conn, false);

        printf("new client accepted.\n");

        char client_ip[INET_ADDRSTRLEN] = "";
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

        while (true) {
            requestReader.read(conn);
        }

        // while (recv(conn, recv_buf, sizeof(recv_buf), 0) > 0) {
        //     printf("recv %s from client(%s:%d). \n", recv_buf, client_ip,
        //            ntohs(client_addr.sin_port));
        //     memset(recv_buf, '\0', strlen(recv_buf));
        //     break;
        // }
    }

    printf("closed. \n");
    close(server_sockfd);
    return 0;
}