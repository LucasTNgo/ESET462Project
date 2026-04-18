#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "TempHandlers.h"
#include "HeaterControl.h"

#define I2C0_PORT i2c0
#define I2C0_SDA  16
#define I2C0_SCL  17

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

    i2c_setup(I2C0_PORT, I2C0_SDA, I2C0_SCL, 400 * 1000);

    void *sensor = setup_sht4x(i2c0, 'A');

    HeaterControl heater(HEATER_PIN, HEATER_FREQ);
    heater.init();
    heater.set_duty(HEATER_DUTY);

    printf("time_ms, temp, humidity\n");

    uint32_t start = to_ms_since_boot(get_absolute_time());

    while (true) {
        SensorReading r = get_sensor_reading(sensor);

        if (r.error == 0) {
            uint32_t t = to_ms_since_boot(get_absolute_time()) - start;
            printf("%lu, %.2f, %.2f\n", t, r.temp, r.humidity);
        }

        sleep_ms(SAMPLE_MS);
    }
}