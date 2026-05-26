#include "chat.pb.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdint>

std::string encode_message(uint32_t message_type, google::protobuf::Message& message) {
    uint32_t message_length = message.ByteSizeLong();
    uint32_t total_length = 4 + message_length;
    std::string buffer(4 + total_length, '\0');
    uint32_t total_length_network = htonl(total_length);
    std::memcpy(&buffer[0], &total_length_network, 4);
    uint32_t message_type_network = htonl(message_type);
    std::memcpy(&buffer[4], &message_type_network, 4);
    if(!message.SerializeToArray(&buffer[8], message_length)) {
        std::cerr << "Failed to serialize message." << std::endl;
        return "";
    }
    return buffer;
}

std::string decode_message(int sockfd){
    char total_length[4];
    if(recv(sockfd, total_length, 4, 0) <= 0) {
        std::cerr << "Failed to read total length." << std::endl;
        close(sockfd);
        return "";
    }
    std::uint32_t total_length_network;
    std::memcpy(&total_length_network, total_length, 4);
    std::uint32_t total_length_host = ntohl(total_length_network);
    std::string buffer(total_length_host, '\0');
    ssize_t bytes_received = 0;
    while(bytes_received < total_length_host) {
        ssize_t result = recv(sockfd, &buffer[bytes_received], total_length_host - bytes_received, 0);
        if(result <= 0) {
            std::cerr << "Failed to read message." << std::endl;
            close(sockfd);
            return "";
        }
        bytes_received += result;
    }
    return buffer;
}

std::uint32_t send_message(std::uint32_t message_type, google::protobuf::Message& message, int sockfd){
    std::string encoded_message = encode_message(message_type, message);
    ssize_t bytes_sent = 0;
    while(bytes_sent < encoded_message.size()) {
        ssize_t result = send(sockfd, encoded_message.data() + bytes_sent, encoded_message.size() - bytes_sent, 0);
        if(result <= 0) {
            std::cerr << "Failed to send message." << std::endl;
            close(sockfd);
            return -1;
        }
        bytes_sent += result;
    }
    return 0;
}