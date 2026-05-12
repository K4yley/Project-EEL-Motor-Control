#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// Calculations for 10 Hz at 125 MHz clock
// Freq = 125M / (clkdiv * wrap)
// 10 = 125M / (200 * 62500)

//clock divider, the speed of the hardware ticks
//max. 255 zijn
float CLKDIV = 200.0f;        
//how long a period is. till what value the ticks go and then reset
uint16_t WRAP = 65200;


void setup_phase(uint gpio_a, uint gpio_b, uint16_t phase_delay) {
    uint slice = pwm_gpio_to_slice_num(gpio_a);
    
    //Say GPIO is for PWM
    gpio_set_function(gpio_a, GPIO_FUNC_PWM);
    gpio_set_function(gpio_b, GPIO_FUNC_PWM);

    //Set clk and wrap for the right slice -> Set period of 4 cycles (0 to 3 inclusive) (duty cicle?)
    pwm_set_clkdiv(slice, CLKDIV);
    pwm_set_wrap(slice, WRAP);  

    //set duty circle of 50%
    pwm_set_chan_level(slice, PWM_CHAN_A, WRAP / 2 - 13); // add Dead Time; a max. of 15 ticks deadtime per slice! 1 tick = (CLKDIV / 125MHz = 1.6 us), to get 20 ms => 20000/1,6 = 12500 ticks
    pwm_set_chan_level(slice, PWM_CHAN_B, WRAP / 2);

    /// @brief inverted of GPIO_b
    /// @param slice_num PWM slice number
    /// @param a true to invert output A
    /// @param b true to invert output B
    pwm_set_output_polarity(slice, false, true);

    //add the fase delay
    pwm_set_counter(slice, phase_delay);
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
    /// @param phase_delay Fase 240° -> (240/360) * 255 = 41666
    setup_phase(6, 7, 41666);
    
    //sync and start all slices at the same time
    pwm_set_mask_enabled(0x0E); //00001110
    int Timer = time_us_32();

    while (true) {
        int time_new = time_us_32();
        if(time_new >= Timer + 60000000){
            // printf("Slice 0: %d\n", pwm_get_counter(1));
            // printf("Slice 1: %d\n", pwm_get_counter(2) - 20833);
            // printf("Slice 2: %d\n\n", pwm_get_counter(3) - 41666);
            Timer = time_us_32();
            for(int i = 1; i < 4; i++){
                CLKDIV = 100;
                WRAP = 49999;
                pwm_set_clkdiv(i, CLKDIV);
                pwm_set_wrap(i, WRAP);  
            }
        }
        tight_loop_contents();
    }
}
