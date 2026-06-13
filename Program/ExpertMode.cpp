#include <Arduino.h>
#include <SPI.h>
#include <Arduino_GFX_Library.h>

// DISPLAY CONFIGURATION

#define SPI_SCK_PIN   18
#define SPI_MOSI_PIN  19
#define SPI_MISO_PIN  16

#define LCD_CS_PIN    26
#define LCD_DC_PIN    21
#define LCD_RST_PIN   20
#define LCD_BL_PIN    22

#define SCREEN_W      480
#define SCREEN_H      320

// COLORS

#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define CYAN    0x07FF
#define GRAY    0x8410
#define DARK    0x2104
#define ORANGE  0xFD20

// DISPLAY OBJECT

Arduino_DataBus *bus = new Arduino_RPiPicoSPI(
    LCD_DC_PIN,
    LCD_CS_PIN,
    SPI_SCK_PIN,
    SPI_MOSI_PIN,
    SPI_MISO_PIN
);

Arduino_GFX *display = new Arduino_ILI9488(
    bus,
    LCD_RST_PIN,
    1,      // rotation: 1 = landscape
    false  // IPS
);

// DEMO VALUES

float carriagePosition = 420.5;
float carriageSpeed = 35.0;
float currentValue = 1.25;
float voltageValue = 24.1;
float temperatureValue = 36.8;

String carriageStatus = "IDLE";
int selectedSlot = 1;
int speedSetting = 30;

float MIN_POSITION_MM = 0.0;
float MAX_POSITION_MM = 1000.0;
float STOP_MARGIN_MM = 20.0;
float SLOW_ZONE_MM = 100.0;

// BASIC DRAWING FUNCTIONS

void drawText(int x, int y, const String &text, uint16_t color, uint8_t size = 2) {
    display->setCursor(x, y);
    display->setTextColor(color);
    display->setTextSize(size);
    display->print(text);
}

void drawButton(int x, int y, int w, int h, const String &label, uint16_t color) {
    display->fillRect(x, y, w, h, color);
    display->drawRect(x, y, w, h, WHITE);

    display->setTextColor(WHITE);
    display->setTextSize(2);

    int textX = x + 10;
    int textY = y + h / 2 - 8;

    display->setCursor(textX, textY);
    display->print(label);
}

void drawPositionBar(float position) {
    int barX = 20;
    int barY = 140;
    int barW = 440;
    int barH = 8;

    display->fillRect(barX, barY, barW, barH, GRAY);

    float ratio = (position - MIN_POSITION_MM) / (MAX_POSITION_MM - MIN_POSITION_MM);

    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    int filled = (int)(barW * ratio);

    uint16_t color = GREEN;

    if (position <= MIN_POSITION_MM + SLOW_ZONE_MM) {
        color = YELLOW;
    }

    if (position >= MAX_POSITION_MM - SLOW_ZONE_MM) {
        color = YELLOW;
    }

    if (position <= MIN_POSITION_MM + STOP_MARGIN_MM) {
        color = RED;
    }

    if (position >= MAX_POSITION_MM - STOP_MARGIN_MM) {
        color = RED;
    }

    display->fillRect(barX, barY, filled, barH, color);
}

void drawStaticScreen() {
    display->fillScreen(BLACK);

    drawText(20, 15, "EXPERT MODE - CARRIAGE HMI", CYAN, 2);

    display->drawRect(10, 50, 460, 100, GRAY);
    display->drawRect(10, 155, 460, 95, GRAY);
    display->drawRect(10, 215, 460, 90, GRAY);

    drawButton(20, 220, 100, 60, "LEFT", BLUE);
    drawButton(140, 220, 100, 60, "STOP", RED);
    drawButton(260, 220, 100, 60, "RIGHT", BLUE);
    drawButton(380, 220, 80, 60, "CAL", ORANGE);

    drawButton(20, 160, 80, 45, "SPD-", DARK);
    drawButton(380, 160, 80, 45, "SPD+", DARK);

    drawButton(130, 160, 70, 45, "S1", DARK);
    drawButton(210, 160, 70, 45, "S2", DARK);
    drawButton(290, 160, 70, 45, "S3", DARK);
}

void updateTelemetryArea() {
    display->fillRect(15, 55, 450, 90, BLACK);

    drawText(20, 60, "Pos: " + String(carriagePosition, 1) + " mm", WHITE, 2);
    drawText(250, 60, "Spd: " + String(carriageSpeed, 1) + " mm/s", WHITE, 2);

    drawText(20, 90, "I: " + String(currentValue, 2) + " A", WHITE, 2);
    drawText(170, 90, "V: " + String(voltageValue, 1) + " V", WHITE, 2);
    drawText(310, 90, "T: " + String(temperatureValue, 1) + " C", WHITE, 2);

    drawText(20, 120, "Status: " + carriageStatus, YELLOW, 2);
    drawText(330, 120, "COMMS OK", GREEN, 2);

    drawPositionBar(carriagePosition);
}

void updateSettingsArea() {
    display->fillRect(105, 165, 120, 35, BLACK);
    drawText(105, 170, String(speedSetting) + "%", GREEN, 2);

    display->fillRect(300, 20, 150, 25, BLACK);
    drawText(300, 20, "Slot: " + String(selectedSlot), WHITE, 2);
}

void updateDisplay() {
    updateTelemetryArea();
    updateSettingsArea();
}

// SETUP

void setup() {
    Serial.begin(115200);
    delay(500);

    pinMode(LCD_BL_PIN, OUTPUT);
    digitalWrite(LCD_BL_PIN, HIGH);

    SPI.begin();

    if (!display->begin()) {
        Serial.println("Display initialization failed");
        while (true) {delay(1000);}
    }

    display->fillScreen(BLACK);

    drawStaticScreen();
    updateDisplay();

    Serial.println("Display started");
}

// DEMO LOOP

void loop() {
    static unsigned long lastUpdate = 0;

    if (millis() - lastUpdate >= 500) {
        lastUpdate = millis();
        carriagePosition += 5.0;
        if (carriagePosition > MAX_POSITION_MM) {
            carriagePosition = MIN_POSITION_MM;
        }
        updateDisplay();
    }
}
