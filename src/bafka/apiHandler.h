#pragma once

struct PutRecordRequest {
    std::string topic;
    size_t sizeOfRecords;
};

struct GetRecordRequest {
    std::string topic;
    MessageId messageId;
    size_t bufferSize;
};

struct GetRecordResponse {
    size_t numberOfRecords;
};


struct IRequestHandler {
    virtual void process(const PutRecordRequest& request) = 0;
    virtual void process(const GetRecordRequest& request) = 0;
    virtual bool consumeData(char* data, size_t size) = 0;
    virtual bool consumeMessage(int fd) = 0;
    virtual bool produceData(int fd) = 0;
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

    virtual ~IRequestBuilder() {
        free(handler);
    }
};
