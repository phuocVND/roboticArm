#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "esp_log.h"
#include "udp_server.h"

static const char *TAG = UDP_TAG;
static int udp_socket = -1;
static struct sockaddr_in server_addr;

bool udp_server_start(void) {
    udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (udp_socket < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return false;
    }

    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_SERVER_PORT);

    if (bind(udp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        close(udp_socket);
        return false;
    }

    ESP_LOGI(TAG, "UDP server listening on port %d", UDP_SERVER_PORT);
    return true;
}

int udp_server_receive(uint8_t *data, size_t max_len, char *from_ip, uint16_t *from_port) {
    struct sockaddr_in source_addr;
    socklen_t socklen = sizeof(source_addr);
    int len = recvfrom(udp_socket, data, max_len, 0, (struct sockaddr *)&source_addr, &socklen);
    if (len < 0) {
        ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
        return -1;
    }

    if (from_ip)
        strcpy(from_ip, inet_ntoa(source_addr.sin_addr));
    if (from_port)
        *from_port = ntohs(source_addr.sin_port);

    ESP_LOGI(TAG, "Received %d bytes from %s:%d", len, from_ip, *from_port);
    return len;
}

int udp_server_send(const uint8_t *data, size_t len, const char *dest_ip, uint16_t dest_port) {
    struct sockaddr_in dest_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(dest_port),
        .sin_addr.s_addr = inet_addr(dest_ip)
    };

    int sent = sendto(udp_socket, data, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (sent < 0) {
        ESP_LOGE(TAG, "Failed to send data: errno %d", errno);
        return -1;
    }

    ESP_LOGI(TAG, "Sent %d bytes to %s:%d", sent, dest_ip, dest_port);
    return sent;
}

void udp_server_close(void) {
    if (udp_socket != -1) {
        close(udp_socket);
        udp_socket = -1;
        ESP_LOGI(TAG, "UDP server closed");
    }
}
