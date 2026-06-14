#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdint.h>
#include <math.h>
#include <ctype.h>
#include "hardware/adc.h"

#pragma once 

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

// The fase GPIO pins
#define Ph1_A 2 //8
#define Ph1_B 3 //9

#define Ph2_A 4 //10
#define Ph2_B 5 //11

#define Ph3_A 6 //12
#define Ph3_B 7 //13

//Encoder
#define HALL_A 16
#define HALL_B 17

#define TICKS_PER_REV   24      // encoder ticks per rev
#define WHEEL_CIRC_M    0.003   // circumfrence of gear (cm)

#define MAX_PWM         10      // max PWM frequency for motorcontrol
#define DT              0.01f   // 10 ms loop

#define MAX_SPEED       1.0f    // m/s
#define ACCEL           2.0f    // m/s^2
#define DECEL           2.5f    // m/s^2

typedef struct{
    int pulseCount;
    int positionTicks;
    int new_dir_state;
    int old_dir_state;
} Encoder_t;
Encoder_t Encoder;

typedef struct {
    float voltage_v;
    float current1_a;
    float current2_a;
    float temperature_c;
    int32_t position_mm;
} Sensor_t;
Sensor_t Data;

typedef enum {
    STOP,
    FORWARD,
    BACKWARD
} PWM_t;

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

/// @brief Controls the motor
/// @param Option three options: forward, backwards, stop 
void Motor(PWM_t Option);

 