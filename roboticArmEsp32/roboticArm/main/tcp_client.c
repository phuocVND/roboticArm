#include "tcp_client.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>



static int sock = -1; // Socket file descriptor

// Gửi mảng float
void send_array(int client_socket, short* arr, size_t size) {
    ssize_t bytes_sent = send(client_socket, arr, size * sizeof(short), 0);
    if (bytes_sent == -1) {
        fprintf(stderr, "[%s] Error sending array: %s\n", TCP_TAG, strerror(errno));
    } else {
        printf("[%s] Array sent successfully!\n", TCP_TAG);
    }
}

// Nhận mảng int
bool recv_array(int client_socket, short* arr, size_t size) {
    ssize_t bytes_received = recv(client_socket, arr, size * sizeof(short), 0);
    if (bytes_received > 0) {
        printf("[%s] Array received successfully!\n", TCP_TAG);
        for (size_t i = 0; i < size; ++i) {
            printf("[%s] arr[%zu] = %d\n", TCP_TAG, i, arr[i]);
        }
        return true;
    } else if (bytes_received == 0) {
        printf("[%s] Client disconnected!\n", TCP_TAG);
        return false;
    } else {
        fprintf(stderr, "[%s] Error receiving array: %s\n", TCP_TAG, strerror(errno));
        return false;
    }
}

// Gửi struct
void send_struct(int client_socket, const MyData* data) {
    ssize_t bytes_sent = send(sock, data, sizeof(MyData), 0);
    if (bytes_sent == -1) {
        fprintf(stderr, "[%s] Error sending struct: %s\n", TCP_TAG, strerror(errno));
    } else {
        printf("[%s] Struct sent successfully!\n", TCP_TAG);
    }
}

// Nhận struct
bool recv_struct(ParameterAngle* data) {
    ssize_t bytes_received = recv(sock, data, sizeof(ParameterAngle), 0);
    if (bytes_received > 0) {
        return true;
    } else if (bytes_received == 0) {
        printf("[%s] Client disconnected!\n", TCP_TAG);
        return false;
    } else {
        fprintf(stderr, "[%s] Error receiving struct: %s\n", TCP_TAG, strerror(errno));
        return false;
    }
}

// Khởi tạo và kết nối tới server
int tcp_client_init(void) {
    struct sockaddr_in server_addr;

    // Tạo socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fprintf(stderr, "[%s] Failed to create socket: %s\n", TCP_TAG, strerror(errno));
        return -1;
    }

    // Cấu hình địa chỉ server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TCP_SERVER_PORT);
    if (inet_pton(AF_INET, TCP_SERVER_IP, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "[%s] Invalid server IP address\n", TCP_TAG);
        close(sock);
        sock = -1;
        return -1;
    }

    // Kết nối tới server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        fprintf(stderr, "[%s] Socket connect failed: %s\n", TCP_TAG, strerror(errno));
        close(sock);
        sock = -1;
        return -1;
    }

    printf("[%s] Connected to server %s:%d\n", TCP_TAG, TCP_SERVER_IP, TCP_SERVER_PORT);
    return 0;
}

// Gửi dữ liệu tới server
int tcp_client_send(const char *data) {
    if (sock < 0) {
        fprintf(stderr, "[%s] Socket not initialized\n", TCP_TAG);
        return -1;
    }

    int len = send(sock, data, strlen(data), 0);
    if (len < 0) {
        fprintf(stderr, "[%s] Error occurred during sending: %s\n", TCP_TAG, strerror(errno));
        return -1;
    }

    printf("[%s] Sent %d bytes: %s\n", TCP_TAG, len, data);
    return 0;
}

// Nhận mảng int
short tcp_client_receive_ints(short *array) {
    if (sock < 0) {
        fprintf(stderr, "[%s] Socket not initialized\n", TCP_TAG);
        return -1;
    }

    int total_bytes = NUM_INTS * sizeof(short); // 20 bytes cho 5 int
    uint8_t *buffer = (uint8_t *)array;
    int received = 0;

    while (received < total_bytes) {
        int len = recv(sock, buffer + received, total_bytes - received, 0);
        if (len < 0) {
            fprintf(stderr, "[%s] Error receiving data: %s\n", TCP_TAG, strerror(errno));
            return -1;
        } else if (len == 0) {
            printf("[%s] Server closed connection\n", TCP_TAG);
            return -1;
        }
        received += len;
    }

    printf("[%s] Received %d bytes (%d ints)\n", TCP_TAG, received, NUM_INTS);
    for (int i = 0; i < NUM_INTS; i++) {
        printf("[%s] Value int[%d] = %d\n", TCP_TAG, i, array[i]);
    }

    return 0;
}

// Nhận mảng int
short tcp_client_send_ints(short *array) {
    if (sock < 0) {
        fprintf(stderr, "[%s] Socket not initialized\n", TCP_TAG);
        return -1;
    }

    int total_bytes = NUM_INTS * sizeof(short); // 20 bytes cho 5 int
    uint8_t *buffer = (uint8_t *)array;
    int received = 0;

    while (received < total_bytes) {
        int len = send(sock, buffer + received, total_bytes - received, 0);
        if (len < 0) {
            fprintf(stderr, "[%s] Error receiving data: %s\n", TCP_TAG, strerror(errno));
            return -1;
        } else if (len == 0) {
            printf("[%s] Server closed connection\n", TCP_TAG);
            return -1;
        }
        received += len;
    }

    printf("[%s] Received %d bytes (%d ints)\n", TCP_TAG, received, NUM_INTS);
    for (int i = 0; i < NUM_INTS; i++) {
        printf("[%s] Value int[%d] = %d\n", TCP_TAG, i, array[i]);
    }

    return 0;
}


// Nhận dữ liệu từ server
int tcp_client_receive(char *buffer, size_t buffer_size) {
    if (sock < 0) {
        fprintf(stderr, "[%s] Socket not initialized\n", TCP_TAG);
        return -1;
    }

    int len = recv(sock, buffer, buffer_size - 1, 0);
    if (len < 0) {
        fprintf(stderr, "[%s] Error occurred during receiving: %s\n", TCP_TAG, strerror(errno));
        return -1;
    } else if (len == 0) {
        printf("[%s] Connection closed by server\n", TCP_TAG);
        return -1;
    }

    buffer[len] = '\0'; // Thêm ký tự kết thúc chuỗi
    printf("[%s] Received %d bytes: %s\n", TCP_TAG, len, buffer);
    return 0;
}

// Đóng kết nối
void tcp_client_close(void) {
    if (sock >= 0) {
        close(sock);
        sock = -1;
        printf("[%s] Socket closed\n", TCP_TAG);
    }
}
