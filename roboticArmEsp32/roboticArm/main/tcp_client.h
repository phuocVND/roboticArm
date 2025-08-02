#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>


#include "structData.h"
// Định nghĩa TAG và hằng số (thay vì dùng ESP-IDF)
#define TCP_TAG "TCP_CLIENT"
#define TCP_SERVER_IP "192.168.1.99" // Thay bằng IP server của bạn
#define TCP_SERVER_PORT 8888
#define NUM_INTS 6
// typedef struct {
//     int id;
//     float value;
//     char name[32];
// } MyData;

// typedef struct {
//     short angle1;
//     short angle2;
//     short angle3;
//     short angle4;
//     short angle5;
//     short angle6;
// } ParameterAngle;

void send_array(int client_socket, short* arr, size_t size);
bool recv_array(int client_socket, short* arr, size_t size);
void send_struct(int client_socket, const MyData* data);
bool recv_struct(ParameterAngle* data);
int tcp_client_init(void);
int tcp_client_send(const char *data);
short tcp_client_receive_ints(short *int_array);
short tcp_client_send_ints(short *int_array);
int tcp_client_receive(char *buffer, size_t buffer_size);
void tcp_client_close(void);

#endif
