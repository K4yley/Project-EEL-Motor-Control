/*
The switching of states
PLC     -> Get information from the PLC, send dataq back to PLC
Expert  -> Get information from the Expert controller, send data back to Contoller and PLC
Driving -> Control for moving to the right position given by PLc or Expert controller
Error   -> Something went wrong / emerency button is pressed
*/

#include "pico/stdlib.h"
#include "UART_Expert.h"
#include "PWM_Encoder.h"
#include "DEV_Config.h"
#include "MCP2515.h"

/// @brief All the states
typedef enum {
    PLC = 0,
    EXPERT = 1,
    PLC_EXPERT = 2,
    ERROR = 3
} state_t;

/// @brief //All the possible kind of error to made the problem easier to find (not needed for PLC, but for expert controller)
typedef enum {      
    CURRENT_1,
    CURRENT_2,
    TEMP,
    MOVEMENT
} ERROR_t;

/// @brief Checkes when to switch cases
void next_state();

/// @brief What happends in a case
void output();

/// @brief The code for the Expert mode
void Expert_mode();

/// @brief The code for the PLC mode
void PLC_mode();

/// @brief The code for the Error_mode
void Error_mode();

/// @brief The code for moving the Motor to an given position
/// @param go Whether to move the motor or not. This is needed for the free movement option.
/// @return Give back the current position
void Movement(int Position, int Forward, int Backward, int Stop);

/// @brief The code for sending data to the PLC 
void PLC_Sending();

void Closed_loop();