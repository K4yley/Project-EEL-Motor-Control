#include <stdio.h>
#include "pico/stdlib.h"
//#include "hardware/pwm.h"
#include <stdint.h>
#include <math.h>
#include <ctype.h>

//PWM
#define SpeedCLK 200.0f
#define SpeedWRAP 65200
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

uint32_t Enc_measure = 250000;
uint32_t Enc_timer_old;

int pulseCount;
int positionTicks;
int new_dir_state;
int old_dir_state;

/// @brief Setups the PWM and encoder pins
void setup_PWM_Encoder();

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

/// @brief Controlls the motor
/// @param stop When true, the motor stops
/// @param direction when true, forward, otherwise backward
void Speed(bool stop, bool direction);

