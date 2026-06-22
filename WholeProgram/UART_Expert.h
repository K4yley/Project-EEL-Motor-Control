#include "pico/stdlib.h" // Standard library for Pico
#include "stdio.h"

#include "PWM_Encoder_Sensors.h"

#pragma once 

#define MAX_SIZE 128

/// @brief Commands that can be received from the external Expert controller.
typedef enum {
    CMD_STOP,
    CMD_JOG_LEFT,
    CMD_JOG_RIGHT,
    CMD_MOVE_ABS,
    CMD_CAL_SLOT
} expert_cmd_t;

/// @brief 
typedef struct {
    expert_cmd_t command;
    int32_t value;          // Used only for target position [mm] or jog direction.
} ExpertCommand_t;
extern volatile ExpertCommand_t UART;

/// @brief The interrupt for the uart reading
void on_uart_rx();

/// @brief Setup for the Expert mode
void setup_uart();

/// @brief Sending information to the expert mode
void sending_uart();
