#pragma once
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <capnp/common.h>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <cstring>
#include <sys/socket.h>
#include "apiHandler.h"
#include <idl/idl.capnp.h>


struct CapProtoRequestBuilder : public IRequestBuilder {
    char* recv_buf = (char*)std::malloc(65536);
    size_t bytesRead = 0;
    bool errored = false;


    ~CapProtoRequestBuilder(){
        free(recv_buf);
    }

    kj::ArrayPtr<const capnp::word> BufPtr() {
        auto begin = &recv_buf[0];
        auto end = &begin[0] + bytesRead;
        return kj::ArrayPtr<const capnp::word>((const capnp::word*)begin,
                                               (const capnp::word*)end);
    }

    size_t expectedLeft() {
        return capnp::expectedSizeInWordsFromPrefix(BufPtr()) *
               sizeof(capnp::word);
    }

    void read(int socket) override {
        int ret = 1;
        while(ret > 0)
        if (mode == Mode::API) {
            auto expected = expectedLeft();
            if (expected > bytesRead) {
                auto bytesLeft = expected - bytesRead;
                ret = recv(socket, recv_buf + bytesRead, bytesLeft, 0);
                if (ret == -1) {
                    errored = true;
                    return;
                }
                bytesRead += ret;
            }

            if(ret > 0){
            expected = expectedLeft();

            if (expected <= bytesRead) {
                processMessage(expected);

                char temp[65536];
                memcpy(temp, &recv_buf[0], 65536);
                memset(&recv_buf[0], 0, 65536);
                memcpy(&recv_buf[0], temp + bytesRead, bytesRead - expected);
                bytesRead -= expected;
            }
            }
        } else if (mode == Mode::Producing) {
            if(handler->consumeMessage(socket)){
                mode = Mode::API;
            }
        } else if (mode == Mode::Consuming){
            if(handler->produceData(socket)){
                mode = Mode::API;
            }
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
                r.sizeOfRecords =  message.getPayload().getMessagePut().getSizeOfRecordsComming();
                handler->process(r);
                bool finished = handler->consumeData(recv_buf+size, bytesRead-size);
                if(!finished)
                    mode = Mode::Producing;
            } break;
            case Cap::Message::Payload::MESSAGE_GET: {
                GetRecordRequest r;
                r.topic = message.getPayload().getMessageGet().getTopic().cStr();
                r.bufferSize = message.getPayload().getMessageGet().getBufferSize();
                r.messageId = message.getPayload().getMessageGet().getMessageId();
                handler->process(r);
                mode = Mode::Consuming;
            } break;
        }
    }
};
