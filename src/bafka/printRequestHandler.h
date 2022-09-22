#pragma once
#include "apiHandler.h"
#include <cstdlib>
#include <string>
#include <iostream>
#include <ostream>
#include <algorithm>
#include "topic.h"
#include "message.h"
#include <variant>



struct PrintRequestHandler : public IRequestHandler {
    static size_t BUF_SIZE;
    char* recv_buf = (char*)std::malloc(BUF_SIZE);

    std::variant<PutRecordRequest, GetRecordRequest> mRequest;

    void process(const PutRecordRequest& request) {
        mRequest = request;
        std::cout << request.topic << std::endl;
    };

    void process(const GetRecordRequest& request) {
        mRequest = request;
        std::cout << request.topic << std::endl;
    };

    bool consumeData(char* data, size_t size){
        Topic top(std::get<PutRecordRequest>(mRequest).topic);
        top.write(data, size);
        std::get<PutRecordRequest>(mRequest).sizeOfRecords -= size;
        
        return std::get<PutRecordRequest>(mRequest).sizeOfRecords == 0;
    }

    bool consumeMessage(int fd) {
        int ret;
        auto& request = std::get<PutRecordRequest>(mRequest);
        while((ret = recv(fd, recv_buf, std::min(BUF_SIZE, request.sizeOfRecords), 0)) > 0){
            consumeData(recv_buf, ret);
        }
        
        return std::get<PutRecordRequest>(mRequest).sizeOfRecords == 0;
    }

    bool produceData(int fd){
        auto& request = std::get<GetRecordRequest>(mRequest);
        Topic top(request.topic);
        auto sent = top.sendTo(fd, request.messageId, request.bufferSize);
        request.bufferSize -= sent;
        request.messageId += sent;
        return request.bufferSize == 0;
    }
};

size_t PrintRequestHandler::BUF_SIZE = 65536;