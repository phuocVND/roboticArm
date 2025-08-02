#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "wifi_connect.h"
#include "tcp_client.h"
#include "led_control.h"
#include "servo_control.h"
#include "udp_server.h"


EventGroupHandle_t event_group;
#define BIT_Servo0_DONE    BIT0
#define BIT_Servo1_DONE    BIT1
#define BIT_Servo2_DONE    BIT2
#define BIT_Servo3_DONE    BIT3
#define BIT_Servo4_DONE    BIT4
#define BIT_Servo5_DONE    BIT5
#define BIT_DONE    BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5
#define BIT_READY    BIT6

typedef struct {
    uint8_t servo_index;
    uint8_t servo_angle;
    uint8_t servo_angle_front;
} ParamsServo;


typedef struct {
    ParameterAngle* parameterAngleFront;
    ParameterAngle* parameterAngle;

    ParamsServo* paramsServo1;
    ParamsServo* paramsServo2;
    ParamsServo* paramsServo3;
    ParamsServo* paramsServo4;
    ParamsServo* paramsServo5;
    ParamsServo* paramsServo6;
} Parametters;



// static SemaphoreHandle_t angles_mutex;

// Task điều khiển LED
void led_task(void *arg) {
    while (1) {
        led1_control(true);
        // vTaskDelay(500 / portTICK_PERIOD_MS);
        led1_control(false);
        led2_control(true);
        // vTaskDelay(500 / portTICK_PERIOD_MS);
        led2_control(false);
        led3_control(true);
        // vTaskDelay(500 / portTICK_PERIOD_MS);
        led3_control(false);
        // vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Task điều khiển Servo
void control_servo(uint8_t id, uint8_t angle, uint8_t angleFront) {
    if (angle == angleFront) {
        servo_set_angle(id, angle);
        return;
    }

    bool positive = angle > angleFront;

    if (positive) {
        for (uint8_t i = angleFront; i < angle; i += 3) {
            servo_set_angle(id, i);
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    } else {
        for (uint8_t i = angleFront; i > angle; i -= 3) {
            servo_set_angle(id, i);
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }

    // Đảm bảo kết thúc chính xác tại `angle`
    servo_set_angle(id, angle);
}

void setParamServo(ParamsServo* paramsServo, uint8_t id, uint8_t angle, uint8_t angleFront){
    paramsServo->servo_index = id;
    paramsServo->servo_angle = angle;
    paramsServo->servo_angle_front = angleFront;
}

// Task điều khiển Servo
void servo_task(void *arg) {
    while(1){
        EventBits_t bits = xEventGroupWaitBits(
            event_group,
            BIT_READY,   // các bit cần chờ
            pdFALSE,       // clear các bit sau khi nhận
            pdTRUE,       // chờ tất cả bit (AND). Nếu pdFALSE thì chỉ cần 1 bit (OR)
            portMAX_DELAY // thời gian chờ (block vĩnh viễn)
        );

        ParamsServo* paramsServo = (ParamsServo*)arg;
        control_servo(paramsServo->servo_index, paramsServo->servo_angle, paramsServo->servo_angle_front);
        // vTaskDelay(pdMS_TO_TICKS(20));
        switch (paramsServo->servo_index)
        {
        case 0:
            xEventGroupSetBits(event_group, BIT_Servo0_DONE);
            break;
        case 1:
            xEventGroupSetBits(event_group, BIT_Servo1_DONE);
            break;
        case 2:
            xEventGroupSetBits(event_group, BIT_Servo2_DONE);
            break;
        case 3:
            xEventGroupSetBits(event_group, BIT_Servo3_DONE);
            break;
        case 4:
            xEventGroupSetBits(event_group, BIT_Servo4_DONE);
            break;
        case 5:
            xEventGroupSetBits(event_group, BIT_Servo5_DONE);
            break;
        }
    }
    vTaskDelete(NULL);
}

void udp_task(void *arg) {
    Parametters *parametters = (Parametters*)arg;
    uint8_t buffer[128];
    char sender_ip[INET_ADDRSTRLEN];
    uint16_t sender_port;

    // if (!udp_server_start()) {
    //     ESP_LOGE("UDP", "Failed to start UDP server");
    //     vTaskDelete(NULL);
    //     return;
    // }

        while (1) {
        int len = udp_server_receive(buffer, sizeof(buffer), sender_ip, &sender_port);
        if (len > 0) {
            // deserialize buffer thành struct nếu cần
            memcpy(parametters->parameterAngle, buffer, sizeof(ParameterAngle));

            // Điều khiển servo dựa trên struct
            setParamServo(parametters->paramsServo1, 0, parametters->parameterAngle->angle1, parametters->parameterAngleFront->angle1);
            setParamServo(parametters->paramsServo2, 1, parametters->parameterAngle->angle2, parametters->parameterAngleFront->angle2);
            setParamServo(parametters->paramsServo3, 2, parametters->parameterAngle->angle3, parametters->parameterAngleFront->angle3);
            setParamServo(parametters->paramsServo4, 3, parametters->parameterAngle->angle4, parametters->parameterAngleFront->angle4);
            setParamServo(parametters->paramsServo5, 4, parametters->parameterAngle->angle5, parametters->parameterAngleFront->angle5);
            setParamServo(parametters->paramsServo6, 5, parametters->parameterAngle->angle6, parametters->parameterAngleFront->angle6);
            *parametters->parameterAngleFront = *parametters->parameterAngle;

            const char *reply = "UDP: OK";
            udp_server_send((const uint8_t *)reply, strlen(reply), sender_ip, sender_port);
        }
    }

    udp_server_close();
    vTaskDelete(NULL);
}



// Task xử lý TCP
void tcp_task(void *arg) {
    Parametters *parametters = (Parametters*)arg;
    while (1) {
        EventBits_t bits = xEventGroupWaitBits(
            event_group,
            BIT_DONE,   // các bit cần chờ
            pdTRUE,       // clear các bit sau khi nhận
            pdTRUE,       // chờ tất cả bit (AND). Nếu pdFALSE thì chỉ cần 1 bit (OR)
            portMAX_DELAY // thời gian chờ (block vĩnh viễn)
        );
        xEventGroupClearBits(event_group, BIT_READY);

        if(recv_struct(parametters->parameterAngle)){
            printf("Done\n");
        }

        setParamServo(parametters->paramsServo1, (uint8_t)0, (uint8_t)parametters->parameterAngle->angle1, (uint8_t)parametters->parameterAngleFront->angle1);
        setParamServo(parametters->paramsServo2, (uint8_t)1, (uint8_t)parametters->parameterAngle->angle2, (uint8_t)parametters->parameterAngleFront->angle2);
        setParamServo(parametters->paramsServo3, (uint8_t)2, (uint8_t)parametters->parameterAngle->angle3, (uint8_t)parametters->parameterAngleFront->angle3);
        setParamServo(parametters->paramsServo4, (uint8_t)3, (uint8_t)parametters->parameterAngle->angle4, (uint8_t)parametters->parameterAngleFront->angle4);
        setParamServo(parametters->paramsServo5, (uint8_t)4, (uint8_t)parametters->parameterAngle->angle5, (uint8_t)parametters->parameterAngleFront->angle5);
        setParamServo(parametters->paramsServo6, (uint8_t)5, (uint8_t)parametters->parameterAngle->angle6, (uint8_t)parametters->parameterAngleFront->angle6);
        *parametters->parameterAngleFront = *parametters->parameterAngle;

        xEventGroupSetBits(event_group, BIT_READY);

    }
    tcp_client_close();
    vTaskDelete(NULL);
}

void init(){
    //  // Khởi tạo LED và servo
    // init_leds();
    init_servos();

    // Khởi tạo Wi-Fi
    if (wifi_connect_init() != ESP_OK) {
        ESP_LOGE("MAIN", "Failed to initialize Wi-Fi");
        return;
    }
    // Thử lại kết nối Wi-Fi
    int retry_count = 0;
    const int max_retries = 5;
    while (wifi_wait_for_connection() != ESP_OK && retry_count < max_retries) {
        ESP_LOGW("MAIN", "Failed to connect to Wi-Fi, retrying (%d/%d)...", retry_count + 1, max_retries);
        retry_count++;
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    if (retry_count >= max_retries) {
        ESP_LOGE("MAIN", "Failed to connect to Wi-Fi after %d attempts", max_retries);
        return;
    }
    ESP_LOGI("MAIN", "Wi-Fi connected successfully");

}

void app_main(void) {
    init();

    event_group = xEventGroupCreate();
    if (event_group == NULL) {
        ESP_LOGE("MAIN", "Failed to create event group");
        return;
    }
    // Khởi tạo UDP client
    if (!udp_server_start()) {
    ESP_LOGE("UDP", "Failed to start UDP server");
    vTaskDelete(NULL);
    return;
}
    // // Khởi tạo TCP client
    // if (tcp_client_init() != ESP_OK) {
    //     ESP_LOGE("MAIN", "Failed to initialize TCP client");
    //     return;
    // }
    // const char *message = "Hello, Esp32 of Duy Phuoc!";
    // if (tcp_client_send(message) != ESP_OK) {
    //     ESP_LOGE("TCP_TASK", "Failed to send data");
    // }

    Parametters* parametters = (Parametters*)malloc(sizeof(Parametters));

    parametters->parameterAngle  = (ParameterAngle*)malloc(sizeof(ParameterAngle));

    parametters->parameterAngleFront  = (ParameterAngle*)malloc(sizeof(ParameterAngle));

    parametters->paramsServo1 = (ParamsServo*)malloc(sizeof(ParamsServo));
    parametters->paramsServo2 = (ParamsServo*)malloc(sizeof(ParamsServo));
    parametters->paramsServo3 = (ParamsServo*)malloc(sizeof(ParamsServo));
    parametters->paramsServo4 = (ParamsServo*)malloc(sizeof(ParamsServo));
    parametters->paramsServo5 = (ParamsServo*)malloc(sizeof(ParamsServo));
    parametters->paramsServo6 = (ParamsServo*)malloc(sizeof(ParamsServo));

    parametters->parameterAngle->angle1 = (uint8_t)90;
    parametters->parameterAngle->angle2 = (uint8_t)90;
    parametters->parameterAngle->angle3 = (uint8_t)90;
    parametters->parameterAngle->angle4 = (uint8_t)90;
    parametters->parameterAngle->angle5 = (uint8_t)90;
    parametters->parameterAngle->angle6 = (uint8_t)90;

    *parametters->parameterAngleFront = *parametters->parameterAngle;

    setParamServo(parametters->paramsServo1, (uint8_t)0, (uint8_t)parametters->parameterAngle->angle1, (uint8_t)parametters->parameterAngleFront->angle1);
    setParamServo(parametters->paramsServo2, (uint8_t)1, (uint8_t)parametters->parameterAngle->angle2, (uint8_t)parametters->parameterAngleFront->angle2);
    setParamServo(parametters->paramsServo3, (uint8_t)2, (uint8_t)parametters->parameterAngle->angle3, (uint8_t)parametters->parameterAngleFront->angle3);
    setParamServo(parametters->paramsServo4, (uint8_t)3, (uint8_t)parametters->parameterAngle->angle4, (uint8_t)parametters->parameterAngleFront->angle4);
    setParamServo(parametters->paramsServo5, (uint8_t)4, (uint8_t)parametters->parameterAngle->angle5, (uint8_t)parametters->parameterAngleFront->angle5);
    setParamServo(parametters->paramsServo6, (uint8_t)5, (uint8_t)parametters->parameterAngle->angle6, (uint8_t)parametters->parameterAngleFront->angle6);


    // xTaskCreate(
    //     tcp_task,         // Task function
    //     "TCP-TASK",       // Task name
    //     4096,          // Stack size (words, 1 word = 4 bytes)
    //     (void*)parametters,          // Task input parameter
    //     5,             // Priority
    //     NULL           // Task handle
    // );  
    xTaskCreate(udp_task, "UDP-TASK", 4096, (void*)parametters, 5, NULL);

    xTaskCreate(servo_task, "servo_task1", 2048, (void*)parametters->paramsServo1, 5, NULL);
    xTaskCreate(servo_task, "servo_task2", 2048, (void*)parametters->paramsServo2, 5, NULL);
    xTaskCreate(servo_task, "servo_task3", 2048, (void*)parametters->paramsServo3, 5, NULL);
    xTaskCreate(servo_task, "servo_task4", 2048, (void*)parametters->paramsServo4, 5, NULL);
    xTaskCreate(servo_task, "servo_task5", 2048, (void*)parametters->paramsServo5, 5, NULL);
    xTaskCreate(servo_task, "servo_task6", 2048, (void*)parametters->paramsServo6, 5, NULL);
    
    xEventGroupSetBits(event_group, BIT_DONE);

    // free(parametters->paramsServo1);
    // free(parametters->paramsServo2);
    // free(parametters->paramsServo3);
    // free(parametters->paramsServo4);
    // free(parametters->paramsServo5);
    // free(parametters->paramsServo6);
    // free(parametters->parameterAngleFront);
    // free(parametters->parameterAngle);
    // free(parametters);
    
}
