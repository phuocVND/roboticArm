#include "wifi_connect.h"
#include "nvs_flash.h"
#include <arpa/inet.h> 
static int s_retry_num = 0;
static bool s_wifi_connected = false;

// Xử lý sự kiện Wi-Fi
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(WIFI_TAG, "Retrying to connect to the AP");
        } else {
            ESP_LOGE(WIFI_TAG, "Failed to connect to the AP after %d retries", WIFI_MAX_RETRY);
            s_wifi_connected = false;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(WIFI_TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        s_wifi_connected = true;
    }
}

// Khởi tạo Wi-Fi
esp_err_t wifi_connect_init(void) {
    // Khởi tạo NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Khởi tạo TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());

    // Tạo default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Tạo netif cho Wi-Fi STA
    esp_netif_create_default_wifi_sta();

    // Lấy handle của netif
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    // Tắt DHCP client để dùng IP tĩnh
    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(netif));
    // Thiết lập thông tin IP tĩnh
    esp_netif_ip_info_t ip_info;
    ip_info.ip.addr = inet_addr("192.168.1.68");
    ip_info.gw.addr = inet_addr("192.168.1.1");           // Gateway
    ip_info.netmask.addr = inet_addr("255.255.255.0");    // Subnet mask

    ESP_ERROR_CHECK(esp_netif_set_ip_info(netif, &ip_info));

    // Khởi tạo Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Đăng ký event handler
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    // Cấu hình Wi-Fi
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    // Thiết lập chế độ STA và cấu hình
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(WIFI_TAG, "Wi-Fi initialized, connecting to SSID: %s", WIFI_SSID);
    return ESP_OK;
}

// Chờ kết nối Wi-Fi
esp_err_t wifi_wait_for_connection(void) {
    int timeout_ms = 30000; // Thời gian chờ tối đa 30 giây
    int wait_ms = 500;      // Thời gian chờ mỗi lần kiểm tra
    int elapsed_ms = 0;

    while (!s_wifi_connected && elapsed_ms < timeout_ms) {
        vTaskDelay(wait_ms / portTICK_PERIOD_MS);
        elapsed_ms += wait_ms;
    }

    if (!s_wifi_connected) {
        ESP_LOGE(WIFI_TAG, "Failed to connect to Wi-Fi within %d ms", timeout_ms);
        return ESP_FAIL;
    }

    ESP_LOGI(WIFI_TAG, "Wi-Fi connected successfully");
    return ESP_OK;
}