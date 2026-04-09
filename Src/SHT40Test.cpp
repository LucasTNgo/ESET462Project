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

// Humidity Sensor 1
#define I2C0_PORT i2c0
#define I2C0_SDA 16
#define I2C0_SCL 17

// Humidity Sensor 2
#define I2C1_PORT i2c1
#define I2C1_SDA 14
#define I2C1_SCL 15

//NTC Sensors
#define GPIO_NTC1 26
#define ADC_NTC1 0
#define GPIO_NTC2 27
#define ADC_NTC2 1

void i2c_setup(i2c_inst_t *i2c, uint sda_pin, uint scl_pin, uint baudrate);

//int blink = 0;
int count = 0;

int main()
{
    stdio_init_all();
    adc_init();

    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    //Init I2C
    i2c_setup(I2C0_PORT, I2C0_SDA, I2C0_SCL, 400 * 1000);
    i2c_setup(I2C1_PORT, I2C1_SDA, I2C1_SCL, 400 * 1000);

    //Init ADC pins
    adc_gpio_init(GPIO_NTC1); // ADC0
    adc_gpio_init(GPIO_NTC2); // ADC1

    // Example to turn on the Pico W LED
    //cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    void *SHT40_1 = setup_sht4x(i2c0, 'A'); // setup_sht4x generates a pointer handle for the sensor
    void *SHT40_2 = setup_sht4x(i2c1, 'A');
    SensorReading reading1, reading2;

    while (true) {
        count++;
        //cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, blink++%2);
        
        //Get SHT40 Readings
        reading1 = get_sensor_reading(SHT40_1);
        reading2 = get_sensor_reading(SHT40_2);
        printf("\nCount %d =========\n", count);
        printf("Sensor 1:\tTemp: %0.2f\tHumidity: %0.2f\n", reading1.temp, reading1.humidity);
        printf("Sensor 2:\tTemp: %0.2f\tHumidity: %0.2f\n", reading2.temp, reading2.humidity);

        //Get NTC readings
        float temp_NTC1 = ntc_temperature_c(ADC_NTC1);
        printf("\nNTC 1: Temp: %0.3f\t", temp_NTC1);

        sleep_ms(1000);
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

