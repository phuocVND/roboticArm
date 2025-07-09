#include <stdio.h>
#include "driver/gpio.h"
#include "led_control.h"

// Định nghĩa chân GPIO cho LED
#define LED1_PIN 12
#define LED2_PIN 14
#define LED3_PIN 27

// Hàm khởi tạo GPIO cho LED
void init_leds(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED1_PIN) | (1ULL << LED2_PIN) | (1ULL << LED3_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    printf("LEDs initialized on GPIO %d, %d, %d\n", LED1_PIN, LED2_PIN, LED3_PIN);
}

// Hàm bật/tắt LED1
void led1_control(bool state) {
    gpio_set_level(LED1_PIN, state);
    // printf("LED1 %s\n", state ? "ON" : "OFF");
}

// Hàm bật/tắt LED2
void led2_control(bool state) {
    gpio_set_level(LED2_PIN, state);
    // printf("LED2 %s\n", state ? "ON" : "OFF");
}

// Hàm bật/tắt LED3
void led3_control(bool state) {
    gpio_set_level(LED3_PIN, state);
    // printf("LED3 %s\n", state ? "ON" : "OFF");
}