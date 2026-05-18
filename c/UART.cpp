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
   
    uart_puts(uart0, "voltage, current1, current2, position, speed");
    //uart_set_irqs_enabled(uart0, true, false);      //dus RX heeft iets, interrupts
    char string[64] = {};
    //  if(uart_is_writeable()){
    //         uart_puts(uart0, "1, 0.4, 0.5, 4.5, 3");
    //     }

    while (true) {  
        for (int i = 0; uart_is_readable(uart0) && i < 64; i++){
            char ch = uart_getc(uart0);
            string[i] = ch;
        } 
        printf("%c\n", string);


        // uart_write_blocking(uart0, 00b0100, 1); // output
        // uart_read_blocking();                   // input
        // bool uart_is_readible();    //bool , checks if input is avaiable
        // bool uart_is_writeable();   //bool , checks if output is possible
        // uart_puts();                //outputs a string
        // uart_getc();                //read single character

        sleep_ms(100);     
    }
}