#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

// Định nghĩa số lượng servo và số góc
#define NUM_SERVOS 6
#define NUM_ANGLES 3

// Cấu trúc cho tham số servo
typedef struct {
    uint8_t servo_id;           // ID của servo (0-5)
    uint32_t angles[NUM_ANGLES]; // Mảng các góc
    uint32_t delay_ms;         // Thời gian delay (ms)
} ServoParams;

// Khai báo các hàm
void init_servos(void);
void servo_set_angle(uint8_t servo_id, uint8_t angle);

#endif // SERVO_CONTROL_H