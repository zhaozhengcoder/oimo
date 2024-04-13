#pragma once
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
#include "log.h"

namespace Oimo {

template<typename PbType>
bool pbToString(const PbType& message, std::string& output) {
    int serializedSize = message.ByteSizeLong();
    output.resize(serializedSize);
    return message.SerializeToArray(output.data(), serializedSize);
}

template<typename PbType>
bool stringToPb(const char* data, int size, PbType& message) {
    bool success = message.ParseFromArray(data, size);
    if (!success) {
        LOG_ERROR << "Failed to parse the protobuf message: " 
                  << message.InitializationErrorString();
    }
    return success;
}

template<typename PbType>
std::string pbEncode(PbType& message) {
    std::string pb_data;
    pbToString(message, pb_data);

    uint32_t pb_size = htonl(static_cast<uint32_t>(pb_data.size()));
    std::string length((char*)&pb_size, sizeof(pb_size));
    std::string output = length + pb_data;
    return output;
}

std::string getCurrentTime();

}