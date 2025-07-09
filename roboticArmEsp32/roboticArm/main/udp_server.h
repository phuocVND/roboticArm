#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include <stddef.h>
#include <stdbool.h>

#define UDP_TAG "UDP_SERVER"
#define UDP_SERVER_PORT 8888
#define NUM_INTS 6

typedef struct {
    int id;
    float value;
    char name[32];
} MyData;

typedef struct {
    short angle1;
    short angle2;
    short angle3;
    short angle4;
    short angle5;
    short angle6;
} ParameterAngle;

bool udp_server_start(void); // bắt đầu server
int udp_server_receive(uint8_t *data, size_t max_len, char *from_ip, uint16_t *from_port);
int udp_server_send(const uint8_t *data, size_t len, const char *dest_ip, uint16_t dest_port);

void udp_server_close(void);

#endif
