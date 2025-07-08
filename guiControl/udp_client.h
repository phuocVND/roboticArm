#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include "parameter.h"
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
typedef int socklen_t;
#pragma comment(lib, "ws2_32.lib")
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif


class udp_client
{
public:
    udp_client(const std::string& ip, int port, Parameter *parameter);
    ~udp_client();

    bool send_data(ParameterAngle *data);
    void startConnect();

private:
    Parameter *parameter;
    ParameterAngle *parameterAngle;
    int m_socket;
    struct sockaddr_in m_server_addr;

#ifdef _WIN32
    WSADATA wsaData;
#endif
};

#endif // UDP_CLIENT_H
