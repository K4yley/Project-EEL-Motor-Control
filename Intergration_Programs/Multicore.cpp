#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/multicore.h"
#include <stdint.h>
#include <math.h>
#include <ctype.h>

#define CLK 200.0f
#define WRAP 65200

#define PWM_A0 8//2    //8
#define PWM_A1 9//3    //9
#define PWM_B0 10//4    //10
#define PWM_B1 11//5    //11
#define PWM_C0 12//6    //12
#define PWM_C1 13//7    //13
//#define mask 0x0E //for the pins 2 to 7
#define mask 0x70 //for the pins 8 to 13
int slice;

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

//Position Control
float speed = 0;
float filteredSpeed = 0;
float targetSpeed = 0;
long targetPosition = 0;
float posError = 0;

void PWM_TurnOff();
void PWM_TurnOn();
void setup_Encoder();
float RPM_counting(int pulses, float time_s);
void setup_phase(uint gpio_a, uint gpio_b, uint16_t phase_delay);

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
}

//Second Core
void core1_entry() {
    setup_Encoder();

    uint32_t Enc_measure = 250000;  
    uint32_t Enc_timer_old = time_us_32();
    float RPM;
    
    posError = targetPosition - pulseCount;
    if(posError > 0){
        PWM_TurnOff();
        pwm_set_mask_enabled(0x00);    //stops PWM
        slice = pwm_gpio_to_slice_num(PWM_A0);
        pwm_set_counter(slice, 41666);        //phase 1 to 240
        pwm_set_counter(slice+1, 20833);
        pwm_set_counter(slice+2, 0);        //phase 3 to 0
        PWM_TurnOn();
        pwm_set_mask_enabled(mask);
    }
    else if(posError < 0){
        PWM_TurnOff();
        pwm_set_mask_enabled(0x00);    //stops PWM
        slice = pwm_gpio_to_slice_num(PWM_A0);
        pwm_set_counter(slice, 0);        //phase 1 to 240
        pwm_set_counter(slice+1, 20833);
        pwm_set_counter(slice+2, 41666);        //phase 3 to 0
        PWM_TurnOn();
        pwm_set_mask_enabled(mask);
    }

    while(true) {
        uint32_t Enc_timer = time_us_32();
        uint32_t elapsed = Enc_timer - Enc_timer_old;

        if (elapsed >= Enc_measure) {
            float time_s = elapsed / 1000000.0f;
             RPM = RPM_counting(pulseCount, time_s);
            printf("PulseCount: %d | RPM: %.2f, Position: %d\n", pulseCount, RPM, positionTicks);
            Enc_timer_old = time_us_32();
        }
        if(pulseCount == 0){
            posError = targetPosition - pulseCount;
            if(posError > 0){
                PWM_TurnOff();
                pwm_set_mask_enabled(0x00);    //stops PWM
                slice = pwm_gpio_to_slice_num(PWM_A0);
                pwm_set_counter(slice, 41666);        //phase 1 to 240
                pwm_set_counter(slice+1, 20833);
                pwm_set_counter(slice+2, 0);        //phase 3 to 0
                PWM_TurnOn();
                pwm_set_mask_enabled(mask);
            }
            else if(posError < 0){
                PWM_TurnOff();
                pwm_set_mask_enabled(0x00);    //stops PWM
                slice = pwm_gpio_to_slice_num(PWM_A0);
                pwm_set_counter(slice, 0);        //phase 1 to 240
                pwm_set_counter(slice+1, 20833);
                pwm_set_counter(slice+2, 41666);        //phase 3 to 0
                PWM_TurnOn();
                pwm_set_mask_enabled(mask);
            }
            else{
                printf("Stopped: PulseCount: %d\n", pulseCount, RPM);
                pwm_set_mask_enabled(0x00);    //stops PWM
            }
        }

    }
}

//First Core
int main() {
    stdio_init_all();

    /// @param gpio_a 2     8
    /// @param gpio_b 3     9          
    /// @param phase_delay Phase 0 
    setup_phase(PWM_A0, PWM_A1, 0);
    
    /// @param gpio_a 4     10
    /// @param gpio_b 5     11
    /// @param phase_delay Phase 120° -> (120/360) * 255 = 20833
    setup_phase(PWM_B0, PWM_B1, 20833);
    
    /// @param gpio_a 6     12
    /// @param gpio_b 7     13
    /// @param phase_delay Phase 240° -> (240/360) * 255 = 41666
    setup_phase(PWM_C0, PWM_C1, 41666);

    multicore_launch_core1(core1_entry);

    while (true) {
        char input = stdio_getchar_timeout_us(0.1);
        if(isalpha((unsigned char)input)){
            if(input == 'F' || input == 'f'){ //clockwise
                //PWM_TurnOff();
                pwm_set_mask_enabled(0x00);    //stops PWM
                // slice = pwm_gpio_to_slice_num(PWM_A0);
                // pwm_set_counter(slice, 41666);        //phase 1 to 240
                // pwm_set_counter(slice+1, 20833);
                // pwm_set_counter(slice+2, 0);        //phase 3 to 0
                // PWM_TurnOn();
                // pwm_set_mask_enabled(mask);
                // printf("Forward: Clockwise\n");
                pulseCount = 0;
                targetPosition = 40000;
            }
            else if(input == 'B' || input == 'b'){ //counter clockwise
                //PWM_TurnOff();
                pwm_set_mask_enabled(0x00);    //stops PWM
                // slice = pwm_gpio_to_slice_num(PWM_A0);
                // pwm_set_counter(slice, 0);        //phase 1 to 240
                // pwm_set_counter(slice+1, 20833);
                // pwm_set_counter(slice+2, 41666);        //phase 3 to 0
                // PWM_TurnOn();
                // pwm_set_mask_enabled(mask);
                // printf("Backward: Counter Clockwise\n");
                pulseCount = 0;
                targetPosition = -40000;
            }
            else if(input == 'i' || input == 'I'){
                pwm_set_mask_enabled(0x00);    //stops PWM
                printf("Idle State\n");
                pulseCount = 0;
                targetPosition = 0;
            }
        }    
    }
}


void PWM_TurnOff(){
    int slice = pwm_gpio_to_slice_num(PWM_A0);
    pwm_set_chan_level(slice, PWM_A0, 0);
    pwm_set_chan_level(slice, PWM_A1, 0);
    slice++;
    pwm_set_chan_level(slice, PWM_B0, 0);
    pwm_set_chan_level(slice, PWM_B1, 0);
    slice++;
    pwm_set_chan_level(slice, PWM_C0, 0);
    pwm_set_chan_level(slice, PWM_C1, 0);
    sleep_us(10);    
}
void PWM_TurnOn(){
    int slice = pwm_gpio_to_slice_num(PWM_A0);
    pwm_set_chan_level(slice, PWM_CHAN_A, WRAP / 2 - 13);
    pwm_set_chan_level(slice, PWM_CHAN_B, WRAP / 2);
    slice++;
    pwm_set_chan_level(slice, PWM_CHAN_A, WRAP / 2 - 13);
    pwm_set_chan_level(slice, PWM_CHAN_B, WRAP / 2);
    slice++;
    pwm_set_chan_level(slice, PWM_CHAN_A, WRAP / 2 - 13);
    pwm_set_chan_level(slice, PWM_CHAN_B, WRAP / 2);   
}

void setup_Encoder(){
    gpio_init(HALL_A);
    gpio_set_dir(HALL_A, GPIO_IN);
    gpio_init(HALL_B);
    gpio_set_dir(HALL_B, GPIO_IN);

    //Interrupts
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
}

// float getPosition(void)
// {
//     return ((float)positionTicks / TICKS_PER_REV) * WHEEL_CIRC_M;
// }

float RPM_counting(int pulses, float time_s) { // RPM calculation
    return (pulses / 1024.0f) * (60.0f / time_s);
}

void setup_phase(uint gpio_a, uint gpio_b, uint16_t phase_delay) {
    uint slice = pwm_gpio_to_slice_num(gpio_a);
    
    //Say GPIO is for PWM
    gpio_set_function(gpio_a, GPIO_FUNC_PWM);
    gpio_set_function(gpio_b, GPIO_FUNC_PWM);

    //Set clk and wrap for the right slice -> Set period of 4 cycles (0 to 3 inclusive) (duty cicle?)
    pwm_set_clkdiv(slice, CLK);
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