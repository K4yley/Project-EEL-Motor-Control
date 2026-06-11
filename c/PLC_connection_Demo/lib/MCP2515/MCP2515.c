/*****************************************************************************
 * | File      	:   MCP2515.c
 * | Author      :   EZIO
 * | Function    :   MCP2515 CAN controller driver
 * | Version     :   V1.1
 ******************************************************************************/
#include "MCP2515.h"
#include "DEV_Config.h"

static void MCP2515_WriteBytes(uint8_t Addr, uint8_t Data)
{
    DEV_Digital_Write(MCP2515_CS_PIN, 0);
    DEV_SPI_WriteByte(CAN_WRITE);
    DEV_SPI_WriteByte(Addr);
    DEV_SPI_WriteByte(Data);
    DEV_Digital_Write(MCP2515_CS_PIN, 1);
}

static uint8_t MCP2515_ReadByte(uint8_t Addr)
{
    uint8_t rdata;
    DEV_Digital_Write(MCP2515_CS_PIN, 0);
    DEV_SPI_WriteByte(CAN_READ);
    DEV_SPI_WriteByte(Addr);
    rdata = DEV_SPI_ReadByte();
    DEV_Digital_Write(MCP2515_CS_PIN, 1);
    return rdata;
}

void MCP2515_Reset(void)
{
    DEV_Digital_Write(MCP2515_CS_PIN, 0);
    DEV_SPI_WriteByte(CAN_RESET);
    DEV_Digital_Write(MCP2515_CS_PIN, 1);
}

uint8_t CAN_RATE[10][3] = {
    {0xA7, 0XBF, 0x07},
    {0x31, 0XA4, 0X04},
    {0x18, 0XA4, 0x04},
    {0x09, 0XA4, 0x04},
    {0x04, 0x9E, 0x03},
    {0x03, 0x9E, 0x03},
    {0x01, 0x1E, 0x03},
    {0x00, 0x9E, 0x03},
    {0x00, 0x92, 0x02},
    {0x00, 0x82, 0x02}
};

void MCP2515_Init(void)
{
    printf("MCP2515 Init\r\n");
    MCP2515_Reset();
    DEV_Delay_ms(100);

    // 500 kbps
    MCP2515_WriteBytes(CNF1, CAN_RATE[KBPS500][0]);
    MCP2515_WriteBytes(CNF2, CAN_RATE[KBPS500][1]);
    MCP2515_WriteBytes(CNF3, CAN_RATE[KBPS500][2]);

    // TX buffer 0: pre-load CAN ID 0x123 (also overwritten per-send in MCP2515_Send)
    MCP2515_WriteBytes(TXB0SIDH, (0x123 >> 3) & 0xFF);
    MCP2515_WriteBytes(TXB0SIDL, (0x123 & 0x07) << 5);
    MCP2515_WriteBytes(TXB0DLC, 0x40 | DLC_8);

    // RX buffer 0: receive-all mode (RXM=11 in bits[6:5] bypasses filters)
    MCP2515_WriteBytes(RXB0CTRL, 0x60);
    MCP2515_WriteBytes(RXB0DLC, DLC_8);

    // RX filter 0 set to 0x321 with exact-match mask
    MCP2515_WriteBytes(RXF0SIDH, (0x321 >> 3) & 0xFF);
    MCP2515_WriteBytes(RXF0SIDL, (0x321 & 0x07) << 5);
    MCP2515_WriteBytes(RXM0SIDH, 0xFF);
    MCP2515_WriteBytes(RXM0SIDL, 0xE0);

    MCP2515_WriteBytes(CANINTF, 0x00);
    MCP2515_WriteBytes(CANINTE, 0x01);

    MCP2515_WriteBytes(CANCTRL, REQOP_NORMAL | CLKOUT_ENABLED);

    uint8_t dummy = MCP2515_ReadByte(CANSTAT);
    if ((dummy & 0xE0) != OPMODE_NORMAL) {
        printf("Re-entering normal mode\r\n");
        MCP2515_WriteBytes(CANCTRL, REQOP_NORMAL | CLKOUT_ENABLED);
    }
}

void MCP2515_Send(uint32_t Canid, uint8_t *Buf, uint8_t len)
{
    uint8_t dly = 0;
    while ((MCP2515_ReadByte(TXB0CTRL) & 0x08) && (dly < 50)) {
        DEV_Delay_ms(1);
        dly++;
    }

    MCP2515_WriteBytes(TXB0SIDH, (Canid >> 3) & 0xFF);
    MCP2515_WriteBytes(TXB0SIDL, (Canid & 0x07) << 5);
    MCP2515_WriteBytes(TXB0EID8, 0);
    MCP2515_WriteBytes(TXB0EID0, 0);
    MCP2515_WriteBytes(TXB0DLC, len);

    for (uint8_t j = 0; j < len; j++) {
        MCP2515_WriteBytes(TXB0D0 + j, Buf[j]);
    }
    MCP2515_WriteBytes(TXB0CTRL, 0x08);
}

uint8_t MCP2515_MessageAvailable(void)
{
    return MCP2515_ReadByte(CANINTF) & 0x01;
}

uint32_t MCP2515_GetRxId(void)
{
    // Lees het ontvangen CAN ID uit de RX buffer registers
    uint8_t sidh = MCP2515_ReadByte(RXB0SIDH);         // SID bits 10:3
    uint8_t sidl = MCP2515_ReadByte(RXB0SIDL);         // SID bits 2:0 in bits [7:5]
    return ((uint32_t)sidh << 3) | ((sidl >> 5) & 0x07);
}

void MCP2515_Receive(uint32_t Canid, uint8_t *CAN_RX_Buf)
{
    (void)Canid;  // hardware is in receive-all mode; Canid is not used for filtering

    uint8_t len = MCP2515_ReadByte(RXB0DLC) & 0x0F;
    if (len > 8) len = 8;

    for (uint8_t i = 0; i < len; i++) {
        CAN_RX_Buf[i] = MCP2515_ReadByte(RXB0D0 + i);
    }

    MCP2515_WriteBytes(CANINTF, 0x00);
    MCP2515_WriteBytes(CANINTE, 0x01);
}
