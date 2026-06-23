#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "States.h"

/// @brief Reads the sensors
/// @brief Interrupt; Encoder reading
void core1_entry() {
    uint32_t Enc_measure = 250000;  
    uint32_t Enc_timer_old = time_us_32();
    
    setup_Encoder();

    while(true) {
        uint32_t Enc_timer = time_us_32();
        uint32_t elapsed = Enc_timer - Enc_timer_old;

        if (elapsed >= Enc_measure) {
            float time_s = elapsed / 1000000.0f;
            Encoder.RPM = RPM_counting(Encoder.pulseCount, time_s);
            printf("PulseCount: %d | positionTicks: %d | RPM: %0.2f | Position: %0.2f | Target: %0.2f\n", Encoder.pulseCount, Encoder.positionTicks, Encoder.RPM, Encoder.Position_Motor, Encoder.targetPosition);
            Encoder.pulseCount = 0;
            Enc_timer_old = time_us_32();
        }
        // //reading Voltage
        // setup_Sensor(Voltage_Pin, Voltage_CH);
        // Data.voltage_v = adc_read();
        // //reading current1
        // setup_Sensor(Current1_Pin, Current1_CH);
        // Data.current1_a = adc_read();
        // //reading current2
        // setup_Sensor(Current2_Pin, Current2_CH);
        // Data.current2_a = adc_read();
        // printf("Voltage: %d, Current1: %d, Current2: %d", Data.voltage_v, Data.current1_a, Data.current2_a);
    }
}

//First Core
int main(){
    stdio_init_all();

    //PWM setup
    setup_PWM();

    //PLC connection setup
    // DEV_Module_Init();
    // while (!stdio_usb_connected()) {
    //     sleep_ms(100);
    // }
    // printf("MCP2515_Init\r\n");
    // MCP2515_Init();
    // DEV_Delay_ms(3000);

    //Launch the second core
    multicore_launch_core1(core1_entry);

    // gpio_init(EXPERT_MODE_ACTIVE_PIN);
    // gpio_set_dir(EXPERT_MODE_ACTIVE_PIN, GPIO_IN);
    state = TEST;

    while(true) {
        // if(gpio_get(EXPERT_MODE_ACTIVE_PIN)){
        //     state = EXPERT;    
        // }
        // else{
        //     state = PLC_MODE;
        // }
        output(state);
    }
}

