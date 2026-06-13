#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <math.h>

//const int sensorPin = A0;
const float VCC = 5.0;
const float sensitivity = 0.100; // 20A version

float offsetVoltage = 1.67;

// 🔧 tuning
const int samples = 2000;       // more samples = better low-current detection
const float noiseCutoff = 0.08; // filters small noise (in Amps)

int main() {
    stdio_init_all(); // needed for picotool to autoload
    
    adc_init(); // Initialize the ADC
    adc_gpio_init(26); // Initialize the GPIO pin for the ADC
    adc_select_input(0); // Select the ADC input, channel 0 = pin 26
    
    while(true){
        float sumSquares = 0;
        uint16_t raw = 0;

        for (int i = 0; i < samples; i++) {
            uint16_t raw = adc_read();
            float voltage = ((float)raw / 4095.0f) * VCC;
            float current = (voltage - offsetVoltage) / sensitivity;

            // 🔧 remove tiny noise BEFORE squaring
            if (fabsf(current) < noiseCutoff) {
                current = 0;
            }

            sumSquares += current * current;
        }

        float rms = sqrt(sumSquares / (float)samples);
        
        printf("RMS Current: %0.3f A, last raw: %u\n", rms, raw);
         sleep_ms(500); 
    }
}