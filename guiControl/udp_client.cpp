#include "udp_client.h"
#include <cstring>
#include <thread>
#include <iostream>

udp_client::udp_client(const std::string& ip, int port, Parameter *parameter)
    : parameter(parameter)
{
#ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        m_socket = -1;
        return;
    }
#endif

    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket < 0) {
        perror("Socket creation failed");
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    std::memset(&m_server_addr, 0, sizeof(m_server_addr));
    m_server_addr.sin_family = AF_INET;
    m_server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &m_server_addr.sin_addr) <= 0) {
        perror("Invalid address");
#ifdef _WIN32
        closesocket(m_socket);
        WSACleanup();
#else
        close(m_socket);
#endif
        m_socket = -1;
    }
}

udp_client::~udp_client()
{
    if (m_socket >= 0) {
#ifdef _WIN32
        closesocket(m_socket);
        WSACleanup();
#else
        close(m_socket);
#endif
    }
}

bool udp_client::send_data(ParameterAngle* data)
{
    if (m_socket < 0) return false;

    ssize_t sent = sendto(m_socket, (const char*)data, sizeof(ParameterAngle), 0,
                          (struct sockaddr*)&m_server_addr, sizeof(m_server_addr));
    return sent >= 0;
}

void udp_client::startConnect() {
    while (true) {
        if (parameter->isChanged) {
            parameterAngle = parameter->get_parameter();
            ParameterAngle data = *parameterAngle;

            data.angle1 = htons(data.angle1);
            data.angle2 = htons(data.angle2);
            data.angle3 = htons(data.angle3);
            data.angle4 = htons(data.angle4);
            data.angle5 = htons(data.angle5);
            data.angle6 = htons(data.angle6);

            send_data(&data);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
