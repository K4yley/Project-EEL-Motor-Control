#include "PWM_Encoder_Sensors.h"

volatile Encoder_t Encoder;
volatile Sensors_t Sensor;

volatile bool Pos_Control = true;

void setup_PWM(){         
    /// @param phase_delay 0 
    setup_phase(PWM_A0, PWM_A1, 0);
    
    /// @param phase_delay Fase 120° -> (120/360) * 255 = 20833
    setup_phase(PWM_B0, PWM_B1, 20833);
    
    /// @param phase_delay Fase 240° -> (240/360) * 255 = 41666
    setup_phase(PWM_C0, PWM_C1, 41666);
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
        &PulseCounting         //pas op dat er een A achterstaat
    );

    gpio_set_irq_enabled_with_callback(
        HALL_B,
        GPIO_IRQ_EDGE_RISE,
        true,
        &PulseCounting     //pas op dat er een B achterstaat 
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
}

/// @brief the interrupt for the Encoder
/// @param positionticks are off with 50 looking at pulsecount
void PulseCounting(uint gpio, uint32_t events) {
    if (gpio == HALL_A) {
        if (gpio_get(HALL_B) == 0) {
            Encoder.positionTicks--;
        } else{
            Encoder.positionTicks++;
        }
    } else if (gpio == HALL_B) {
        if (gpio_get(HALL_A) == 1) {
            Encoder.positionTicks--;
        } else {
            Encoder.positionTicks++;
        }
    }
    Encoder.pulseCount++;
}


float RPM_counting(int pulses, float time_s) { 
    return (pulses / 1024.0f) * (60.0f / time_s);
}


void Motor(PWM_t Option){
    int slice;
    switch(Option){
        case STOP:
           pwm_set_mask_enabled(0x00);
            break;
        case FORWARD:
            PWM_TurnOff();
            pwm_set_mask_enabled(0x00);    //stops PWM
            slice = pwm_gpio_to_slice_num(PWM_A0);
            pwm_set_counter(slice, 41666);        //phase 1 to 240
            pwm_set_counter(slice+1, 20833);
            pwm_set_counter(slice+2, 0);        //phase 3 to 0
            PWM_TurnOn();
            pwm_set_mask_enabled(mask);
            printf("Motor goes forward: Clockwise\n");
            break;
        case BACKWARD:
            PWM_TurnOff();
            pwm_set_mask_enabled(0x00);    //stops PWM
            slice = pwm_gpio_to_slice_num(PWM_A0);
            pwm_set_counter(slice, 0);        //phase 1 to 240
            pwm_set_counter(slice+1, 20833);
            pwm_set_counter(slice+2, 41666);        //phase 3 to 0
            PWM_TurnOn();
            pwm_set_mask_enabled(mask);
            printf("Motor goed backward: Counter Clockwise\n");
            break;
    }
}

void PWM_TurnOff(){
    int slice = pwm_gpio_to_slice_num(PWM_A0);
    pwm_set_chan_level(slice, PWM_A0, 0);
    pwm_set_chan_level(slice, PWM_A1, 0);
    slice++;
    pwm_set_chan_level(slice, PWM_B0, 0);
    pwm_set_chan_level(slice, PWM_B1, 0);
    slice++;
    pwm_set_chan_level(slice, PWM_C0, 0);
    pwm_set_chan_level(slice, PWM_C1, 0);
    sleep_us(10);    
}
void PWM_TurnOn(){
    int slice = pwm_gpio_to_slice_num(PWM_A0);
    pwm_set_chan_level(slice, PWM_CHAN_A, WRAP / 2 - 13);
    pwm_set_chan_level(slice, PWM_CHAN_B, WRAP / 2);
    slice++;
    pwm_set_chan_level(slice, PWM_CHAN_A, WRAP / 2 - 13);
    pwm_set_chan_level(slice, PWM_CHAN_B, WRAP / 2);
    slice++;
    pwm_set_chan_level(slice, PWM_CHAN_A, WRAP / 2 - 13);
    pwm_set_chan_level(slice, PWM_CHAN_B, WRAP / 2);   
}

float LocationCal(int positionticks){
    //printf("Calculate: %d, %0.2f\n", positionticks, (float)positionticks / Refpulse);
    return (float)positionticks / Refpulse;
}

void Motor_newpos(float Target, float live_motor){
    Pos_Control = true;
    int slice;
    float targetPosition = distanceCal(Target, live_motor);
    printf("Target: %0.2f, %0.2f\n", Target, Encoder.targetPosition);
    if(targetPosition > 0){
        PWM_TurnOff();
        pwm_set_mask_enabled(0x00);    //stops PWM
        slice = pwm_gpio_to_slice_num(PWM_A0);
        pwm_set_counter(slice, 41666);        //phase 1 to 240
        pwm_set_counter(slice+1, 20833);
        pwm_set_counter(slice+2, 0);        //phase 3 to 0
        PWM_TurnOn();
        pwm_set_mask_enabled(mask);
        printf("Motor Forwards\n");
    }
    else if(targetPosition < 0){
        PWM_TurnOff();
        pwm_set_mask_enabled(0x00);    //stops PWM
        slice = pwm_gpio_to_slice_num(PWM_A0);
        pwm_set_counter(slice, 0);        //phase 1 to 240
        pwm_set_counter(slice+1, 20833);
        pwm_set_counter(slice+2, 41666);        //phase 3 to 0
        PWM_TurnOn();
        pwm_set_mask_enabled(mask);
        printf("Motor Backwards\n");
    }
    Encoder.targetPosition = targetPosition + Encoder.positionTicks;
}

float distanceCal(int Target, float live_motor){
    float new_position = (float)Target - live_motor;     //if you want to go to 5, and are at 3 at the moment; 2 to move;
    return new_position * Refpulse;
}