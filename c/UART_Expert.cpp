#include "pico/stdlib.h" // Standard library for Pico
#include "stdio.h"
//The Reciever; We wait til we get information and send sometimes something back

#define MAX_SIZE 64

char tx_buff[MAX_SIZE];     //The buffer for the incomming data
char rx_buff[MAX_SIZE];     //the buffer for the outgoing data
int rx_i = 0;

//Incomming Data
float I_Voltage;        //Live Voltage level
float I_Current1;       //Live Current 1 level
float I_Current2;       //Live Current 2 level
float I_Temp;           //Live Temperature Level
int I_Position;         //Live Position
float I_Speed;          //Live Speed
int I_Status;           //Live Status
/*Expect: 
    "Voltage, Current1, Current2, Temperature, Speed, Position, Status"
    " 5, 3, 4.3, 4.4, 3, 4, 2"
    " 10, 9, 5.4, 3, 2"
*/

//Outgoing Data
int O_Position;         //Set Position
int O_Forwards;         //Set move forward; 0 or 1
int O_Backwards;        //Set move backwards; 0 or 1
int O_Stop;             //Emergency Stop

//Interrupt
void on_uart_rx() {
    while(uart_is_readable(uart0)){     //Waits till information is send
        char input = uart_getc(uart0);

        if(input == '\n' || input == '\r'){
            if(rx_i > 0){
                rx_buff[rx_i] = '\0';   
                int result = sscanf(rx_buff, "%0.2f, %0.2f, %0.2f, %0.2f, %d, %0.2f, %d", I_Voltage, I_Current1, I_Current2, I_Temp, I_Position, I_Speed, I_Status);         

                if(result == 7){
                    printf("Ontvangen!\n");
                }
                else{
                    printf("Fout: %s\n", rx_buff);
                }
                uart_puts(uart0, "Currents have been recieved\n");
                rx_i = 0;
            }
        }
        else if(rx_i < MAX_SIZE - 1){
            rx_buff[rx_i++] = input;
        }
    }
    //Sending
    uart_puts(uart0, tx_buff);
}   
int main() {
    stdio_init_all(); 

    //UART
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);           //1 is the first TX UART GPIO pin
    gpio_set_function(1, GPIO_FUNC_UART);           //2 is the first RX UART GPIO pin

    uart_set_hw_flow(uart0, false, false); //interupt

    while (true) {      //Reciever
       tight_loop_contents();
       //The Expert Mode code:
       //Update the variables; and it'll be send
       sprintf(tx_buff, "%d, %d, %d, %d", O_Position, O_Forwards, O_Backwards, O_Stop);
    }
}