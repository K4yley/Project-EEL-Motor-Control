#include "UART_Expert.h"
#include "States.h"

volatile ExpertCommand_t UART;

void on_uart_rx(){
    char rx_buff[MAX_SIZE];     //the buffer for the outgoing data
    int rx_i = 0;

    while(uart_is_readable(uart0)){     //Waits till information is send
        char input = uart_getc(uart0);

        if(input == '\n' || input == '\r'){
            if(rx_i > 0){
                rx_buff[rx_i] = '\0';   
                int result = sscanf(rx_buff, "%d, %d", UART.command, UART.value);

                if(result == 3){
                    printf("Recieved: %d, %d\n\n", UART.command, UART.value);
                }
                else{
                    printf("ERROR: %s\n", rx_buff);
                }
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
    char tx_buff[MAX_SIZE];
    sprintf(tx_buff, "%f, %f, %f, %f, %d, %f, %d\n", Sensor.voltage_v, Sensor.current1_a, Sensor.current2_a, Sensor.temperature_c, Encoder.Position_Motor, Encoder.RPM, state);
    uart_puts(uart0, tx_buff);
}