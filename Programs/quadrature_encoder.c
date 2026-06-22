
#include "pico/stdlib.h"
#include "stdio.h"


#define HALL_A 2
#define HALL_B 3

// static bool Last_A;
// static bool Last_B;
// static bool Last_C;

// Pass pulseCount by pointer so updates persist in main()
// void PulseCounting(int *pulseCount) {
//     if (gpio_get(HALL_A) != Last_A) {
//         (*pulseCount)++;
//         Last_A = !Last_A;
//     }
//     if (gpio_get(HALL_B) != Last_B) {
//         (*pulseCount)++;
//         Last_B = !Last_B;
//     }
//     if (gpio_get(HALL_C) != Last_C) {
//         (*pulseCount)++;
//         Last_C = !Last_C;
//     }
// }



    gpio_init(HALL_A);
    gpio_set_dir(HALL_A, GPIO_IN);
    gpio_init(HALL_B);
    gpio_set_dir(HALL_B, GPIO_IN);           

    gpio_set_irq_enabled_with_callback(
        HALL_A,
        GPIO_IRQ_EDGE_RISE,
        true,
        &PulseCounting
    );

    gpio_set_irq_enabled_with_callback(
        HALL_B,
        GPIO_IRQ_EDGE_RISE,
        true,
        &PulseCounting
    );

    int positionTicks = 0;
    int pulseCount = 0;

void PulseCounting(uint gpio, uint32_t events) {
    if (gpio == HALL_A) {

        if (gpio_get(HALL_B) == 0) {
            positionTicks++;
        } else {
            positionTicks--;
        }
    } else if (gpio == HALL_B) {
        if (gpio_get(HALL_A) == 1) {
            positionTicks++;
        } else {
            positionTicks--;
        }
    }
    pulseCount++;
}

// RPM calculating
float RPM_counting(int pulses, float time_us) {
    return (pulses / 1024.0f) * (60000000.0f / time_us);
}

int main() {
    stdio_init_all()

    float RPM = 0.0f;

    uint32_t Enc_measure = 100000; //100ms sample time
    uint32_t Enc_timer_old = time_us_32();

    // // Initialize all three Hall sensor pins
    // gpio_init(HALL_A);
    // gpio_set_dir(HALL_A, GPIO_IN);
    // gpio_init(HALL_B);
    // gpio_set_dir(HALL_B, GPIO_IN);

    // Last_A = gpio_get(HALL_A);
    // Last_B = gpio_get(HALL_B);

    while (true) {
        PulseCounting(&pulseCount);        

        uint32_t Enc_timer = time_us_32();
        uint32_t elapsed_us = Enc_timer - Enc_timer_old;

        if (elapsed >= Enc_measure) {
            RPM = RPM_counting(pulseCount, time_s);
            printf("PulseCount: %d | PulseCount: %d | RPM: %.2f\n", pulseCount, positionTicks, RPM);
            pulseCount = 0;
            Enc_timer_old = time_us_32();
        }
    }
}
