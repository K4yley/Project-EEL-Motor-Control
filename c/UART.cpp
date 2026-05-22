#include "pico/stdlib.h" // Standard library for Pico
#include "stdio.h"
//The Sender; After some time we send information.
//then we waits till we get answers of not

#define MAX_SIZE 64

/// @brief First sending stuff via UART then waiting on recieving
/// @param tx_buff The information that has to be send
/// @return The information that has been recieved
char UART_Sending_Recieving(char tx_buff){
    char rx_buff[MAX_SIZE];     //the buffer for the outgoing data
    int rx_i = 0;

    uart_puts(uart0, tx_buff);
           
    while(uart_is_readable(uart0)){
        char input = uart_getc(uart0);

        if(input == '\n' || input == '\r'){
            if(rx_i > 0){
                rx_buff[rx_i] = '\0';
                return rx_buff;
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

    uint32_t old_time = time_us_32();
    char tx_buff[MAX_SIZE];     //The buffer for the incomming data
    char string[MAX_SIZE];
    
    
    //Incomming Data
    int I_Position;         //Set Position
    int I_Forwards;         //Set move forward; 0 or 1
    int I_Backwards;        //Set move backwards; 0 or 1
    int I_Stop;             //Emergency Stop
    /*Expect: 
        "Position, Forwards, backwards, Stop"
        " 5, T, F, F"
        " 10, F, T, F"
        //State is everything about emerency; when true, stop all
    */
    
    //Outgoing Data
    float O_Voltage;        //Live Voltage level
    float O_Current1;       //Live Current 1 level
    float O_Current2;       //Live Current 2 level
    float O_Temp;           //Live Temperature Level
    int O_Position;         //Live Position
    float O_Speed;          //Live Speed
    int O_Status;           //Live Status

    while(true){        //Verzender
        uint32_t new_time = time_us_32();
        //update variables
        
        if(new_time - old_time > 20000000){     //Change value for when to send
            sprintf(tx_buff, "%0.2f, %0.2f, %0.2f, %0.2f, %d, %0.2f, %d", O_Voltage, O_Current1, O_Current2, O_Temp, O_Position, O_Speed, O_Status);
            
            string = UART_Sending_Recieving(tx_buff);
            int result = sscanf(string, "%d, %d, %d, %d", I_Position, I_Forwards, I_Backwards, I_Stop);

            // if(result == 4){    //this is not needed
            //     printf("Ontvangen!");
            // }
            // else{
            //     printf("Fout: %s\n", string);
            // }
            old_time = time_us_32();
        }
    }
}
 