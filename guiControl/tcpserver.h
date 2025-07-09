#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <parameter.h>

struct MyData {
    int id;
    float value;
    char name[50];
};
class TcpServer {
public:
    TcpServer(const std::string& ip, short port, Parameter *parameter);

    void start_accept();

private:
    Parameter *parameter;
    ParameterAngle *parameterAngle;
    void handle_client(int client_socket);

    int server_fd;
    struct sockaddr_in server_address;
};

#endif // TCPSERVER_Hs
