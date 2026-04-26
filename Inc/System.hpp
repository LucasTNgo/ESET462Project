#pragma once

extern "C" {
#include "pico_sensor_lib.h"
#include "HardwareConfig.h"
}
#include "TempHandlers.h"
#include "FanControl.h"
#include "HeaterControl.h"
#include "PIDHandler.h"
#include "HardwareConfig.h"

typedef struct {
    SensorReading int_reading;
    SensorReading ext_reading;
    float ext_equiv_rh;
    float rh_diff;
} SystemReadings;

typedef struct {
    float heater_duty;
    float fan_duty;
} SystemActuatorEfforts;

float clamp_float(float value, float min, float max);
uint64_t get_time_us();
int read_usb_float(float *out);
float absolute_humidity(float temp_c, float rel_humidity);
float relative_humidity(float temp_c, float abs_humidity);
float get_equiv_rhdiff();
