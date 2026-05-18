#include "pico/stdlib.h" // Standard library for Pico
#include "stdio.h"
#include "hardware/adc.h"

void UART0_IRQk(){
    while(uart_is_readable(uart0)){     
        int num = uart_getc(uart0);     
    }
}
            
int main() {
    stdio_init_all(); // needed for picotool to autoload

    //UART
    gpio_set_function(0, GPIO_FUNC_UART);           //1 is the first TX UART GPIO pin
    gpio_set_function(1, GPIO_FUNC_UART);           //2 is the first RX UART GPIO pin

    uart_init(uart0, 9600);
    uart_set_hw_flow(uart0, false, false);
    uart_set_fifo_enabled(uart0, false); 
   

    uart_set_irqs_enabled(uart0, true, false);      //dus RX heeft iets, interups

    while (true) {
        uart_putc(uart0, new_value);   
        sleep_ms(100);     
    }
}