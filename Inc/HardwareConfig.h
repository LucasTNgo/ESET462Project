#pragma once

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