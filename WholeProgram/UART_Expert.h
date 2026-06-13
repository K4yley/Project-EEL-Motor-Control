#include "pico/stdlib.h" // Standard library for Pico
#include "stdio.h"

#define MAX_SIZE 128
char rx_buff[MAX_SIZE];     //the buffer for the outgoing data
char tx_buff[MAX_SIZE];
int rx_i = 0;

//Incomming Data
int I_Status;         //Set Position
int I_Value1;         //Set move forward; 0 or 1
int I_Value2;        //Set move backwards; 0 or 1
/*Expect: 
    "Position, Forwards, backwards, Stop"
    " 5, T, F, F"
    " 10, F, T, F"
    //State is everything about emerency; when true, stop all
*/

/// @brief The interrupt for the uart reading
void on_uart_rx();

/// @brief Setup for the Expert mode
void setup_uart();

/// @brief Sending information to the expert mode
void sending_uart();
