#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <optional>
#include<chrono>

#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <idl/idl.capnp.h>


struct PutRecordRequest { std::string topic; };

struct GetRecordRequest {
    std::string topic;
    std::optional<std::chrono::nanoseconds> start;
    std::optional<std::chrono::nanoseconds> end;
};

int main(int argc, char **argv) {
    std::cout << "start" << std::endl;

    auto ip = "127.0.0.1";

    struct sockaddr_in server_addr;  // set server addr and port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(8000);  // server default port

    int sock_client;


    if ((sock_client = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return 0;
    }

    // connect server, return 0 with success, return -1 with error
    if (connect(sock_client, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        perror("connect");
        return 0;
    }

    char server_ip[INET_ADDRSTRLEN] = "";
    inet_ntop(AF_INET, &server_addr.sin_addr, server_ip, INET_ADDRSTRLEN);
    printf("connected server(%s:%d). \n", server_ip,
           ntohs(server_addr.sin_port));

    {

        const char *send_content = "I am client";
        auto content_size = strlen(send_content);

        std::cout << "Content size " << content_size << std::endl;

        ::capnp::MallocMessageBuilder message;
        auto reader = message.initRoot<Cap::Message>();
        auto payload = reader.initPayload();
        auto putMessage = payload.initMessagePut();
        putMessage.setTopic("Hello World");
        putMessage.setSizeOfRecordsComming(content_size);

        auto flat = ::capnp::messageToFlatArray(message);
        auto size = flat.asChars().end() - flat.asChars().begin();
        send(sock_client, flat.asChars().begin(), size , 0);
        send(sock_client, send_content, content_size, 0);

        capnp::FlatArrayMessageReader r(flat);
        // ::capnp::writeMessageToFd(sock_client, message);

    }

    // send a message to server
    close(sock_client);

    return 0;
}