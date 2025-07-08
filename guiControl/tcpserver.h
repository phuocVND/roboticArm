#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <string>
#include <parameter.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <BaseTsd.h>
#pragma comment(lib, "ws2_32.lib")
typedef SSIZE_T ssize_t;         // Định nghĩa lại ssize_t cho Windows
typedef int socklen_t;           // socklen_t không tồn tại trên Windows
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

struct MyData {
    uint8_t angle1;
    char name[50];
};

class TcpServer {
public:
    TcpServer(const std::string& ip, short port, Parameter* parameter);
    ~TcpServer();

    void start_accept();

private:
    void handle_client(int client_socket);

    Parameter* parameter;
    ParameterAngle* parameterAngle;

    int server_fd;
    struct sockaddr_in server_address;
};

#endif // TCPSERVER_H
