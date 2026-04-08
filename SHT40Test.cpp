#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include "sht40x_driver_basic.h"

// Humidity Sensor 1
#define I2C0_PORT i2c0
#define I2C0_SDA 16
#define I2C0_SCL 17

// Humidity Sensor 2
#define I2C1_PORT i2c1
#define I2C1_SDA 14
#define I2C1_SCL 15

uint8_t i2c_write(uint8_t addr, uint8_t reg, uint8_t *pBuf, uint16_t len);
uint8_t i2c_read(uint8_t addr, uint8_t reg, uint8_t *pBuf, uint16_t len);
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


int blink = 0;

int main()
{
    stdio_init_all();


    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    
    i2c_setup(I2C0_PORT, I2C0_SDA, I2C0_SCL, 400 * 1000);
    i2c_setup(I2C1_PORT, I2C1_SDA, I2C1_SCL, 400 * 1000);

    // Example to turn on the Pico W LED
    //cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    while (true) {
        //cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, blink++%2);
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}

typedef struct {
    i2c_inst_t *i2c;
    uint sda;
    uint scl;
    uint baud;
} i2c_config_t;


/**
 * @brief This function re-format the i2c write function to be implemented into the sht4x driver
 * 
 * @param addr is the device slave address 
 * @param reg is the register to write
 * @param pBuf point to data to write to the sensor
 * @param len is the number of byte to write
 * @return none 
 */
uint8_t i2c_write(uint8_t addr, uint8_t reg, uint8_t *pBuf, uint16_t len)
{
 i2c_write_blocking(i2c_default, addr, (uint8_t*)&reg, 1, false);
}

/**
 * @brief This function re-format the i2c read function to be implemented into the sht4x driver
 * 
 * @param addr is the device slave address 
 * @param reg is the register to read
 * @param pBuf point to data read from the sensor
 * @param len is the number of byte to read
 * @return none 
 */
uint8_t i2c_read(uint8_t addr, uint8_t reg, uint8_t *pBuf, uint16_t len)
{
    i2c_read_blocking(i2c_default, addr, (uint8_t *)pBuf, len, false);
}