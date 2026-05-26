#pragma once
#include "chat.pb.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

std::string encode_message(std::uint32_t message_type, google::protobuf::Message& message);
std::string decode_message(int sockfd);
std::uint32_t send_message(std::uint32_t message_type, google::protobuf::Message& message, int sockfd);