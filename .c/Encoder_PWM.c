#include "pico/stdlib.h" // Standard library for Pico
#include "stdio.h"

//Encoder
//pins has to be changed
#define HALL_A 2
#define HALL_B 3
#define HALL_C 4

static bool Last_A;
static bool Last_B;
static bool Last_C;

//10 Hz
float CLKDIV = 200.0f;        
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

int PulseCounting(pulseCount){
	if(HALL_A != Last_A){
        pulseCount++;
        Last_A = !Last_A
    }
    if(HALL_B != Last_B){
        pulseCount++;
        Last_B = !Last_B
    }
    if(HALL_C != Last_C){
        pulseCount++;
        Last_C = !Last_C
    }
    return pulseCount;
}

float RPM_counting(){
    return (Pulse / 24) * (60 / time);
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
    
    int pulseCount = 0;
    int Enc_measure = 5;        //The sample time, we can change it
    int Enc_timer_old = time_us_32();
    
    gpio_init(HALL_A);
    gpio_set_dir(HALL_A, GPIO_IN);
    gpio_init(HALL_B);
    gpio_set_dir(HALL_B, GPIO_IN);

    //sync and start all slices at the same time
    pwm_set_mask_enabled(0x0E); //00001110

    while (true) {
        PulseCounting(pulseCount);
        int Enc_timer = time_us_32();

        if(Enc_timer - Enc_timer_old >= Enc_measure){
            int RPM = RPM_counting();
            printf("RPM: %d", RPM);
            pulseCount = 0;
            Enc_timer_old = time_us_32();
        }
    }
}





