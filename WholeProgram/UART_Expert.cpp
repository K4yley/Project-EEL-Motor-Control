#include "UART_Expert.h"


void on_uart_rx(){
    while(uart_is_readable(uart0)){     //Waits till information is send
        char input = uart_getc(uart0);

        if(input == '\n' || input == '\r'){
            if(rx_i > 0){
                rx_buff[rx_i] = '\0';   
                int result = sscanf(rx_buff, "%d, %d, %d", &I_Status, &I_Value2, &I_Value1);

                if(result == 3){
                    printf("Recieved: %d, %d, %d\n\n", I_Status, I_Value2, I_Value1);
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


void setup_uart(){
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);           //1 is the first TX UART GPIO pin
    gpio_set_function(1, GPIO_FUNC_UART);           //2 is the first RX UART GPIO pin

    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);

    uart_set_irq_enables(uart0, true, false);
}


void sending_uart(){
    sprintf(tx_buff, "%f, %f, %f, %f, %d, %f, %d\n", O_Voltage, O_Current1, O_Current2, O_Temp, O_Position, O_Speed, O_Status);
    uart_puts(uart0, tx_buff);
}