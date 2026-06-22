#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdint.h>
#include <math.h>
#include <ctype.h>
#include "hardware/adc.h"

#pragma once 
//position
#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#define Refpulse 53333  //the distance for moving to next box (16cm)

//Sensors
#define Voltage_CH 0
#define Voltage_Pin 26
#define Current1_CH 1
#define Current1_Pin 27
#define Current2_CH 2
#define Current2_Pin 28

//PWM
#define CLKDIV 200.0f
#define WRAP 65200

#define PWM_A0 8//2    //8
#define PWM_A1 9//3    //9
#define PWM_B0 10//4    //10
#define PWM_B1 11//5    //11
#define PWM_C0 12//6    //12
#define PWM_C1 13//7    //13
//#define mask 0x0E //for the pins 2 to 7
#define mask 0x70 //for the pins 8 to 13

// The fase GPIO pins
#define Ph1_A 2 //8
#define Ph1_B 3 //9

#define Ph2_A 4 //10
#define Ph2_B 5 //11

#define Ph3_A 6 //12
#define Ph3_B 7 //13

//Encoder
#define HALL_A 14
#define HALL_B 15

typedef enum {
    STOP,
    FORWARD,
    BACKWARD
} PWM_t;

typedef struct {
    int pulseCount;
    int positionTicks;
    float Position_Motor; 
    float targetPosition;
    int RPM; 
} Encoder_t;
extern volatile Encoder_t Encoder;

typedef struct {
    float voltage_v;
    float current1_a;    
    float current2_a;
    float temperature_c;
} Sensors_t;
extern volatile Sensors_t Sensor;


/// @brief Sets up the PWM
void setup_PWM();

/// @brief Sets up the Encoder
void setup_Encoder();

/// @brief Sets up the ADC
/// @param gpio which ADC you want to read
/// @param channel which channel the ADC has
void setup_Sensor(uint gpio, int channel);

/// @brief Set up the PWM signals per slice
/// @param gpio_a The first signal pin from the phase
/// @param gpio_b The inverted signal pin from the phase
/// @param phase_delay The delay thats depending on which fase it is
void setup_phase(uint gpio_a, uint gpio_b, uint16_t phase_delay);

/// @brief Interrupt of the Encoder
/// @param gpio Which pin for the interrupt
/// @param events Which event needs to happen for the interrupt
void PulseCounting(uint gpio, uint32_t events);

/// @brief Calculate the RPM
/// @param pulses the sensored signals
/// @param time_s the sample time
/// @return the RPM
float RPM_counting(int pulses, float time_s);

/// @brief Stops the motor
void PWM_TurnOff();

/// @brief Start the motor
void PWM_TurnOn();

/// @brief Controls the motor
/// @param Option three options: forward, backwards, stop 
void Motor(PWM_t Option);

/// @brief Calculates howmany pulses to travel
/// @param Live_location the location of the motor
/// @param Target The new target where the motor needs to travel to
/// @return the pulses to travel
float distanceCal(int Target, float live_motor);

/// @brief Turn de motor depending on the target (uses distanceCal)
/// @param Target the new target (same as distanceCal)
void Motor_newpos(float Target, float live_motor);

/// @brief Calculate the location of the motor
/// @param Live_location the last calculated position
/// @return the new location of the motor
float LocationCal(int positionticks);