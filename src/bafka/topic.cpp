#include "topic.h"
#include <unistd.h>
#include <iostream>
#include <cstring>

Topic::Topic(std::string topicName):topicName(std::move(topicName)){
    std::string path = this->topicName;
    try{
        for (const auto& entry : std::filesystem::directory_iterator(path)){
            auto fileName = entry.path().filename().c_str();
            startFiles.push_back(std::stol(fileName));
        }
    }catch(std::exception e){}
    std::sort(startFiles.begin(), startFiles.end());

    if(startFiles.size() > 0){
        auto fileId = startFiles.back();
        auto fileName = messageToFileName(fileId);
        auto file_fd = fopen(fileName.c_str(), "r");
        fseek(file_fd, 0L, SEEK_END);
        auto sz = ftell(file_fd);
        head = sz + fileId;
    }
}

MessageId Topic::fileIn(MessageId id){
    auto it = std::lower_bound(startFiles.begin(), startFiles.end(), id);
    if(it == startFiles.end())  
        return 0;
    else
        return *it;
}



std::string Topic::messageToFileName(MessageId id){
    return topicName + "/" + std::to_string(id) + ".bafka";
}



size_t Topic::sendTo(int out_file, MessageId id, size_t length){
    try{
    if(length == 0)
        return 0;
    auto fileId = fileIn(id);
    auto fileName = messageToFileName(fileId);
    off_t offset = id - fileId;
    auto file_fd = open(fileName.c_str(), O_RDONLY, S_IRUSR | S_IWUSR);
    if(file_fd != -1){
        auto written = sendfile64(out_file, file_fd, &offset, length);
        if(written == 0){
            char* zerobuf = (char*)malloc(length);
            memset(zerobuf, 0, length);
            send(out_file, zerobuf, length, 0);
            free(zerobuf);
            return length;
        }
        close(file_fd);
        return written;
    }
    }catch(std::exception e){
        return 0;
    }
    return 0;
}



void Topic::write(const char* data, size_t length){
    if(length == 0)
        return;
    auto fileId = fileIn(head);
    auto fileName = messageToFileName(fileId);
    std::filesystem::create_directory(topicName);
    auto file_fd = open(fileName.c_str(), O_CREAT | O_APPEND | O_RDWR, S_IRUSR | S_IWUSR);

    // write(file_fd, data, length);
    if(file_fd != -1){
        auto res = ::write(file_fd, data, length);
        close(file_fd);
    }
}





