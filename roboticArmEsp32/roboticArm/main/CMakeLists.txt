set(requires esp_event nvs_flash driver esp_wifi)

idf_component_register(SRCS "udp_server.c" "main.c"
                            "tcp_client.c"
                            "udp_server.c"
                            "wifi_connect.c"
                            "servo_control.c"
                            "led_control.c"
                            "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${requires})
