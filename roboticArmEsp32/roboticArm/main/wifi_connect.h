#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"

// Cấu hình Wi-Fi
#define WIFI_SSID      "duyphuoc"      // Thay bằng SSID của Wi-Fi
#define WIFI_PASSWORD  "66666666"  // Thay bằng mật khẩu Wi-Fi
#define WIFI_MAX_RETRY 5                // Số lần thử kết nối lại
#define WIFI_TAG       "WIFI_CONNECT"

// Hàm khởi tạo Wi-Fi
esp_err_t wifi_connect_init(void);

// Hàm chờ kết nối Wi-Fi
esp_err_t wifi_wait_for_connection(void);

#endif // WIFI_CONNECT_H
