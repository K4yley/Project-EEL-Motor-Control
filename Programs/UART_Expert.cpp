#include "pico/stdlib.h" // Standard library for Pico
#include "stdio.h"
//UART for Pico - Expert Mode

#define MAX_SIZE 128
char rx_buff[MAX_SIZE];     //the buffer for the outgoing data
int rx_i = 0;

//Incomming Data
    int I_Status;         //Set Position
    int I_Value;         //Set move forward; 0 or 1

    /*Expect: 
        "Position, Forwards, backwards, Stop"
        " 5, T, F, F"
        " 10, F, T, F"
        //State is everything about emerency; when true, stop all
    */


//Interrupt
void on_uart_rx() {
    while(uart_is_readable(uart0)){     //Waits till information is send
        char input = uart_getc(uart0);

        if(input == '\n' || input == '\r'){
            if(rx_i > 0){
                rx_buff[rx_i] = '\0';   
                int result = sscanf(rx_buff, "%d, %d, %d", &I_Status, &I_Value);

                if(result == 2){
                    printf("Recieved: %d, %d, %d\n\n", I_Status, I_Value);
                }
                else{
                    printf("ERROR: %s\n", rx_buff);
                }
                rx_i = 0;
            }
        }
        else if(rx_i < MAX_SIZE - 1){
            rx_buff[rx_i++] = input;
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
            sprintf(tx_buff, "%f, %f, %f, %f, %d, %f, %d\n", O_Voltage, O_Current1, O_Current2, O_Temp, O_Position, O_Speed, O_Status);
            uart_puts(uart0, tx_buff);
        
            old_time = time_us_32();
        }
    }
}
 