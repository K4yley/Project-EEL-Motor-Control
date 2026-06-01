#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdint.h>
#include <math.h>
//Test if, 0hz is stopping!


// Calculations for 10 Hz at 125 MHz clock
// Freq = 125M / (clkdiv * wrap)
// 10 = 125M / (200 * 62500)
#define SpeedCLK 200.0f
#define SpeedWRAP 65200

// Encoder pins
#define HALL_A 2
#define HALL_B 3
#define HALL_C 4

static bool Last_A;
static bool Last_B;
static bool Last_C;

//clock divider, the speed of the hardware ticks
//max. 255 zijn
float CLKDIV = 0;        
//how long a period is. till what value the ticks go and then reset
uint16_t WRAP = 0;


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

void PulseCounting(int *pulseCount) { //pulse counting
    if (gpio_get(HALL_A) != Last_A) {
        (*pulseCount)++;
        Last_A = !Last_A;
    }
    if (gpio_get(HALL_B) != Last_B) {
        (*pulseCount)++;
        Last_B = !Last_B;
    }
    if (gpio_get(HALL_C) != Last_C) {
        (*pulseCount)++;
        Last_C = !Last_C;
    }
}


float RPM_counting(int pulses, float time_s) { // RPM calculation
    return (pulses / 24.0f) * (60.0f / time_s);
}

void Speed(float CLK, int WRAP){
    CLKDIV = CLK;
    WRAP = WRAP;
    for(int i = 1; i < 4; i++){
        pwm_set_clkdiv(i, CLKDIV);
        pwm_set_wrap(i, WRAP);  
    }

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

    int pulseCount = 0;
    float RPM = 0.0f;

    uint32_t Enc_measure = 250000;         //sampletime: 250 ms
    uint32_t Enc_timer_old = time_us_32();

    gpio_init(HALL_A);
    gpio_set_dir(HALL_A, GPIO_IN);
    gpio_init(HALL_B);
    gpio_set_dir(HALL_B, GPIO_IN);
    gpio_init(HALL_C);                      
    gpio_set_dir(HALL_C, GPIO_IN);           

    Last_A = gpio_get(HALL_A);
    Last_B = gpio_get(HALL_B);
    Last_C = gpio_get(HALL_C);

    while (true) {
        //encoder
        PulseCounting(&pulseCount);        

        uint32_t Enc_timer = time_us_32();
        uint32_t elapsed = Enc_timer - Enc_timer_old;

        if (elapsed >= Enc_measure) {
            float time_s = elapsed / 1000000.0f;
            RPM = RPM_counting(pulseCount, time_s);
            printf("PulseCount: %d | RPM: %.2f\n", pulseCount, RPM);
            pulseCount = 0;
            Enc_timer_old = time_us_32();
        } 
        // pos code
        Speed(SpeedCLK, SpeedWRAP); //Or change it to 0 for no speed        
    }
}



