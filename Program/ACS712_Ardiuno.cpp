#include <Arduino.h>

const int sensorPin = A0;
const float VCC = 5.0;
const float sensitivity = 0.100; // 20A version

float offsetVoltage = 2.5;

// 🔧 tuning
const int samples = 2000;       // more samples = better low-current detection
const float noiseCutoff = 0.08; // filters small noise (in Amps)

void setup() {
  Serial.begin(9600);
  Serial.available();

  Serial.println("Calibrating... NO current!");
  delay(2000);

  long sum = 0;
  for (int i = 0; i < 1000; i++) {
    sum += analogRead(sensorPin);
    delay(2);
  }

  offsetVoltage = (sum / 1000.0) * (VCC / 1023.0);

  Serial.print("Offset: ");
  Serial.println(offsetVoltage, 4);
}

void loop() {
  float sumSquares = 0;

  for (int i = 0; i < samples; i++) {
    int raw = analogRead(sensorPin);
    float voltage = raw * (VCC / 1023.0);
    float current = (voltage - offsetVoltage) / sensitivity;

    // 🔧 remove tiny noise BEFORE squaring
    if (abs(current) < noiseCutoff) {
      current = 0;
    }

    sumSquares += current * current;
  }

  float rms = sqrt(sumSquares / samples);

  Serial.print("RMS Current: ");
  Serial.print(rms, 3);
  Serial.println(" A");

  delay(500);
}