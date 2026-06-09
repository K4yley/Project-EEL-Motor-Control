#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "driver/twai.h"
#include "esp_log.h"

// UART Pico
#define UART_PORT       UART_NUM_1
#define UART_TX_PIN     43
#define UART_RX_PIN     44
#define UART_BAUD_RATE  115200
#define BUF_SIZE        256

// TWAI CAN PLC
static const char *TAG = "WAGO_CAN";
#define RX_PIN 4
#define TX_PIN 5

void setup() {
    // Configure UART for Pico
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0
    };
  
    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);

    ESP_LOGI(TAG, "UART1 started on TX=%d RX=%d at %d baud", UART_TX_PIN, UART_RX_PIN, UART_BAUD_RATE);

    // Configure TWAI for PLC
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
}

void app_main(void) {
    setup();

    twai_message_t rx_message;
    
    twai_message_t tx_message = {
        .identifier = 0x321,        // Outbound target CAN ID
        .extd = 0,                  // Standard 11-bit ID
        .rtr = 0,                   // Data frame
        .data_length_code = 28,      // Expecting 28 bytes back from Pico
        .data = {0}
    };

    while (1) {
        // Recieving from PLC
        if (twai_receive(&rx_message, pdMS_TO_TICKS(10)) == ESP_OK) {       //small timeout
            if (!(rx_message.rtr)) {        //check if it not empty, has usable data
                ESP_LOGI(TAG, "Data from PLC! ID: 0x%03X, Sending to Pico...", (unsigned int)rx_message.identifier);
                
                //immidelate send to pico, 4 packetjes, dus let op
                uart_write_bytes(UART_PORT, rx_message.data, rx_message.data_length_code);
            }
        }

        //Recieving from Pico
        int bytes_read = uart_read_bytes(UART_PORT, tx_message.data, tx_message.data_length_code, pdMS_TO_TICKS(10));
        
        if (bytes_read > 0) {
            ESP_LOGI(TAG, "Data from Pico! Received %d bytes.", bytes_read);
            
            // Adjust the length
            tx_message.data_length_code = bytes_read;

            // Transmit it onto the CAN bus to the PLC
            if (twai_transmit(&tx_message, pdMS_TO_TICKS(50)) == ESP_OK) {
                ESP_LOGI(TAG, "Forwarded Pico message to PLC on ID: 0x%03X", (unsigned int)tx_message.identifier);
            } else {
                ESP_LOGE(TAG, "Failed to transmit Pico message to PLC.");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1)); //little delay
    }
}
