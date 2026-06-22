#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdint.h>
#include <math.h>
#include <ctype.h>

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

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

//Expert mode
#define MAX_SIZE 128
char rx_buff[MAX_SIZE];     //the buffer for the outgoing data
int rx_i = 0;

// Encoder pins
#define HALL_A 14
#define HALL_B 15

#define distance 53333  //the distance for moving to next box (16cm)

int pulseCountA;
int pulseCountB;
int positionTicks;

int I_Status;         //Set Position
int I_Value;         //Set move forward; 0 or 1
float Live_MotorLocation = 0;       //the live location of the motor 

//Position Control
float targetPosition = 0;


void PWM_TurnOn();
void PWM_TurnOff();
float distanceCal(float Live_location, int Target);
float distanceCal(float Live_location, int Target);
float RPM_counting(int pulses, float time_s);
void setup_phase(uint gpio_a, uint gpio_b, uint16_t phase_delay);
void Motor_newpos(float Target);
float LocationCal(float Live_location);

//Interrupts
void PulseCountingA(uint gpio, uint32_t events) { //pulse counting
    pulseCountA++;
    if(pulseCountA > pulseCountB){
        positionTicks++;
    }

    //Live_MotorLocation = LocationCal(Live_MotorLocation);
    if(targetPosition == positionTicks){
        pwm_set_mask_enabled(mask);     //stop motor
    }
}

void PulseCountingB(uint gpio, uint32_t events) { //pulse counting
    pulseCountB++;

    if(pulseCountA < pulseCountB){
        positionTicks--;
    }
    
    //Live_MotorLocation = LocationCal(Live_MotorLocation);
    if(targetPosition == positionTicks){
        pwm_set_mask_enabled(mask);     //stop motor
    }
}

int main() {
    stdio_init_all();

    /// @param gpio_a 2     8
    /// @param gpio_b 3     9          
    /// @param phase_delay 0 
    setup_phase(PWM_A0, PWM_A1, 0);
    
    /// @param gpio_a 4     10
    /// @param gpio_b 5     11
    /// @param phase_delay Fase 120° -> (120/360) * 255 = 20833
    setup_phase(PWM_B0, PWM_B1, 20833);
    
    /// @param gpio_a 6     12
    /// @param gpio_b 7     13
    /// @param phase_delay Fase 240° -> (240/360) * 255 = 41666
    setup_phase(PWM_C0, PWM_C1, 41666);

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
        &PulseCountingA
    );

    gpio_set_irq_enabled_with_callback(
        HALL_B,
        GPIO_IRQ_EDGE_RISE,
        true,
        &PulseCountingB
    );

    while (true) {
        uint32_t Enc_timer = time_us_32();
        uint32_t elapsed = Enc_timer - Enc_timer_old;

        if (elapsed >= Enc_measure) {
            float time_s = elapsed / 1000000.0f;
            RPM = RPM_counting((pulseCountA + pulseCountB), time_s);
            printf("PulseCount: %d | RPM: %.2f | Live Location: %.2f | target: %.2f\n", (pulseCountA + pulseCountB), RPM, positionTicks, targetPosition);
            Enc_timer_old = time_us_32();
        }

        char input = stdio_getchar_timeout_us(0.1);
        if(isalpha((unsigned char)input)){
            if(input == 'F' || input == 'f'){ //clockwise
                Motor_newpos(3);
            }
            else if(input == 'B' || input == 'b'){ //counter clockwise
                Motor_newpos(0);
            }
            else if(input == 'i' || input == 'I'){
                pwm_set_mask_enabled(0x00);    //stops PWM
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

/// @brief calculates the position of the motor
/// @param Live_location the last calculated value of the position of the motor
/// @return the position of the motor in the system
float LocationCal(float Live_location){
    Live_location = Live_location + (positionTicks / distance);
    positionTicks = 0;
    return Live_location;
}

void Motor_newpos(float Target){
    targetPosition = distanceCal(Live_MotorLocation, Target);
    if(targetPosition > 0){
        PWM_TurnOff();
        pwm_set_mask_enabled(0x00);    //stops PWM
        slice = pwm_gpio_to_slice_num(PWM_A0);
        pwm_set_counter(slice, 41666);        //phase 1 to 240
        pwm_set_counter(slice+1, 20833);
        pwm_set_counter(slice+2, 0);        //phase 3 to 0
        PWM_TurnOn();
        pwm_set_mask_enabled(mask);
    }
    else if(targetPosition < 0){
        PWM_TurnOff();
        pwm_set_mask_enabled(0x00);    //stops PWM
        slice = pwm_gpio_to_slice_num(PWM_A0);
        pwm_set_counter(slice, 0);        //phase 1 to 240
        pwm_set_counter(slice+1, 20833);
        pwm_set_counter(slice+2, 41666);        //phase 3 to 0
        PWM_TurnOn();
        pwm_set_mask_enabled(mask);
    }
}

/// @brief calculates the new target when given
/// @param Live_location the last calculated value of the position of the motor
/// @param Target the new position
/// @return how many pulses to moving
float distanceCal(float Live_location, int Target){
    float new_position = (float)Target - Live_location;     //if you want to go to 5, and are at 3 at the moment; 2 to move;
    positionTicks = 0;      //reset
    return new_position * distance;
}

float RPM_counting(int pulses, float time_s) { // RPM calculation
    return (pulses / 1024.0f) * (60000000.0f / time_s);
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