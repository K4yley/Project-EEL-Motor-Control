#include "States.h"     //The .h file with all the information

#define Expert_mode_active 3    //The GPIO that gives a HIGH signal when the expert controller has been connected
state_t state = EXPERT;        //start state when activate the program

void next_state(){
    switch(state){
        case EXPERT:
            if(Expert_mode_active == false){     //When the expert mode controller has been disconnected 
                state = PLC;
            }
            break;
        case PLC:
            if(Expert_mode_active == true){
                state = EXPERT;
            }
            break;
        case ERROR:
            if(Expert_mode_active == true){         //when the expert mode is connected, its going out of the Error state
                state = EXPERT;
                setup_uart();       //Connection UART
            }
            break;
    }
}

void output() {
    //reading the sensors
    //sending data to PLC
    switch (state) {
        case EXPERT:
            //sending read data to Expert controller
            Expert_mode();
            break;
        case PLC:
              PLC_mode();
            break;
        case ERROR:
            Error_mode();
            break;
        case PLC_EXPERT:
            PLC_Expert();
    }
}

void Expert_mode(){
    //afhankelijk van wat Nikita stuur komen hier if-else statements
    Movement(rx_Position, rx_Forward, rx_Backward, rx_Stop);
    /* 
        Reads input data
            when position; call Movement();
            other options:
                free movement
                stop
                start
                up / right
                down / left
    */
   sending_uart();
}

void PLC_mode(){
    if (MCP2515_MessageAvailable()) {       //reading input
        // Reads the CAN ID
        uint32_t rxId = MCP2515_GetRxId();
        MCP2515_Receive(rxId, rxBuf);

        rx_Position = rxBuf[0]; 
        rx_Forward  = rxBuf[1];     
        rx_Backward = rxBuf[2];     
        rx_Stop     = rxBuf[3];    

        printf("RX frame: Position: %u, Forward: %u, Backward: %u, Stop: %u\r\n",(int)rx_Position, (int)rx_Forward, (int)rx_Backward, (int)rx_Stop);

        if(rx_Forward == 1 || rx_Backward == 1 || rx_Stop == 1){  //checking if expert PLC mode
            Expert_plc_bool = true;
        }
        Movement(rx_Position, rx_Forward, rx_Backward, rx_Stop);  //position movement
    }

    PLC_Sending();
}

void Error_mode(){
    /*
        Stop the system
            call movement();        //for stopping
            sending error code to PLC
            waiting for Expert mode
    */
}

void PLC_Sending(){
    // Frame 0: voltage + current1  →  TX ID 0x123
    memcpy(txBuf + 0, &tx_voltage,  sizeof(float));
    memcpy(txBuf + 4, &tx_current1, sizeof(float));
    MCP2515_Send(TX_ID_0, txBuf, 8);

    // Frame 1: current2 + temperature  →  TX ID 0x124
    memcpy(txBuf + 0, &tx_current2,    sizeof(float));
    memcpy(txBuf + 4, &tx_temperature, sizeof(float));
    MCP2515_Send(TX_ID_1, txBuf, 8);

    // Frame 2: position + speed  →  TX ID 0x125
    memcpy(txBuf + 0, &tx_position, sizeof(int32_t));
    memcpy(txBuf + 4, &tx_speed,    sizeof(float));
    MCP2515_Send(TX_ID_2, txBuf, 8);

    // Frame 3: status  →  TX ID 0x126
    memcpy(txBuf + 0, &tx_status, sizeof(int32_t));
    MCP2515_Send(TX_ID_3, txBuf, 4);

    printf("TX: V=%.2f  I1=%.2f  I2=%.2f  T=%.2f  pos=%ld  spd=%.2f  st=0x%lX\r\n",
        (double)tx_voltage, (double)tx_current1,
        (double)tx_current2, (double)tx_temperature,
        (long)tx_position, (double)tx_speed, (long)tx_status);
}

void Movement(int Position, int Forward, int Backward, int Stop){
    if(Position != 0){
        //Blah blah blah
    }
    else if(Forward == 1 && Backward == 0){     //forward: clockwise
        for(int i = 1; i < 4; i++){
            pwm_set_enabled(i, false);  
        }
        pwm_set_counter(1, 41666);        //phase 1 to 240
        pwm_set_counter(3, 0);        //phase 3 to 0

        pwm_set_mask_enabled(0x0E);
        printf("Forward: Clockwise\n");
    }    
    else if(Backward == 1 && Forward == 0){     //backward: counter clockwise
        for(int i = 1; i < 4; i++){
            pwm_set_enabled(i, false);  
        }
        pwm_set_counter(1, 0);        //phase 1 to 0
        pwm_set_counter(3, 41666);        //phase 3 to 240

        pwm_set_mask_enabled(0x0E);
        printf("Backward: Counter Clockwise\n");
    }
    else if(Stop == 1 || (Backward == 1 && Forward == 1)){
        for(int i = 1; i < 4; i++){
            pwm_set_enabled(i, false);  
        }
        printf("Idle State\n");
    }
}

void Closed_loop(){
    
}
