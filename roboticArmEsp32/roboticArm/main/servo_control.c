#include <stdio.h>
#include "driver/ledc.h"
#include "servo_control.h"

// Định nghĩa chân GPIO cho 6 servo (tránh xung đột với LED)
static const int servo_pins[NUM_SERVOS] = {4, 16, 17, 5, 18, 19};

// Định nghĩa kênh PWM cho 6 servo
static const ledc_channel_t servo_channels[NUM_SERVOS] = {
    LEDC_CHANNEL_0,
    LEDC_CHANNEL_1,
    LEDC_CHANNEL_2,
    LEDC_CHANNEL_3,
    LEDC_CHANNEL_4,
    LEDC_CHANNEL_5
};

// Định nghĩa thông số PWM
#define SERVO_PWM_FREQ 50 // Tần số PWM cho servo (50Hz)
#define SERVO_LEDC_TIMER LEDC_TIMER_0

// Hàm khởi tạo PWM cho 6 servo
void init_servos(void) {
    // Cấu hình timer cho LEDC
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = SERVO_PWM_FREQ,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = SERVO_LEDC_TIMER
    };
    ledc_timer_config(&ledc_timer);

    // Cấu hình kênh PWM cho từng servo
    for (int i = 0; i < NUM_SERVOS; i++) {
        ledc_channel_config_t ledc_channel = {
            .channel = servo_channels[i],
            .duty = 0,
            .gpio_num = servo_pins[i],
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .hpoint = 0,
            .timer_sel = SERVO_LEDC_TIMER
        };
        ledc_channel_config(&ledc_channel);
        printf("Servo %d initialized on GPIO %d\n", i, servo_pins[i]);
    }
}

// Hàm điều khiển góc servo (0-180 độ)
void servo_set_angle(uint8_t servo_id, uint8_t angle) {
    if (servo_id >= NUM_SERVOS) {
        printf("Invalid servo ID: %d\n", servo_id);
        return;
    }
    if (angle > 180) angle = 180; // Giới hạn góc

    // Servo PWM: 0.5ms (0°) đến 2.5ms (180°) trong chu kỳ 20ms (50Hz)
    uint32_t duty = (angle * (2500 - 500) / 180 + 500) * 1023 / 20000;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, servo_channels[servo_id], duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, servo_channels[servo_id]);

    // printf("Servo %d set to %d degrees\n", servo_id, angle);
}