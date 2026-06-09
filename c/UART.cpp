#include "pico/stdlib.h" // Standard library for Pico
#include "stdio.h"
//UART connection for ESP32

#define MAX_SIZE 128
char rx_buff[MAX_SIZE];     //the buffer for the outgoing data
int rx_i = 0;

const int EXPECTED_BYTES = 4; 

typedef struct{
    float voltage;
    float current1;
    float current2;
    float temp;
    int32_t position;
    float speed;
    int32_t status;
} PicoTxPacket;
PicoTxPacket out_packet = {3.2f, 4.5f, 7.3f, 9.2f, 7, 45.6f, 3};    //need to change

//Incomming Data
    int I_Status;         //Set Position
    int I_Value1;         //Set move forward; 0 or 1
    int I_Value2;        //Set move backwards; 0 or 1
    int overig;
    /*Expect: 
        "Position, Forwards, backwards, Stop"
        " 5, T, F, F"
        " 10, F, T, F"
        //State is everything about emerency; when true, stop all
    */

//Interrupt
void on_uart_rx() {
    while(uart_is_readable(uart0)){     //Waits till information is send
        uint8_t input = uart_getc(uart0);

        if(rx_i < MAX_SIZE){
            rx_buff[rx_i++] = input;
        }

        if(rx_i == EXPECTED_BYTES){
            I_Status = rx_buff[0];
            I_Value2 = rx_buff[1];
            I_Value1 = rx_buff[2];
            overig   = rx_buff[3];

            printf("Received Binary: %d, %d, %d, %d\n\n", I_Status, I_Value2, I_Value1, overig);
            rx_i = 0;
        }
    } 
}

int main() {
    stdio_init_all(); 

    //UART
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);           //1 is the first TX UART GPIO pin
    gpio_set_function(1, GPIO_FUNC_UART);           //2 is the first RX UART GPIO pin

    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);

    uart_set_irq_enables(uart0, true, false);

    uint32_t old_time = time_us_32();
    char tx_buff[MAX_SIZE];     //The buffer for the incomming data
    
    //Outgoing Data
    float O_Voltage = 3.2;        //Live Voltage level
    float O_Current1 = 4.5;       //Live Current 1 level
    float O_Current2 = 7.3;       //Live Current 2 level
    float O_Temp = 9.2;           //Live Temperature Level
    int O_Position = 7;         //Live Position
    float O_Speed = 45.6;          //Live Speed
    int O_Status = 3;           //Live Status

    while(true){        //Verzender
        uint32_t new_time = time_us_32();
        //update variables
        
        if(new_time - old_time > 200000){     //Change value for when to send
            uint8_t *raw_bytes = (uint8_t*)&mijn_data;
            // byte 0 t/m 6 (Voltage en een deel van Current1)
            uart_write_blocking(uart0, &raw_bytes[0], 7);
            sleep_us(100); 

            // byte 7 t/m 13
            uart_write_blocking(uart0, &raw_bytes[7], 7);
            sleep_us(100);

            // byte 14 t/m 20
            uart_write_blocking(uart0, &raw_bytes[14], 7);
            sleep_us(100);

            // byte 21 t/m 27
            uart_write_blocking(uart0, &raw_bytes[21], 7);
        
            old_time = time_us_32();
        }
    }
}
 