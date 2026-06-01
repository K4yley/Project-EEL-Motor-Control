
#include "pico/stdlib.h"
#include "stdio.h"

// Encoder pins
#define HALL_A 2
#define HALL_B 3
#define HALL_C 4

static bool Last_A;
static bool Last_B;
static bool Last_C;

// Pass pulseCount by pointer so updates persist in main()
void PulseCounting(int *pulseCount) {
    if (gpio_get(HALL_A) != Last_A) {
        (*pulseCount)++;
        Last_A = !Last_A;
    }
    if (gpio_get(HALL_B) != Last_B) {
        (*pulseCount)++;
        Last_B = !Last_B;
    }
    if (gpio_get(HALL_C) != Last_C) {
        (*pulseCount)++;
        Last_C = !Last_C;
    }
}

// Now takes pulses and elapsed time (in seconds) as parameters
float RPM_counting(int pulses, float time_s) {
    return (pulses / 24.0f) * (60.0f / time_s);
}

int main() {
    stdio_init_all();

    int pulseCount = 0;
    float RPM = 0.0f;

    // 5,000,000 us = 5 seconds sample time; adjust as needed
    uint32_t Enc_measure = 5000000;
    uint32_t Enc_timer_old = time_us_32();

    // Initialize all three Hall sensor pins
    gpio_init(HALL_A);
    gpio_set_dir(HALL_A, GPIO_IN);
    gpio_init(HALL_B);
    gpio_set_dir(HALL_B, GPIO_IN);
    gpio_init(HALL_C);                       // was missing
    gpio_set_dir(HALL_C, GPIO_IN);           // was missing

    Last_A = gpio_get(HALL_A);
    Last_B = gpio_get(HALL_B);
    Last_C = gpio_get(HALL_C);

    while (true) {
        PulseCounting(&pulseCount);          // pass by pointer

        uint32_t Enc_timer = time_us_32();
        uint32_t elapsed = Enc_timer - Enc_timer_old;

        if (elapsed >= Enc_measure) {
            float time_s = elapsed / 1000000.0f; // convert µs → seconds
            RPM = RPM_counting(pulseCount, time_s);
            printf("PulseCount: %d | RPM: %.2f\n", pulseCount, RPM);
            pulseCount = 0;
            Enc_timer_old = time_us_32();
        }
    }
}
