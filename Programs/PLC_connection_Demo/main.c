#include "MCP2515.h"
#include "DEV_Config.h"
#include "Debug.h"
#include "pico/stdio_usb.h"
#include <stdbool.h>
#include <string.h>  // memcpy

// ─── Frame-indeling ───────────────────────────────────────────────────────────
// 7 waarden × 4 bytes = 28 bytes → verdeeld over 4 CAN frames (max 8 bytes elk)
//
//  Frame 0  (8 bytes)  │  float voltage   (0-3)  │  float current1  (4-7)
//  Frame 1  (8 bytes)  │  float current2  (0-3)  │  float temp      (4-7)
//  Frame 2  (8 bytes)  │  int32_t position(0-3)  │  float speed     (4-7)
//  Frame 3  (4 bytes)  │  int32_t status  (0-3)

// ─── TX CAN IDs (Pico → WAGO) ────────────────────────────────────────────────
#define TX_ID_0  0x123   // voltage + current1
#define TX_ID_1  0x124   // current2 + temperature
#define TX_ID_2  0x125   // position + speed
#define TX_ID_3  0x126   // status

// ─── RX CAN IDs (WAGO → Pico) ────────────────────────────────────────────────
#define RX_ID_0  0x321  

int main(void) {
    DEV_Module_Init();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("MCP2515_Init\r\n");
    MCP2515_Init();
    DEV_Delay_ms(3000);

    // ── TX data (Pico stuurt deze waarden naar de WAGO) ───────────────────────
    float   tx_voltage     = 15.0f;   // Spanning in volt
    float   tx_current1    = 2.0f;    // Stroom 1 in ampère
    float   tx_current2    = 0.0f;    // Stroom 2 in ampère
    float   tx_temperature = 50.0f;   // Temperatuur in °C
    int32_t tx_position    = 10;    // Positie in stappen/pulsen
    float   tx_speed       = 3.0f;   // Snelheid in m/s of rpm
    int32_t tx_status      = 1;    // Statusbits (bitmasker)

    // ── RX data (Pico ontvangt deze waarden van de WAGO) ─────────────────────
    int rx_Position = 0;
    int rx_Forward = 0;
    int rx_Backward = 0;
    int rx_Stop = 0;

    // ── Ruwe CAN buffers ──────────────────────────────────────────────────────
    uint8_t txBuf[8] = {0};
    uint8_t rxBuf[8] = {0};

    bool bSendTrigger = false;

    while (1) {

        // ── ONTVANGEN ────────────────────────────────────────────────────────
        // Controleer non-blocking of er een bericht klaarstaat
        if (MCP2515_MessageAvailable()) {
            
            // Lees het CAN ID van  het ontvangen frame
            uint32_t rxId = MCP2515_GetRxId();
            MCP2515_Receive(rxId, rxBuf);

            printf("RX [0x%03X]:", rxId);
 
            
            rx_Position = rxBuf[0]; 
            rx_Forward  = rxBuf[1];     
            rx_Backward = rxBuf[2];     
            rx_Stop     = rxBuf[3];    

            printf("RX frame: Position: %u, Forward: %u, Backward: %u, Stop: %u\r\n",(int)rx_Position, (int)rx_Forward, (int)rx_Backward, (int)rx_Stop);
            bSendTrigger = true;
        }

        // ── VERSTUREN ────────────────────────────────────────────────────────
        if (bSendTrigger) {

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

            bSendTrigger = false;
        }

        DEV_Delay_ms(10);
    }
    return 0;
}


