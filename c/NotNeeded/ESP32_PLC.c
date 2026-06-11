#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/twai.h"
#include "driver/gpio.h"
c

static const char *TAG = "WAGO_CAN";

#define RX_PIN 4
#define TX_PIN 5

void app_main(void)
{
    // Configure everything
    ESP_LOGI(TAG, "Starting WAGO CAN Receiver/Transmitter on ESP32...");

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS(); 
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        ESP_LOGI(TAG, "CAN Driver installed successfully.");
    } else {
        ESP_LOGE(TAG, "Failed to install CAN driver.");
        return;
    }

    if (twai_start() == ESP_OK) {
        ESP_LOGI(TAG, "CAN Driver started. Ready for two-way communication.");
    } else {
        ESP_LOGE(TAG, "Failed to start CAN driver.");
        return;
    }

    twai_message_t rx_message;
    
    // Setup for sending timer (sends message every 2000 ms)
    TickType_t last_send_time = xTaskGetTickCount();
    const TickType_t send_interval = pdMS_TO_TICKS(2000);

    while (1) {
        // RECEIVE FROM PLC
        // Wait 10 ticks & check send timer
        if (twai_receive(&rx_message, pdMS_TO_TICKS(10)) == ESP_OK) {
            ESP_LOGI(TAG, "Message received! ID: 0x%03X", (unsigned int)rx_message.identifier);
            if (!(rx_message.rtr)) {
                printf("Payload: ");
                for (int i = 0; i < rx_message.data_length_code; i++) {
                    printf("0x%02X ", rx_message.data[i]);
                }
                printf("\n\n");
            }
        }

        // SEND TO PLC
        if ((xTaskGetTickCount() - last_send_time) >= send_interval) {
            // Build message
            twai_message_t tx_message = {
                .identifier = 0x321,        // CAN ID for the PLC to listen to
                .extd = 0,                  // 0 is the standard 11-bit ID
                .rtr = 0,                   // 0 is the standard data frame
                .data_length_code = 4,      // Send 4 bytes
                .data = {0xAA, 0xBB, 0xCC, 0xDD} // Test data payload thing
            };

            // Verstuur het bericht
            if (twai_transmit(&tx_message, pdMS_TO_TICKS(10)) == ESP_OK) {
                ESP_LOGI(TAG, "Test message sent to PLC! (ID: 0x321)");
            } else {
                ESP_LOGE(TAG, "Failed to send message to PLC.");
            }
            
            // Reset timer
            last_send_time = xTaskGetTickCount();
        }
        
        vTaskDelay(pdMS_TO_TICKS(1)); // Watchdog
    }
}