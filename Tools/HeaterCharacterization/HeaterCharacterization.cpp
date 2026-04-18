#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "TempHandlers.h"
#include "HeaterControl.h"

// Humidity Sensor 1
#define I2C0_PORT i2c0
#define I2C0_SDA 16
#define I2C0_SCL 17

// Humidity Sensor 2
#define I2C1_PORT i2c1
#define I2C1_SDA 14
#define I2C1_SCL 15

#define HEATER_PIN  20
#define HEATER_FREQ 0.6f
#define HEATER_DUTY 0.10f

#define SAMPLE_MS 500

void i2c_setup(i2c_inst_t *i2c, uint sda, uint scl, uint baud)
{
    i2c_init(i2c, baud);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);
    gpio_pull_up(scl);
}

int main()
{
    stdio_init_all();
    sleep_ms(3000);


    //Init I2C
    i2c_setup(I2C0_PORT, I2C0_SDA, I2C0_SCL, 400 * 1000);
    i2c_setup(I2C1_PORT, I2C1_SDA, I2C1_SCL, 400 * 1000);

    void *SHT40_1 = setup_sht4x(i2c0, 'A'); // setup_sht4x generates a pointer handle for the sensor
    void *SHT40_2 = setup_sht4x(i2c1, 'A');

    SensorReading reading_inside = get_sensor_reading(SHT40_1);
    SensorReading reading_outside = get_sensor_reading(SHT40_2);

    HeaterControl heater(HEATER_PIN, HEATER_FREQ);
    heater.init();
    heater.set_duty(HEATER_DUTY);

    printf("time_ms, temp_inside, humidity_inside, error_inside, temp_outside, humidity_outside, error_outside\n");

    uint32_t start = to_ms_since_boot(get_absolute_time());

    while (true) {
        reading_inside = get_sensor_reading(SHT40_1);
        reading_outside = get_sensor_reading(SHT40_2);
        uint32_t t = to_ms_since_boot(get_absolute_time()) - start;
        printf("%lu, %.3f, %.3f, %d, %.3f, %.3f, %d\n", t, reading_inside.temp, reading_inside.humidity, reading_inside.error, reading_outside.temp, reading_outside.humidity, reading_outside.error);

        sleep_ms(SAMPLE_MS);
    }
}