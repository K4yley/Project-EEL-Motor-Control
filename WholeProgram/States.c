#include "States.h"  

volatile PLC_t PLC;
volatile state_t state;
repeating_timer_t motor_timer; 

void next_state(){
    //char input = stdio_getchar_timeout_us(0.1);
    switch(state){
        //EXPERT: going to PLC when disconnected
        //Otherwise with reading of terminal bec of testing
        case EXPERT:
            if(gpio_get(EXPERT_MODE_ACTIVE_PIN) == false){     //When the expert mode controller has been disconnected 
                state = PLC_MODE;
                printf("PLC_Mode\n");
                Motor(STOP);
            }
            break;
        //PLC: going to PLC when pin is connected
        //Otherwise with reading of terminal bec of testing
        case PLC_MODE:
            if(gpio_get(EXPERT_MODE_ACTIVE_PIN) == true){
                //state = EXPERT;
                state = TEST;
                printf("Expert\n");
                Motor(STOP);
            }
            break;
        //ERROR: Only solved via EXPERT mode
        case ERROR:
            if(gpio_get(EXPERT_MODE_ACTIVE_PIN) == true){         //when the expert mode is connected, its going out of the Error state
                state = EXPERT;
                Motor(STOP);
            }
            break;
        //TEST: just for testing
        //Can choose where to go
        case TEST:
            // if(isalpha((unsigned char)input)){
            //     if(input == 'E' || input == 'e'){
            //         state = EXPERT;
            //         printf("Expert mode\n");
            //     }   
            //     else if(input == 'P' || input == 'p'){
            //         state = PLC_MODE;
            //         printf("PLC mode\n");
            //     }
            // }
            if(gpio_get(EXPERT_MODE_ACTIVE_PIN) == false){     //When the expert mode controller has been disconnected 
                state = PLC_MODE;
                printf("PLC_Mode\n");
                Motor(STOP);
            }
            break;
    }
}

void output(state_t state) {
    switch (state) {
        case EXPERT:
            sending_uart();
            Expert_mode();
            break;
        case PLC_MODE:
            PLC_mode();
            break;
        case ERROR:
            Error_mode();
            break;
        case TEST:
            test3();    
            break;
    }
    Closed_loop();
    sending_PLC();
}

void Expert_mode(){
    if(UART.command == CMD_STOP){
        Pos_Control = false;
        Motor(STOP);
    }
    else if(UART.command == CMD_JOG_LEFT){
        Pos_Control = false;
        Motor(BACKWARD);
    }
    else if(UART.command == CMD_JOG_RIGHT){
        Pos_Control = false;
        Motor(FORWARD);    
    }
    else if(UART.command == CMD_MOVE_ABS){
        Pos_Control = true;
        Motor_newpos(UART.value, Encoder.Position_Motor);
    }
    else if(UART.command == CMD_CAL_SLOT){
        //does something
    }
}

void PLC_mode(){
    uint8_t rxBuf[MAX_SIZE];
    if (MCP2515_MessageAvailable()) {       //reading input
        // Reads the CAN ID
        uint32_t rxId = MCP2515_GetRxId();
        MCP2515_Receive(rxId, rxBuf);

        PLC.Forward = rxBuf[0]; 
        PLC.Backward  = rxBuf[1];     
        PLC.Position = rxBuf[2];     
        PLC.Stop     = rxBuf[3];    

        printf("RX frame: Position: %u, Forward: %u, Backward: %u, Stop: %u\r\n",(int)PLC.Position, (int)PLC.Forward, (int)PLC.Backward, (int)PLC.Stop);

        if(PLC.Stop == 1){
            Motor(STOP);
            Pos_Control = false;
        }
        else if(PLC.Forward == 1 && PLC.Backward != 1){
            Motor(FORWARD);
            Pos_Control = false;
        }
        else if(PLC.Backward == 1 && PLC.Forward != 1){
            Motor(BACKWARD);
            Pos_Control = false;
        }
        else{
            Motor_newpos(PLC.Position, Encoder.Position_Motor);
            Pos_Control = true;
        }
    }
}

void Error_mode(){
    /*
        Stop the system
            call movement();        //for stopping
            sending error code to PLC
            waiting for Expert mode
    */
}

void sending_PLC(){
    uint8_t txBuf[8] = {0};
    // Frame 0: voltage + current1  →  TX ID 0x123
    volatile_memcpy(txBuf + 0, &Sensor.voltage_v,  sizeof(float));
    volatile_memcpy(txBuf + 4, &Sensor.current1_a, sizeof(float));
    MCP2515_Send(TX_ID_0, txBuf, 8);

    // Frame 1: current2 + temperature  →  TX ID 0x124
    volatile_memcpy(txBuf + 0, &Sensor.current2_a,    sizeof(float));
    volatile_memcpy(txBuf + 4, &Sensor.temperature_c, sizeof(float));
    MCP2515_Send(TX_ID_1, txBuf, 8);

    // Frame 2: position + speed  →  TX ID 0x125
    volatile_memcpy(txBuf + 0, &Encoder.Position_Motor, sizeof(float));
    volatile_memcpy(txBuf + 4, &Encoder.RPM, sizeof(float));
    MCP2515_Send(TX_ID_2, txBuf, 8);

    float new = (float)state;
    // Frame 3: status  →  TX ID 0x126
    volatile_memcpy(txBuf + 0, &new, sizeof(float));
    MCP2515_Send(TX_ID_3, txBuf, 4);

    // printf("TX: V=%.2f  I1=%.2f  I2=%.2f  T=%.2f  pos=%.2f  spd=%.2f  st=%.2f\r\n",
    //     (float)Sensor.voltage_v, (float)Sensor.current1_a,
    //     (float)Sensor.current2_a, (float)Sensor.temperature_c,
    //     (float)Encoder.Position_Motor, (float)Encoder.RPM, (float)new);
}

void Closed_loop(){
    //Calculate the Live position
    Encoder.Position_Motor = LocationCal(Encoder.positionTicks);

    if(Pos_Control){    //if position control, then interrupt active
        add_repeating_timer_ms(-10, motor_control_callback, NULL, &motor_timer);
    }
    else{   //if no position control, interrupt stops
        cancel_repeating_timer(&motor_timer); 
    }
}

bool motor_control_callback(struct repeating_timer *t){
    if(Encoder.targetPosition == Encoder.positionTicks){
        Motor(STOP);
        return false;
    }
    return true;
}


void test1(){
    char input = stdio_getchar_timeout_us(0.1);
        if(isalpha((unsigned char)input)){
            if(input == 'F' || input == 'f'){ //clockwise
                Pos_Control = false;
                Motor(FORWARD);
            }
            else if(input == 'B' || input == 'b'){ //counter clockwise
                Pos_Control = false;
                Motor(BACKWARD);
            }
            else if(input == 'i' || input == 'I'){
                Pos_Control = false;
                Motor(STOP);
            }
        }
}

void test2(){
    char input = stdio_getchar_timeout_us(0);
    if(isdigit(input)){
        float new_input = atof(&input);
        Motor_newpos(new_input, Encoder.Position_Motor);

    }
    else if(isalpha((unsigned char)input)){
        if(input == 'i' || input == 'I'){
                pwm_set_mask_enabled(0x00);    //stops PWM
                printf("Idle State\n");
        }
    }
}

void test3(){
    char input = stdio_getchar_timeout_us(0);
    if(isdigit(input)){
        float new_input = atof(&input);
        Motor_newpos(new_input, Encoder.Position_Motor);

    }
    else if(isalpha((unsigned char)input)){
        if(input == 'F' || input == 'f'){ //clockwise
                Pos_Control = false;
                Motor(FORWARD);
                printf("Forward\n");
            }
            else if(input == 'B' || input == 'b'){ //counter clockwise
                Pos_Control = false;
                Motor(BACKWARD);
                printf("Backward\n");
            }
            else if(input == 'i' || input == 'I'){
                Pos_Control = false;
                Motor(STOP);
                printf("Idle State\n");
            }
    }
}

void volatile_memcpy(volatile void *dest, const volatile void *src, size_t n) {
    volatile uint8_t *d = (volatile uint8_t *)dest;
    const volatile uint8_t *s = (const volatile uint8_t *)src;
    
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
}
