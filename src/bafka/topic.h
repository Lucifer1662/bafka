#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include "message.h"
#include <filesystem>
#include <fcntl.h>


struct Topic{
    std::string topicName;
    std::vector<MessageId> startFiles;
    MessageId head = 0;

    Topic(std::string topicName);
    
    MessageId fileIn(MessageId id);

    std::string messageToFileName(MessageId id);

    size_t sendTo(int out_file, MessageId id, size_t length);

    void write(const char* data, size_t length);
};
