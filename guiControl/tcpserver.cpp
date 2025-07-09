#include "tcpServer.h"
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <thread>
#include <cerrno>

void send_array(int client_socket, short* arr, size_t size) {
    std::vector<short> network_arr(size);

    for (size_t i = 0; i < size; ++i) {
        network_arr[i] = htons(arr[i]);
    }
    ssize_t bytes_sent = send(client_socket, network_arr.data(), size * sizeof(short), 0);

    if (bytes_sent == -1) {
        std::cerr << "Error sending array: " << strerror(errno) << std::endl;
    } else {
        std::cout << "Array sent successfully!" << std::endl;
    }
}

bool recv_array(int client_socket, short* arr, size_t size) {
    std::vector<short> buffer(size); // buffer tạm để nhận dữ liệu thô

    ssize_t bytes_received = recv(client_socket, buffer.data(), size * sizeof(short), 0);
    if (bytes_received > 0) {
        size_t elements_received = bytes_received / sizeof(short);
        for (size_t i = 0; i < elements_received; ++i) {
            arr[i] = ntohs(buffer[i]); // chuyển về host byte order
            std::cout << "arr[" << i << "] = " << arr[i] << std::endl;
        }
        std::cout << "✅ Array received successfully!" << std::endl;
        return true;
    } else if (bytes_received == 0) {
        std::cout << "⚠️ Client disconnected!" << std::endl;
        return false;
    } else {
        std::cerr << "❌ Error receiving array: " << strerror(errno) << std::endl;
        return false;
    }
}


void send_struct(int client_socket, const MyData& data) {
    ssize_t bytes_sent = send(client_socket, &data, sizeof(data), 0);
    if (bytes_sent == -1) {
        std::cerr << "Error sending struct: " << strerror(errno) << std::endl;
    } else {
        std::cout << "Struct sent successfully!" << std::endl;
    }
}

bool recv_struct(int client_socket, MyData& data) {
    ssize_t bytes_received = recv(client_socket, &data, sizeof(data), 0);
    if (bytes_received > 0) {
        std::cout << "Struct received successfully!" << std::endl;
        std::cout << "ID: " << data.id << std::endl;
        std::cout << "Value: " << data.value << std::endl;
        std::cout << "Name: " << data.name << std::endl;
        return true;
    } else if (bytes_received == 0) {
        std::cout << "Client disconnected!" << std::endl;
        return false;
    } else {
        std::cerr << "Error receiving struct: " << strerror(errno) << std::endl;
        return false;
    }
}

TcpServer::TcpServer(const std::string& ip, short port, Parameter *parameter) {

    this->parameter = parameter;
    parameterAngle = new ParameterAngle;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(ip.c_str());

    if (bind(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) == -1) {
        std::cerr << "Listen failed: " << strerror(errno) << std::endl;
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening on " << ip << ":" << port << std::endl;
}

void TcpServer::start_accept() {
    while (true) {
        int client_socket;
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);

        client_socket = accept(server_fd, (struct sockaddr*)&client_address, &client_len);
        if (client_socket == -1) {
            std::cerr << "Accept failed: " << strerror(errno) << std::endl;
            continue;
        }

        std::cout << "Client connected!" << std::endl;

        std::thread t([this, client_socket]() {
            handle_client(client_socket);
            close(client_socket); // Đóng socket trong luồng
        });
        t.detach();
    }
}

void TcpServer::handle_client(int client_socket) {
    const char* message = "Hello from TCP server!";
    if (send(client_socket, message, strlen(message), 0) == -1) {
        std::cerr << "Error sending welcome message: " << strerror(errno) << std::endl;
        return;
    }

    char buffer[1024];
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        std::cout << "Client disconnected or error: " << (bytes_received == 0 ? "disconnected" : strerror(errno)) << std::endl;
        return;
    }
    buffer[bytes_received] = '\0';
    std::cout << "Received from client: " << buffer << std::endl;

    while (true) {
        if(parameter->isChanged) {
            parameterAngle = parameter->get_parameter();
            short arr[6] = {parameterAngle->angle1,
                            parameterAngle->angle2,
                            parameterAngle->angle3,
                            parameterAngle->angle4,
                            parameterAngle->angle5,
                            parameterAngle->angle6};

            short size = sizeof(arr) / sizeof(arr[0]);

            send_array(client_socket, arr, size);

            short client_arr[size];
            if (!recv_array(client_socket, client_arr, size)) {
                break; // Thoát nếu client ngắt kết nối hoặc lỗi
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::cout << "Closing client connection" << std::endl;
}
