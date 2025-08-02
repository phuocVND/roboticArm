#include "udp_server.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include <string.h>

static int sockfd = -1;

bool udp_server_start(void) {
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sockfd < 0) {
        ESP_LOGE(UDP_TAG, "Failed to create socket");
        return false;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int err = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err < 0) {
        ESP_LOGE(UDP_TAG, "Socket unable to bind: errno %d", errno);
        close(sockfd);
        sockfd = -1;
        return false;
    }

    ESP_LOGI(UDP_TAG, "UDP server listening on port %d", UDP_SERVER_PORT);
    return true;
}

int udp_server_receive(uint8_t *data, size_t max_len, char *from_ip, uint16_t *from_port) {
    struct sockaddr_in source_addr;
    socklen_t addr_len = sizeof(source_addr);

    int len = recvfrom(sockfd, data, max_len, 0, (struct sockaddr *)&source_addr, &addr_len);
    if (len < 0) {
        ESP_LOGE(UDP_TAG, "recvfrom failed: errno %d", errno);
        return -1;
    }

    if (from_ip) {
        inet_ntoa_r(source_addr.sin_addr, from_ip, INET_ADDRSTRLEN);
    }
    if (from_port) {
        *from_port = ntohs(source_addr.sin_port);
    }

    return len;
}

int udp_server_send(const uint8_t *data, size_t len, const char *dest_ip, uint16_t dest_port) {
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(dest_port);
    inet_pton(AF_INET, dest_ip, &dest_addr.sin_addr.s_addr);

    int sent = sendto(sockfd, data, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (sent < 0) {
        ESP_LOGE(UDP_TAG, "sendto failed: errno %d", errno);
    }
    return sent;
}

void udp_server_close(void) {
    if (sockfd >= 0) {
        close(sockfd);
        sockfd = -1;
        ESP_LOGI(UDP_TAG, "UDP socket closed");
    }
}
