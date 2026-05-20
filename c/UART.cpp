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
    uart_set_fifo_enabled(uart0, false); 
    

    /*Expect: 
        "Position, Forwards, backwards, States"
        " 5, T, F, F"
        " 10, F, T, F"
        //State is everything about emerency; when true, stop all
    */
    size_t length = 10;   
    char string[Length] = {};   

    int speed = 0;
    int position = 0;
    char forwards = false;
    char backwards = false;

    while (true) {
        if(uart_is_writeable(uart0)){
            uart_puts(uart0, "5, 3, 4.3, 4.4");
        }
        
        //If there is value to read; Read it one char at a time with a max
        if(uart_is_readable(uart0)){
            for (size_t i = 0; uart_is_readable(uart0) && i < length; i++){
                char ch = uart_getc(uart0);
                string[i] = ch;
            }
            
            if(sscanf(string, "%d, %d, %c, %c", speed, position, forwards, backwards) == 4){
                printf("Position to go to is %d, with speed: %d", position, speed);
            }
        }
        sleep_ms(100);     
    }
}

    // uart_write_blocking(uart0, 00b0100, 1); // output
    // uart_read_blocking();                   // input
    // bool uart_is_readible();    //bool , checks if input is avaiable
    // bool uart_is_writeable();   //bool , checks if output is possible
    // uart_puts();                //outputs a string
    // uart_getc();                //read single character
    //uart_set_irqs_enabled(uart0, true, false);      //dus RX heeft iets, interrupts
    //uart_set_format(uart0, /*databits*/, /*parity*/);  