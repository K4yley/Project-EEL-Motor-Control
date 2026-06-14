#include "PWM_Encoder_Sensors.h"

void setup_PWM(){
    /// @param phase_delay 0 
    setup_phase(Ph1_A, Ph1_B, 0);
    
    /// @param phase_delay Fase 120° -> (120/360) * 255 = 20833
    setup_phase(Ph2_A, Ph2_A, 20833); 
    
    /// @param phase_delay Fase 240° -> (240/360) * 255 = 41666x
    setup_phase(Ph3_A, Ph3_B, 41666);
}

void setup_Encoder(){
    gpio_init(HALL_A);
    gpio_set_dir(HALL_A, GPIO_IN);
    gpio_init(HALL_B);
    gpio_set_dir(HALL_B, GPIO_IN);

    //Interrupts
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
}

void setup_Sensor(uint gpio, int channel){
    adc_init(); // Initialize the ADC
    adc_gpio_init(gpio); // Initialize the GPIO pin for the ADC
    adc_select_input(channel); // Select the ADC input, channel 0 = pin 26
}

void setup_phase(uint gpio_a, uint gpio_b, uint16_t phase_delay){
    uint slice = pwm_gpio_to_slice_num(gpio_a);
    
    //Say GPIO is for PWM
    gpio_set_function(gpio_a, GPIO_FUNC_PWM);
    gpio_set_function(gpio_b, GPIO_FUNC_PWM);

    //Set clk and wrap for the right slice -> Set period of 4 cycles (0 to 3 inclusive) (duty cicle?)
    pwm_set_clkdiv(slice, CLKDIV);
    pwm_set_wrap(slice, WRAP);  

    //set duty circle of 50%
    pwm_set_chan_level(slice, PWM_CHAN_A, WRAP / 2 - 13); // add Dead Time; a max. of 15 ticks deadtime per slice! 1 tick = (CLKDIV / 125MHz = 1.6 us), to get 20 ms => 20000/1,6 = 12500 ticks
    pwm_set_chan_level(slice, PWM_CHAN_B, WRAP / 2);

    /// @brief inverted of GPIO_b
    /// @param slice_num PWM slice number
    /// @param a true to invert output A
    /// @param b true to invert output B
    pwm_set_output_polarity(slice, false, true);

    //add the fase delay
    pwm_set_counter(slice, phase_delay);
    //to go the other way, chance the fase1 to 240 and fase3 to 0
}

void PulseCounting(uint gpio, uint32_t events){
    if (!gpio_get(HALL_A) && gpio_get(HALL_B)) {   // 01
        Encoder.pulseCount++;
        Encoder.new_dir_state = 1;
    }
    if (gpio_get(HALL_A) && gpio_get(HALL_B)) {    // 11
        Encoder.pulseCount++;
        Encoder.new_dir_state = 2;
    }
    if (gpio_get(HALL_A) && !gpio_get(HALL_B)) {   // 10
        Encoder.pulseCount++;
        Encoder.new_dir_state = 3;
    }
    if (!gpio_get(HALL_A) && !gpio_get(HALL_B)) {  // 00
        Encoder.pulseCount++;
        Encoder.new_dir_state = 4;
    }

    if ((Encoder.new_dir_state > Encoder.old_dir_state && !(Encoder.new_dir_state == 4 && Encoder.old_dir_state == 1)) ||
        (Encoder.new_dir_state == 1 && Encoder.old_dir_state == 4)) {
        Encoder.positionTicks++;
    } else {
        Encoder.positionTicks--;
    }
    Encoder.old_dir_state = Encoder.new_dir_state; 
}


float RPM_counting(int pulses, float time_s){
    return (pulses / 2048.0f) * (60.0f / time_s);
}


void Motor(PWM_t Option){
    switch(Option){
        case STOP:
           for(int i = 1; i < 4; i++){
                pwm_set_enabled(i, false);  
            }
            printf("Motor Stopped\n"); 
            break;
        case FORWARD:
            for(int i = 1; i < 4; i++){
                pwm_set_enabled(i, false);  
            }
            pwm_set_counter(1, 41666);        //phase 1 to 240
            pwm_set_counter(3, 0);        //phase 3 to 0

            pwm_set_mask_enabled(0x0E);
            printf("Motor goes forward: Clockwise\n");
            break;
        case BACKWARD:
            for(int i = 1; i < 4; i++){
                pwm_set_enabled(i, false);  
            }
            pwm_set_counter(1, 0);        //phase 1 to 0
            pwm_set_counter(3, 41666);        //phase 3 to 240

            pwm_set_mask_enabled(0x0E);
            printf("Motor goed backward: Counter Clockwise\n");
            break;
    }
}
