#pragma once

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico_sensor_lib.h"


struct SensorReading {
    float temp;
    float pressure;
    float humidity;
};

void* setup_sht4x(i2c_inst_t *i2c, char port_code);
SensorReading get_sensor_reading(void* ctx);
float ntc_temperature_c(uint8_t adc_port);