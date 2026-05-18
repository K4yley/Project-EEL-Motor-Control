#include "pico/stdlib.h" // Standard library for Pico
#include "stdio.h"
//#include "hardware/adc.h"

            
int main() {
    stdio_init_all(); // needed for picotool to autoload

    //UART
    gpio_set_function(0, GPIO_FUNC_UART);           //1 is the first TX UART GPIO pin
    gpio_set_function(1, GPIO_FUNC_UART);           //2 is the first RX UART GPIO pin

    uart_init(uart0, 9600);
    uart_set_hw_flow(uart0, false, false);
    //uart_set_format(uart0, /*databits*/, /*parity*/);    
    uart_set_fifo_enabled(uart0, false); 
   
     /*Expect: 
        "Voltage, Current1, Current2, Temperature, Speed, Position"
        " 5, 3, 4.3, 4.4"
        " 10, 9, 5.4, 3"
    */
    size_t length = 5;   
    char string[Length] = {};   
    
    //or Floats
    int speed = 0;
    int position = 0;
    int Volt = 0;
    int Current1 = 0;
    int Current2 = 0;
    int Temp = 0;

    while (true) {  
        if(uart_is_writeable(uart0)){
            uart_puts(uart0, "5, 3, T, F");
        }
        
        //If there is value to read; Read it one char at a time with a max
        if(uart_is_readable(uart0)){
            for (size_t i = 0; uart_is_readable(uart0) && i < length; i++){
                char ch = uart_getc(uart0);
                string[i] = ch;
            }
            
            If(sscanf(string, "%d, %d, %d, %d, %d, %d", Volt, Current1, Current2, Temp, speed, position) == 6){
                printf("The Volt: %d, Current1: %d, Current2: %d, Temperature: %d\n Position: %d, Speed: %d", Volt, Current1, Current2, Temp, position, speed);
            }
            
        }
        sleep_ms(100);     
    }
}