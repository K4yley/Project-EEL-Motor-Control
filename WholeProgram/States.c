#include "States.h"  

volatile PLC_t PLC;
volatile state_t state;

void output(state_t state) {
    switch (state) {
        case EXPERT:
            sending_uart();
            Expert_mode();
            break;
        case PLC_MODE:
            sending_PLC();
            PLC_mode();
            break;
        case ERROR:
            Error_mode();
            break;
        case TEST:
            test2();
            break;
    }
    Closed_loop();
}

void Expert_mode(){
    if(UART.command == CMD_STOP){
        Motor(STOP);
    }
    else if(UART.command == CMD_JOG_LEFT){
        Motor(BACKWARD);
    }
    else if(UART.command == CMD_JOG_RIGHT){
        Motor(FORWARD);    
    }
    else if(UART.command == CMD_MOVE_ABS){
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

        PLC.Position = rxBuf[0]; 
        PLC.Forward  = rxBuf[1];     
        PLC.Backward = rxBuf[2];     
        PLC.Stop     = rxBuf[3];    

        printf("RX frame: Position: %u, Forward: %u, Backward: %u, Stop: %u\r\n",(int)PLC.Position, (int)PLC.Forward, (int)PLC.Backward, (int)PLC.Stop);

        if(PLC.Stop == 1){
            Motor(STOP);
        }
        else if(PLC.Forward == 1 && PLC.Backward != 1){
            Motor(FORWARD);
        }
        else if(PLC.Backward == 1 && PLC.Forward != 1){
            Motor(BACKWARD);
        }
        else{
            Motor_newpos(PLC.Position, Encoder.Position_Motor);
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
    memcpy(txBuf + 0, &Sensor.voltage_v,  sizeof(float));
    memcpy(txBuf + 4, &Sensor.current1_a, sizeof(float));
    MCP2515_Send(TX_ID_0, txBuf, 8);

    // Frame 1: current2 + temperature  →  TX ID 0x124
    memcpy(txBuf + 0, &Sensor.current2_a,    sizeof(float));
    memcpy(txBuf + 4, &Sensor.temperature_c, sizeof(float));
    MCP2515_Send(TX_ID_1, txBuf, 8);

    // Frame 2: position + speed  →  TX ID 0x125
    memcpy(txBuf + 0, &Encoder.Position_Motor, sizeof(int32_t));
    memcpy(txBuf + 4, &Encoder.RPM, sizeof(float));
    MCP2515_Send(TX_ID_2, txBuf, 8);

    // Frame 3: status  →  TX ID 0x126
    memcpy(txBuf + 0, &state, sizeof(int32_t));
    MCP2515_Send(TX_ID_3, txBuf, 4);

    // printf("TX: V=%.2f  I1=%.2f  I2=%.2f  T=%.2f  pos=%ld  spd=%.2f  st=0x%lX\r\n",
    //     (double)Data.voltage_v, (double)Data.current1_a,
    //     (double)Data.current2_a, (double)Data.temperature_c,
    //     (long)Data.position_mm, (double)tx_speed, (long)tx_status);
}

void Closed_loop(){
    //Calculate the Live position
    Encoder.Position_Motor = LocationCal(Encoder.positionTicks);

    if(Encoder.targetPosition == Encoder.positionTicks){
        Motor(STOP);
    }
}


void test1(){
    int slice;
    char input = stdio_getchar_timeout_us(0.1);
        if(isalpha((unsigned char)input)){
            if(input == 'F' || input == 'f'){ //clockwise
                PWM_TurnOff();
                pwm_set_mask_enabled(0x00);    //stops PWM
                slice = pwm_gpio_to_slice_num(PWM_A0);
                pwm_set_counter(slice, 41666);        //phase 1 to 240
                pwm_set_counter(slice+1, 20833);
                pwm_set_counter(slice+2, 0);        //phase 3 to 0
                PWM_TurnOn();
                pwm_set_mask_enabled(mask);
                printf("Forward: Clockwise\n");
            }
            else if(input == 'B' || input == 'b'){ //counter clockwise
                PWM_TurnOff();
                pwm_set_mask_enabled(0x00);    //stops PWM
                slice = pwm_gpio_to_slice_num(PWM_A0);
                pwm_set_counter(slice, 0);        //phase 1 to 240
                pwm_set_counter(slice+1, 20833);
                pwm_set_counter(slice+2, 41666);        //phase 3 to 0
                PWM_TurnOn();
                pwm_set_mask_enabled(mask);
                printf("Backward: Counter Clockwise\n");
            }
            else if(input == 'i' || input == 'I'){
                pwm_set_mask_enabled(0x00);    //stops PWM
                printf("Idle State\n");
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