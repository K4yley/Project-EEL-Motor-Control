/*
The switching of states
PLC     -> Get information from the PLC, send dataq back to PLC
Expert  -> Get information from the Expert controller, send data back to Contoller and PLC
Driving -> Control for moving to the right position given by PLc or Expert controller
Error   -> Something went wrong / emerency button is pressed
*/
#pragma once

#include "pico/stdlib.h"
#include <stdlib.h>
#include <string.h>  // memcpy

#include "UART_Expert.h"
#include "PWM_Encoder_Sensors.h"
#include "DEV_Config.h"
#include "MCP2515.h"
#include "Debug.h"

#define EXPERT_MODE_ACTIVE_PIN 1

#define TX_ID_0  0x123   // voltage + current1
#define TX_ID_1  0x124   // current2 + temperature
#define TX_ID_2  0x125   // position + speed
#define TX_ID_3  0x126   // status

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

/// @brief All the states
typedef enum {
    PLC_MODE,
    EXPERT,
    ERROR
} state_t;
state_t state = PLC_MODE;

/// @brief //All the possible kind of error to made the problem easier to find (not needed for PLC, but for expert controller)
typedef enum {      
    CURRENT_1,
    CURRENT_2,
    TEMP,
    MOVEMENT
} ERROR_t;

typedef struct {
  float kp, ki, kd;
  float i;
  float last;
} PID_t;

typedef struct {      
    int Position;
    int Forward;
    int Backward;
    int Stop;
} PLC_t;
PLC_t PLC;
PID_t positionPID = {0.18, 0.0, 0.01, 0, 0};

/// @brief The case statement 
/// @param state 
void output(state_t state);

/// @brief The code for the Expert mode
void Expert_mode();

/// @brief The code for the PLC mode
void PLC_mode();

/// @brief The code for the Error_mode
void Error_mode();

/// @brief The code for sending data to the PLC 
void sending_PLC();

/// @brief 
void Closed_loop(long targetPosition);

float computePID(PID_t *p, float e);