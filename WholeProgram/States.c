#include "States.h"  

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
    }
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
        Closed_loop(UART.value);
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
            Closed_loop(PLC.Position);
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
    memcpy(txBuf + 0, &Data.voltage_v,  sizeof(float));
    memcpy(txBuf + 4, &Data.current1_a, sizeof(float));
    MCP2515_Send(TX_ID_0, txBuf, 8);

    // Frame 1: current2 + temperature  →  TX ID 0x124
    memcpy(txBuf + 0, &Data.current2_a,    sizeof(float));
    memcpy(txBuf + 4, &Data.temperature_c, sizeof(float));
    MCP2515_Send(TX_ID_1, txBuf, 8);

    // Frame 2: position + speed  →  TX ID 0x125
    memcpy(txBuf + 0, &Data.position_mm, sizeof(int32_t));
    //memcpy(txBuf + 4, &Data.target_position_mm, sizeof(float));
    MCP2515_Send(TX_ID_2, txBuf, 8);

    // Frame 3: status  →  TX ID 0x126
    memcpy(txBuf + 0, &state, sizeof(int32_t));
    MCP2515_Send(TX_ID_3, txBuf, 4);

    // printf("TX: V=%.2f  I1=%.2f  I2=%.2f  T=%.2f  pos=%ld  spd=%.2f  st=0x%lX\r\n",
    //     (double)Data.voltage_v, (double)Data.current1_a,
    //     (double)Data.current2_a, (double)Data.temperature_c,
    //     (long)Data.position_mm, (double)tx_speed, (long)tx_status);
}

// void Closed_loop(long targetPosition){
//     const int controlPeriod = 20;   // 50 Hz (important for stability)
//     long pulses = Encoder.pulseCount;
//     Encoder.pulseCount = 0;
    
//     long position += pulses;

//     float posError = targetPosition - position;

//     float posOutput = computePID(positionPID, posError);
// }

// float computePID(PID_t *p, float e){
//   float dt = controlPeriod / 1000.0;

//   p->i += e * dt;
//   p->i = constrain(p->i, -80, 80);

//   float d = (e - p->last) / dt;
//   p->last = e;

//   return p->kp * e + p->ki * p->i + p->kd * d;
// }

