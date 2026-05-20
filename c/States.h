/*
The switching of states
PLC     -> Get information from the PLC, send dataq back to PLC
Expert  -> Get information from the Expert controller, send data back to Contoller and PLC
Driving -> Control for moving to the right position given by PLc or Expert controller
Error   -> Something went wrong / emerency button is pressed
*/

#include "pico/stdlib.h"

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

/// @brief The code for the Expert mode of the PLC 
void PLC_Expert();

/// @brief The code for the Error_mode
void Error_mode();

/// @brief The code for moving the Motor to an given position
/// @param position The coordinate where the motor need to move to
/// @return Give back the current position
int Movement(int position);

/// @brief The code for sending data to the PLC 
/// @param Data The Array of data that has to be send
void PLC_Sending(uint8_t Data[]);

/// @brief The code for sending data to the Expert controller
/// @param Data The Array of data that has to be send
void Expert_Sending(uint8_t Data[]);