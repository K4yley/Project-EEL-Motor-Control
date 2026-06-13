#include "pico/stdlib.h" // Standard library for Pico
#include "stdio.h"

//Selfmade libaries
#include "DEV_Config.h"
#include "Debug.h"
#include "MCP2515.h"
#include "PWM_Encoder.h"

//PLC connection
#define TX_ID_0  0x123   // voltage + current1
#define TX_ID_1  0x124   // current2 + temperature
#define TX_ID_2  0x125   // position + speed
#define TX_ID_3  0x126   // status
#define RX_ID_0  0x321  

//sensor data
float   tx_voltage     = 15.0f;   // Spanning in volt
float   tx_current1    = 2.0f;    // Stroom 1 in ampère
float   tx_current2    = 0.0f;    // Stroom 2 in ampère
float   tx_temperature = 50.0f;   // Temperatuur in °C
int32_t tx_position    = 10;    // Positie in stappen/pulsen
float   tx_speed       = 3.0f;   // Snelheid in m/s of rpm
int32_t tx_status      = 1;    // Statusbits (bitmasker)

int main() {
    stdio_init_all(); // needed for picotool to autoload
    
    int rx_Position = 0;
    int rx_Forward = 0;
    int rx_Backward = 0;
    int rx_Stop = 0;

    uint8_t txBuf[8] = {0};
    uint8_t rxBuf[8] = {0};

    bool bSendTrigger = false;

    //PWM and Encoder setup
    setup_PWM_Encoder();

    //PLC connection setup
    DEV_Module_Init();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("MCP2515_Init\r\n");
    MCP2515_Init();
    DEV_Delay_ms(3000);

    while (true) {
        //Encoder
        if (elapsed >= Enc_measure) {
            float time_s = elapsed / 1000000.0f;
            RPM = RPM_counting(pulseCount, time_s);
            printf("PulseCount: %d | RPM: %.2f, Position: %d\n", pulseCount, RPM, positionTicks);
            pulseCount = 0;
            Enc_timer_old = time_us_32();
        }
    }   
}