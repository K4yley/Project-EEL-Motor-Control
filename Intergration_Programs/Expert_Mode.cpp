#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdint.h>
#include <math.h>
#include <ctype.h>

#define SpeedCLK 200.0f
#define SpeedWRAP 65200

//Expert mode
#define MAX_SIZE 128
char rx_buff[MAX_SIZE];     //the buffer for the outgoing data
int rx_i = 0;

// Encoder pins
#define HALL_A 14
#define HALL_B 15

#define TICKS_PER_REV   24      // encoder ticks per rev
#define WHEEL_CIRC_M    0.003   // circumfrence of gear (cm)

#define MAX_PWM         10      // max PWM frequency for motorcontrol
#define DT              0.01f   // 10 ms loop

#define MAX_SPEED       1.0f    // m/s
#define ACCEL           2.0f    // m/s^2
#define DECEL           2.5f    // m/s^2

int pulseCount;
int positionTicks;
int new_dir_state;
int old_dir_state;

int I_Status;         //Set Position
int I_Value;         //Set move forward; 0 or 1

//Position Control
long position = 0;

    //Outgoing Data
    float O_Voltage = 4.4;        //Live Voltage level
    float O_Current1 = 7.0;       //Live Current 1 level
    float O_Current2 = 2.8;       //Live Current 2 level
    float O_Temp = 21.2;           //Live Temperature Level
    float O_Speed = 56.6;          //Live Speed
    int O_Status = 1;           //Live Status

/// @brief Commands that can be received from the external Expert controller.
typedef enum {
    CMD_STOP,
    CMD_JOG_LEFT,
    CMD_JOG_RIGHT,
    CMD_MOVE_ABS,
    CMD_CAL_SLOT
} expert_cmd_t;

//Interrupt
void on_uart_rx() {
    while(uart_is_readable(uart0)){     //Waits till information is send
        char input = uart_getc(uart0);

        if(input == '\n' || input == '\r'){
            if(rx_i > 0){
                rx_buff[rx_i] = '\0';   
                int result = sscanf(rx_buff, "%d, %d", &I_Status, &I_Value);

                if(result == 2){
                    printf("Recieved: %d, %d\n\n", I_Status, I_Value);
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

void setup_phase(uint gpio_a, uint gpio_b, uint16_t phase_delay) {
    uint slice = pwm_gpio_to_slice_num(gpio_a);
    
    //Say GPIO is for PWM
    gpio_set_function(gpio_a, GPIO_FUNC_PWM);
    gpio_set_function(gpio_b, GPIO_FUNC_PWM);

    //Set clk and wrap for the right slice -> Set period of 4 cycles (0 to 3 inclusive) (duty cicle?)
    pwm_set_clkdiv(slice, SpeedCLK);
    pwm_set_wrap(slice, SpeedWRAP);  

    //set duty circle of 50%
    pwm_set_chan_level(slice, PWM_CHAN_A, SpeedWRAP / 2 - 13); // add Dead Time; a max. of 15 ticks deadtime per slice! 1 tick = (CLKDIV / 125MHz = 1.6 us), to get 20 ms => 20000/1,6 = 12500 ticks
    pwm_set_chan_level(slice, PWM_CHAN_B, SpeedWRAP / 2);

    /// @brief inverted of GPIO_b
    /// @param slice_num PWM slice number
    /// @param a true to invert output A
    /// @param b true to invert output B
    pwm_set_output_polarity(slice, false, true);

    //add the fase delay
    pwm_set_counter(slice, phase_delay);
}

void PulseCounting(uint gpio, uint32_t events) { //pulse counting
    if (!gpio_get(HALL_A) && gpio_get(HALL_B)) {   // 01
        pulseCount++;
        new_dir_state = 1;
    }
    if (gpio_get(HALL_A) && gpio_get(HALL_B)) {    // 11
        pulseCount++;
        new_dir_state = 2;
    }
    if (gpio_get(HALL_A) && !gpio_get(HALL_B)) {   // 10
        pulseCount++;
        new_dir_state = 3;
    }
    if (!gpio_get(HALL_A) && !gpio_get(HALL_B)) {  // 00
        pulseCount++;
        new_dir_state = 4;
    }

    if ((new_dir_state > old_dir_state && !(new_dir_state == 4 && old_dir_state == 1)) ||
        (new_dir_state == 1 && old_dir_state == 4)) {
        positionTicks++;
    } else {
        positionTicks--;
    }
    old_dir_state = new_dir_state;    
}

// float getPosition(void)
// {
//     return ((float)positionTicks / TICKS_PER_REV) * WHEEL_CIRC_M;
// }

float RPM_counting(int pulses, float time_s) { // RPM calculation
    return (pulses / 2048.0f) * (60.0f / time_s);
}

int main() {
    stdio_init_all();

    /// @param gpio_a 2
    /// @param gpio_b 3
    /// @param phase_delay 0 
    setup_phase(2, 3, 0);
    
    /// @param gpio_a 4
    /// @param gpio_b 5
    /// @param phase_delay Fase 120° -> (120/360) * 255 = 20833
    setup_phase(4, 5, 20833); 
    
    /// @param gpio_a 6
    /// @param gpio_b 7
    /// @param phase_delay Fase 240° -> (240/360) * 255 = 41666x
    setup_phase(6, 7, 41666);

    float RPM = 0.0f;

    uint32_t Enc_measure = 250000;         //sampletime: 250 ms
    uint32_t Enc_timer_old = time_us_32();

    gpio_init(HALL_A);
    gpio_set_dir(HALL_A, GPIO_IN);
    gpio_init(HALL_B);
    gpio_set_dir(HALL_B, GPIO_IN);         

    gpio_set_irq_enabled_with_callback(
        HALL_A,
        GPIO_IRQ_EDGE_RISE,
        true,
        &PulseCounting
    );

    gpio_set_irq_enabled_with_callback(
        HALL_B,
        GPIO_IRQ_EDGE_RISE,
        true,
        &PulseCounting
    );

    char tx_buff[MAX_SIZE];   

    //UART
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);           //1 is the first TX UART GPIO pin
    gpio_set_function(1, GPIO_FUNC_UART);           //2 is the first RX UART GPIO pin

    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);

    uart_set_irq_enables(uart0, true, false);

    //sync and start all slices at the same time
    pwm_set_mask_enabled(0x0E); //00001110

    while (true) {
        uint32_t Enc_timer = time_us_32();
        uint32_t elapsed = Enc_timer - Enc_timer_old;

        char input = stdio_getchar_timeout_us(0.1);
        if(isalpha((unsigned char)input)){
            if(input == 'F' || input == 'f'){ //clockwise
                for(int i = 1; i < 4; i++){
                    pwm_set_enabled(i, false);  
                }
                pwm_set_counter(1, 41666);        //phase 1 to 240
                pwm_set_counter(3, 0);        //phase 3 to 0

                pwm_set_mask_enabled(0x0E);
                //printf("Forward: Clockwise\n");
            }
            
            else if(input == 'B' || input == 'b'){ //counter clockwise
                for(int i = 1; i < 4; i++){
                    pwm_set_enabled(i, false);  
                }
                pwm_set_counter(1, 0);        //phase 1 to 0
                pwm_set_counter(3, 41666);        //phase 3 to 240

                pwm_set_mask_enabled(0x0E);
                //printf("Backward: Counter Clockwise\n");
            }
            else if(input == 'i' || input == 'I'){
                for(int i = 1; i < 4; i++){
                    pwm_set_enabled(i, false);  
                }
                //printf("Idle State\n");
            }
        }
        // if(I_Status == CMD_STOP){
        //     for(int i = 1; i < 4; i++){
        //         pwm_set_enabled(i, false);  
        //     }
        //     //printf("Stopped\n");
        // }
        else if(I_Status == CMD_JOG_LEFT){
            for(int i = 1; i < 4; i++){
                pwm_set_enabled(i, false);  
            }
            pwm_set_counter(1, 0);        //phase 1 to 0
            pwm_set_counter(3, 41666);        //phase 3 to 240

            pwm_set_mask_enabled(0x0E);
            //printf("Left: Counter Clockwise\n");
        }
        else if(I_Status == CMD_JOG_RIGHT){
            for(int i = 1; i < 4; i++){
                pwm_set_enabled(i, false);  
            }
            pwm_set_counter(1, 41666);        //phase 1 to 240
            pwm_set_counter(3, 0);        //phase 3 to 0

            pwm_set_mask_enabled(0x0E);
            //printf("Right: Clockwise\n");
        }
        else if(I_Status == CMD_MOVE_ABS){
            position = I_Value;
            printf("Moving to position: %d\n", position);

        }
        // else if(I_Status == CMD_CAL_SLOT){
        //     //don't know
        // }

        if (elapsed >= Enc_measure) {
            float time_s = elapsed / 1000000.0f;
            RPM = RPM_counting(pulseCount, time_s);
            printf("PulseCount: %d | RPM: %.2f\n", pulseCount, RPM);
            
            Enc_timer_old = time_us_32();

            sprintf(tx_buff, "%f, %f, %f, %f, %d, %f, %d\n", O_Voltage, O_Current1, O_Current2, O_Temp, pulseCount, RPM, O_Status);
            uart_puts(uart0, tx_buff);
            pulseCount = 0;
        }
    }       
}


