#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <list>
#include <iostream>
#include <optional>
#include <chrono>
#include <tuple>

#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <idl/idl.capnp.h>
#include <memory>

using MessageId = size_t;

struct PutRecordRequest {
    std::string topic;
    size_t sizeOfRecords;
};

struct GetRecordRequest {
    std::string topic;
    MessageId messageId;
    size_t bufferSize;
};



struct BafkaConsumerIterator{
    char* buf;
    size_t size = 0;

    BafkaConsumerIterator(char *buf,size_t size)
    :buf(buf)
    ,size(size){}


    void next(){
        auto l = length();
        buf += l + sizeof(uint32_t);
        size -= l + sizeof(uint32_t);
    }

    bool hasNext(){
        if(size > sizeof(uint32_t)){
            if(size - sizeof(uint32_t) >= length()){
                return true;
            }
        }
        return false;
    }

    char* data(){
        return buf + sizeof(int32_t);
    }

    int32_t length(){
        return ntohl(*(uint32_t*)buf);
    }

};

struct BafkaRecord {
    virtual char* data() = 0;
    virtual uint32_t size() = 0;
    virtual ~BafkaRecord(){}
};

class BafkaProducer {
protected:
    int sock_client;
    std::list<std::unique_ptr<BafkaRecord>> queue;
    size_t queueSize = 0;
    std::string topic;

public:
    BafkaProducer(int sock_client, std::string topic):sock_client(sock_client), topic(std::move(topic)) {}

    void push(std::unique_ptr<BafkaRecord>&& record){
        queueSize += record->size() + sizeof(uint32_t);
        queue.push_back(std::move(record));
    }   

    void publish(){
        sendProduceMessage(topic, queueSize);
        for(auto& record : queue){
            auto size = record->size();
            uint32_t lengthN = htonl(size);
            send(sock_client, &lengthN, sizeof(lengthN) , 0);
            send(sock_client, record->data(), size , 0);
        }

        queueSize = 0;
        queue.clear();
    }
protected:
    virtual void sendProduceMessage(const std::string& topic, size_t size) = 0;

public:
    virtual ~BafkaProducer(){}
};

struct CapnpProtoBafkaProducer : public BafkaProducer {
    CapnpProtoBafkaProducer(int sock_client, std::string topic):BafkaProducer(sock_client, std::move(topic)){}

    void sendProduceMessage(const std::string& topic, size_t size) override {
        ::capnp::MallocMessageBuilder message;
        auto reader = message.initRoot<Cap::Message>();
        auto payload = reader.initPayload();
        auto putMessage = payload.initMessagePut();
        putMessage.setTopic(topic.c_str());
        putMessage.setSizeOfRecordsComming(size);

        auto flat = ::capnp::messageToFlatArray(message);
        auto size_flat = flat.asChars().size();
        send(sock_client, flat.asChars().begin(), size_flat , 0);
    }
};


class CapnpFlatBafkaRecord : public BafkaRecord {
    kj::Array<capnp::word> mData;
public:
    CapnpFlatBafkaRecord(kj::Array<capnp::word>&& data): mData(std::move(data)){}

    char* data() override {
        return mData.asChars().begin();
    };

    uint32_t size() override {
        return mData.asChars().size();
    };
};  


class BafkaRecords{
    char* buffer;
    size_t buf_size;

public:
    BafkaRecords(char* buffer, size_t buf_size):buffer(buffer), buf_size(buf_size) {}

    BafkaRecords(const BafkaRecords&) = delete;
    BafkaRecords(BafkaRecords&& r):buffer(r.buffer),buf_size(r.buf_size){
        r.buffer = nullptr;
    }

    BafkaConsumerIterator getIt(){
        return BafkaConsumerIterator(buffer, buf_size);
    }

    ~BafkaRecords(){
        if(buffer)
            free(buffer);
    }
};

class BafkaConsumer {
protected:
    std::string topic;
    int sock_client;

virtual void sendGetRequest(MessageId messageId, size_t buffer_size) =0;

public:
    BafkaConsumer(std::string topic, int sock_client):topic(std::move(topic)), sock_client(sock_client) {}

    BafkaRecords poll(MessageId messageId, size_t buffer_size){
        auto buffer = (char*) malloc(buffer_size);
        memset(buffer, 0, buffer_size);

        BafkaRecords records(buffer, buffer_size);

        sendGetRequest(messageId, buffer_size);
    
        int recieved;
        int totalRecieved = 0;
        while(totalRecieved < buffer_size && (recieved = recv(sock_client, buffer + totalRecieved, buffer_size - totalRecieved, 0)) > 0){
            totalRecieved += recieved;
        }

        return records;
    }
};


class CapnpBafkaConsumer : public BafkaConsumer {
public:
    CapnpBafkaConsumer(std::string topic, int sock_client): BafkaConsumer(std::move(topic), sock_client) {}

protected:
    void sendGetRequest(MessageId messageId, size_t buffer_size) override {
        ::capnp::MallocMessageBuilder message;
        auto reader = message.initRoot<Cap::Message>();
        auto payload = reader.initPayload();
        auto getMessage = payload.initMessageGet();
        getMessage.setTopic(topic);
        getMessage.setMessageId(messageId);
        getMessage.setBufferSize(buffer_size);

        auto flat = ::capnp::messageToFlatArray(message);
        send(sock_client, flat.asChars().begin(), flat.asChars().size() , 0);
    }
};


int main(int argc, char **argv) {
    std::cout << "start" << std::endl;

    auto ip = "127.0.0.1";

    struct sockaddr_in server_addr;  // set server addr and port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(3006);  // server default port

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

    if(false){
        ::capnp::MallocMessageBuilder message1;
        auto record1 = message1.initRoot<Cap::Record>();
        record1.setEventName("Sold");
        record1.setTime(323);
        auto flat1 = ::capnp::messageToFlatArray(message1);

        ::capnp::MallocMessageBuilder message2;
        auto record2 = message2.initRoot<Cap::Record>();
        record2.setEventName("Sold");
        record2.setTime(323);
        auto flat2 = ::capnp::messageToFlatArray(message2);
        
        CapnpProtoBafkaProducer producer(sock_client, "Hello World");
        producer.push(std::make_unique<CapnpFlatBafkaRecord>(std::move(flat2)));
        

        // auto content_size1 = flat1.asBytes().size();
        // auto content_size2 = flat1.asBytes().size();
        // auto total_content_size = content_size1 + content_size2 + sizeof(uint32_t)  + sizeof(uint32_t);

        // ::capnp::MallocMessageBuilder message;
        // auto reader = message.initRoot<Cap::Message>();
        // auto payload = reader.initPayload();
        // auto putMessage = payload.initMessagePut();
        // putMessage.setTopic("Hello World");
        // putMessage.setSizeOfRecordsComming(total_content_size);

        // auto flat = ::capnp::messageToFlatArray(message);
        // auto size = flat.asChars().end() - flat.asChars().begin();
        // send(sock_client, flat.asChars().begin(), size , 0);
        

        // uint32_t lengthN = htonl(content_size1);
        // send(sock_client, &lengthN, sizeof(lengthN) , 0);
        // send(sock_client, flat1.asChars().begin(), flat1.asChars().size() , 0);
        // send(sock_client, &lengthN, sizeof(lengthN) , 0);
        // send(sock_client, flat2.asChars().begin(), flat2.asChars().size() , 0);
    }

    {   
        
        CapnpBafkaConsumer consumer("Hello World", sock_client);

        // char buffer[100];
        // size_t buffer_size = 100;
        // memset(buffer, 0, buffer_size);

        // ::capnp::MallocMessageBuilder message;
        // auto reader = message.initRoot<Cap::Message>();
        // auto payload = reader.initPayload();
        // auto getMessage = payload.initMessageGet();
        // getMessage.setTopic("Hello World");
        // getMessage.setMessageId(0);
        // getMessage.setBufferSize(100);



        // auto flat = ::capnp::messageToFlatArray(message);
        // auto size = flat.asChars().end() - flat.asChars().begin();
        // send(sock_client, flat.asChars().begin(), size , 0);


        // int recieved;
        // int totalRecieved = 0;
        // std::cout << "Reading" << std::endl;
        // while(totalRecieved < buffer_size && (recieved = recv(sock_client, buffer + totalRecieved, buffer_size - totalRecieved, 0)) > 0){
        //     std::cout << "Recieved: " << recieved << std::endl;
        //     totalRecieved += recieved;
        // }

        // BafkaConsumerIterator it(buffer, totalRecieved);

        MessageId id = 0;
        size_t bufSize = 200;
        auto result = consumer.poll(id, bufSize);
        auto it = result.getIt();

        while(it.hasNext()){

            auto length = it.length();
            
            auto data = it.data();
            //align memory
            char* begin = (char*)malloc(length);
            memcpy(begin, data, length);

            auto end = begin + length;
            auto bufPtr = kj::ArrayPtr<const capnp::word>((const capnp::word*)begin, (const capnp::word*)end);
            capnp::FlatArrayMessageReader reader(bufPtr);
            auto record = reader.getRoot<Cap::Record>();

            std::cout << record.getEventName().cStr() <<std::endl;
            std::cout << record.getTime() <<std::endl;

            free(begin);

            it.next();
        }

    }

    // send a message to server
    close(sock_client);

    return 0;
}