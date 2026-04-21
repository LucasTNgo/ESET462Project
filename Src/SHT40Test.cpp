#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

#include "pico_sensor_lib.h"
#include "TempHandlers.h"
#include "FanControl.h"
#include "HeaterControl.h"
#include "PIDHandler.h"

// Humidity Sensor 1
#define I2C0_PORT i2c0
#define I2C0_SDA 16
#define I2C0_SCL 17

// Humidity Sensor 2
#define I2C1_PORT i2c1
#define I2C1_SDA 14
#define I2C1_SCL 15

//NTC Sensors
//#define GPIO_NTC1 26
//#define ADC_NTC1 0
//#define GPIO_NTC2 27
//#define ADC_NTC2 1

//Fan
#define FAN_PWM 13
#define FAN_TACH 26

//Heater
#define HEATER_PIN 20
#define HEATER_FREQ 0.6f

// IO buffer
#define BUF_SIZE 64

void i2c_setup(i2c_inst_t *i2c, uint sda_pin, uint scl_pin, uint baudrate);
float clamp_float(float value, float min, float max);
uint64_t get_time_us();
int read_usb_float(float *out);
float absolute_humidity(float temp_c, float rel_humidity);
float relative_humidity(float temp_c, float abs_humidity);
float get_equiv_rhdiff();

SensorReading int_reading;
SensorReading ext_reading;
float ext_abs_h;
float ext_equiv_rh;

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
    int_reading = get_sensor_reading(SHT40_1);
    ext_reading = get_sensor_reading(SHT40_2);
    auto get_temperature = []()->float { return int_reading.temp; };

    PIDHandler::Gains heater_gains = {
        .kp = 0.0756782425937777f,
        .ki = 0.000328958796246057f,
        .kd = 0.001f
    };
    PIDHandler::Gains heater_with_fan_gains = {
        .kp = 0.0851739669788485f,
        .ki = 0.000443182667027436f,
        .kd = 0.001f
    };
    PIDHandler::Gains interpolated_gains = heater_gains;
    PIDHandler heater_controller(heater_gains, get_time_us, get_temperature);
    heater_controller.set_target(50.0f);
    heater_controller.set_windup_clamp(-.05, 0.05);
    heater_controller.set_anti_windup(PIDHandler::AntiWindupType::CLAMP);

    PIDHandler::Gains fan_gains = {
        .kp = -2.0585,
        .ki = -0.078125f,
        .kd = -0.001f
    };
    PIDHandler fan_controller(fan_gains, get_time_us, get_equiv_rhdiff);
    fan_controller.set_target(0.0f);
    fan_controller.set_windup_clamp(-.5, 0.5);
    fan_controller.set_anti_windup(PIDHandler::AntiWindupType::CLAMP);

    //Initialize heater_controller
    float heater_effort = heater_controller.tick();
    float fan_effort = fan_controller.tick();
    while (true) {
        count++;
        //cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, blink++%2);
        
        //Get SHT40 Readings
        int_reading = get_sensor_reading(SHT40_1);
        ext_reading = get_sensor_reading(SHT40_2);

        /*
        //Get NTC readings
        float temp_NTC1 = ntc_temperature_c(ADC_NTC1);
        */ 

        if (int_reading.error == 0 && ext_reading.error == 0)
        {
            // Calculate external equivalent humidity
            ext_abs_h = absolute_humidity(ext_reading.temp, ext_reading.humidity);
            ext_equiv_rh = relative_humidity(int_reading.temp, ext_abs_h);

            // Set heater
            interpolated_gains = { .kp = heater_gains.kp*(1-fan_duty) + heater_with_fan_gains.kp*fan_duty,
                                   .ki = heater_gains.ki*(1-fan_duty) + heater_with_fan_gains.ki*fan_duty,
                                   .kd = heater_gains.kd*(1-fan_duty) + heater_with_fan_gains.kd*fan_duty };
            heater_controller.set_gains(interpolated_gains);
            heater_effort = heater_controller.tick();
            heater.set_duty(clamp_float(heater_effort, 0.0f, 1.0f));

            // Set fan
            fan_effort = fan_controller.tick();
            fan_duty = clamp_float(fan_effort, 0.0f, 1.0f);
            fan.set_duty(fan_duty);
        }

        if(count % 10 == 0)
        {
            printf("\nCount %d =========\n", count/10);
            printf("Internal:\tTemp: %0.2f\tRH: %0.2f%\n", int_reading.temp, int_reading.humidity);
            printf("External:\tTemp: %0.2f\tRH: %0.2f%\teRH: %0.2f\t deltaRH: %0.2f\n", ext_reading.temp, ext_reading.humidity, ext_equiv_rh, get_equiv_rhdiff());
            //printf("NTC 1: Temp: %0.3f\n", temp_NTC1);
            printf("Fan effort: %0.2f, Fan RPM: %.2f\n", fan_effort, fan.get_rpm()); 
            printf("Heater effort: %0.3f\n", heater_effort);
        }
        
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

float clamp_float(float value, float min, float max)
{
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

uint64_t get_time_us()
{
    return time_us_64();
}

// Call this periodically in your main loop
// uart = uart0 or uart1
// returns 1 when a new float is ready
int read_usb_float(float *out)
{
    static char buf[64];
    static int idx = 0;

    while (true) {
        int c = getchar_timeout_us(0); // non-blocking

        if (c == PICO_ERROR_TIMEOUT)
            break;

        if (c == '\n' || c == '\r') {
            if (idx == 0) continue;

            buf[idx] = '\0';

            char *end;
            float val = strtof(buf, &end);

            if (end != buf) {
                *out = val;
                idx = 0;
                return 1;
            }

            idx = 0;
        }
        else {
            if (idx < 63) buf[idx++] = (char)c;
            else idx = 0;
        }
    }

    return 0;
}

float absolute_humidity(float temp_c, float rel_humidity)
{
    // Saturation vapor pressure (hPa)
    float es = 6.112f * expf((17.62f * temp_c) / (243.12f + temp_c));
    
    // Actual vapor pressure (hPa)
    float e = (rel_humidity / 100.0f) * es;
    
    // Absolute humidity (g/m^3)
    float ah = 216.7f * (e / (temp_c + 273.15f));
    
    return ah;
}

float relative_humidity(float temp_c, float abs_humidity)
{
    // Saturation vapor pressure (hPa)
    float es = 6.112f * expf((17.62f * temp_c) / (243.12f + temp_c));
    
    // Actual vapor pressure from absolute humidity (hPa)
    float e = (abs_humidity * (temp_c + 273.15f)) / 216.7f;
    
    // Relative humidity (%)
    float rh = (e / es) * 100.0f;

    return rh;
}

float get_equiv_rhdiff(){
    return int_reading.humidity-ext_equiv_rh;
}