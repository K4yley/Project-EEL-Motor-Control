
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#define UART_PORT       UART_NUM_1
#define UART_TX_PIN     43
#define UART_RX_PIN     44
#define UART_BAUD_RATE  115200
#define BUF_SIZE        256
#define MAX_SIZE        64

// Incoming sensor data from Pico
float I_Voltage  = 0.0f;
float I_Current1 = 0.0f;
float I_Current2 = 0.0f;
float I_Temp     = 0.0f;
int   I_Position = 0;
float I_Speed    = 0.0f;
int   I_Status   = 0;

// Outgoing commands to Pico
// Replace these with real command values
int O_Position  = 5;
int O_Forwards  = 1;
int O_Backwards = 0;
int O_Stop      = 0;

// Function declarations
void uart_setup();
bool receive_from_pico(uint8_t* rx_buff, int len);
void send_to_pico();
void print_received_data();


// Initializes UART1 with defined pins and baud rate
void uart_setup()
{
    uart_config_t uart_config = {};
        uart_config.baud_rate           = UART_BAUD_RATE;
        uart_config.data_bits           = UART_DATA_8_BITS;
        uart_config.parity              = UART_PARITY_DISABLE;
        uart_config.stop_bits           = UART_STOP_BITS_1;
        uart_config.flow_ctrl           = UART_HW_FLOWCTRL_DISABLE;
        uart_config.rx_flow_ctrl_thresh = 0;
  

    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);

    printf("[INIT] UART1 started on TX=%d RX=%d at %d baud\n", UART_TX_PIN, UART_RX_PIN, UART_BAUD_RATE);
}


// Reads incoming message from Pico
// Returns true if valid message received and parsed
bool receive_from_pico(uint8_t* rx_buff, int len){
    if (len <= 0) return false;

    rx_buff[len] = '\0';

    int result = sscanf((char*)rx_buff, "%f,%f,%f,%f,%d,%f,%d",
        &I_Voltage, &I_Current1, &I_Current2,
        &I_Temp, &I_Position, &I_Speed, &I_Status);

    if (result == 7)
    {
        return true;
    }
    else
    {
        printf("Parse error: %s\n", rx_buff);
        return false;
    }
}


// Sends command message to Pico
void send_to_pico(){
    char tx_buff[MAX_SIZE];
    sprintf(tx_buff, "%d,%d,%d,%d\n", O_Position, O_Forwards, O_Backwards, O_Stop);
    uart_write_bytes(UART_PORT, tx_buff, strlen(tx_buff));
    printf("Sent -> %s", tx_buff);
}


// Prints received sensor data
void print_received_data()
{
    printf("--- Received from Pico ---\n");
    printf("Voltage:  %.2f V\n",  I_Voltage);
    printf("Current1: %.2f A\n",  I_Current1);
    printf("Current2: %.2f A\n",  I_Current2);
    printf("Temp:     %.1f C\n",  I_Temp);
    printf("Position: %d\n",      I_Position);
    printf("Speed:    %.1f\n",    I_Speed);
    printf("Status:   %d\n",      I_Status);
    printf("--------------------------\n");
}


extern "C" void app_main()
{
    uart_setup();

    uint8_t rx_buff[BUF_SIZE];

    while (true)
    {
        // Read incoming data, wait up to 100ms
        int len = uart_read_bytes(UART_PORT, rx_buff, BUF_SIZE - 1, pdMS_TO_TICKS(100));

        if (len > 0)
        {
            if (receive_from_pico(rx_buff, len))
            {
                print_received_data();
                send_to_pico();
            }
        }
    }
}
