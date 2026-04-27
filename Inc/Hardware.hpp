#pragma once

extern "C" {
    #include "pico.h"
    #include "pico/stdlib.h"
    #include "pico/cyw43_arch.h"
    #include "pico/time.h"
    #include "hardware/i2c.h" //These after platform.h
    #include "hardware/gpio.h"
    #include "hardware/adc.h"
    #include "hardware/pwm.h"
    #include "pico_sensor_lib.h"
}
#include "TempHandlers.h"
#include "FanControl.h"
#include "HeaterControl.h"

extern "C" {
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
#define SERIAL_BUF_SIZE 64
}

// Misc hardware helpers
float clamp_float(float value, float min, float max);
uint64_t get_time_us();
int read_usb_float(float *out);
float absolute_humidity(float temp_c, float rel_humidity);
float relative_humidity(float temp_c, float abs_humidity);
void i2c_setup(i2c_inst_t *i2c, uint sda_pin, uint scl_pin, uint baudrate);

namespace Hardware
{
    typedef struct {
        SensorReading int_reading;
        SensorReading ext_reading;
        float ext_equiv_rh;
        float rh_diff;
    } SystemReadings;

    typedef struct {
        float heater_duty;
        float fan_duty;
    } SystemActuatorDuties;

    int init();
    SystemReadings read_state();
    void set_actuators(SystemActuatorDuties efforts);
}