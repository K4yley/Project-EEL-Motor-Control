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

int PulseCounting(int pulseCount){
	if(gpio_get(HALL_A) != Last_A){
        pulseCount++;
        Last_A = !Last_A;
    }
    if(gpio_get(HALL_B) != Last_B){
        pulseCount++;
        Last_B = !Last_B;
    }
    if(gpio_get(HALL_C) != Last_C){
        pulseCount++;
        Last_C = !Last_C;
    }
    printf("PulseCount: %d", pulseCount);
    return pulseCount;
}

float RPM_counting(){
    return (Pulse / 24) * (60 / time);
}

int main() {
    stdio_init_all();  
    
    int pulseCount = 0;
    int Enc_measure = 5;        //The sample time, we can change it
    int Enc_timer_old = time_us_32();
    

    gpio_init(HALL_A);
    gpio_set_dir(HALL_A, GPIO_IN);
    gpio_init(HALL_B);
    gpio_set_dir(HALL_B, GPIO_IN);

    Last_A = gpio_get(HALL_A);
    Last_B = gpio_get(HALL_B);
    Last_C = gpio_get(HALL_C);

    while (true) {
        PulseCounting(pulseCount);
        int Enc_timer = time_us_32();

        if(Enc_timer - Enc_timer_old >= Enc_measure){
            RPM = RPM_counting();
            pulseCount = 0;
            Enc_timer_old = time_us_32();
        }
    }
}


/*
Test plan

Ossiloscope:
    Run the motor;
    Connect the encoder wires with an ossiloscope;
        See if there is any data
    
    Ramp up, and Ramp down measurement
    Position working...    
*/



