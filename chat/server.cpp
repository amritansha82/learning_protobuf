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
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    if(bind(tcp_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        std::cerr << "Failed to bind socket" << std::endl;
        return 1;
    }
    if(listen(tcp_socket, 5) == -1){
        std::cerr << "Failed to listen on socket" << std::endl;
        return 1;
    }
    std::cout << "Server is listening on port " << port << std::endl;
    while(true){
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(tcp_socket, (struct sockaddr*)&client_addr, &client_len);
        if(client_socket == -1){
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }
        while(true){
            std::string message = decode_message(client_socket);
            std::uint32_t message_type;
            if(message.empty()){
                std::cerr << "Failed to decode message" << std::endl;
                close(client_socket);
                break;
            }
            std::memcpy(&message_type, message.data(), 4);
            message_type = ntohl(message_type);
            if(message_type == 0){
                SendMessage msg;
                msg.ParseFromString(message.substr(4));
                std::cout << "[" << msg.username() << "]: " << msg.text() << std::endl;
                SendMessageResponse response;
                response.set_success(true);
                send_message(1, response, client_socket);
            }
            else if(message_type == 2){
                UserJoined msg;
                msg.ParseFromString(message.substr(4));
                std::cout << msg.username() << " joined!!" << std::endl;
            }
            else{
                std::cerr << "Unknown message type: " << message_type << std::endl;
            }
        }

        close(client_socket);
    }


    return 0;
}