extern "C" {
    // STL and pico libraries
    #include "pico.h"
    #include "pico/stdlib.h"
    #include "pico/cyw43_arch.h"
    #include "pico/time.h"
    #include "hardware/i2c.h" //These after platform.h
    #include "hardware/gpio.h"
    #include "hardware/adc.h"
    #include "hardware/pwm.h"
    #include "FreeRTOS.h"
    #include "task.h"
}

#include <stdio.h>
#include <math.h>

#include "System.hpp"

// Helper functions
void i2c_setup(i2c_inst_t *i2c, uint sda_pin, uint scl_pin, uint baudrate);

SystemReadings state;

int main()
{
    stdio_init_all();

    //int blink = 0;
    int count = 0;

    // Fan init
    FanControl fan(FAN_PWM, FAN_TACH);   // PWM pin 0, tach pin 2
    float fan_duty = 0.f;
    fan.init();
    fan.set_duty(0.0f);

    // Heater init
    HeaterControl heater(HEATER_PIN, HEATER_FREQ);
    heater.init();  // missing
    heater.set_duty(0.25f);

    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    //Init I2C
    i2c_setup(I2C0_PORT, I2C0_SDA, I2C0_SCL, 400 * 1000);
    i2c_setup(I2C1_PORT, I2C1_SDA, I2C1_SCL, 400 * 1000);

    /*
    //Init ADC pins
    adc_init();
    adc_gpio_init(GPIO_NTC1); // ADC0
    adc_gpio_init(GPIO_NTC2); // ADC1
    */

    // Example to turn on the Pico W LED
    //cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    void *SHT40_1 = setup_sht4x(i2c0, 'A'); // setup_sht4x generates a pointer handle for the sensor
    void *SHT40_2 = setup_sht4x(i2c1, 'A');
    state.int_reading = get_sensor_reading(SHT40_1);
    state.ext_reading = get_sensor_reading(SHT40_2);
    auto get_temperature = []()->float { return state.int_reading.temp; };

    PIDHandler::Gains heater_gains = {
        .kp = 0.0848136068469526f,
        .ki = 6.59728735270854e-05f,
        .kd = 0.001f
    };
    PIDHandler::Gains heater_with_fan_gains = {
        .kp = 0.0806963431668464f,
        .ki = 6.2770230388285e-05f,
        .kd = 0.001f
    };
    PIDHandler::Gains interpolated_gains = heater_gains;
    PIDHandler heater_controller(heater_gains, get_time_us, get_temperature);
    heater_controller.set_target(50.0f);
    heater_controller.set_windup_clamp(-.05, 0.05);
    heater_controller.set_anti_windup(PIDHandler::AntiWindupType::CLAMP);

    PIDHandler::Gains fan_gains = {
        .kp = 2.0585,
        .ki = 0.078125f,
        .kd = 0.0f
    };
    PIDHandler fan_controller(fan_gains, get_time_us, get_equiv_rhdiff);
    fan_controller.set_target(0.03f);
    fan_controller.set_windup_clamp(-.3, 0.3);
    fan_controller.set_anti_windup(PIDHandler::AntiWindupType::CLAMP);

    //Initialize heater_controller
    float heater_effort = heater_controller.tick();
    float fan_effort = fan_controller.tick();
    while (true) {
        count++;
        //cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, blink++%2);
        
        //Get SHT40 Readings
        state.int_reading = get_sensor_reading(SHT40_1);
        state.ext_reading = get_sensor_reading(SHT40_2);

        /*
        //Get NTC readings
        float temp_NTC1 = ntc_temperature_c(ADC_NTC1);
        */ 

        if (state.int_reading.error == 0 && state.ext_reading.error == 0)
        {
            // Calculate external equivalent humidity
            state.ext_equiv_rh = relative_humidity(state.int_reading.temp, absolute_humidity(state.ext_reading.temp, state.ext_reading.humidity));

            // Set heater
            interpolated_gains = { .kp = heater_gains.kp*(1-fan_duty) + heater_with_fan_gains.kp*fan_duty,
                                   .ki = heater_gains.ki*(1-fan_duty) + heater_with_fan_gains.ki*fan_duty,
                                   .kd = heater_gains.kd*(1-fan_duty) + heater_with_fan_gains.kd*fan_duty };
            heater_controller.set_gains(interpolated_gains);
            heater_effort = clamp_float(heater_controller.tick(), 0.0f, 1.0f);
            heater.set_duty(heater_effort);

            // Set fan
            fan_effort = fan_controller.tick();
            fan_duty = clamp_float(-fan_effort, 0.0f, 1.0f);
            fan.set_duty(fan_duty);
        }
        
        
        if(count % 9 == 0)
        {
            /*
            printf("\nCount %d =========\n", count/10);
            printf("Internal:\tTemp: %0.2f\tRH: %0.2f%\n", state.int_reading.temp, state.int_reading.humidity);
            printf("External:\tTemp: %0.2f\tRH: %0.2f%\teRH: %0.2f\t deltaRH: %0.2f\n", state.ext_reading.temp, state.ext_reading.humidity, state.ext_equiv_rh, get_equiv_rhdiff());
            //printf("NTC 1: Temp: %0.3f\n", temp_NTC1);
            printf("Fan effort: %0.2f, Fan RPM: %.2f\n", fan_effort, fan.get_rpm()); 
            printf("Heater effort: %0.3f\n", heater_effort);
            */

        }
        
        printf("%.3f,%.3f,%.3f,%.3f\n", state.int_reading.temp, state.int_reading.humidity, heater_effort, fan_duty);

        sleep_ms(100);
    }
}

void i2c_setup(i2c_inst_t *i2c, uint sda_pin, uint scl_pin, uint baudrate)
{
    // Initialize I2C peripheral at given baudrate
    i2c_init(i2c, baudrate);

    // Set GPIO function to I2C
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);

    // Enable internal pull-ups (important for I2C bus idle state)
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
}

// TODO: Fix later
float get_equiv_rhdiff(){
    return (state.int_reading.humidity-state.ext_equiv_rh)/100;
}