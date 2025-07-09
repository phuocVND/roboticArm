#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include "parameter.h"
#include <netinet/in.h>
#include <string>

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
};

#endif // UDP_CLIENT_H
