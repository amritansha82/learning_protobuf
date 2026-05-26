#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include "codec.h"

int main(int argc, char* argv[]){
    int port = argc > 1 ? std::stoi(argv[1]) : 8080;
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(tcp_socket == -1){
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address / Address not supported" << std::endl;
        return -1;
    }
    server_addr.sin_port = htons(port);
    if(connect(tcp_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to connect to server." << std::endl;
        close(tcp_socket);
        return 1;
    }
    std::string message;
    std::cout << "Enter your username: ";
    std::string username;
    std::getline(std::cin, username);
    UserJoined join_msg;
    join_msg.set_username(username);
    if(send_message(2, join_msg, tcp_socket) == -1){
        std::cerr << "Failed to send join message" << std::endl;
        return 1;
    }
    while(true){
        std::cout << "> ";
        std::getline(std::cin, message);
        if(message == "exit"){
            break;
        }
        SendMessage msg;
        msg.set_username(username);
        msg.set_text(message);
        if(send_message(0, msg, tcp_socket) == -1){
            std::cerr << "Failed to send message" << std::endl;
            break;
        }
        SendMessageResponse response;
        std::string response_str = decode_message(tcp_socket);
        if(response_str.empty()){
            std::cerr << "Failed to receive response" << std::endl;
            break;
        }
        response.ParseFromString(response_str.substr(4));
        if(response.success()){
            std::cout << "Server confirmed: True" << std::endl;
        }
    }
    close(tcp_socket);
    return 0;
}