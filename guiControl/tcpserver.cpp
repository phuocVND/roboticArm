#include "tcpServer.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cstring>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#else
#include <arpa/inet.h>
#include <errno.h>
#endif

// ------------------ Helper ------------------

static void close_socket(int sock) {
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

static std::string get_last_error() {
#ifdef _WIN32
    char* msg = nullptr;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL, WSAGetLastError(), 0, (LPSTR)&msg, 0, NULL);
    std::string err = msg ? msg : "Unknown error";
    LocalFree(msg);
    return err;
#else
    return std::strerror(errno);
#endif
}

// ------------------ Data Send/Recv ------------------

void send_array(int client_socket, short* arr, size_t size) {
    std::vector<short> network_arr(size);
    for (size_t i = 0; i < size; ++i)
        network_arr[i] = htons(arr[i]);

    ssize_t bytes_sent = send(client_socket, reinterpret_cast<const char*>(network_arr.data()), size * sizeof(short), 0);
    if (bytes_sent == -1)
        std::cerr << "Error sending array: " << get_last_error() << std::endl;
    else
        std::cout << "Array sent successfully!" << std::endl;
}

bool recv_array(int client_socket, short* arr, size_t size) {
    std::vector<short> buffer(size);
    ssize_t bytes_received = recv(client_socket, reinterpret_cast<char*>(buffer.data()), size * sizeof(short), 0);

    if (bytes_received > 0) {
        size_t elements_received = bytes_received / sizeof(short);
        for (size_t i = 0; i < elements_received; ++i)
            arr[i] = ntohs(buffer[i]);

        std::cout << "✅ Array received successfully!" << std::endl;
        return true;
    } else if (bytes_received == 0) {
        std::cout << "⚠️ Client disconnected!" << std::endl;
        return false;
    } else {
        std::cerr << "❌ Error receiving array: " << get_last_error() << std::endl;
        return false;
    }
}

void send_struct(int client_socket, const ParameterAngle *data) {
    ssize_t bytes_sent = send(client_socket, reinterpret_cast<const char*>(data), sizeof(*data), 0);
    if (bytes_sent == -1)
        std::cerr << "Error sending struct: " << get_last_error() << std::endl;
    else
        std::cout << "Struct sent successfully!" << std::endl;
}

// bool recv_struct(int client_socket, ParameterAngle& data) {
//     ssize_t bytes_received = recv(client_socket, reinterpret_cast<char*>(&data), sizeof(data), 0);
//     if (bytes_received > 0) {
//         std::cout << "Struct received successfully!" << std::endl;
//         // std::cout << "ID: " << data.id << ", Value: " << data.value << ", Name: " << data.name << std::endl;
//         return true;
//     } else if (bytes_received == 0) {
//         std::cout << "Client disconnected!" << std::endl;
//         return false;
//     } else {
//         std::cerr << "Error receiving struct: " << get_last_error() << std::endl;
//         return false;
//     }
// }

// ------------------ TcpServer Methods ------------------

TcpServer::TcpServer(const std::string& ip, short port, Parameter* parameter)
    : parameter(parameter)
{
    parameterAngle = new ParameterAngle;

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        exit(EXIT_FAILURE);
    }
#endif

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed: " << get_last_error() << std::endl;
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(ip.c_str());

    if (bind(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Bind failed: " << get_last_error() << std::endl;
        close_socket(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) == -1) {
        std::cerr << "Listen failed: " << get_last_error() << std::endl;
        close_socket(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening on " << ip << ":" << port << std::endl;
}

TcpServer::~TcpServer() {
    close_socket(server_fd);
#ifdef _WIN32
    WSACleanup();
#endif
}

void TcpServer::start_accept() {
    while (true) {
        int client_socket;
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);

        client_socket = accept(server_fd, (struct sockaddr*)&client_address, &client_len);
        if (client_socket == -1) {
            std::cerr << "Accept failed: " << get_last_error() << std::endl;
            continue;
        }

        std::cout << "Client connected!" << std::endl;

        std::thread t([this, client_socket]() {
            handle_client(client_socket);
            close_socket(client_socket);
        });
        t.detach();
    }
}

void TcpServer::handle_client(int client_socket) {
    char buffer[1024];
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        std::cout << "Client disconnected or error: " << (bytes_received == 0 ? "disconnected" : get_last_error()) << std::endl;
        return;
    }

    buffer[bytes_received] = '\0';
    std::cout << "Received from client: " << buffer << std::endl;
    parameterAngle = parameter->get_parameter();
    send_struct(client_socket, parameterAngle);
    while (true) {

        if (parameter->isChanged) {
            parameterAngle = parameter->get_parameter();
            send_struct(client_socket, parameterAngle);
        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Closing client connection" << std::endl;
}
